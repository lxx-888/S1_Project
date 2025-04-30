/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#include "ssos_def.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_packet_audio.h"
#include "ptree_packet_video.h"
#include "ptree_packet_ts.h"
#include "ptree_mod_mux.h"
#include "ptree_sur_mux.h"
#include "ptree_sur_sys.h"
#include "ptree_maker.h"
#include "ptree_packer.h"
#include "ssos_task.h"
#include "ptree_linker.h"
#include "ptree_nlinker_out.h"
#include "ptree_packet.h"
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#define STREAM_DURATION                                 60.0
#define CALCULATE_RESERVED_MOOV_SIZE(fps, duration_sec) (64 * (fps) * (duration_sec))

typedef struct PTREE_MOD_MUX_Stream_s
{
    AVStream *st;
    AVPacket *tmpPkt;
} PTREE_MOD_MUX_Stream_t;

typedef struct PTREE_MOD_MUX_Obj_s    PTREE_MOD_MUX_Obj_t;
typedef struct PTREE_MOD_MUX_InObj_s  PTREE_MOD_MUX_InObj_t;
typedef struct PTREE_MOD_MUX_OutObj_s PTREE_MOD_MUX_OutObj_t;

struct PTREE_MOD_MUX_InObj_s
{
    PTREE_MOD_InObj_t  base;
    PTREE_LINKER_Obj_t syncLinker;
    union
    {
        PTREE_PACKET_VIDEO_Info_t videoInfo;
        PTREE_PACKET_AUDIO_Info_t audioInfo;
    };
};
struct PTREE_MOD_MUX_OutObj_s
{
    PTREE_MOD_OutObj_t      base;
    PTREE_NLINKER_OUT_Obj_t outLinker;
};

struct PTREE_MOD_MUX_Obj_s
{
    PTREE_MOD_Obj_t               base;
    SSOS_THREAD_Mutex_t           mutex;
    enum PTREE_PACKET_VIDEO_Fmt_e inVideoFmt;
    unsigned int                  inPortWidth;
    unsigned int                  inPortHeight;
    unsigned int                  inSampleRate;
    unsigned int                  inChn;
    unsigned int                  duration;
    PTREE_MOD_MUX_Stream_t        videoSt;
    PTREE_MOD_MUX_Stream_t        audioSt;
    AVFormatContext *             oc;
    AVIOContext *                 avioCtx;
    uint8_t *                     avioBuf;
    size_t                        avioBufsize;
};

static int                 _PTREE_MOD_MUX_InSyncLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_MUX_InSyncLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms);
static int                 _PTREE_MOD_MUX_InGetType(PTREE_MOD_InObj_t *modIn);
static PTREE_LINKER_Obj_t *_PTREE_MOD_MUX_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_MUX_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);
static PTREE_LINKER_Obj_t *_PTREE_MOD_MUX_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut);
static int                 _PTREE_MOD_MUX_OutGetType(PTREE_MOD_OutObj_t *modOut);

static int                 _PTREE_MOD_MUX_Init(PTREE_MOD_Obj_t *sysMod);
static int                 _PTREE_MOD_MUX_Deinit(PTREE_MOD_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_MUX_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_MUX_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_MUX_Destruct(PTREE_MOD_Obj_t *sysMod);
static void                _PTREE_MOD_MUX_Free(PTREE_MOD_Obj_t *sysMod);

static void               _PTREE_MOD_MUX_InDestruct(PTREE_MOD_InObj_t *modIn);
static void               _PTREE_MOD_MUX_InFree(PTREE_MOD_InObj_t *modIn);
static PTREE_MOD_InObj_t *_PTREE_MOD_MUX_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static void                _PTREE_MOD_MUX_OutDestruct(PTREE_MOD_OutObj_t *modOut);
static void                _PTREE_MOD_MUX_OutFree(PTREE_MOD_OutObj_t *modOut);
static PTREE_MOD_OutObj_t *_PTREE_MOD_MUX_OutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static const PTREE_LINKER_Ops_t G_PTREE_MOD_MUX_IN_SYNC_LINKER_OPS = {
    .enqueue = _PTREE_MOD_MUX_InSyncLinkerEnqueue,
    .dequeue = _PTREE_MOD_MUX_InSyncLinkerDequeue,
};
static const PTREE_LINKER_Hook_t G_PTREE_MOD_MUX_IN_SYNC_LINKER_HOOK = {};

static const PTREE_NLINKER_OUT_Ops_t G_PTREE_MOD_MUX_OUT_LINKER_OPS = {};

static const PTREE_NLINKER_OUT_Hook_t G_PTREE_MOD_MUX_OUT_LINKER_HOOK = {};

static const PTREE_MOD_InOps_t G_PTREE_MOD_MUX_IN_OPS = {
    .getType       = _PTREE_MOD_MUX_InGetType,
    .createNLinker = _PTREE_MOD_MUX_InCreateNLinker,
    .createPacker  = _PTREE_MOD_MUX_InCreatePacker,
};

static const PTREE_MOD_InHook_t G_PTREE_MOD_MUX_IN_HOOK = {
    .destruct = _PTREE_MOD_MUX_InDestruct,
    .free     = _PTREE_MOD_MUX_InFree,
};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_MUX_OUT_OPS = {
    .getType = _PTREE_MOD_MUX_OutGetType, .createNLinker = _PTREE_MOD_MUX_OutCreateNLinker,
    //.getPacketInfo = _PTREE_MOD_MUX_OutGetPacketInfo,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_MUX_OUT_HOOK = {
    .destruct = _PTREE_MOD_MUX_OutDestruct,
    .free     = _PTREE_MOD_MUX_OutFree,
};

static const PTREE_MOD_Ops_t G_PTREE_MOD_MUX_OPS = {
    .init         = _PTREE_MOD_MUX_Init,
    .deinit       = _PTREE_MOD_MUX_Deinit,
    .createModIn  = _PTREE_MOD_MUX_CreateModIn,
    .createModOut = _PTREE_MOD_MUX_CreateModOut,
};
static const PTREE_MOD_Hook_t G_PTREE_MOD_MUX_HOOK = {
    .destruct = _PTREE_MOD_MUX_Destruct,
    .free     = _PTREE_MOD_MUX_Free,
};

#if 0
static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    if(fmt_ctx == NULL)
    {
        printf("fmt_ctx == NULL \n");
    }

    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s keyframe:%d stream_index:%d\n",
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           ((pkt->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY), pkt->stream_index);
}
#endif

static int _PTREE_MOD_MUX_WritePacket(void *opaque, uint8_t *buf, int bufSize)
{
    PTREE_PACKET_Obj_t *    sendPacket = NULL;
    PTREE_PACKET_TS_Obj_t * tsPacket   = NULL;
    PTREE_PACKET_TS_Info_t  tsInfo;
    PTREE_MOD_Obj_t *       sysMod    = (PTREE_MOD_Obj_t *)opaque;
    PTREE_MOD_MUX_OutObj_t *muxModOut = CONTAINER_OF(*(sysMod->arrModOut), PTREE_MOD_MUX_OutObj_t, base);

    tsInfo.base.type                  = "ts";
    tsInfo.packetInfo.pktInfo[0].size = bufSize;
    tsInfo.packetInfo.pktCount        = 1;
    sendPacket                        = PTREE_PACKER_Make(&muxModOut->base.packer.base, &tsInfo.base);
    if (!sendPacket)
    {
        return SSOS_DEF_FAIL;
    }
    tsPacket = CONTAINER_OF(sendPacket, PTREE_PACKET_TS_Obj_t, base);
    memcpy(tsPacket->packetData.pktData[0].data, buf, tsInfo.packetInfo.pktInfo[0].size);
    PTREE_LINKER_Enqueue(&muxModOut->base.plinker.base, sendPacket);
    PTREE_PACKET_Del(sendPacket);

    return bufSize;
}

/* Add an output stream. */
static void _PTREE_MOD_MUX_AddStream(PTREE_MOD_Obj_t *sysMod, PTREE_MOD_MUX_Stream_t *ost, AVFormatContext *oc,
                                     const char *type)
{
    PTREE_MOD_MUX_Obj_t * muxMod  = CONTAINER_OF(sysMod, PTREE_MOD_MUX_Obj_t, base);
    PTREE_SUR_MUX_Info_t *muxInfo = CONTAINER_OF(sysMod->info, PTREE_SUR_MUX_Info_t, base);
    PTREE_MOD_InObj_t *   modIn   = sysMod->arrModIn[0];

    ost->tmpPkt = av_packet_alloc();
    if (!ost->tmpPkt)
    {
        PTREE_ERR("Could not allocate AVPacket\n");
        exit(1);
    }
    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st)
    {
        PTREE_ERR("Could not allocate stream\n");
        exit(1);
    }
    ost->st->id = oc->nb_streams - 1;

    if (strcmp(type, "video") == 0)
    {
        // case AVMEDIA_TYPE_VIDEO:
        ost->st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
        ost->st->codecpar->codec_id =
            (muxMod->inVideoFmt == E_PTREE_PACKET_VIDEO_STREAM_H264) ? AV_CODEC_ID_H264 : AV_CODEC_ID_HEVC;
        ost->st->codecpar->format   = AV_PIX_FMT_YUV420P;
        ost->st->codecpar->bit_rate = muxInfo->bitRateV;
        // Resolution must be a multiple of two
        ost->st->codecpar->width  = muxMod->inPortWidth;
        ost->st->codecpar->height = muxMod->inPortHeight;
        ost->st->time_base        = (AVRational){1, modIn->info->fps};
    }
    if (strcmp(type, "audio") == 0)
    {
#if 0
        uint8_t asc[]                = {0x14, 0x10, 0x56, 0xe5, 0x0};
        ost->st->codecpar->extradata = av_mallocz(sizeof(asc) + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!ost->st->codecpar->extradata)
        {
            PTREE_ERR("extradata = NULL\n");
            exit(1);
        }
        memcpy(ost->st->codecpar->extradata, asc, sizeof(asc));
        ost->st->codecpar->extradata_size = sizeof(asc);
#endif
        ost->st->codecpar->codec_type     = AVMEDIA_TYPE_AUDIO;
        ost->st->codecpar->codec_id       = AV_CODEC_ID_MP3;
        ost->st->codecpar->sample_rate    = muxMod->inSampleRate;
        ost->st->codecpar->channel_layout = (2 == muxMod->inChn) ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO;
        ost->st->codecpar->channels       = muxMod->inChn;
        ost->st->codecpar->format         = AV_SAMPLE_FMT_S32P;
        ost->st->codecpar->bit_rate       = muxInfo->bitRateA;
        ost->st->time_base                = (AVRational){1, muxMod->inSampleRate};
        ost->st->codecpar->frame_size     = muxInfo->audioFrameSize;
    }
}

static int _PTREE_MOD_MUX_WriteFrame(AVFormatContext *oc, PTREE_PACKET_Obj_t *packet, PTREE_MOD_MUX_Obj_t *muxMod)
{
    unsigned int            i = 0;
    int                     ret;
    int64_t                 timeStamp = 0;
    PTREE_MOD_MUX_Stream_t *ost       = NULL;

    if (PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(video)))
    {
        ost                                         = &muxMod->videoSt;
        PTREE_PACKET_VIDEO_Obj_t *videoPacket       = NULL;
        videoPacket                                 = CONTAINER_OF(packet, PTREE_PACKET_VIDEO_Obj_t, base);
        const PTREE_PACKET_VIDEO_PacketInfo_t *info = &videoPacket->info.packetInfo;
        const PTREE_PACKET_VIDEO_PacketData_t *data = &videoPacket->packetData;

        ost->tmpPkt->duration     = muxMod->duration;
        ost->tmpPkt->stream_index = ost->st->index;

        timeStamp        = videoPacket->base.stamp.tvSec * 1000 * 1000 + videoPacket->base.stamp.tvUSec;
        int64_t pts      = av_rescale_q(timeStamp, (AVRational){1, 1000000}, ost->st->time_base);
        ost->tmpPkt->pts = pts;
        ost->tmpPkt->dts = pts;

        if (ost->tmpPkt->buf)
        {
            av_buffer_unref(&ost->tmpPkt->buf);
        }

        for (i = 0; i < info->pktCount; ++i)
        {
            ost->tmpPkt->size += info->pktInfo[i].size;
        }
        ost->tmpPkt->data = malloc(ost->tmpPkt->size);
        if (ost->tmpPkt->data == NULL)
        {
            PTREE_ERR("realloc failed\n");
            exit(1);
        }
        ost->tmpPkt->buf = av_buffer_create(ost->tmpPkt->data, ost->tmpPkt->size, NULL, NULL, 0);
        if (ost->tmpPkt->buf == NULL)
        {
            PTREE_ERR("tmpPkt->buf == NULL\n");
            exit(1);
        }
        for (i = 0; i < info->pktCount; ++i)
        {
            memcpy(ost->tmpPkt->data, data->pktData[i].data, info->pktInfo[i].size);
        }
    }

    if (PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        ost                                         = &muxMod->audioSt;
        PTREE_PACKET_AUDIO_Obj_t *audioPacket       = NULL;
        audioPacket                                 = CONTAINER_OF(packet, PTREE_PACKET_AUDIO_Obj_t, base);
        const PTREE_PACKET_AUDIO_PacketInfo_t *info = &audioPacket->info.packetInfo;
        const PTREE_PACKET_AUDIO_PacketData_t *data = &audioPacket->packetData;

        timeStamp   = audioPacket->base.stamp.tvSec * 1000 * 1000 + audioPacket->base.stamp.tvUSec;
        int64_t pts = av_rescale_q(timeStamp, (AVRational){1, 1000000}, ost->st->time_base);

        ost->tmpPkt->duration     = muxMod->duration;
        ost->tmpPkt->stream_index = ost->st->index;
        ost->tmpPkt->pts          = pts;
        ost->tmpPkt->dts          = pts;

        if (ost->tmpPkt->buf)
        {
            av_buffer_unref(&ost->tmpPkt->buf);
        }

        for (i = 0; i < info->pktCount; ++i)
        {
            ost->tmpPkt->size += info->pktInfo[i].size;
        }
        ost->tmpPkt->data = malloc(ost->tmpPkt->size);
        if (ost->tmpPkt->data == NULL)
        {
            perror("realloc failed");
            exit(1);
        }
        ost->tmpPkt->buf = av_buffer_create(ost->tmpPkt->data, ost->tmpPkt->size, NULL, NULL, 0);
        for (i = 0; i < info->pktCount; ++i)
        {
            memcpy(ost->tmpPkt->data, data->pktData[i].data, info->pktInfo[i].size);
        }
    }

    /* Write the compressed frame to the media file. */
    // log_packet(oc, ost->tmpPkt);
    SSOS_THREAD_MutexLock(&muxMod->mutex);
    ret = av_interleaved_write_frame(oc, ost->tmpPkt);
    // ret = av_write_frame(oc, ost->tmpPkt);
    SSOS_THREAD_MutexUnlock(&muxMod->mutex);
    if (ret < 0)
    {
        PTREE_ERR("Error while writing output packet\n");
        exit(1);
    }

    return ret == AVERROR_EOF ? 1 : 0;
}

static int _PTREE_MOD_MUX_InSyncLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_MUX_InObj_t *muxModIn = CONTAINER_OF(linker, PTREE_MOD_MUX_InObj_t, syncLinker);
    PTREE_MOD_MUX_Obj_t *  muxMod   = CONTAINER_OF(muxModIn->base.thisMod, PTREE_MOD_MUX_Obj_t, base);
    AVFormatContext *      oc       = muxMod->oc;

    if (!PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(video))
        && !PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        PTREE_ERR("packet info type %s is not support", packet->info->type);
        return SSOS_DEF_FAIL;
    }
    _PTREE_MOD_MUX_WriteFrame(oc, packet, muxMod);

    return SSOS_DEF_OK;
}

static PTREE_PACKET_Obj_t *_PTREE_MOD_MUX_InSyncLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms)
{
    (void)linker;
    (void)ms;
    return NULL;
}

static int _PTREE_MOD_MUX_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}

static PTREE_LINKER_Obj_t *_PTREE_MOD_MUX_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_MUX_InObj_t *muxModIn = CONTAINER_OF(modIn, PTREE_MOD_MUX_InObj_t, base);

    return PTREE_LINKER_Dup(&muxModIn->syncLinker);
}

static PTREE_PACKER_Obj_t *_PTREE_MOD_MUX_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    // PTREE_MOD_MUX_Obj_t *muxMod = CONTAINER_OF(modIn->thisMod, PTREE_MOD_MUX_Obj_t, base.base);
    (void)modIn;
    *isFast = SSOS_DEF_FALSE;
    return PTREE_PACKER_NormalNew();
}

static int _PTREE_MOD_MUX_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}

static PTREE_LINKER_Obj_t *_PTREE_MOD_MUX_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_MUX_OutObj_t *muxModOut = CONTAINER_OF(modOut, PTREE_MOD_MUX_OutObj_t, base);
    return PTREE_LINKER_Dup(&muxModOut->outLinker.base);
}

static int _PTREE_MOD_MUX_Init(PTREE_MOD_Obj_t *sysMod)
{
    PTREE_MOD_MUX_Obj_t * muxMod  = CONTAINER_OF(sysMod, PTREE_MOD_MUX_Obj_t, base);
    PTREE_SUR_MUX_Info_t *muxInfo = CONTAINER_OF(sysMod->info, PTREE_SUR_MUX_Info_t, base);
    const AVOutputFormat *fmt     = NULL;
    int                   ret;
    int                   i               = 0;
    AVDictionary *        opt             = NULL;
    const char *          filename        = muxInfo->fileName;
    muxMod->avioBufsize                   = 0xfffff;
    PTREE_MOD_MUX_Stream_t *   video      = &muxMod->videoSt;
    PTREE_MOD_MUX_Stream_t *   audio      = &muxMod->audioSt;
    PTREE_PACKET_Info_t *      packetInfo = NULL;
    PTREE_PACKET_VIDEO_Info_t *videoInfo  = NULL;
    PTREE_PACKET_AUDIO_Info_t *audioInfo  = NULL;
    PTREE_MOD_InObj_t *        modIn      = NULL;

    for (i = 0; i < 2; i++)
    {
        modIn      = sysMod->arrModIn[i];
        packetInfo = PTREE_MESSAGE_GetPacketInfo(&modIn->message);
        if (!packetInfo)
        {
            PTREE_ERR("Packet info is null");
            return SSOS_DEF_FAIL;
        }
        if (PTREE_PACKET_InfoLikely(packetInfo, PTREE_PACKET_INFO_TYPE(video)))
        {
            videoInfo            = CONTAINER_OF(packetInfo, PTREE_PACKET_VIDEO_Info_t, base);
            muxMod->inVideoFmt   = videoInfo->packetInfo.fmt;
            muxMod->inPortWidth  = videoInfo->packetInfo.width;
            muxMod->inPortHeight = videoInfo->packetInfo.height;
            videoInfo            = NULL;
        }
        if (PTREE_PACKET_InfoLikely(packetInfo, PTREE_PACKET_INFO_TYPE(audio)))
        {
            audioInfo            = CONTAINER_OF(packetInfo, PTREE_PACKET_AUDIO_Info_t, base);
            muxMod->inSampleRate = audioInfo->packetInfo.sampleRate;
            muxMod->inChn        = audioInfo->packetInfo.channels;
            audioInfo            = NULL;
        }

        PTREE_PACKET_InfoDel(packetInfo);
        packetInfo = NULL;
    }

    if (E_PTREE_SUR_MUX_TYPE_TS == muxInfo->format)
    {
        avformat_alloc_output_context2(&muxMod->oc, NULL, "mpegts", NULL);
        if (!muxMod->oc)
        {
            PTREE_ERR("Could not deduce output format from file extension\n");
            return SSOS_DEF_FAIL;
        }

        muxMod->avioBuf = (unsigned char *)av_malloc(muxMod->avioBufsize);
        if (!muxMod->avioBuf)
        {
            PTREE_ERR("Could not allocate buffer\n");
            return SSOS_DEF_FAIL;
        }
        muxMod->avioCtx =
            avio_alloc_context(muxMod->avioBuf, muxMod->avioBufsize, 1, sysMod, NULL, _PTREE_MOD_MUX_WritePacket, NULL);
        if (!muxMod->avioCtx)
        {
            av_free(muxMod->avioBuf);
            PTREE_ERR("Could not allocate AVIOContext\n");
            return SSOS_DEF_FAIL;
        }
        muxMod->oc->pb   = muxMod->avioCtx;
        muxMod->duration = 3240;
    }
    else
    {
        avformat_alloc_output_context2(&muxMod->oc, NULL, NULL, filename);
        if (!muxMod->oc)
        {
            PTREE_ERR("Could not deduce output format from file extension\n");
            return SSOS_DEF_FAIL;
        }
        muxMod->duration = 1024;
    }
    _PTREE_MOD_MUX_AddStream(sysMod, video, muxMod->oc, "video");
    _PTREE_MOD_MUX_AddStream(sysMod, audio, muxMod->oc, "audio");

    // mov auto flush configuration
    av_dict_set(&opt, "movflags", "flush_moov", 0);
    av_dict_set_int(&opt, "moov_size", CALCULATE_RESERVED_MOOV_SIZE(modIn->info->fps, STREAM_DURATION), 0);
    av_dict_set_int(&opt, "flush_moov_duration", 1000, 0);

    if (E_PTREE_SUR_MUX_TYPE_TS != muxInfo->format)
    {
        av_dump_format(muxMod->oc, 0, filename, 1);
        fmt = muxMod->oc->oformat;
        /* open the output file, if needed */
        if (!(fmt->flags & AVFMT_NOFILE))
        {
            ret = avio_open(&muxMod->oc->pb, filename, AVIO_FLAG_WRITE);
            if (ret < 0)
            {
                PTREE_ERR("Could not open '%s'\n", filename);
                return SSOS_DEF_FAIL;
            }
        }
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(muxMod->oc, &opt);
    if (ret < 0)
    {
        PTREE_ERR("Error occurred when opening output file\n");
        return SSOS_DEF_FAIL;
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_MUX_Deinit(PTREE_MOD_Obj_t *sysMod)
{
    PTREE_MOD_MUX_Obj_t * muxMod  = CONTAINER_OF(sysMod, PTREE_MOD_MUX_Obj_t, base);
    PTREE_SUR_MUX_Info_t *muxInfo = CONTAINER_OF(sysMod->info, PTREE_SUR_MUX_Info_t, base);
    const AVOutputFormat *fmt     = muxMod->oc->oformat;

    av_write_trailer(muxMod->oc);
    av_packet_free(&muxMod->videoSt.tmpPkt);
    av_packet_free(&muxMod->audioSt.tmpPkt);

    if (E_PTREE_SUR_MUX_TYPE_TS == muxInfo->format)
    {
        av_free(muxMod->avioCtx->buffer);
        av_free(muxMod->avioCtx);
    }
    else
    {
        if (!(fmt->flags & AVFMT_NOFILE))
        {
            /* Close the output file. */
            avio_closep(&muxMod->oc->pb);
        }
    }
    avformat_free_context(muxMod->oc);

    return SSOS_DEF_OK;
}

static PTREE_MOD_InObj_t *_PTREE_MOD_MUX_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    // PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    //(void)loopId;
    return _PTREE_MOD_MUX_InNew(mod, loopId);
}

static PTREE_MOD_OutObj_t *_PTREE_MOD_MUX_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    return _PTREE_MOD_MUX_OutNew(mod, loopId);
}

static void _PTREE_MOD_MUX_Destruct(PTREE_MOD_Obj_t *sysMod)
{
    (void)sysMod;
    PTREE_MOD_MUX_Obj_t *muxMod = CONTAINER_OF(sysMod, PTREE_MOD_MUX_Obj_t, base);
    SSOS_THREAD_MutexDeinit(&muxMod->mutex);
}

static void _PTREE_MOD_MUX_Free(PTREE_MOD_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_MUX_Obj_t, base));
}

static void _PTREE_MOD_MUX_InDestruct(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_MUX_InObj_t *muxModIn = CONTAINER_OF(modIn, PTREE_MOD_MUX_InObj_t, base);

    PTREE_LINKER_Del(&muxModIn->syncLinker);

    PTREE_MOD_InObjDel(&muxModIn->base);
}

static void _PTREE_MOD_MUX_InFree(PTREE_MOD_InObj_t *modIn)
{
    SSOS_MEM_Free(CONTAINER_OF(modIn, PTREE_MOD_MUX_InObj_t, base));
}
static PTREE_MOD_InObj_t *_PTREE_MOD_MUX_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_MUX_InObj_t *muxModIn = NULL;

    muxModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_MUX_InObj_t));
    if (!muxModIn)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(muxModIn, 0, sizeof(PTREE_MOD_MUX_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(&muxModIn->base, &G_PTREE_MOD_MUX_IN_OPS, mod, loopId))
    {
        goto ERR_MOD_IN_INIT;
    }
    if (SSOS_DEF_OK != PTREE_LINKER_Init(&muxModIn->syncLinker, &G_PTREE_MOD_MUX_IN_SYNC_LINKER_OPS))
    {
        goto ERR_LINKER_INIT;
    }
    PTREE_LINKER_Register(&muxModIn->syncLinker, &G_PTREE_MOD_MUX_IN_SYNC_LINKER_HOOK);

    PTREE_MOD_InObjRegister(&muxModIn->base, &G_PTREE_MOD_MUX_IN_HOOK);
    return &muxModIn->base;

ERR_LINKER_INIT:
    PTREE_MOD_InObjDel(&muxModIn->base);
ERR_MOD_IN_INIT:
    SSOS_MEM_Free(muxModIn);
ERR_MEM_ALLOC:
    return NULL;
}

static void _PTREE_MOD_MUX_OutDestruct(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_MUX_OutObj_t *muxModOut = CONTAINER_OF(modOut, PTREE_MOD_MUX_OutObj_t, base);

    PTREE_LINKER_Del(&muxModOut->outLinker.base);
    PTREE_MOD_OutObjDel(&muxModOut->base);
}

static void _PTREE_MOD_MUX_OutFree(PTREE_MOD_OutObj_t *modOut)
{
    SSOS_MEM_Free(CONTAINER_OF(modOut, PTREE_MOD_MUX_OutObj_t, base));
}

static PTREE_MOD_OutObj_t *_PTREE_MOD_MUX_OutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_MUX_OutObj_t *muxModOut = NULL;

    muxModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_MUX_OutObj_t));
    if (!muxModOut)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(muxModOut, 0, sizeof(PTREE_MOD_MUX_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&muxModOut->base, &G_PTREE_MOD_MUX_OUT_OPS, mod, loopId))
    {
        goto ERR_MOD_OUT_INIT;
    }

    if (SSOS_DEF_OK != PTREE_NLINKER_OUT_Init(&muxModOut->outLinker, &G_PTREE_MOD_MUX_OUT_LINKER_OPS))
    {
        goto ERR_OUT_LINKER_INIT;
    }

    PTREE_MOD_OutObjRegister(&muxModOut->base, &G_PTREE_MOD_MUX_OUT_HOOK);
    PTREE_NLINKER_OUT_Register(&muxModOut->outLinker, &G_PTREE_MOD_MUX_OUT_LINKER_HOOK);

    return &muxModOut->base;

ERR_OUT_LINKER_INIT:
    PTREE_MOD_OutObjDel(&muxModOut->base);
ERR_MOD_OUT_INIT:
    SSOS_MEM_Free(muxModOut);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MOD_Obj_t *PTREE_MOD_MUX_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_MUX_Obj_t *muxMod = NULL;

    muxMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_MUX_Obj_t));
    if (!muxMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(muxMod, 0, sizeof(PTREE_MOD_MUX_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_ObjInit(&muxMod->base, &G_PTREE_MOD_MUX_OPS, tag))
    {
        goto ERR_MOD_SYS_INIT;
    }

    SSOS_THREAD_MutexInit(&muxMod->mutex);

    PTREE_MOD_ObjRegister(&muxMod->base, &G_PTREE_MOD_MUX_HOOK);
    return &muxMod->base;

ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(muxMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(MUX, PTREE_MOD_MUX_New);
