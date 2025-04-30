/*
 * module_AudioDecoder.cpp- Sigmastar
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

#include <unistd.h>
#include <fcntl.h>
#include <sys/prctl.h>

#include "mi_ao.h"
#include "mi_sys.h"
#include "mid_common.h"
#include "mid_audio_type.h"
#include "mid_AudioEncoder.h"
#include "module_AudioDecoder.h"

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern MI_AudioEncoder *g_pAudioEncoderArray[];

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

static pthread_mutex_t  g_audioMutex      = PTHREAD_MUTEX_INITIALIZER;
static MI_AudioDecoder *g_pstAudioDecoder = NULL;

//==============================================================================
//
//                              LOCAL FUNCTIONS
//
//==============================================================================

void audioInToaudioOut(MI_AI_Data_t *pstAudioFrame)
{
    pthread_mutex_lock(&g_audioMutex);
    if (g_pstAudioDecoder)
    {
        MI_S32       s32RetSendStatus = MI_SUCCESS;
        MI_AUDIO_DEV u32DevId;
        MI_AI_Data_t stAudioFrame;

        stAudioFrame.u32Byte[0]   = pstAudioFrame->u32Byte[0];
        stAudioFrame.u64Pts       = pstAudioFrame->u64Pts;
        stAudioFrame.u64Seq       = pstAudioFrame->u64Seq;
        stAudioFrame.apvBuffer[0] = (MI_S16 *)pstAudioFrame->apvBuffer[0];
        u32DevId                  = g_pstAudioDecoder->m_pstAudioOutParam->u32DevId;
        s32RetSendStatus =
            MI_AO_Write(u32DevId, &stAudioFrame.apvBuffer[0], stAudioFrame.u32Byte[0], stAudioFrame.u64Pts, 0);
        if (MI_SUCCESS != s32RetSendStatus)
            printf("AI to AO dev%u error 0x%x\n", u32DevId, s32RetSendStatus);
    }
    pthread_mutex_unlock(&g_audioMutex);
}

void *AudioOut_Task(void *argv)
{
    int              fd = -1;
    MI_AUDIO_DEV     u32DevId;
    MI_S32           s32ReadLen       = 0, s32Len;
    MI_U8 *          pu8ReadBuf       = NULL;
    MI_U8 *          pu8OutBuf        = NULL;
    MI_S32           s32RetSendStatus = MI_SUCCESS;
    MI_U32           u32SleepTime;
    MI_AudioDecoder *pstAudioDecoder = NULL;

    CARDV_THREAD();

    pstAudioDecoder = (MI_AudioDecoder *)argv;
    fd              = pstAudioDecoder->m_s32Fd;
    s32Len          = pstAudioDecoder->m_stAoDevAttr.u32PeriodSize;
    u32DevId        = pstAudioDecoder->m_pstAudioOutParam->u32DevId;
    u32SleepTime = pstAudioDecoder->m_stAoDevAttr.u32PeriodSize * 1000000 / pstAudioDecoder->m_stAoDevAttr.enSampleRate;

    pu8ReadBuf = (MI_U8 *)MALLOC(s32Len);
    if (pu8ReadBuf == NULL)
    {
        CARDV_ERR("Alloc audio out buffer fail\n");
        goto AudioOut_Task_Exit;
    }

    prctl(PR_SET_NAME, "Aout_Task");

    while (pstAudioDecoder->m_bAudioOutExit == FALSE)
    {
        s32ReadLen = read(fd, pu8ReadBuf, s32Len);
        if (s32ReadLen == 0)
            break;

        if (s32ReadLen != s32Len)
            memset(pu8ReadBuf + s32ReadLen, 0x00, (s32Len - s32ReadLen));

        pstAudioDecoder->ReSample(pu8ReadBuf, pstAudioDecoder->m_stAoDevAttr.u32PeriodSize, &pu8OutBuf);

        // read data and send to AO module
        do
        {
            s32RetSendStatus = MI_AO_Write(u32DevId, pu8OutBuf, s32ReadLen, 0, -1);
        } while (s32RetSendStatus == MI_AO_ERR_NOBUF && pstAudioDecoder->m_bAudioOutExit == FALSE);

        if (MI_SUCCESS != s32RetSendStatus)
            CARDV_WARN("error 0x%x: \n", s32RetSendStatus);
    }

    if (MI_SUCCESS == s32RetSendStatus)
        CARDV_INFO("MI_AO_SendFrame done!\n");

    while (pstAudioDecoder->m_bAudioOutExit == FALSE)
    {
        MI_U32 u32Remaining;
        MI_U64 u64TStamp;

        s32RetSendStatus = MI_AO_GetTimestamp(u32DevId, &u32Remaining, &u64TStamp);
        if (u32Remaining == 0)
            break;
        else
            usleep(u32SleepTime);
    }

    /* Clear ao channel buff of device*/
    MI_AO_Stop(u32DevId);

AudioOut_Task_Exit:
    close(fd);
    FREEIF(pu8ReadBuf);
    pstAudioDecoder->AdecDeinit();
    pstAudioDecoder->ReSampleDeInit();
    pthread_exit(NULL);
}

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_AudioDecoder *MI_AudioDecoder::createNew(void)
{
    MI_AudioDecoder *pAudioEncoder = NULL;

    do
    {
        pAudioEncoder = new MI_AudioDecoder();
        return pAudioEncoder;
    } while (0);

    return NULL;
}

MI_AudioDecoder::MI_AudioDecoder(void)
{
    m_bAudioOutExit  = FALSE;
    m_pthreadId_Aout = 0;
    m_bAudioEnable   = FALSE;
    m_s32Fd          = -1;
    m_pstSwrCtx      = NULL;
    m_pu8OutBuf      = NULL;
}

MI_AudioDecoder::~MI_AudioDecoder(void)
{
    AdecDeinit();
    ReSampleDeInit();
}

MI_S32 MI_AudioDecoder::ReSampleInit(MI_S32 s32InputSampleRate)
{
    int64_t        s64layout;
    AVSampleFormat eSampleFmt;

    ReSampleDeInit();

    s64layout  = m_stAoDevAttr.enSoundMode == E_MI_AUDIO_SOUND_MODE_STEREO ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO;
    eSampleFmt = AV_SAMPLE_FMT_S16;
    m_u32InSampleRate  = s32InputSampleRate;
    m_u32OutSampleRate = (int)m_stAoDevAttr.enSampleRate;
    m_pstSwrCtx        = swr_alloc_set_opts(NULL, s64layout, eSampleFmt, m_u32OutSampleRate, s64layout, eSampleFmt,
                                            m_u32InSampleRate, 0, NULL);

    if (m_pstSwrCtx == NULL || swr_init(m_pstSwrCtx) < 0)
    {
        printf("swr init fail\n");
        ReSampleDeInit();
        return -1;
    }

    m_pu8OutBuf = (MI_U8 *)MALLOC(m_stAoDevAttr.u32PeriodSize * 2 * m_u32OutSampleRate / m_u32InSampleRate);
    if (m_pu8OutBuf == NULL)
    {
        printf("swr alloc mem fail\n");
        ReSampleDeInit();
        return -1;
    }

    return 0;
}

MI_S32 MI_AudioDecoder::ReSampleDeInit(void)
{
    if (m_pstSwrCtx)
    {
        swr_free(&m_pstSwrCtx);
        m_pstSwrCtx = NULL;
    }

    FREEIF(m_pu8OutBuf);

    return 0;
}

MI_S32 MI_AudioDecoder::ReSample(MI_U8 *pu8InBuf, MI_U32 u32SampleCnt, MI_U8 **ppu8OutBuf)
{
    if (m_pstSwrCtx && m_pu8OutBuf)
    {
        *ppu8OutBuf = m_pu8OutBuf;
        return swr_convert(m_pstSwrCtx, ppu8OutBuf, u32SampleCnt * m_u32OutSampleRate / m_u32InSampleRate,
                           (const uint8_t **)&pu8InBuf, u32SampleCnt);
    }
    else
    {
        *ppu8OutBuf = pu8InBuf;
        return u32SampleCnt;
    }
    return 0;
}

MI_S32 MI_AudioDecoder::AdecInit(void)
{
    MI_AUDIO_DEV AoDevId;
    MI_AO_If_e   AoIf;

    AoDevId = m_pstAudioOutParam->u32DevId;
    AoIf    = m_pstAudioOutParam->enAoIfs;

    AdecDeinit();

    /* enable ao device */
    if (m_bAudioEnable == FALSE)
    {
        ExecFunc(MI_AO_Open(AoDevId, &m_stAoDevAttr), MI_SUCCESS);
        ExecFunc(MI_AO_AttachIf(AoDevId, AoIf, 0), MI_SUCCESS);
        m_bAudioEnable = TRUE;
    }

#ifdef CHIP_IFORD
    if (m_pstAudioOutParam->s8VolumeOutDb <= (MIN_AO_VOLUME * 8))
#else
    if (m_pstAudioOutParam->s8VolumeOutDb <= MIN_AO_VOLUME)
#endif
    {
        ExecFunc_OS(MI_AO_SetMute(AoDevId, TRUE, TRUE, m_pstAudioOutParam->eAudioOutGainFading), MI_SUCCESS);
        return 0;
    }

    /* Not need to set gain fading in initial stage */
    ExecFunc(MI_AO_SetVolume(AoDevId, m_pstAudioOutParam->s8VolumeOutDb, m_pstAudioOutParam->s8VolumeOutDb,
                             E_MI_AO_GAIN_FADING_OFF),
             MI_SUCCESS);

    return 0;
}

MI_S32 MI_AudioDecoder::AdecDeinit(void)
{
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_If_e   AoIf;

    AoDevId = m_pstAudioOutParam->u32DevId;
    AoIf    = m_pstAudioOutParam->enAoIfs;

    /* disable ao device */
    if (m_bAudioEnable == TRUE)
    {
        ExecFunc(MI_AO_DetachIf(AoDevId, AoIf), MI_SUCCESS);
        ExecFunc(MI_AO_Close(AoDevId), MI_SUCCESS);
        m_bAudioEnable = FALSE;
    }

    return 0;
}

MI_S32 MI_AudioDecoder::initAudioDecoder(CarDVAudioOutParam *pstAudioParam)
{
    m_pstAudioOutParam = pstAudioParam;
    memcpy(&m_stAoDevAttr, &m_pstAudioOutParam->stAoDevAttr, sizeof(MI_AO_Attr_t));
    m_eAudioStatus = AUDIO_OUT_IDLE;
    return 0;
}

MI_S32 MI_AudioDecoder::startPlayMedia(char *audioFileName)
{
    MI_S32      s32Ret = E_MI_ERR_FAILED;
    MI_S32      s32RdSize;
    WavHeader_t stWavHeaderInput;

    m_bAudioOutExit = FALSE;

    if (m_eAudioStatus != AUDIO_OUT_IDLE)
    {
        printf("current audio out status [%d]\n", m_eAudioStatus);
        return 0;
    }

    if (audioFileName)
    {
        m_s32Fd = open(audioFileName, O_RDONLY);
        if (m_s32Fd < 0)
        {
            printf("Open file %s fail\n", audioFileName);
            return s32Ret;
        }
    }
    else
    {
        printf("The audio file name is NULL\n");
        return s32Ret;
    }

    // read input wav file
    s32RdSize = read(m_s32Fd, &stWavHeaderInput, sizeof(WavHeader_t));
    if (s32RdSize != sizeof(WavHeader_t))
    {
        printf("Read input file size %d fail\n", s32RdSize);
        close(m_s32Fd);
        return s32Ret;
    }

    printf("\nAudio play: %s\n", audioFileName);
    printf("WAV channel         %d\n", stWavHeaderInput.channels);
    printf("WAV samplerate      %d\n", stWavHeaderInput.sample_rate);
    printf("WAV bits per sample %d\n", stWavHeaderInput.bits_per_sample);

    switch (stWavHeaderInput.format_type)
    {
    case 0x01:
        printf("WAV format type     %d (%s)\n\n", stWavHeaderInput.format_type, "PCM");
        break;
    case 0x06:
        printf("WAV format type     %d (%s)\n\n", stWavHeaderInput.format_type, "G711A");
        break;
    case 0x07:
        printf("WAV format type     %d (%s)\n\n", stWavHeaderInput.format_type, "G711U");
        break;
    case 0x45:
        printf("WAV format type     %d (%s)\n\n", stWavHeaderInput.format_type, "G726");
        break;
    default:
        printf("WAV format type     %d (Set wrong param)\n\n", stWavHeaderInput.format_type);
    }

    m_stAoDevAttr.enSoundMode =
        (stWavHeaderInput.channels == 2 ? E_MI_AUDIO_SOUND_MODE_STEREO : E_MI_AUDIO_SOUND_MODE_MONO);

    if (m_stAoDevAttr.enSoundMode == E_MI_AUDIO_SOUND_MODE_MONO)
    {
        m_stAoDevAttr.enChannelMode = E_MI_AO_CHANNEL_MODE_ONLY_LEFT;
    }
    else if (m_stAoDevAttr.enSoundMode == E_MI_AUDIO_SOUND_MODE_STEREO)
    {
        m_stAoDevAttr.enChannelMode = E_MI_AO_CHANNEL_MODE_STEREO;
    }

    if (m_pstAudioOutParam->enAoIfs == E_MI_AO_IF_HDMI_A
        && m_stAoDevAttr.enSampleRate != (MI_AUDIO_SampleRate_e)stWavHeaderInput.sample_rate)
    {
        if (ReSampleInit(stWavHeaderInput.sample_rate) < 0)
            return -1;
    }
    else
    {
        m_stAoDevAttr.enSampleRate = (MI_AUDIO_SampleRate_e)stWavHeaderInput.sample_rate;
    }

    AdecInit();

    // create a thread to send stream to AO
    pthread_create(&m_pthreadId_Aout, NULL, AudioOut_Task, this);
    m_eAudioStatus = AUDIO_OUT_PLAYING_WAV;
    return 0;
}

MI_S32 MI_AudioDecoder::startPlayMedia(void)
{
    if (m_eAudioStatus != AUDIO_OUT_IDLE)
    {
        printf("current audio out status [%d]\n", m_eAudioStatus);
        return 0;
    }

    m_stAoDevAttr.enFormat = g_pAudioEncoderArray[0]->m_stAudioInParam.stAudioAttr.enFormat;

    m_stAoDevAttr.enSampleRate  = g_pAudioEncoderArray[0]->m_stAudioInParam.stAudioAttr.enSampleRate;
    m_stAoDevAttr.enSoundMode   = g_pAudioEncoderArray[0]->m_stAudioInParam.stAudioAttr.enSoundMode;
    m_stAoDevAttr.u32PeriodSize = g_pAudioEncoderArray[0]->m_stAudioInParam.stAudioAttr.u32PeriodSize;

    AdecInit();

    pthread_mutex_lock(&g_audioMutex);
    g_pstAudioDecoder = this;
    pthread_mutex_unlock(&g_audioMutex);
    m_eAudioStatus = AUDIO_OUT_PLAYING_LIVE;
    return 0;
}

void MI_AudioDecoder::stopPlayMedia(MI_BOOL bStopNow)
{
    if (m_eAudioStatus == AUDIO_OUT_PLAYING_WAV)
    {
        if (bStopNow)
            m_bAudioOutExit = TRUE;

        if (m_pthreadId_Aout)
        {
            pthread_join(m_pthreadId_Aout, NULL);
            m_pthreadId_Aout = 0;
        }

        m_bAudioOutExit = FALSE;
    }
    else if (m_eAudioStatus == AUDIO_OUT_PLAYING_LIVE)
    {
        pthread_mutex_lock(&g_audioMutex);
        g_pstAudioDecoder = NULL;
        pthread_mutex_unlock(&g_audioMutex);
    }

    m_eAudioStatus = AUDIO_OUT_IDLE;
}

MI_S32 MI_AudioDecoder::setVolume(MI_S32 s8VolumeOutDb, MI_AO_GainFading_e eAudioOutGainFading)
{
    MI_AUDIO_DEV AoDevId = m_pstAudioOutParam->u32DevId;
    if (m_bAudioEnable)
    {
#ifdef CHIP_IFORD
        if (s8VolumeOutDb <= (MIN_AO_VOLUME * 8))
#else
        if (s8VolumeOutDb <= MIN_AO_VOLUME)
#endif
        {
            ExecFunc_OS(MI_AO_SetMute(AoDevId, TRUE, TRUE, m_pstAudioOutParam->eAudioOutGainFading), MI_SUCCESS);
            return 0;
        }

        ExecFunc_OS(MI_AO_SetVolume(AoDevId, m_pstAudioOutParam->s8VolumeOutDb, m_pstAudioOutParam->s8VolumeOutDb,
                                    m_pstAudioOutParam->eAudioOutGainFading),
                    MI_SUCCESS);
    }

    return 0;
}