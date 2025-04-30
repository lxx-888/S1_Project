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
#include "ptree_mod_mp3en.h"
#include "ptree_sur_mp3en.h"
#include "ptree_sur_sys.h"
#include "ptree_maker.h"
#include "ptree_packer.h"
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

typedef struct PTREE_MOD_MP3EN_OutputStream_s
{
    AVCodecContext *enc;

    int64_t nextPts;
    int     samplesCount;

    AVFrame *frame;
    AVFrame *tmpFrame;

    AVPacket *tmpPkt;

    float t, tincr, tincr2;

    struct SwrContext *swrCtx;
} PTREE_MOD_MP3EN_Stream_t;

typedef struct PTREE_MOD_MP3EN_Obj_s    PTREE_MOD_MP3EN_Obj_t;
typedef struct PTREE_MOD_MP3EN_InObj_s  PTREE_MOD_MP3EN_InObj_t;
typedef struct PTREE_MOD_MP3EN_OutObj_s PTREE_MOD_MP3EN_OutObj_t;

struct PTREE_MOD_MP3EN_InObj_s
{
    PTREE_MOD_InObj_t  base;
    PTREE_LINKER_Obj_t syncLinker;
};
struct PTREE_MOD_MP3EN_OutObj_s
{
    PTREE_MOD_OutObj_t      base;
    PTREE_NLINKER_OUT_Obj_t outLinker;
};

struct PTREE_MOD_MP3EN_Obj_s
{
    PTREE_MOD_Obj_t           base;
    PTREE_PACKET_AUDIO_Info_t audioInfo;
    PTREE_MOD_MP3EN_Stream_t  audioStream;
};

static int                 _PTREE_MOD_MP3EN_InSyncLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_MP3EN_InSyncLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms);
static int                 _PTREE_MOD_MP3EN_InGetType(PTREE_MOD_InObj_t *modIn);
static PTREE_LINKER_Obj_t *_PTREE_MOD_MP3EN_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_MP3EN_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);
static PTREE_LINKER_Obj_t *_PTREE_MOD_MP3EN_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut);
static PTREE_PACKET_Info_t *_PTREE_MOD_MP3EN_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut);

static int _PTREE_MOD_MP3EN_OutGetType(PTREE_MOD_OutObj_t *modOut);

static int                 _PTREE_MOD_MP3EN_Init(PTREE_MOD_Obj_t *sysMod);
static int                 _PTREE_MOD_MP3EN_Deinit(PTREE_MOD_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_MP3EN_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_MP3EN_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_MP3EN_Free(PTREE_MOD_Obj_t *sysMod);

static void               _PTREE_MOD_MP3EN_InDestruct(PTREE_MOD_InObj_t *modIn);
static void               _PTREE_MOD_MP3EN_InFree(PTREE_MOD_InObj_t *modIn);
static PTREE_MOD_InObj_t *_PTREE_MOD_MP3EN_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static void                _PTREE_MOD_MP3EN_OutDestruct(PTREE_MOD_OutObj_t *modOut);
static void                _PTREE_MOD_MP3EN_OutFree(PTREE_MOD_OutObj_t *modOut);
static PTREE_MOD_OutObj_t *_PTREE_MOD_MP3EN_OutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static const PTREE_LINKER_Ops_t G_PTREE_MOD_MP3EN_IN_SYNC_LINKER_OPS = {
    .enqueue = _PTREE_MOD_MP3EN_InSyncLinkerEnqueue,
    .dequeue = _PTREE_MOD_MP3EN_InSyncLinkerDequeue,
};
static const PTREE_LINKER_Hook_t G_PTREE_MOD_MP3EN_IN_SYNC_LINKER_HOOK = {};

static const PTREE_NLINKER_OUT_Ops_t G_PTREE_MOD_MP3EN_OUT_LINKER_OPS = {};

static const PTREE_NLINKER_OUT_Hook_t G_PTREE_MOD_MP3EN_OUT_LINKER_HOOK = {};

static const PTREE_MOD_InOps_t G_PTREE_MOD_MP3EN_IN_OPS = {
    .getType       = _PTREE_MOD_MP3EN_InGetType,
    .createNLinker = _PTREE_MOD_MP3EN_InCreateNLinker,
    .createPacker  = _PTREE_MOD_MP3EN_InCreatePacker,
};

static const PTREE_MOD_InHook_t G_PTREE_MOD_MP3EN_IN_HOOK = {
    .destruct = _PTREE_MOD_MP3EN_InDestruct,
    .free     = _PTREE_MOD_MP3EN_InFree,
};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_MP3EN_OUT_OPS = {
    .getType       = _PTREE_MOD_MP3EN_OutGetType,
    .createNLinker = _PTREE_MOD_MP3EN_OutCreateNLinker,
    .getPacketInfo = _PTREE_MOD_MP3EN_OutGetPacketInfo,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_MP3EN_OUT_HOOK = {
    .destruct = _PTREE_MOD_MP3EN_OutDestruct,
    .free     = _PTREE_MOD_MP3EN_OutFree,
};

static const PTREE_MOD_Ops_t G_PTREE_MOD_MP3EN_OPS = {
    .init         = _PTREE_MOD_MP3EN_Init,
    .deinit       = _PTREE_MOD_MP3EN_Deinit,
    .createModIn  = _PTREE_MOD_MP3EN_CreateModIn,
    .createModOut = _PTREE_MOD_MP3EN_CreateModOut,
};
static const PTREE_MOD_Hook_t G_PTREE_MOD_MP3EN_HOOK = {
    //.destruct = _PTREE_MOD_MP3EN_Destruct,
    .free = _PTREE_MOD_MP3EN_Free,
};

static void _PTREE_MOD_MP3EN_ConvertPcm(const PTREE_PACKET_AUDIO_PacketInfo_t *info,
                                        const PTREE_PACKET_AUDIO_PacketData_t *data, AVFrame *frame)
{
    int          nbSamples = frame->nb_samples; // 1024 or 576
    int          channels  = frame->channels;
    unsigned int i         = 0;

    int16_t *interleavedData = (int16_t *)av_malloc(sizeof(int16_t) * nbSamples * channels);
    for (i = 0; i < info->pktCount; i++)
    {
        memcpy(interleavedData, data->pktData[i].data, info->pktInfo[i].size);
    }

    for (int i = 0; i < nbSamples; i++)
    {
        for (int j = 0; j < channels; j++)
        {
            // float convertedSample          = interleavedData[i * channels + j] / 32768.0F; //AAC need
            //((float *)(frame->data[j]))[i] = convertedSample;
            int32_t convertedSample          = (int32_t)interleavedData[i * channels + j] << 16; // MP3 need
            ((int32_t *)(frame->data[j]))[i] = convertedSample;
        }
    }

    av_free(interleavedData);
}

static AVFrame *_PTREE_MOD_MP3EN_AllocFrame(enum AVSampleFormat sampleFmt, uint64_t channelLayout, int sampleRate,
                                            int nbSamples)
{
    AVFrame *frame = av_frame_alloc();
    int      ret;

    if (!frame)
    {
        PTREE_ERR("Error allocating an audio frame\n");
        exit(1);
    }

    frame->format         = sampleFmt;
    frame->channel_layout = channelLayout;
    frame->sample_rate    = sampleRate;
    frame->nb_samples     = nbSamples;

    if (nbSamples)
    {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0)
        {
            PTREE_ERR("Error allocating an audio buffer\n");
            exit(1);
        }
    }

    return frame;
}

static int _PTREE_MOD_MP3EN_InSyncLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_MP3EN_InObj_t * mp3enModIn  = CONTAINER_OF(linker, PTREE_MOD_MP3EN_InObj_t, syncLinker);
    PTREE_MOD_MP3EN_Obj_t *   mp3enMod    = CONTAINER_OF(mp3enModIn->base.thisMod, PTREE_MOD_MP3EN_Obj_t, base);
    PTREE_MOD_MP3EN_OutObj_t *mp3enModOut = CONTAINER_OF(*(mp3enMod->base.arrModOut), PTREE_MOD_MP3EN_OutObj_t, base);
    PTREE_PACKET_Obj_t *      sendPacket  = NULL;
    PTREE_PACKET_AUDIO_Obj_t *audioPacket = NULL;
    AVCodecContext *          c;
    AVFrame *                 frame;
    int                       ret;
    int                       dstNbSamples;
    PTREE_MOD_MP3EN_Stream_t *ost = &mp3enMod->audioStream;

    if (!PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        PTREE_ERR("packet info type %s is not support", packet->info->type);
        return SSOS_DEF_FAIL;
    }
    audioPacket                           = CONTAINER_OF(packet, PTREE_PACKET_AUDIO_Obj_t, base);
    PTREE_PACKET_AUDIO_PacketInfo_t *info = &audioPacket->info.packetInfo;
    PTREE_PACKET_AUDIO_PacketData_t *data = &audioPacket->packetData;

    c          = ost->enc;
    frame      = ost->tmpFrame;
    frame->pts = ost->nextPts;
    ost->nextPts += frame->nb_samples;

    if (frame)
    {
        /* convert samples from native format to destination codec format, using the resampler */
        /* compute destination number of samples */
        dstNbSamples = av_rescale_rnd(swr_get_delay(ost->swrCtx, c->sample_rate) + frame->nb_samples, c->sample_rate,
                                      c->sample_rate, AV_ROUND_UP);
        av_assert0(dstNbSamples == frame->nb_samples);

        /* when we pass a frame to the encoder, it may keep a reference to it
         * internally;
         * make sure we do not overwrite it here
         */
        ret = av_frame_make_writable(ost->frame);
        if (ret < 0)
        {
            exit(1);
        }

        /* convert to destination format */
        ret =
            swr_convert(ost->swrCtx, ost->frame->data, dstNbSamples, (const uint8_t **)frame->data, frame->nb_samples);
        if (ret < 0)
        {
            PTREE_ERR("Error while converting\n");
            exit(1);
        }
        frame = ost->frame;

        frame->pts = av_rescale_q(ost->samplesCount, (AVRational){1, c->sample_rate}, c->time_base);
        ost->samplesCount += dstNbSamples;
    }

    if (!frame || !frame->data[0])
    {
        PTREE_ERR("Invalid AVFrame data\n");
        exit(1);
    }

    _PTREE_MOD_MP3EN_ConvertPcm(info, data, frame);

    ret = avcodec_send_frame(c, frame);
    if (ret < 0)
    {
        PTREE_ERR("Error sending a frame to the encoder\n");
        exit(1);
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(c, ost->tmpPkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        if (ret < 0)
        {
            PTREE_ERR("Error encoding a frame\n");
            exit(1);
        }

        mp3enMod->audioInfo.base.type                  = packet->info->type;
        mp3enMod->audioInfo.packetInfo.pktInfo[0].size = ost->tmpPkt->size;
        mp3enMod->audioInfo.packetInfo.pktCount        = 1;
        sendPacket = PTREE_PACKER_Make(&mp3enModOut->base.packer.base, &mp3enMod->audioInfo.base);
        if (!sendPacket)
        {
            return SSOS_DEF_FAIL;
        }
        sendPacket->stamp.tvSec  = packet->stamp.tvSec;
        sendPacket->stamp.tvUSec = packet->stamp.tvUSec;
        audioPacket              = CONTAINER_OF(sendPacket, PTREE_PACKET_AUDIO_Obj_t, base);
        memcpy(audioPacket->packetData.pktData[0].data, ost->tmpPkt->data, ost->tmpPkt->size);
        PTREE_LINKER_Enqueue(&mp3enModOut->base.plinker.base, sendPacket);
        PTREE_PACKET_Del(sendPacket);
    }

    return SSOS_DEF_OK;
}

static PTREE_PACKET_Obj_t *_PTREE_MOD_MP3EN_InSyncLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms)
{
    (void)linker;
    (void)ms;
    return NULL;
}

static int _PTREE_MOD_MP3EN_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}

static PTREE_LINKER_Obj_t *_PTREE_MOD_MP3EN_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_MP3EN_InObj_t *mp3enModIn = CONTAINER_OF(modIn, PTREE_MOD_MP3EN_InObj_t, base);

    return PTREE_LINKER_Dup(&mp3enModIn->syncLinker);
}

static PTREE_PACKER_Obj_t *_PTREE_MOD_MP3EN_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    // PTREE_MOD_MP3EN_Obj_t *mp3enMod = CONTAINER_OF(modIn->thisMod, PTREE_MOD_MP3EN_Obj_t, base.base);
    (void)modIn;
    *isFast = SSOS_DEF_FALSE;
    return PTREE_PACKER_NormalNew();
}

static int _PTREE_MOD_MP3EN_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}

static PTREE_LINKER_Obj_t *_PTREE_MOD_MP3EN_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_MP3EN_OutObj_t *mp3enModOut = CONTAINER_OF(modOut, PTREE_MOD_MP3EN_OutObj_t, base);
    return PTREE_LINKER_Dup(&mp3enModOut->outLinker.base);
}

static PTREE_PACKET_Info_t *_PTREE_MOD_MP3EN_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_MP3EN_Obj_t *mp3enMod = CONTAINER_OF(modOut->thisMod, PTREE_MOD_MP3EN_Obj_t, base);

    return PTREE_PACKET_AUDIO_InfoNew(&mp3enMod->audioInfo.packetInfo);
}

static int _PTREE_MOD_MP3EN_Init(PTREE_MOD_Obj_t *sysMod)
{
    PTREE_MOD_MP3EN_Obj_t *    mp3enMod   = CONTAINER_OF(sysMod, PTREE_MOD_MP3EN_Obj_t, base);
    PTREE_SUR_MP3EN_Info_t *   mp3enInfo  = CONTAINER_OF(sysMod->info, PTREE_SUR_MP3EN_Info_t, base);
    PTREE_MOD_MP3EN_Stream_t * ost        = &mp3enMod->audioStream;
    AVCodecContext *           c          = NULL;
    int                        i          = 0;
    const AVCodec *            audioCodec = NULL;
    enum AVCodecID             codecId    = AV_CODEC_ID_MP3;
    AVDictionary *             opt        = NULL;
    int                        ret;
    int                        nbSamples;
    PTREE_PACKET_Info_t *      packetInfo    = NULL;
    PTREE_PACKET_AUDIO_Info_t *audioInfo     = NULL;
    PTREE_MOD_InObj_t *        modIn         = NULL;
    uint64_t                   channelLayout = AV_CH_LAYOUT_STEREO;

    modIn      = sysMod->arrModIn[0];
    packetInfo = PTREE_MESSAGE_GetPacketInfo(&modIn->message);
    if (!packetInfo)
    {
        PTREE_ERR("Packet info is null");
        return SSOS_DEF_FAIL;
    }
    if (PTREE_PACKET_InfoLikely(packetInfo, PTREE_PACKET_INFO_TYPE(audio)))
    {
        audioInfo = CONTAINER_OF(packetInfo, PTREE_PACKET_AUDIO_Info_t, base);
        memcpy(&mp3enMod->audioInfo, audioInfo, sizeof(PTREE_PACKET_AUDIO_Info_t));
        channelLayout = (2 == mp3enMod->audioInfo.packetInfo.channels) ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO;
        audioInfo     = NULL;
    }
    PTREE_PACKET_InfoDel(packetInfo);
    packetInfo = NULL;

    /* find the encoder */
    audioCodec = avcodec_find_encoder(codecId);
    if (!audioCodec)
    {
        PTREE_ERR("not find codec: %s\n", avcodec_get_name(codecId));
    }
    ost->tmpPkt = av_packet_alloc();
    if (!ost->tmpPkt)
    {
        PTREE_ERR("Could not allocate AVPacket\n");
        exit(1);
    }
    c = avcodec_alloc_context3(audioCodec);
    if (!c)
    {
        PTREE_ERR("Could not alloc an encoding context\n");
        exit(1);
    }
    ost->enc       = c;
    c->sample_fmt  = audioCodec->sample_fmts ? audioCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    c->bit_rate    = mp3enInfo->bitRate;
    c->sample_rate = mp3enMod->audioInfo.packetInfo.sampleRate;
    if (audioCodec->supported_samplerates)
    {
        c->sample_rate = audioCodec->supported_samplerates[0];
        for (i = 0; audioCodec->supported_samplerates[i]; i++)
        {
            if (audioCodec->supported_samplerates[i] == mp3enMod->audioInfo.packetInfo.sampleRate)
            {
                c->sample_rate = mp3enMod->audioInfo.packetInfo.sampleRate;
            }
        }
    }
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
    c->channel_layout = channelLayout;
    if (audioCodec->channel_layouts)
    {
        c->channel_layout = audioCodec->channel_layouts[0];
        for (i = 0; audioCodec->channel_layouts[i]; i++)
        {
            if (audioCodec->channel_layouts[i] == channelLayout)
            {
                c->channel_layout = channelLayout;
            }
        }
    }
    c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
    // ost->st->time_base = (AVRational){1, c->sample_rate};

    /* Some formats want stream headers to be separate. */
    // if (oc->oformat->flags & AVFMT_GLOBALHEADER)
    c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // mov auto flush configuration
    av_dict_set(&opt, "movflags", "flush_moov", 0);
    av_dict_set_int(&opt, "moov_size", CALCULATE_RESERVED_MOOV_SIZE(modIn->info->fps, STREAM_DURATION), 0);
    av_dict_set_int(&opt, "flush_moov_duration", 1000, 0);

    /* open it */
    ret = avcodec_open2(c, audioCodec, &opt);
    av_dict_free(&opt);
    if (ret < 0)
    {
        PTREE_ERR("Could not open audio codec\n");
        exit(1);
    }

    /* init signal generator */
    ost->t     = 0;
    ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
    {
        nbSamples = 10000;
    }
    else
    {
        nbSamples = c->frame_size;
    }
    ost->frame    = _PTREE_MOD_MP3EN_AllocFrame(c->sample_fmt, c->channel_layout, c->sample_rate, nbSamples);
    ost->tmpFrame = _PTREE_MOD_MP3EN_AllocFrame(AV_SAMPLE_FMT_S16, c->channel_layout, c->sample_rate, nbSamples);

    /* create resampler context */
    ost->swrCtx = swr_alloc();
    if (!ost->swrCtx)
    {
        PTREE_ERR("Could not allocate resampler context\n");
        exit(1);
    }

    /* set options */
    av_opt_set_int(ost->swrCtx, "in_channel_count", c->channels, 0);
    av_opt_set_int(ost->swrCtx, "in_sample_rate", c->sample_rate, 0);
    av_opt_set_sample_fmt(ost->swrCtx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int(ost->swrCtx, "out_channel_count", c->channels, 0);
    av_opt_set_int(ost->swrCtx, "out_sample_rate", c->sample_rate, 0);
    av_opt_set_sample_fmt(ost->swrCtx, "out_sample_fmt", c->sample_fmt, 0);

    /* initialize the resampling context */
    if ((ret = swr_init(ost->swrCtx)) < 0)
    {
        PTREE_ERR("Failed to initialize the resampling context\n");
        exit(1);
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_MP3EN_Deinit(PTREE_MOD_Obj_t *sysMod)
{
    PTREE_MOD_MP3EN_Obj_t *   mp3enMod = CONTAINER_OF(sysMod, PTREE_MOD_MP3EN_Obj_t, base);
    PTREE_MOD_MP3EN_Stream_t *ost      = &mp3enMod->audioStream;

    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmpFrame);
    av_packet_free(&ost->tmpPkt);
    swr_free(&ost->swrCtx);

    return SSOS_DEF_OK;
}

static PTREE_MOD_InObj_t *_PTREE_MOD_MP3EN_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    return _PTREE_MOD_MP3EN_InNew(mod, loopId);
}

static PTREE_MOD_OutObj_t *_PTREE_MOD_MP3EN_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    return _PTREE_MOD_MP3EN_OutNew(mod, loopId);
}

static void _PTREE_MOD_MP3EN_Free(PTREE_MOD_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_MP3EN_Obj_t, base));
}

static void _PTREE_MOD_MP3EN_InDestruct(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_MP3EN_InObj_t *mp3enModIn = CONTAINER_OF(modIn, PTREE_MOD_MP3EN_InObj_t, base);

    PTREE_LINKER_Del(&mp3enModIn->syncLinker);

    PTREE_MOD_InObjDel(&mp3enModIn->base);
}

static void _PTREE_MOD_MP3EN_InFree(PTREE_MOD_InObj_t *modIn)
{
    SSOS_MEM_Free(CONTAINER_OF(modIn, PTREE_MOD_MP3EN_InObj_t, base));
}
static PTREE_MOD_InObj_t *_PTREE_MOD_MP3EN_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_MP3EN_InObj_t *mp3enModIn = NULL;

    mp3enModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_MP3EN_InObj_t));
    if (!mp3enModIn)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(mp3enModIn, 0, sizeof(PTREE_MOD_MP3EN_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(&mp3enModIn->base, &G_PTREE_MOD_MP3EN_IN_OPS, mod, loopId))
    {
        goto ERR_MOD_IN_INIT;
    }
    if (SSOS_DEF_OK != PTREE_LINKER_Init(&mp3enModIn->syncLinker, &G_PTREE_MOD_MP3EN_IN_SYNC_LINKER_OPS))
    {
        goto ERR_LINKER_INIT;
    }
    PTREE_LINKER_Register(&mp3enModIn->syncLinker, &G_PTREE_MOD_MP3EN_IN_SYNC_LINKER_HOOK);
    PTREE_MOD_InObjRegister(&mp3enModIn->base, &G_PTREE_MOD_MP3EN_IN_HOOK);
    return &mp3enModIn->base;

ERR_LINKER_INIT:
    PTREE_MOD_InObjDel(&mp3enModIn->base);
ERR_MOD_IN_INIT:
    SSOS_MEM_Free(mp3enModIn);
ERR_MEM_ALLOC:
    return NULL;
}

static void _PTREE_MOD_MP3EN_OutDestruct(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_MP3EN_OutObj_t *mp3enModOut = CONTAINER_OF(modOut, PTREE_MOD_MP3EN_OutObj_t, base);

    PTREE_LINKER_Del(&mp3enModOut->outLinker.base);
    PTREE_MOD_OutObjDel(&mp3enModOut->base);
}

static void _PTREE_MOD_MP3EN_OutFree(PTREE_MOD_OutObj_t *modOut)
{
    SSOS_MEM_Free(CONTAINER_OF(modOut, PTREE_MOD_MP3EN_OutObj_t, base));
}

static PTREE_MOD_OutObj_t *_PTREE_MOD_MP3EN_OutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_MP3EN_OutObj_t *mp3enModOut = NULL;

    mp3enModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_MP3EN_OutObj_t));
    if (!mp3enModOut)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(mp3enModOut, 0, sizeof(PTREE_MOD_MP3EN_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&mp3enModOut->base, &G_PTREE_MOD_MP3EN_OUT_OPS, mod, loopId))
    {
        goto ERR_MOD_OUT_INIT;
    }

    if (SSOS_DEF_OK != PTREE_NLINKER_OUT_Init(&mp3enModOut->outLinker, &G_PTREE_MOD_MP3EN_OUT_LINKER_OPS))
    {
        goto ERR_OUT_LINKER_INIT;
    }

    PTREE_MOD_OutObjRegister(&mp3enModOut->base, &G_PTREE_MOD_MP3EN_OUT_HOOK);
    PTREE_NLINKER_OUT_Register(&mp3enModOut->outLinker, &G_PTREE_MOD_MP3EN_OUT_LINKER_HOOK);

    return &mp3enModOut->base;

ERR_OUT_LINKER_INIT:
    PTREE_MOD_OutObjDel(&mp3enModOut->base);
ERR_MOD_OUT_INIT:
    SSOS_MEM_Free(mp3enModOut);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MOD_Obj_t *PTREE_MOD_MP3EN_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_MP3EN_Obj_t *mp3enMod = NULL;

    mp3enMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_MP3EN_Obj_t));
    if (!mp3enMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(mp3enMod, 0, sizeof(PTREE_MOD_MP3EN_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_ObjInit(&mp3enMod->base, &G_PTREE_MOD_MP3EN_OPS, tag))
    {
        goto ERR_MOD_SYS_INIT;
    }

    PTREE_MOD_ObjRegister(&mp3enMod->base, &G_PTREE_MOD_MP3EN_HOOK);
    return &mp3enMod->base;

ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(mp3enMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(MP3EN, PTREE_MOD_MP3EN_New);
