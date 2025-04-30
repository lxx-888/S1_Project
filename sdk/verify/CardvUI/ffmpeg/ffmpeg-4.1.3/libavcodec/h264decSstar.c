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

/*
 * H.26L/H.264/AVC/JVT/14496-10/... decoder
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <sys/time.h>
#include "h264decSstar.h"
#include "avcodec.h"
#include "h264data.h"
#include "decode.h"

#define VDEC_DEV_ID (0)

static void export_stream_params(AVCodecContext *avctx, const SPS *sps)
{
    unsigned int num = 0, den = 0;

    avctx->pix_fmt = AV_PIX_FMT_NV12;
    avctx->profile = sps->profile_idc;
    avctx->level   = sps->level_idc;
    avctx->width   = sps->mb_width * 16;
    avctx->height  = sps->mb_height * 16 * (2 - sps->frame_mbs_only_flag);

    if (sps->crop)
    {
        // avctx->width -= 2 * (sps->crop_left + sps->crop_right);
        // if (sps->frame_mbs_only_flag)
        //     avctx->height -= 2 * (sps->crop_top + sps->crop_bottom);
        // else
        //     avctx->height -= 4 * (sps->crop_top + sps->crop_bottom);
        avctx->width -= (sps->crop_left + sps->crop_right);
        avctx->height -= (sps->crop_top + sps->crop_bottom);
    }

    avctx->coded_width  = avctx->width;
    avctx->coded_height = avctx->height;

    ff_set_sar(avctx, sps->sar);

    avctx->color_range = sps->full_range ? AVCOL_RANGE_JPEG : AVCOL_RANGE_MPEG;

    if (sps->colour_description_present_flag)
    {
        avctx->color_primaries = sps->color_primaries;
        avctx->color_trc       = sps->color_trc;
        avctx->colorspace      = sps->colorspace;
    }
    else
    {
        avctx->color_primaries = AVCOL_PRI_UNSPECIFIED;
        avctx->color_trc       = AVCOL_TRC_UNSPECIFIED;
        avctx->colorspace      = AVCOL_SPC_UNSPECIFIED;
    }

    if (sps->timing_info_present_flag)
    {
        num = sps->num_units_in_tick;
        den = sps->time_scale;
    }

    if (num != 0 && den != 0)
        av_reduce(&avctx->framerate.den, &avctx->framerate.num, num, den, 1 << 30);
}

static int ss_h264_decode_extradata(SsH264Context *s, uint8_t *buf, int length)
{
    int ret, i;
    ret = ff_h264_decode_extradata(buf, length, &s->ps, &s->is_avc, &s->nal_length_size, s->avctx->err_recognition,
                                   s->avctx);
    if (ret < 0)
        return ret;

    /* export stream parameters from the first SPS */
    for (i = 0; i < FF_ARRAY_ELEMS(s->ps.sps_list); i++)
    {
        if (s->ps.sps_list[i])
        {
            const SPS *sps = (const SPS *)s->ps.sps_list[i]->data;
            export_stream_params(s->avctx, sps);
            break;
        }
    }

    return 0;
}

static int ss_h264_decode_sps(SsH264Context *s, H2645NAL *nal)
{
    int ret, i;
    ret = ff_h264_decode_seq_parameter_set(&nal->gb, s->avctx, &s->ps, 0);
    if (ret < 0)
        return ret;

    /* export stream parameters from the first SPS */
    for (i = 0; i < FF_ARRAY_ELEMS(s->ps.sps_list); i++)
    {
        if (s->ps.sps_list[i])
        {
            const SPS *sps = (const SPS *)s->ps.sps_list[i]->data;
            export_stream_params(s->avctx, sps);
            break;
        }
    }
    return 0;
}

static int ss_h264_get_frame(SsH264Context *s, AVFrame *frame)
{
    MI_U32            ysize;
    MI_S32            s32Ret;
    MI_SYS_BUF_HANDLE stHandle;
    MI_SYS_BufInfo_t  stBufInfo;
    MI_SYS_ChnPort_t  stChnPort;
    fd_set            read_fds;
    struct timeval    TimeoutVal;

    if (!s->decoder_initialized)
        return AVERROR(EAGAIN);

    if (s->u32SendCnt > s->u32RecvCnt)
    {
        TimeoutVal.tv_sec  = 3;
        TimeoutVal.tv_usec = 0;
        FD_ZERO(&read_fds);
        FD_SET(s->s32Fd, &read_fds);
        s32Ret = select(s->s32Fd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret <= 0 || !FD_ISSET(s->s32Fd, &read_fds))
            return AVERROR(EAGAIN);
    }

    stChnPort.eModId    = E_MI_MODULE_ID_VDEC;
    stChnPort.u32DevId  = 0;
    stChnPort.u32ChnId  = s->s32VdecChn;
    stChnPort.u32PortId = 0;
    if (MI_SUCCESS == (s32Ret = MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &stHandle)))
    {
        s->u32RecvCnt++;
        if (!frame->buf[0])
        {
            frame->width  = s->u16DispW;
            frame->height = s->u16DispH;
            frame->format = AV_PIX_FMT_NV12;
            s32Ret        = av_frame_get_buffer(frame, 32);
            if (s32Ret < 0)
            {
                MI_SYS_ChnOutputPortPutBuf(stHandle);
                return s32Ret;
            }

            frame->pts = stBufInfo.u64Pts;
            // get image data form vdec module
            ysize = frame->width * frame->height;
            memcpy(frame->data[0], stBufInfo.stFrameData.pVirAddr[0], ysize);
            memcpy(frame->data[1], stBufInfo.stFrameData.pVirAddr[1], ysize / 2);
        }

        MI_SYS_ChnOutputPortPutBuf(stHandle);
    }
    else
    {
        s32Ret = AVERROR(EAGAIN);
    }

    return s32Ret;
}

static MI_S32 ss_h264_send_stream(SsH264Context *s, MI_U8 *data, MI_U32 size, int64_t pts, MI_BOOL bFrame)
{
    MI_VDEC_VideoStream_t stVdecStream;
    MI_U32                s32Ret;

    if (!s->decoder_initialized)
        return AVERROR(EAGAIN);

    if (0x12 != data[4])
    {
        stVdecStream.pu8Addr      = data;
        stVdecStream.u32Len       = size;
        stVdecStream.u64PTS       = pts;
        stVdecStream.bEndOfFrame  = 1;
        stVdecStream.bEndOfStream = 0;
        if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(VDEC_DEV_ID, s->s32VdecChn, &stVdecStream, 20)))
        {
            av_log(s->avctx, AV_LOG_ERROR, "MI_VDEC_SendStream ret [%x]\n", s32Ret);
            return AVERROR_INVALIDDATA;
        }

        if (bFrame)
        {
            s->u32SendCnt++;
        }
    }

    return s32Ret;
}

static int ss_h264_send_extradata(SsH264Context *s, const uint8_t *data, int size, void *logctx)
{
    char     start_code[] = {0, 0, 0, 1};
    uint8_t *extrabuf;

    if (!data || size <= 0)
        return -1;

    if (data[0] == 1)
    {
        int            i, cnt, nalsize;
        const uint8_t *p = data;

        if (size < 7)
        {
            av_log(logctx, AV_LOG_ERROR, "avcC %d too short\n", size);
            return AVERROR_INVALIDDATA;
        }

        // Decode sps from avcC
        cnt = *(p + 5) & 0x1f; // Number of sps
        p += 6;
        for (i = 0; i < cnt; i++)
        {
            nalsize = AV_RB16(p) + 2;
            if (nalsize > size - (p - data))
                return AVERROR_INVALIDDATA;
            extrabuf = av_mallocz(nalsize - 2 + sizeof(start_code));
            memcpy(extrabuf, start_code, sizeof(start_code));
            memcpy(extrabuf + sizeof(start_code), p + 2, nalsize - 2);
            ss_h264_send_stream(s, extrabuf, nalsize - 2 + sizeof(start_code), 0, FALSE);
            av_freep(&extrabuf);
            p += nalsize;
        }
        // Decode pps from avcC
        cnt = *(p++); // Number of pps
        for (i = 0; i < cnt; i++)
        {
            nalsize = AV_RB16(p) + 2;
            if (nalsize > size - (p - data))
                return AVERROR_INVALIDDATA;
            extrabuf = av_mallocz(nalsize - 2 + sizeof(start_code));
            memcpy(extrabuf, start_code, sizeof(start_code));
            memcpy(extrabuf + sizeof(start_code), p + 2, nalsize - 2);
            ss_h264_send_stream(s, extrabuf, nalsize - 2 + sizeof(start_code), 0, FALSE);
            av_freep(&extrabuf);
            p += nalsize;
        }
    }
    else
    {
        // TBD
    }
    return 0;
}

static av_cold int ss_h264_decode_end(AVCodecContext *avctx)
{
    SsH264Context *s = avctx->priv_data;
    ff_h264_ps_uninit(&s->ps);
    ff_h2645_packet_uninit(&s->pkt);
    if (s->decoder_initialized)
    {
        decoder_type = DEFAULT_DECODING;
        MI_SYS_CloseFd(s->s32Fd);
        MI_VDEC_StopChn(VDEC_DEV_ID, s->s32VdecChn);
        MI_VDEC_DestroyChn(VDEC_DEV_ID, s->s32VdecChn);
        s->decoder_initialized = 0;
    }
    return 0;
}

static MI_S32 ss_h264_vdec_init(SsH264Context *s, AVCodecContext *avctx)
{
    MI_VDEC_ChnAttr_t        stVdecChnAttr;
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
    MI_SYS_ChnPort_t         stChnPort;
    MI_S32                   s32Ret = MI_SUCCESS;

    s->s32VdecChn = 0;
    while (s->s32VdecChn < 32)
    {
        s32Ret = MI_VDEC_GetChnAttr(VDEC_DEV_ID, s->s32VdecChn, &stVdecChnAttr);
        if (s32Ret == MI_ERR_VDEC_CHN_UNEXIST)
            break;
        else
            s->s32VdecChn++;
    }

    if (s32Ret != MI_ERR_VDEC_CHN_UNEXIST)
        return -1;

    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.eCodecType                     = E_MI_VDEC_CODEC_TYPE_H264;
    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 1;
    stVdecChnAttr.eVideoMode                     = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize                     = avctx->width * avctx->height;
    stVdecChnAttr.u32PicWidth                    = avctx->width;
    stVdecChnAttr.u32PicHeight                   = avctx->height;
    stVdecChnAttr.eDpbBufMode                    = E_MI_VDEC_DPB_MODE_NORMAL;
    stVdecChnAttr.u32Priority                    = 0;
    s32Ret                                       = MI_VDEC_CreateChn(VDEC_DEV_ID, s->s32VdecChn, &stVdecChnAttr);
    if (s32Ret != MI_SUCCESS)
    {
        av_log(avctx, AV_LOG_ERROR, "MI_VDEC_CreateChn failed ret [%x]\n", s32Ret);
        return s32Ret;
    }

    s32Ret = MI_VDEC_StartChn(VDEC_DEV_ID, s->s32VdecChn);
    if (s32Ret != MI_SUCCESS)
    {
        av_log(avctx, AV_LOG_ERROR, "MI_VDEC_StartChn failed ret [%x]\n", s32Ret);
        goto L_ERR_DESTROY;
    }

    s->u16DispW = avctx->width;
    s->u16DispH = avctx->height;
    if (avctx->width > 1280 && avctx->height > 720)
    {
        memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
        s->u16DispW = stOutputPortAttr.u16Width  = 1280;
        s->u16DispH = stOutputPortAttr.u16Height = 720;
        s32Ret = MI_VDEC_SetOutputPortAttr(VDEC_DEV_ID, s->s32VdecChn, &stOutputPortAttr);
        if (s32Ret != MI_SUCCESS)
        {
            av_log(avctx, AV_LOG_ERROR, "MI_VDEC_SetOutputPortAttr failed ret [%x]\n", s32Ret);
            goto L_ERR_STOP;
        }
    }

    stChnPort.eModId    = E_MI_MODULE_ID_VDEC;
    stChnPort.u32DevId  = 0;
    stChnPort.u32ChnId  = s->s32VdecChn;
    stChnPort.u32PortId = 0;
    s32Ret              = MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 5, 10);
    if (s32Ret != MI_SUCCESS)
    {
        av_log(avctx, AV_LOG_ERROR, "MI_SYS_SetChnOutputPortDepth failed ret [%x]\n", s32Ret);
        goto L_ERR_STOP;
    }

    MI_SYS_GetFd(&stChnPort, &s->s32Fd);
    if (s->s32Fd < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "MI_SYS_GetFd failed ret [%x]\n", s32Ret);
        goto L_ERR_STOP;
    }

    return MI_SUCCESS;

L_ERR_STOP:
    MI_VDEC_StopChn(VDEC_DEV_ID, s->s32VdecChn);
L_ERR_DESTROY:
    MI_VDEC_DestroyChn(VDEC_DEV_ID, s->s32VdecChn);
    return s32Ret;
}

static av_cold int ss_h264_decode_init(AVCodecContext *avctx)
{
    MI_S32         s32Ret;
    SsH264Context *s = avctx->priv_data;

    s->u32SendCnt = 0;
    s->u32RecvCnt = 0;

    s->avctx = avctx;

    if (avctx->extradata_size > 0 && avctx->extradata)
    {
        // parse nal data from packet
        s32Ret = ss_h264_decode_extradata(s, avctx->extradata, avctx->extradata_size);
        if (s32Ret < 0)
        {
            ss_h264_decode_end(avctx);
            return s32Ret;
        }

        avctx->pix_fmt = AV_PIX_FMT_NV12;
        if (!s->decoder_initialized)
        {
            decoder_type = HARD_DECODING;
            // Init vdec module
            if (MI_SUCCESS != (s32Ret = ss_h264_vdec_init(s, avctx)))
            {
                ss_h264_decode_end(avctx);
                return s32Ret;
            }
            s->decoder_initialized = 1;
            ss_h264_send_extradata(s, avctx->extradata, avctx->extradata_size, avctx);
        }
    }
    else
    {
        // No extra data, try decode frame.
        if (avctx->width && avctx->height && avctx->pix_fmt == AV_PIX_FMT_NV12)
        {
            if (!s->decoder_initialized)
            {
                decoder_type = HARD_DECODING;
                // Init vdec module
                if (MI_SUCCESS != (s32Ret = ss_h264_vdec_init(s, avctx)))
                {
                    ss_h264_decode_end(avctx);
                    return s32Ret;
                }
                s->decoder_initialized = 1;
            }
        }
    }

    return 0;
}

static int ss_h264_decode_nalu(SsH264Context *s, AVPacket *avpkt)
{
    int  ret, i;
    char start_code[] = {0, 0, 0, 1};

    if (s->decoder_initialized && s->avctx->extradata_size == 0)
    {
        return ss_h264_send_stream(s, avpkt->data, avpkt->size, avpkt->pts, TRUE);
    }

    ret = ff_h2645_packet_split(&s->pkt, avpkt->data, avpkt->size, s->avctx, s->is_avc, s->nal_length_size,
                                s->avctx->codec_id, 1);
    if (ret < 0)
    {
        av_log(s->avctx, AV_LOG_ERROR, "Error splitting the input into NAL units.\n");
        return ret;
    }

    /* decode the NAL units */
    for (i = 0; i < s->pkt.nb_nals; i++)
    {
        H2645NAL *nal      = &s->pkt.nals[i];
        uint8_t * extrabuf = av_mallocz(nal->size + sizeof(start_code));
        if (!extrabuf)
            return AVERROR(ENOMEM);

        switch (nal->type)
        {
            case H264_NAL_SLICE:
            case H264_NAL_DPA:
            case H264_NAL_DPB:
            case H264_NAL_DPC:
            case H264_NAL_IDR_SLICE:
                memcpy(extrabuf, start_code, sizeof(start_code));
                memcpy(extrabuf + sizeof(start_code), nal->data, nal->size);
                ss_h264_send_stream(s, extrabuf, nal->size + sizeof(start_code), avpkt->pts, TRUE);
                break;

            case H264_NAL_SPS:
                if (!s->decoder_initialized)
                {
                    ret = ss_h264_decode_sps(s, nal);
                    if (ret < 0)
                        return ret;
                    if (s->avctx->width != 0 && s->avctx->height != 0)
                    {
                        decoder_type      = HARD_DECODING;
                        s->avctx->pix_fmt = AV_PIX_FMT_NV12;
                        // Init vdec module
                        if (MI_SUCCESS != (ret = ss_h264_vdec_init(s, s->avctx)))
                        {
                            return ret;
                        }
                        s->decoder_initialized = 1;
                    }
                }
            case H264_NAL_PPS:
            case H264_NAL_SEI:
                memcpy(extrabuf, start_code, sizeof(start_code));
                memcpy(extrabuf + sizeof(start_code), nal->data, nal->size);
                ss_h264_send_stream(s, extrabuf, nal->size + 4, 0, FALSE);
                break;

            default:
                break;
        }
        av_freep(&extrabuf);
    }

    return ret;
}

static int ss_h264_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    MI_S32         s32Ret = 0;
    SsH264Context *s      = avctx->priv_data;

    // end of stream and vdec buf is null
    if (!avpkt->size)
    {
        av_log(avctx, AV_LOG_INFO, "packet size is 0\n");
        return AVERROR_EOF;
    }
    else
    {
        // continue to send stream to vdec
        s32Ret = ss_h264_decode_nalu(s, avpkt);
        if (MI_SUCCESS != s32Ret)
        {
            av_log(avctx, AV_LOG_ERROR, "ss_h264_decode_nalu ret [%x]\n", s32Ret);
            return s32Ret;
        }
    }

    return avpkt->size;
}

static int ss_h264_receive_frame(AVCodecContext *avctx, AVFrame *frame)
{
    int                  ret, got_frame;
    SsH264Context *      s     = avctx->priv_data;
    AVCodecInternal *    avci  = avctx->internal;
    DecodeSimpleContext *ds    = &avci->ds;
    AVPacket *           avpkt = ds->in_pkt;

    while (!frame->buf[0])
    {
        if (!avpkt->data && !avci->draining && !(avctx->flags & (1 << 5)))
        {
            av_packet_unref(avpkt);
            ret = ff_decode_get_packet(avctx, avpkt);
            if (ret < 0 && ret != AVERROR_EOF)
            {
                return AVERROR(EAGAIN);
            }
            else
            {
                return ret;
            }
        }

        got_frame = 0;
        if (MI_SUCCESS != (ret = ss_h264_get_frame(s, frame)))
        {
            // vdec wait for 10ms and continue to send stream
            usleep(10000);
        }
        else
        {
            got_frame                    = 1;
            frame->best_effort_timestamp = frame->pts;
        }

        if (avpkt->data)
        {
            ret = ss_h264_decode_nalu(s, avpkt);
            if (ret < 0)
                av_log(avctx, AV_LOG_ERROR, "ss_h264_decode_nalu ret [%x]\n", ret);

            if (!(avctx->codec->caps_internal & FF_CODEC_CAP_SETS_PKT_DTS))
                frame->pkt_dts = avpkt->dts;
            if (avctx->codec->type == AVMEDIA_TYPE_VIDEO && !avctx->has_b_frames)
            {
                frame->pkt_pos = avpkt->pos;
            }

            if (!got_frame)
                av_frame_unref(frame);

            if (ret >= 0 && avctx->codec->type == AVMEDIA_TYPE_VIDEO && !(avctx->flags & AV_CODEC_FLAG_TRUNCATED))
                ret = avpkt->size;
            avci->compat_decode_consumed += ret;

            if (ret >= avpkt->size || ret < 0)
            {
                av_packet_unref(avpkt);
            }
            else
            {
                int consumed = ret;
                avpkt->data += consumed;
                avpkt->size -= consumed;
                avci->last_pkt_props->size -= consumed; // See extract_packet_props() comment.
                avpkt->pts                = AV_NOPTS_VALUE;
                avpkt->dts                = AV_NOPTS_VALUE;
                avci->last_pkt_props->pts = AV_NOPTS_VALUE;
                avci->last_pkt_props->dts = AV_NOPTS_VALUE;
            }

            if (got_frame)
                av_assert0(frame->buf[0]);
        }

        if (ret < 0)
            return ret;
    }

    return avpkt->size;
}

AVCodec ff_ssh264_decoder = {
    .name           = "ssh264",
    .long_name      = NULL_IF_CONFIG_SMALL("H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_H264,
    .priv_data_size = sizeof(SsH264Context),
    .init           = ss_h264_decode_init,
    .close          = ss_h264_decode_end,
    .decode         = ss_h264_decode_frame,
    .receive_frame  = ss_h264_receive_frame,
    .capabilities   = /*AV_CODEC_CAP_DRAW_HORIZ_BAND | AV_CODEC_CAP_DR1 |*/
    AV_CODEC_CAP_DELAY | AV_CODEC_CAP_SLICE_THREADS | AV_CODEC_CAP_FRAME_THREADS,
};