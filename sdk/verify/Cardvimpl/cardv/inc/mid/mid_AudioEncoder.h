/*
 * mid_AudioEncoder.h- Sigmastar
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
#ifndef _MI_AUDIO_ENCODER_H_
#define _MI_AUDIO_ENCODER_H_

#include <pthread.h>
#include <sys/queue.h>

#include "mid_common.h"
#include "mid_audio_type.h"
#include "mi_ai.h"
#include "mi_sys.h"
#include "ms_notify.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#ifdef __cplusplus
}
#endif //__cplusplus

struct stAudioInfo
{
    MI_U32 u32FrameBuf;
    MI_U32 u32FrameSize;
    MI_U32 u32SampleCnt;
    TAILQ_ENTRY(stAudioInfo) tailq_entry;
};

class MI_AudioEncoder
{
    friend class MI_Muxer;
    friend class MI_AudioDecoder;
    friend void *MuxerTask(void *argv);
    friend void *AudioIn_Task(void *argv);
    friend void *AudioMux_Task(void *argv);

public:
    static MI_AudioEncoder *createNew(const int streamId);
    ~MI_AudioEncoder(void);

    void   startAudioEncoder(void);
    void   stopAudioEncoder(void);
    MI_S32 initAudioEncoder(CarDVAudioInParam *pstAudioParam);
    MI_S32 uninitAudioEncoder(void);
    void   startAudioRecord(void);
    void   stopAudioRecord(void);
    void   startAudioRtsp(void);
    void   stopAudioRtsp(void);
    void   startAudioFile(char *fileName);
    void   stopAudioFile(void);

    MI_U32 getAudioChannels(void);
    MI_U32 getAudioSampleRate(void);
    MI_U8  getAudioBitWidth(void);

    MI_S32 setAudioInputVolume(CarDVAudioInParam *pstAudioParam, MI_S8 s8VolumeInDb);
    void   mute(BOOL bMute);

private:
    void           migrateFrame(unsigned char *buf, int len, unsigned long long timestamp);
    unsigned char *encodeFrame(uint8_t *inbuf, int inlen, unsigned int *outlen, unsigned long long timestamp);
    void           releaseFrame(void);

    MI_U32          Move2AudCompBuf(MI_U32 u32SrcAddr, MI_U32 u32Size);
    AVCodecContext *GetCodecContext(void);

    MI_S32 pushQueue(MI_U32 u32FrameBuf, MI_U32 u32FrameSize, MI_U32 u32SampleCnt);
    MI_S32 popQueue(MI_U32 *pu32FrameBuf, MI_U32 *pu32FrameSize, MI_U32 *pu32SampleCnt);

    MI_S32 internalInit(void);
    MI_S32 internalDeInit(void);
    MI_S32 handlerAudioFrame(MI_AI_Data_t *pstAudioFrame);

    MI_AudioEncoder(const int streamId);
    void fillNotifyInfo(MsNotifyInfo_t *notifyInfo);

    CarDVAudioInParam m_stAudioInParam;

    BOOL      m_bAudioInExit;
    pthread_t m_pthreadId;
    pthread_t m_pMuxThreadId;

    /* RTSP */
    int           fNotifyFd;
    int           fStreamId;
    int           fShmId;
    char *        fShmSrc;
    long          fShmSize;
    long          fShmOffset;
    long          fLastShmOffset;
    unsigned long fFrameNum;

    /* FFmpeg Codec Parameters */
    AVCodec *       m_audio_codec;
    AVCodecContext *m_audio_codec_ctx;
    AVPacket        m_pkt;
    char *          m_pFileName;

    /* Audio Compress Buffer */
    MI_U8 *m_pu8AudCompBufAddr;
    MI_U32 m_u32AudCompBufWritePtr;

    MI_U8 *m_pu8G711OutBuf;
    MI_U8 *m_pu8G711SilenceBuf;
    MI_U8 *m_pu8RecSilenceBuf;
    MI_U32 m_u32RecSilenceSize;

    BOOL m_bRecordStart; /* Record status */
    BOOL m_bRtspStart;   /* RTSP status */
    BOOL m_bFileStart;   /* Audio file record status */

    TAILQ_HEAD(tailq_head, stAudioInfo) m_audioQ;
    pthread_mutex_t m_audioMutex;
    pthread_cond_t  m_audioCond;
};

void AudioInSetRecUpdateSizeCallback(AudioRecUpdateSizeCB pFunc);

#endif //_MI_AUDIO_ENCODER_H_
