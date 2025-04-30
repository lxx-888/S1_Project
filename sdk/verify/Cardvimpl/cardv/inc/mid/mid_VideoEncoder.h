/*
 * mid_VideoEncoder.h- Sigmastar
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
#ifndef _MI_VIDEO_ENCODER_H_
#define _MI_VIDEO_ENCODER_H_
#pragma once

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include <pthread.h>
#include <errno.h>
#include <sys/queue.h>

#include "mid_common.h"
#include "mid_utils.h"
#include "mi_venc.h"
#include "mi_sys.h"
#include "ms_notify.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

#ifdef __cplusplus
}
#endif //__cplusplus

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define STREAM_PACK_CNT      (2)
#define MUXER_NUM_SHARE_VENC (2)

#define IS_IFRAME(x) (x == E_MI_VENC_H265E_NALU_ISLICE || x == E_MI_VENC_H264E_NALU_ISLICE)

struct stThumbInfo
{
    MI_U8  u8Used;
    MI_U32 u32VidThumbSize;
    MI_U64 u64VidThumbTime;
    MI_U8 *pu8VidThumbVirBuf;
    MI_PHY u64VidThumbPhyBuf;
    TAILQ_ENTRY(stThumbInfo) tailq_entry;
};

struct stThumbPipeInfo
{
    MI_U32 u32SclDevId;
    MI_U32 u32UseHwSclMask;
    MI_U16 u16Width;
    MI_U16 u16Height;
};

struct stJpegInfo
{
    MI_U32 u32JpegSize;
    MI_U8 *pu8JpegVirBuf;
    MI_PHY u64JpegPhyBuf;
    MI_U8 *pu8ThumbBuf;
    MI_U32 u32ThumbSize;
    TAILQ_ENTRY(stJpegInfo) tailq_entry;
};

typedef void (*VideoCaptureCB)(int, MI_U8 *, MI_U32, MI_U8 *, MI_U32);
typedef void (*VideoCaptureThumbCB)(MI_U8);
typedef void (*VideoUserGetFrameCB)(MI_U32, MI_VENC_Stream_t *, MI_U32);
typedef void (*VideoDurationCB)(int, MI_U64, MI_U32);
typedef void (*VideoSetExpoCB)(MI_U8, MI_U32, MI_U32);

typedef enum _VideoStreamType_e
{
    VIDEO_STREAM_RECORD  = 0,
    VIDEO_STREAM_SUBREC  = 1,
    VIDEO_STREAM_CAPTURE = 2,
    VIDEO_STREAM_RTSP    = 3,
    VIDEO_STREAM_UVC     = 4,
    VIDEO_STREAM_THUMB   = 5,
    VIDEO_STREAM_SHARE   = 6,
    VIDEO_STREAM_USER    = 7,
} VideoStreamType_e;

typedef struct CarDV_VencAttr_s
{
    MI_BOOL bUsed;
    MI_BOOL bCreate;
    MI_U8   u8CamId;

    MI_U32 u32Width;
    MI_U32 u32Height;
    MI_U32 u32MaxWidth;
    MI_U32 u32MaxHeight;
    MI_U32 u32BufSize;
    MI_U32 u32BitRate;
    MI_U8  u8VBR;        // 0 : CBR, 1 : VBR
    MI_S32 s32ChangePos; // For VBR
    MI_U32 u32Gop;
    MI_U8  u8EncoderType;
    MI_U32 u32VencFrameRate;
    MI_U32 u32MaxIQp;
    MI_U32 u32MinIQp;
    MI_U32 u32MaxPQp;
    MI_U32 u32MinPQp;
    MI_S32 s32IPQPDelta;
    MI_U32 u32Qfactor;

    MI_BOOL bPassVenc;      // use external stream instead of MI_VENC stream
    MI_U8   u8ExtStreamChn; // external stream channel. Ex: RTSP input channel

    VideoStreamType_e eVideoType;

    MI_U32 u32VidCompBufSize;
    MI_U8  u8VidThumbBufCnt;

    MI_U8 u8MuxerNum;
    MI_U8 u8MuxerIdx[MUXER_NUM_SHARE_VENC];
    MI_U8 u8MuxerVidTrackId;

    CarDV_BindParam_t stVencInBindParam;
} CarDV_VencAttr_t;

typedef struct CarDV_VencPrivPoolCfg_s
{
    MI_BOOL bConfigPrivPool;
    MI_U16  u16PrivPoolMaxWidth;
    MI_U16  u16PrivPoolMaxHeight;
    MI_U16  u16PrivPoolRingLine;
} CarDV_VencPrivPoolCfg_t;

//==============================================================================
//
//                              CLASS DEFINE
//
//==============================================================================

class MI_VideoEncoder
{
    friend class MI_Muxer;
    friend void *Stream_Task(void *argv);

public:
    static MI_VideoEncoder *createNew(MI_VENC_CHN venChn, CarDV_VencAttr_t *pstVideoParam);
    ~MI_VideoEncoder(void);

private:
    MI_VideoEncoder(MI_VENC_CHN venChn, CarDV_VencAttr_t *pstVideoParam);

public:
    MI_S32 startVideoEncoder(BOOL bStartThread);
    MI_S32 stopVideoEncoder(void);
    MI_S32 captureJpegFile(MI_U32 u32FileCnt);
    MI_S32 captureLongExpoJpegFile(MI_U32 u32FileCnt, MI_U32 u32ShutterUs);
    MI_S32 captureVideoThumbnail(void);
    MI_S32 initVideoEncoder(void);
    MI_S32 uninitVideoEncoder(void);
    MI_S32 setResolution(MI_U32 width, MI_U32 height);
    MI_S32 setBitrate(U32 bitrate);
    MI_S32 setFrameRate(MI_U32 frameRate, MI_U32 bChangeBitrate = 0);
    MI_S32 setGop(MI_U32 gop);
    MI_S32 SetRoiCfg(MI_VENC_RoiCfg_t *pstRoiCfg);
    MI_S32 requestIDR(MI_BOOL bInstant);
    MI_S32 Query(MI_VENC_ChnStat_t *pstStat);
    MI_S32 GetStream(MI_VENC_Stream_t *pstStream, MI_S32 s32MilliSec);
    MI_S32 ReleaseStream(MI_VENC_Stream_t *pstStream);

    MI_U8 getCamId(void)
    {
        return m_stVencAttr.u8CamId;
    };
    MI_S32 getVencDev(void)
    {
        return m_s32VencDev;
    };
    MI_S32 getVencChn(void)
    {
        return m_s32VencChn;
    };
    MI_S32 getVencFd(void)
    {
        return m_s32VencFd;
    };
    MI_U8 getCodec(void)
    {
        return m_stVencAttr.u8EncoderType;
    };
    void setCodec(MI_U8 u8EncoderType)
    {
        m_stVencAttr.u8EncoderType = u8EncoderType;
    };
    MI_U8 getVideoType(void)
    {
        return m_stVencAttr.eVideoType;
    };
    MI_U8 getMuxerNum(void)
    {
        return m_stVencAttr.u8MuxerNum;
    };
    MI_U8 getMuxerIdx(MI_U8 u8Idx)
    {
        return m_stVencAttr.u8MuxerIdx[u8Idx];
    };
    MI_U8 getMuxerTrackId(void)
    {
        return m_stVencAttr.u8MuxerVidTrackId;
    };
    MI_S32 getWidth(void)
    {
        return m_stVencAttr.u32Width;
    };
    MI_S32 getHeight(void)
    {
        return m_stVencAttr.u32Height;
    };
    MI_U32 getBitrate(void)
    {
        return m_stVencAttr.u32BitRate;
    };
    MI_U32 getJpegQFactor(void)
    {
        return m_stVencAttr.u32Qfactor;
    };
    MI_U32 getFrameRate(void)
    {
        return m_stVencAttr.u32VencFrameRate;
    };
    MI_U64 getCheckIInterval(void)
    {
        return m_u64CheckIInterval;
    };
    MI_BOOL isEncoding(void)
    {
        return m_bEncoding;
    };
    MI_BOOL getStreamStatus(void)
    {
        return m_bGetStream;
    };
    CarDV_BindParam_t getInBindParam(void)
    {
        return m_stVencAttr.stVencInBindParam;
    };

private:
    MI_S32 internalInit(void);
    MI_S32 internalDeInit(void);
    void   startGetStreamThread(void);
    void   stopGetStreamThread(void);
    MI_S32 pollingStream(void);
    MI_S32 bindVideoEncoder(MI_BOOL bBind);
    void   handlerStream(MI_VENC_Stream_t *pstStream);
    void   handlerStreamPost(void);

    MI_U32 GetCompBufAddr(void);
    MI_U32 GetCompBufSize(void);
    MI_S32 DmaMove2VidCompBuf(MI_VENC_Pack_t *pstPack, MI_U32 u32FrameSize);
    void   GetCompBufWritePtr(MI_U32 *pu32CompBufWritePtr, MI_U32 *pu32CompBufWriteWrap);

    MI_S32 PushVidThumb(MI_VENC_Pack_t *pstPack, MI_U32 u32FrameSize, MI_U64 u64ThumbTime);
    MI_S32 ReadInfoFromVidThumbQByIdx(MI_U32 u32Idx, MI_U32 *pu32ThumbSize, MI_U64 *pu64ThumbTime);
    MI_U8 *GetVidThumbBufAddrByIdx(MI_U32 u32Idx);
    void   PutVidThumbBufAddrByIdx(MI_U32 u32Idx);

    MI_S32 pushJpegQueue(MI_VENC_Pack_t *pstPack, MI_U32 u32FrameSize);
    MI_S32 popJpegQueue(MI_BOOL bGetThumb);
    MI_S32 kickJpegQueueToEncodeThumbnail(void);

private:
    MI_VENC_DEV m_s32VencDev;
    MI_VENC_CHN m_s32VencChn;
    MI_S32      m_s32VencFd;
    MI_U64      m_u64CheckIInterval;
    MI_U64      m_u64KeyFrameLastPts;

    CarDV_VencAttr_t m_stVencAttr;
    MI_BOOL          m_bBindEncoder;
    MI_BOOL          m_bEncoding;

    /* Video Compress Buffer */
    MI_U8 * m_pu8VidCompBufAddr;
    MI_PHY  m_u64VidCompPhyBufAddr;
    MI_U32  m_u32VidCompBufWritePtr;  ///< Write-in pointer of video compressed buffer
    MI_U32  m_u32VidCompBufWriteWrap; ///< Wrap times of write-in pointer
    MI_BOOL m_bVidCompBufOvlPrintLog[MUXER_NUM_SHARE_VENC];

    /* Thumbnail Buffer */
    MI_U8 m_u8VidThumbBufCnt;
    TAILQ_HEAD(tailq_head, stThumbInfo) m_thumbQ;
    stThumbInfo *   m_pstThumbInfo;
    pthread_mutex_t m_thumbMutex;

    TAILQ_HEAD(jpeg_tailq_head, stJpegInfo) m_jpegQ;
    MI_U32  m_u32JpegFileCnt;
    MI_U32  m_u32GetJpegCnt;
    MI_U32  m_u32MaxJpegSize;
    MI_U32  m_u32ShutterUs;
    MI_BOOL m_bLongExpo;
    MI_BOOL m_bLongExpoJpegMatch;
    MI_BOOL m_bJpegDone;

    pthread_t m_tStreamThread;
    MI_BOOL   m_bGetStream;

    void *m_pvStreamInfo;
};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void WaitImiJpegDone(void);
void WaitFrameJpegDone(void);
void TriggerImiJpegDone(void);
void TriggerFrameJpegDone(void);

void VideoSetCaptureCallback(VideoCaptureCB pFunc);
void VideoSetCaptureThumbCallback(VideoCaptureThumbCB pFunc);
void VideoSetUserGetFrameCallback(VideoUserGetFrameCB pFunc);
void VideoSetDurationFrameCallback(VideoDurationCB pFunc);
void VideoSetExpoCallback(VideoSetExpoCB pFunc);
void VideoSetThumbPipeInfo(stThumbPipeInfo *pstPipeInfo);

#endif //_MI_VIDEO_ENCODER_H_
