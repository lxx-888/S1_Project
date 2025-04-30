/*
 * module_AudioDecoder.h- Sigmastar
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
#ifndef _MODULE_AUDIO_DECODER_H_
#define _MODULE_AUDIO_DECODER_H_

#include "mid_common.h"
#include "mid_audio_type.h"
#include "mi_ao.h"
#include "mi_sys.h"

extern "C" {
#include "libswresample/swresample.h"
}

#ifdef CHIP_IFORD
#define MIN_AO_VOLUME (-63.5)
#define MAX_AO_VOLUME (64)
#else
#define MIN_AO_VOLUME (-60)
#define MAX_AO_VOLUME (30)
#endif

class MI_AudioDecoder
{
    friend void *AudioOut_Task(void *argv);
    friend void  audioInToaudioOut(MI_AI_Data_t *pstAudioFrame);

public:
    static MI_AudioDecoder *createNew(void);
    ~MI_AudioDecoder(void);

    MI_S32 initAudioDecoder(CarDVAudioOutParam *pstAudioParam);
    MI_S32 startPlayMedia(char *audioFileName);
    MI_S32 startPlayMedia(void);
    void   stopPlayMedia(MI_BOOL bStopNow);
    MI_S32 setVolume(MI_S32 s32VolumeOutDb, MI_AO_GainFading_e eAudioOutGainFading);

private:
    MI_AudioDecoder(void);
    MI_S32 AdecInit(void);
    MI_S32 AdecDeinit(void);
    MI_S32 ReSampleInit(MI_S32 s32InputSampleRate);
    MI_S32 ReSampleDeInit(void);
    MI_S32 ReSample(MI_U8 *pu8InBuf, MI_U32 u32SampleCnt, MI_U8 **ppu8OutBuf);

    BOOL                m_bAudioOutExit;
    pthread_t           m_pthreadId_Aout;
    MI_AO_Attr_t        m_stAoDevAttr;
    CarDVAudioOutParam *m_pstAudioOutParam;
    MI_S32              m_s32Fd;
    MI_BOOL             m_bAudioEnable;
    AudioOutStatus_e    m_eAudioStatus;

    struct SwrContext *m_pstSwrCtx;
    MI_U8 *            m_pu8OutBuf;
    MI_U32             m_u32InSampleRate;
    MI_U32             m_u32OutSampleRate;
};

#endif //_MODULE_AUDIO_DECODER_H_