/*
 * mid_mux.h- Sigmastar
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
#ifndef _MID_MUX_H_
#define _MID_MUX_H_

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include "mid_VideoEncoder.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif //__cplusplus

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define MAX_MUXER_NUMBER (8)

#define MUXER_MAX_VIDEO_TRACK  (2)
#define MUXER_FILE_NAME_LEN    (64)
#define MUXER_EXTRADATA_SIZE   (128)
#define MUXER_FRAME_QUEUE_SIZE (1024) ///< queue of frame size for data transfer

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _MuxerType_e
{
    MUXER_NORMAL   = 0,
    MUXER_EMERG    = 1,
    MUXER_SHARE    = 2,
    MUXER_TYPE_NUM = 3,
} MuxerType_e;

typedef enum _MuxerFile_e
{
    MUXER_MAIN_FLIE = 0,
    MUXER_SUB_FLIE  = 1,
    MUXER_FILE_NUM  = 2,
} MuxerFile_e;

typedef enum _MuxerErrCode
{
    MUXER_ERR_NONE = 0,
    MUXER_ERR_HOST_CANCEL_SAVE,
    MUXER_ERR_MEDIA_FILE_FULL,
    MUXER_ERR_AVBUF_FULL,
    MUXER_ERR_AVBUF_EMPTY,
    MUXER_ERR_AVBUF_OVERFLOW,
    MUXER_ERR_AVBUF_FAILURE,
    MUXER_ERR_FTABLE_FULL,
    MUXER_ERR_FTABLE_EMPTY,
    MUXER_ERR_FTABLE_OVERFLOW,
    MUXER_ERR_FTABLE_FAILURE,
    MUXER_ERR_TTABLE_FULL,
    MUXER_ERR_TTABLE_EMPTY,
    MUXER_ERR_TTABLE_OVERFLOW,
    MUXER_ERR_TTABLE_FAILURE,
    MUXER_ERR_ITABLE_FULL,
    MUXER_ERR_QUEUE_UNDERFLOW,
    MUXER_ERR_QUEUE_OVERFLOW,
    MUXER_ERR_EVENT_UNSUPPORT,
    MUXER_ERR_FILL_TAIL,
    MUXER_ERR_POST_PROCESS,
    MUXER_ERR_STREAM_NOMOVE,
    MUXER_ERR_ONLYKEEP_EMERGRECD,
    MUXER_ERR_INVLAID_STATE,
    MUXER_ERR_UNSUPPORT,
    MUXER_ERR_COMP_BUF_OVERFLOW,
    MUXER_ERR_COMP_BUF_UNDERFLOW,
} MuxerErrCode;

typedef enum _MuxerTxStatus
{
    MUXER_TX_STAT_IDLE = 0,
    MUXER_TX_STAT_PRE_RECORD,
    MUXER_TX_STAT_RECORD,
    MUXER_TX_STAT_PAUSE_PRE_RECORD,
    MUXER_TX_STAT_PAUSE_RECORD
} MuxerTxStatus;

typedef enum _MuxerSlowMotionStatus
{
    MUXER_SLOW_MOTION_NONE = 0,
    MUXER_SLOW_MOTION_X2   = 2,
    MUXER_SLOW_MOTION_X4   = 4,
    MUXER_SLOW_MOTION_X8   = 8,
} MuxerSlowMotionStatus;

typedef enum _MuxerMp4TrakType
{
    MUXER_VIDEO = 0,
    MUXER_AUDIO,
    MUXER_THUMB,
    MUXER_GPS,
    MUXER_GSENSOR
} MuxerMp4TrakType;

typedef void (*MuxerErrCallback)(MI_U8, MuxerType_e, MuxerFile_e);
typedef void (*MuxerPostFileCallback)(MuxerType_e, MuxerFile_e, int, char *);
typedef void (*MuxerProcessFileFullCallback)(MuxerType_e);
typedef const char *(*MuxerMp4TrakNameCallback)(MuxerType_e, MuxerFile_e, MuxerMp4TrakType);

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

class MI_Muxer
{
    friend class MI_VideoEncoder;
    friend class MI_AudioEncoder;
    friend void *       MuxerTask(void *argv);
    friend void *       AudioIn_Task(void *argv);
    friend void *       Stream_Task(void *argv);
    friend BOOL         MuxerCheckSeamless(MI_U8 u8MuxerId, MI_U8 u8TrackId);
    friend MuxerErrCode CheckAVTimeOverLimit(MI_U8 u8MuxerId);
    friend MuxerErrCode CheckFileSizeLimit(MI_U8 u8MuxerId);

public:
    static MI_Muxer *createNew(MI_U8 camId, MI_U8 vidTrackNum);
    ~MI_Muxer(void);

    void          MuxerEnable(BOOL bEnable);
    BOOL          IsMuxerEnable(void);
    MuxerTxStatus GetMuxerStatus(void);

    MI_U8       GetMuxerCamID(void);
    void        SetStreamId(MI_U8 u8TrackId, MI_U8 u8VidStreamdId);
    void        SetThumbStreamId(MI_S32 s32ThumbStreamdId);
    void        SetMuxerType(MuxerType_e eMuxerType);
    MuxerType_e GetMuxerType(void);
    void        SetMuxerFile(MuxerFile_e eMuxerFile);
    MuxerFile_e GetMuxerFile(void);
    void        SetGpsTrack(BOOL bGpsTrackEnable);
    void        SetGsnrTrack(BOOL bGsnrTrackEnable);
    void        SetThumbnailTrack(BOOL bThumbTrackEnable);
    void        SetFrameRate(MI_U16 u16FpsNum, MI_U16 u16FpsDen, MI_BOOL bTimeLapse, MuxerSlowMotionStatus eSlowMotion);
    void        SetFileSizeLimit(MI_U64 u64LimitSize);
    void        SetFileNameInfo(char *ubFileName);
    char *      GetFileName(void);

    BOOL IsGpsFrameQFull(void);
    void PushInfoToGpsFrameQ(MI_U32 u32CurAddr, MI_U64 u64CurTime);
    BOOL IsGsnrFrameQFull(void);
    void PushInfoToGsnrFrameQ(MI_U32 u32CurAddr, MI_U64 u64CurTime);

private:
    void InitMuxer(void);
    void UnInitMuxer(void);
    void StartMuxer(void);
    BOOL StopMuxer(BOOL bKeepEncoding);
    void PauseMuxer(void);
    void ResumeMuxer(void);
    void SetDropFrameSeq(MI_U8 u8TrackId, MI_U32 u32FrameSeq);

    BOOL         IsAudFrameQFull(void);
    void         PushInfoToAudFrameQ(MI_U32 u32CurAddr, MI_U32 u32CurSize, MI_U32 u32CurSamples, MI_U64 u64CurTime);
    MuxerErrCode ReadInfoFromAudFrameQ(MI_U32 *pu32FrameAddr, MI_U32 *pu32FrameSize, MI_U32 *pu32FrameSamples,
                                       MI_U64 *pu64FrameTime);
    void         AdvanceAudFrameQRdPtr(void);
    void         ResetAudFrameQRdPtr(void);
    void         ResetAudFrameQWrPtr(void);

    MuxerErrCode ReadInfoFromGpsFrameQ(MI_U32 *pu32FrameAddr, MI_U64 *pu64FrameTime);
    void         AdvanceGpsFrameQRdPtr(void);
    void         ResetGpsFrameQRdPtr(void);
    void         ResetGpsFrameQWrPtr(void);

    MuxerErrCode ReadInfoFromGsnrFrameQ(MI_U32 *pu32FrameAddr, MI_U64 *pu64FrameTime);
    void         AdvanceGsnrFrameQRdPtr(void);
    void         ResetGsnrFrameQRdPtr(void);
    void         ResetGsnrFrameQWrPtr(void);

    BOOL IsVidFrameQFull(MI_U8 u8TrackId);
    void PushInfoToVidFrameQ(MI_U8 u8TrackId, MI_U32 u32CurSize, MI_U64 u64CurTime, MI_U32 u32CurType, MI_U32 u32CurSeq,
                             MI_S32 s32FramePoc);
    MuxerErrCode ReadInfoFromVidFrameQ(MI_U8 u8TrackId, MI_U32 *pu32FrameSize, MI_U64 *pu64FrameTime,
                                       MI_U32 *pu32FrameType, MI_U32 *pu32FrameSeq, MI_S32 *ps32FramePoc);
    MuxerErrCode ReadInfoFromVidFrameQByIdx(MI_U8 u8TrackId, MI_U32 u32Idx, MI_U32 *pu32FrameSize,
                                            MI_U64 *pu64FrameTime, MI_U32 *pu32FrameType);
    void         PutEofMarkToVidFrameQ(MI_U8 u8TrackId, BOOL bFileFull);
    void         AdvanceVidFrameQRdPtr(MI_U8 u8TrackId);
    void         ResetVidFrameQRdPtr(MI_U8 u8TrackId);
    void         ResetVidFrameQWrPtr(MI_U8 u8TrackId);
    void         GetVidFrameQLastWrPtr(MI_U8 u8TrackId, MI_U32 *pu32WrIdx);
    void         DumpVidFrameQInfo(MI_U8 u8TrackId);

    void GetNextAudioFrameReadAddr(MI_U32 *pu32FrameAddr, MI_U32 u32FrameSize);
    void AdvanceAudioBufReadPtr(MI_U32 u32Size);

    void GetNextVideoFrameReadAddr(MI_U8 u8TrackId, MI_U32 *pu32FrameAddr, MI_U32 u32FrameSize);
    void AdvanceVideoBufReadPtr(MI_U8 u8TrackId, MI_U32 u32Size);
    void GetCurVideoBufReadPtr(MI_U8 u8TrackId, MI_U32 *pu32ReadPtr, MI_U32 *pu32ReadWrap);
    void DumpVideoBufPtr(MI_U8 u8TrackId);

    void SetEndOfFrameStatus(MI_U8 u8TrackId, BOOL bEnable);
    BOOL GetEndOfFrameStatus(MI_U8 u8TrackId);

    MuxerErrCode TxAud2RepackBuf(MI_BOOL bPostProcess);
    MuxerErrCode TxVid2RepackBuf(MI_U8 u8TrackId, MI_BOOL bPostProcess, MI_BOOL bProcessOvl);
    MuxerErrCode TxGps2RepackBuf(MI_BOOL bPostProcess);
    MuxerErrCode TxGsnr2RepackBuf(MI_BOOL bPostProcess);
    MuxerErrCode TxThumb2RepackBuf(MI_BOOL bPostProcess);

    MuxerErrCode PostProcess(void);

private:
    void         SetErrCode(MuxerErrCode eErrCode);
    MuxerErrCode GetErrCode(void);

    // Common Information
    MI_U8             m_u8CamId;
    MI_U8             m_u8VidTrackNum;
    MI_VideoEncoder **m_ppVidEncoder[MUXER_MAX_VIDEO_TRACK];
    MI_VideoEncoder **m_ppThumbEncoder;
    MuxerType_e       m_eMuxerType;                                 ///< Normal / emergency / share record
    MuxerFile_e       m_eMuxerFile;                                 ///< Main stream file / Sub stream file
    MuxerTxStatus     m_eRecdTxStatus;                              ///< Merger Tx status
    volatile BOOL     m_bEndOfFrame[MUXER_MAX_VIDEO_TRACK];         ///< End of frame status
    MI_U32            m_u32VidFrameSeqDrop[MUXER_MAX_VIDEO_TRACK];  ///< Drop frames until frame sequence
    MI_U32            m_u32VidFrameLastSeq[MUXER_MAX_VIDEO_TRACK];  ///< Last frame sequence
    MI_U32            m_u32VidFrameFisrtSeq[MUXER_MAX_VIDEO_TRACK]; ///< First frame sequence

    // Statistic Information
    MI_U32 m_u32VideoFrameCount[MUXER_MAX_VIDEO_TRACK]; ///< Accumulated encoded frame count
    MI_U32 m_u32VideoFrameNum[MUXER_MAX_VIDEO_TRACK];   ///< Next frame POC
    MI_U32 m_u32AudioFrameCount;                        ///< Audio frame count
    MI_U32 m_u32GpsFrameCount;                          ///< GPS frame count
    MI_U32 m_u32GsnrFrameCount;                         ///< Gsensor frame count
    MI_U64 m_u64WriteDataLen;                           ///< Vaild data size has been written
    MI_U64 m_u64FileSizeLimit;                          ///< Limit file size

    // Time Information
    MI_U64 m_u64VideoTotalTime[MUXER_MAX_VIDEO_TRACK];    ///< Video total time, update every frame
    MI_U64 m_u64VidFrameDiffTime[MUXER_MAX_VIDEO_TRACK];  ///< Video frame duration, update every frame
    MI_U64 m_u64VideoPreRecdTime[MUXER_MAX_VIDEO_TRACK];  ///< Video total pre-record time
    MI_U64 m_u64VidRecdBaseTime[MUXER_MAX_VIDEO_TRACK];   ///< Time of start capture video
    MI_U64 m_u64VidResumeBaseTime[MUXER_MAX_VIDEO_TRACK]; ///< Time of resume capture video
    MI_U64 m_u64VidPauseTime[MUXER_MAX_VIDEO_TRACK];      ///< Video total pause time

    MI_U64 m_u64AudRecdBaseTime;   ///< Time of start capture audio
    MI_U64 m_u64AudResumeBaseTime; ///< Time of resume capture audio

    MI_U64 m_u64GpsTotalTime;      ///< GPS total time, update every frame
    MI_U64 m_u64GpsFrameDiffTime;  ///< GPS frame duration, update every frame
    MI_U64 m_u64GpsRecdBaseTime;   ///< Time of start capture GPS
    MI_U64 m_u64GpsResumeBaseTime; ///< Time of resume capture GPS
    MI_U64 m_u64GpsPauseTime;      ///< GPS total pause time

    MI_U64 m_u64GsnrTotalTime;      ///< Gsensor total time, update every frame
    MI_U64 m_u64GsnrFrameDiffTime;  ///< Gsensor frame duration, update every frame
    MI_U64 m_u64GsnrRecdBaseTime;   ///< Time of start capture Gsensor
    MI_U64 m_u64GsnrResumeBaseTime; ///< Time of resume capture Gsensor
    MI_U64 m_u64GsnrPauseTime;      ///< Gsensor total pause time

    // Video Frame Queue index Parameters
    MI_BOOL m_bVidFrmQueueFullPrintLog[MUXER_MAX_VIDEO_TRACK];
    MI_U32  m_u32VidFrmQueueBaseReadIdx[MUXER_MAX_VIDEO_TRACK];
    MI_U32  m_u32VidFrmQueueReadIdx[MUXER_MAX_VIDEO_TRACK];                    ///< Read-out index of frame queue
    MI_U32  m_u32VidFrmQueueWriteIdx[MUXER_MAX_VIDEO_TRACK];                   ///< Write-in index of frame queue
    MI_U32  m_u32VidFrmQueueReadWrap[MUXER_MAX_VIDEO_TRACK];                   ///< Read wrap count for frame queue
    MI_U32  m_u32VidFrmQueueWriteWrap[MUXER_MAX_VIDEO_TRACK];                  ///< Write wrap count for frame queue
    MI_U32  m_u32VidFrameSizeQ[MUXER_MAX_VIDEO_TRACK][MUXER_FRAME_QUEUE_SIZE]; ///< Pointer to frame size queue
    MI_U64  m_u64VidFrameTimeQ[MUXER_MAX_VIDEO_TRACK][MUXER_FRAME_QUEUE_SIZE]; ///< Pointer to frame time queue
    MI_U32  m_u32VidFrameTypeQ[MUXER_MAX_VIDEO_TRACK][MUXER_FRAME_QUEUE_SIZE]; ///< Pointer to frame type queue
    MI_U32  m_u32VidFrameSeqQ[MUXER_MAX_VIDEO_TRACK][MUXER_FRAME_QUEUE_SIZE];  ///< Pointer to frame sequence queue
    MI_S32  m_s32VidFramePocQ[MUXER_MAX_VIDEO_TRACK][MUXER_FRAME_QUEUE_SIZE];  ///< Pointer to frame POC queue
    pthread_mutex_t m_stVidFrameQMutex[MUXER_MAX_VIDEO_TRACK];

    // Audio Frame Queue index Parameters
    MI_U32          m_u32AudFrmQueueReadIdx;                      ///< Read-out index of frame queue
    MI_U32          m_u32AudFrmQueueWriteIdx;                     ///< Write-in index of frame queue
    MI_U32          m_u32AudFrmQueueReadWrap;                     ///< Read wrap count for frame queue
    MI_U32          m_u32AudFrmQueueWriteWrap;                    ///< Write wrap count for frame queue
    MI_U32          m_u32AudFrameAddrQ[MUXER_FRAME_QUEUE_SIZE];   ///< Pointer to frame buffer queue
    MI_U32          m_u32AudFrameSizeQ[MUXER_FRAME_QUEUE_SIZE];   ///< Pointer to frame size queue
    MI_U64          m_u64AudFrameTimeQ[MUXER_FRAME_QUEUE_SIZE];   ///< Pointer to frame time queue
    MI_U32          m_u32AudFrameSampleQ[MUXER_FRAME_QUEUE_SIZE]; ///< Pointer to frame samples queue
    pthread_mutex_t m_stAudFrameQMutex;

    // GPS Frame Queue index Parameters
    MI_U32          m_u32GpsFrmQueueReadIdx;                    ///< Read-out index of frame queue
    MI_U32          m_u32GpsFrmQueueWriteIdx;                   ///< Write-in index of frame queue
    MI_U32          m_u32GpsFrmQueueReadWrap;                   ///< Read wrap count for frame queue
    MI_U32          m_u32GpsFrmQueueWriteWrap;                  ///< Write wrap count for frame queue
    MI_U32          m_u32GpsFrameAddrQ[MUXER_FRAME_QUEUE_SIZE]; ///< Pointer to frame buffer queue
    MI_U64          m_u64GpsFrameTimeQ[MUXER_FRAME_QUEUE_SIZE]; ///< Pointer to frame time queue
    pthread_mutex_t m_stGpsFrameQMutex;

    // Gsensor Frame Queue index Parameters
    MI_U32          m_u32GsnrFrmQueueReadIdx;                    ///< Read-out index of frame queue
    MI_U32          m_u32GsnrFrmQueueWriteIdx;                   ///< Write-in index of frame queue
    MI_U32          m_u32GsnrFrmQueueReadWrap;                   ///< Read wrap count for frame queue
    MI_U32          m_u32GsnrFrmQueueWriteWrap;                  ///< Write wrap count for frame queue
    MI_U32          m_u32GsnrFrameAddrQ[MUXER_FRAME_QUEUE_SIZE]; ///< Pointer to frame buffer queue
    MI_U64          m_u64GsnrFrameTimeQ[MUXER_FRAME_QUEUE_SIZE]; ///< Pointer to frame time queue
    pthread_mutex_t m_stGsnrFrameQMutex;

    // Video Compress Buffer Parameters
    MI_U32 m_u32VidCompBufAddr[MUXER_MAX_VIDEO_TRACK];     ///< Video recording buffer address
    MI_U32 m_u32VidCompBufSize[MUXER_MAX_VIDEO_TRACK];     ///< Video recording buffer size
    MI_U32 m_u32VidCompBufReadPtr[MUXER_MAX_VIDEO_TRACK];  ///< Read-out address of video encoded compressed buffer
    MI_U32 m_u32VidCompBufReadWrap[MUXER_MAX_VIDEO_TRACK]; ///< Wrap times of read pointer

    // Video Encode Parameters
    MI_U32    m_u32FrameWidth[MUXER_MAX_VIDEO_TRACK];  ///< Video frame width
    MI_U32    m_u32FrameHeight[MUXER_MAX_VIDEO_TRACK]; ///< Video frame height
    MI_U32    m_u32BitRate[MUXER_MAX_VIDEO_TRACK];     ///< Video bit rate
    AVCodecID m_eEncodeType[MUXER_MAX_VIDEO_TRACK];    ///< Video encoder : H264 / H265
    MI_U16    m_u16FrameRateNum;                       ///< Video encoder frame rate num
    MI_U16    m_u16FrameRateDen;                       ///< Video encoder frame rate den
    MI_U16    m_u16MgrFrameRateNum;                    ///< Video merger frame rate num
    MI_U16    m_u16MgrFrameRateDen;                    ///< Video merger frame rate den
    MI_BOOL   m_ubTimeLapse;                           ///< Merge time-Lapse or not
    MI_U8     m_u8SlowMotion;                          ///< Slow motion status

    // Audio Encode Parameters
    BOOL   m_bAudioTrackAvailable; ///< Is there any audio track
    MI_U32 m_u32AudioSampleFreq;   ///< Audio sample frequency. i.e. 22050.
    MI_U32 m_u32AudioBitPerSample; ///< Audio bits per sample
    MI_U8  m_u8AudioChannelNo;     ///< Audio record channel count. i.e. 1/2

    // GPS & Gsensor info
    BOOL  m_bGpsTrackAvailable;  ///< Is there any GPS track
    MI_U8 m_u8GpsTrackIdx;       ///< GPS stream idx
    BOOL  m_bGsnrTrackAvailable; ///< Is there any Gsensor track
    MI_U8 m_u8GsnrTrackIdx;      ///< Gsensor stream idx

    // Thumbnail info
    BOOL m_bThumbTrackAvailable; ///< Is there any thumbnail track
    BOOL m_bThumbMerge;          ///< Is there any thumbnail has beed merged

    // File Parameters
    char m_szRecdFileName[MUXER_FILE_NAME_LEN];
    char m_szCurRecdFileName[MUXER_FILE_NAME_LEN];

private:
    MI_Muxer(MI_U8 camId, MI_U8 vidTrackNum);

    void OpenAndPreProc(void);
    void CloseAndPostProc(void);

    void ResetAudCommonInfo(void);
    void ResetVidCommonInfo(MI_U8 u8TrackId);
    void ResetGpsCommonInfo(void);
    void ResetGsnrCommonInfo(void);

    MuxerErrCode PauseProcess(void);

    BOOL m_bMuxerEnable;
    BOOL m_bMuxerExitWorking;

    /* FFmpeg */
    AVOutputFormat * m_outfmt;
    AVFormatContext *m_fmt_ctx;
    AVStream *       m_audio_stream;
    AVStream *       m_video_stream[MUXER_MAX_VIDEO_TRACK];
    AVStream *       m_gps_stream;
    AVStream *       m_gsnr_stream;
    AVStream *       m_thumb_stream;
    uint8_t          m_extradata[MUXER_MAX_VIDEO_TRACK][MUXER_EXTRADATA_SIZE];
    int              m_extradata_size[MUXER_MAX_VIDEO_TRACK];

    MuxerErrCode m_eErrCode;
};

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

void  MuxerSendMgr(void);
void  MuxerSendMgrOvl(void);
void  MuxerSendPreStart(void);
void  MuxerSendStart(MuxerType_e eMuxerType);
void  MuxerSendStop(MuxerType_e eMuxerType, BOOL bKeepEncoding);
void  MuxerSendPause(void);
void  MuxerSendResume(void);
void  MuxerStartThread();
void  MuxerStopThread();
BOOL  MuxerCheckSeamless(MI_U8 u8MuxerId, MI_U8 u8TrackId);
void  MuxerSetLimitTime(MuxerType_e eMuxerType, MI_U64 u64LimitTime);
void  MuxerSetPreRecordTime(MuxerType_e eMuxerType, MI_U64 u64PreRecordTime);
void  MuxerSetLimitTimeExcludePreRecord(MuxerType_e eMuxerType, MI_BOOL bExclude);
void  MuxerSetErrCallback(MuxerErrCallback pFunc);
void  MuxerSetOpenFileErrCallback(MuxerErrCallback pFunc);
void  MuxerSetPostFileCallback(MuxerPostFileCallback pFunc);
void  MuxerSetProcessFileFullCallback(MuxerProcessFileFullCallback pFunc);
void  MuxerSetReservedMoovSizePerFrame(MI_U16 u16Size);
void  MuxerSetMp4FileAlignSize(MI_U32 u32Size);
void  MuxerSetGpsChunkSize(MI_U32 u32Size);
void  MuxerSetGsnrChunkSize(MI_U32 u32Size);
void  MuxerSetMpegtsGpsGsnrPid(MI_U16 u16GpsPid, MI_U16 u16GsnrPid);
void  MuxerSetCheckTimeFullRefCamId(MI_U8 u8RefCamId);
MI_U8 MuxerGetCheckTimeFullRefCamId(void);
void  MuxerSetMp4TrakNameCallback(MuxerMp4TrakNameCallback pFunc);

#endif //_MID_MUX_H_