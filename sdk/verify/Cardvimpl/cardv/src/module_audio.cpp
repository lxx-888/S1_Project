/*
 * module_audio.cpp- Sigmastar
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
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
#include "module_common.h"
#include "module_audio.h"
#include "mid_AudioEncoder.h"
#include "module_AudioDecoder.h"
#include "mid_common.h"
#include "speech.h"
#include "DCF.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

MI_AudioEncoder *g_pAudioEncoderArray[MAX_AUDIO_INPUT]  = {NULL};
MI_AudioDecoder *g_pAudioDecoderArray[MAX_AUDIO_OUTPUT] = {NULL};

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDVAudioInParam  g_audioInParam[];
extern CarDVAudioOutParam g_audioOutParam[];

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

/* Audio in */

void audioRecUpdateFileSize(char *pFileName)
{
    DCF_UpdateFileSizeByName(DB_NORMAL, 0, pFileName);
}

int audioInOpen(void)
{
    ExecFunc(MI_AI_InitDev(NULL), MI_SUCCESS);

    for (MI_S32 i = 0; i < MAX_AUDIO_INPUT; i++)
    {
        g_pAudioEncoderArray[i] = NULL;
        g_pAudioEncoderArray[i] = MI_AudioEncoder::createNew(i);
        g_pAudioEncoderArray[i]->initAudioEncoder(&g_audioInParam[i]);
        g_pAudioEncoderArray[i]->startAudioEncoder();
    }

    return 0;
}

int audioInClose(void)
{
    for (MI_S32 i = 0; i < MAX_AUDIO_INPUT; i++)
    {
        g_pAudioEncoderArray[i]->stopAudioEncoder();
        g_pAudioEncoderArray[i]->uninitAudioEncoder();
        delete g_pAudioEncoderArray[i];
        g_pAudioEncoderArray[i] = NULL;
    }
    ExecFunc(MI_AI_DeInitDev(), MI_SUCCESS);
    return 0;
}

int audioInStart(void)
{
    g_pAudioEncoderArray[0]->startAudioRecord();
    return 0;
}

int audioInStop(void)
{
    g_pAudioEncoderArray[0]->stopAudioRecord();
    return 0;
}

int audioInStartRecFile(void)
{
    char   fileName[64];
    time_t timep;
    int    ret;

    AudioInSetRecUpdateSizeCallback(audioRecUpdateFileSize);

    time(&timep);
    ret = DCF_CreateFileName(DB_NORMAL, 0, fileName, timep, "aac");
    if (ret == 0)
    {
        DCF_AddFile(DB_NORMAL, 0, fileName);
        g_pAudioEncoderArray[0]->startAudioFile(fileName);
    }
    return 0;
}

int audioInStopRecFile(void)
{
    g_pAudioEncoderArray[0]->stopAudioFile();
    return 0;
}

int audioInMute(BOOL bMute)
{
    g_pAudioEncoderArray[0]->mute(bMute);
    return 0;
}

/* Audio out */

int audioOutOpen(void)
{
    ExecFunc(MI_AO_InitDev(NULL), MI_SUCCESS);

    for (MI_S32 i = 0; i < MAX_AUDIO_OUTPUT; i++)
    {
        g_pAudioDecoderArray[i] = MI_AudioDecoder::createNew();
        g_pAudioDecoderArray[i]->initAudioDecoder(&g_audioOutParam[i]);
    }

    return 0;
}

int audioOutClose(void)
{
    for (MI_S32 i = 0; i < MAX_AUDIO_OUTPUT; i++)
    {
        if (g_pAudioDecoderArray[i])
        {
            g_pAudioDecoderArray[i]->stopPlayMedia(TRUE);
        }
        delete g_pAudioDecoderArray[i];
        g_pAudioDecoderArray[i] = NULL;
    }

    ExecFunc(MI_AO_DeInitDev(), MI_SUCCESS);
    return 0;
}

void audioVqeVolumeTrans(MI_U32 u32DevId, MI_S8 *s8InputDB)
{
    if ((u32DevId == AI_DEV_ID_ADC_0_1 || u32DevId == AI_DEV_ID_ADC_2_3 || u32DevId == AI_DEV_ID_ADC_0_1_2_3))
    {
        *s8InputDB = MI_S8(*s8InputDB / 100. * 21);
    }
    else if (u32DevId == AI_DEV_ID_DMIC)
    {
        *s8InputDB = MI_S8(*s8InputDB / 100. * 4);
    }
    else if (u32DevId == AI_DEV_ID_LINE_IN)
    {
        *s8InputDB = MI_S8(*s8InputDB / 100. * 7);
    }
}

int audioSetVqeVolume(MI_S8 s8InputDB)
{
    if (s8InputDB > 100 || s8InputDB < 0)
    {
        printf("Input DB must > 0 and < 100\n");
        return -1;
    }
    for (MI_S8 i = 0; i < MAX_AUDIO_INPUT; i++)
    {
        audioVqeVolumeTrans(g_audioInParam[i].u32DevId, &s8InputDB);
        g_pAudioEncoderArray[i]->setAudioInputVolume(&g_audioInParam[i], s8InputDB);
    }
    return 0;
}

int audio_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    switch (id)
    {
    case CMD_AUDIO_IN_OPEN:
        audioInOpen();
        break;

    case CMD_AUDIO_IN_CLOSE:
        audioInClose();
        break;

    case CMD_AUDIO_IN_START:
        audioInStart();
        break;

    case CMD_AUDIO_IN_STOP:
        audioInStop();
        break;

    case CMD_AUDIO_IN_START_FILE:
        audioInStartRecFile();
        break;

    case CMD_AUDIO_IN_STOP_FILE:
        audioInStopRecFile();
        break;

    case CMD_AUDIO_IN_MUTE: {
        BOOL bMute;
        memcpy(&bMute, param, paramLen);
        audioInMute(bMute);
    }
    break;

    case CMD_AUDIO_OUT_OPEN:
        audioOutOpen();
        break;

    case CMD_AUDIO_OUT_CLOSE:
        audioOutClose();
        break;

    case CMD_AUDIO_OUT_PLAY: {
        MI_BOOL bStopNow;
        char    audioPath[64] = {
            0,
        };
        bStopNow = param[0];
        memcpy(audioPath, &param[1], paramLen - 1);
        if (g_pAudioDecoderArray[0])
        {
            g_pAudioDecoderArray[0]->stopPlayMedia(bStopNow);
            g_pAudioDecoderArray[0]->setVolume(g_audioOutParam[0].s8VolumeOutDb,
                                               g_audioOutParam[0].eAudioOutGainFading);
            g_pAudioDecoderArray[0]->startPlayMedia(audioPath);
        }
    }
    break;

    case CMD_AUDIO_OUT_MULTI_PLAY: {
        MI_BOOL bStopNow;
        MI_S8   DevId;
        char    audioPath[128] = {
            0,
        };
        MI_AudioDecoder *pAudioDecoder;

        bStopNow = param[0];
        DevId    = param[1];
        memcpy(audioPath, &param[2], paramLen - 2);

        // echo audioplay 1 1 live  > /tmp/cardv_fifo
        if (DevId < MAX_AUDIO_OUTPUT)
        {
            pAudioDecoder = g_pAudioDecoderArray[DevId];

            if (pAudioDecoder)
            {
                pAudioDecoder->stopPlayMedia(bStopNow);
                pAudioDecoder->setVolume(g_audioOutParam[DevId].s8VolumeOutDb,
                                         g_audioOutParam[DevId].eAudioOutGainFading);

                if (strncmp(audioPath, "live", 4) == 0)
                {
                    pAudioDecoder->startPlayMedia();
                }
                else
                {
                    pAudioDecoder->startPlayMedia(audioPath);
                }
            }
        }
    }
    break;

    case CMD_AUDIO_OUT_STOP: {
        MI_BOOL          bStopNow;
        MI_S8            DevId;
        MI_AudioDecoder *pAudioDecoder;

        bStopNow = param[0];
        DevId    = param[1];

        if (DevId < MAX_AUDIO_OUTPUT)
        {
            pAudioDecoder = g_pAudioDecoderArray[DevId];
            if (pAudioDecoder)
            {
                pAudioDecoder->stopPlayMedia(bStopNow);
            }
        }
    }
    break;

    case CMD_AUDIO_IN_SET_VOLUME: {
        MI_S32 s8InputDB;
        s8InputDB = param[0];
        audioSetVqeVolume(s8InputDB);
    }
    break;

    default:
        break;
    }

    return 0;
}
