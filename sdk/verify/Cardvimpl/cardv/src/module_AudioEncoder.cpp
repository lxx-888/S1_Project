/*
 * module_AudioEncoder.cpp- Sigmastar
 *
 * Copyright (C) 2020 Sigmastar Technology Corp.
 *
 * Author: jeff.li <jeff.li@sigmastar.com.cn>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

using namespace std;

#include "mi_ai.h"
#include "mi_sys.h"
#include "mid_common.h"
#include "mid_audio_type.h"
#include "mid_AudioEncoder.h"
#include "module_config.h"
#include "speech.h"

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern MI_S32 g_s32SocId;
extern void   audioInToaudioOut(MI_AI_Data_t *pstAudioFrame);

//==============================================================================
//
//                              LOCAL FUNCTIONS
//
//==============================================================================

void *AudioIn_Task(void *argv)
{
    MI_S32           s32Ret = MI_SUCCESS;
    MI_AUDIO_DEV     AiDevId;
    MI_U8            u8ChnGrpIdx;
    MI_AI_Data_t     stData;
    MI_AI_Data_t     stEchoRefData;
    MI_AudioEncoder *pstAudioEncoder = NULL;

    CARDV_THREAD();

    pstAudioEncoder = (MI_AudioEncoder *)argv;
    AiDevId         = pstAudioEncoder->m_stAudioInParam.u32DevId;
    u8ChnGrpIdx     = pstAudioEncoder->m_stAudioInParam.u8ChnId;

    CARDV_INFO("AudioIn_Task%d+\n", u8ChnGrpIdx);

    do
    {
        s32Ret = MI_AI_Read(AiDevId, u8ChnGrpIdx, &stData, &stEchoRefData, -1);
        if (MI_SUCCESS != s32Ret)
        {
            continue;
        }

#if (CARDV_SPEECH_ENABLE)
        if (Speech_VoiceIsAnalyzeStart())
        {
            Speech_VoiceSendAudioFrame(&stData);
        }
#endif
        audioInToaudioOut(&stData);
        pstAudioEncoder->handlerAudioFrame(&stData);
        MI_AI_ReleaseData(AiDevId, u8ChnGrpIdx, &stData, &stEchoRefData);
    } while (pstAudioEncoder->m_bAudioInExit == FALSE);

    CARDV_INFO("AudioIn_Task%d-\n", u8ChnGrpIdx);
    pthread_exit(NULL);
}

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 MI_AudioEncoder::initAudioEncoder(CarDVAudioInParam *pstAudioParam)
{
    MI_AUDIO_DEV     AudioInDevId;
    MI_U8            AudioInChnId = 0;
    MI_SYS_ChnPort_t stChnPort;
    MI_AI_If_e       enAiIf[]     = {E_MI_AI_IF_ADC_AB};
    MI_S16           s8DpgaGain[] = {10};
    if (NULL == pstAudioParam)
    {
        CARDV_ERR("input cardv param is NULL!\n");
        return -1;
    }

    memcpy(&m_stAudioInParam, pstAudioParam, sizeof(CarDVAudioInParam));

    AudioInDevId = m_stAudioInParam.u32DevId;
    AudioInChnId = m_stAudioInParam.u8ChnId;
    enAiIf[0]    = m_stAudioInParam.enAiIf;

    /* set ai public attr*/
    ExecFunc_OS(MI_AI_Open(AudioInDevId, &m_stAudioInParam.stAudioAttr), MI_SUCCESS);
    ExecFunc_OS(MI_AI_AttachIf(AudioInDevId, enAiIf, sizeof(enAiIf) / sizeof(enAiIf[0])), MI_SUCCESS);

    if (m_stAudioInParam.s8VolumeInDb)
    {
        // set ADC0/1 Interface gain
        ExecFunc(MI_AI_SetIfGain(enAiIf[0], m_stAudioInParam.s8VolumeInDb, 0), MI_SUCCESS);

        // set ADC0/1 DPGA gain
        ExecFunc(MI_AI_SetGain(AudioInDevId, AudioInChnId, s8DpgaGain, sizeof(s8DpgaGain) / sizeof(s8DpgaGain[0])),
                 MI_SUCCESS);
    }

    stChnPort.eModId    = E_MI_MODULE_ID_AI;
    stChnPort.u32DevId  = m_stAudioInParam.u32DevId;
    stChnPort.u32ChnId  = m_stAudioInParam.u8ChnId;
    stChnPort.u32PortId = 0;
    ExecFunc_OS(MI_SYS_SetChnOutputPortDepth(g_s32SocId, &stChnPort, m_stAudioInParam.u32UserFrameDepth,
                                             m_stAudioInParam.u32BufQueueDepth),
                MI_SUCCESS);

    /* enable ai channel of device*/
    ExecFunc_OS(MI_AI_EnableChnGroup(AudioInDevId, AudioInChnId), MI_SUCCESS);

    m_stAudioInParam.eAudioInEncType = pstAudioParam->eAudioInEncType;
    return internalInit();
}

MI_S32 MI_AudioEncoder::uninitAudioEncoder()
{
    MI_U8        AiChnId = m_stAudioInParam.u8ChnId;
    MI_AUDIO_DEV AiDevId = m_stAudioInParam.u32DevId;

    internalDeInit();

    /* disable ai channel of device */
    ExecFunc(MI_AI_DisableChnGroup(AiDevId, AiChnId), MI_SUCCESS);

    /* disable ai device */
    ExecFunc(MI_AI_Close(AiDevId), MI_SUCCESS);

    return 0;
}