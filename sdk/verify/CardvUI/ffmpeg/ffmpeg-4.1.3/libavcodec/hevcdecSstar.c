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

#include <sys/time.h>
#include "decode.h"
#include "hevc.h"
#include "hevc_data.h"
#include "hevc_parse.h"
#include "hevcdecSstar.h"
#include "h2645_parse.h"

#define VDEC_DEV_ID (0)

static void export_stream_params(AVCodecContext *avctx, const HEVCParamSets *ps, const HEVCSPS *sps)
{
    const HEVCVPS *   vps = (const HEVCVPS *)ps->vps_list[sps->vps_id]->data;
    const HEVCWindow *ow  = &sps->output_window;
    unsigned int      num = 0, den = 0;

    avctx->pix_fmt      = sps->pix_fmt;
    avctx->coded_width  = sps->width;
    avctx->coded_height = sps->height;
    avctx->width        = sps->width - ow->left_offset - ow->right_offset;
    avctx->height       = sps->height - ow->top_offset - ow->bottom_offset;
    avctx->has_b_frames = sps->temporal_layer[sps->max_sub_layers - 1].num_reorder_pics;
    avctx->profile      = sps->ptl.general_ptl.profile_idc;
    avctx->level        = sps->ptl.general_ptl.level_idc;

    ff_set_sar(avctx, sps->vui.sar);

    if (sps->vui.video_signal_type_present_flag)
        avctx->color_range = sps->vui.video_full_range_flag ? AVCOL_RANGE_JPEG : AVCOL_RANGE_MPEG;
    else
        avctx->color_range = AVCOL_RANGE_MPEG;

    if (sps->vui.colour_description_present_flag)
    {
        avctx->color_primaries = sps->vui.colour_primaries;
        avctx->color_trc       = sps->vui.transfer_characteristic;
        avctx->colorspace      = sps->vui.matrix_coeffs;
    }
    else
    {
        avctx->color_primaries = AVCOL_PRI_UNSPECIFIED;
        avctx->color_trc       = AVCOL_TRC_UNSPECIFIED;
        avctx->colorspace      = AVCOL_SPC_UNSPECIFIED;
    }

    if (vps->vps_timing_info_present_flag)
    {
        num = vps->vps_num_units_in_tick;
        den = vps->vps_time_scale;
    }
    else if (sps->vui.vui_timing_info_present_flag)
    {
        num = sps->vui.vui_num_units_in_tick;
        den = sps->vui.vui_time_scale;
    }

    if (num != 0 && den != 0)
        av_reduce(&avctx->framerate.den, &avctx->framerate.num, num, den, 1 << 30);
}

static int ss_hevc_decode_extradata(SsHevcContext *s, uint8_t *buf, int length)
{
    int ret, i;
    ret = ff_hevc_decode_extradata(buf, length, &s->ps, &s->sei, &s->is_nalff, &s->nal_length_size,
                                   s->avctx->err_recognition, 0, s->avctx);
    if (ret < 0)
        return ret;

    /* export stream parameters from the first SPS */
    for (i = 0; i < FF_ARRAY_ELEMS(s->ps.sps_list); i++)
    {
        if (s->ps.sps_list[i])
        {
            const HEVCSPS *sps = (const HEVCSPS *)s->ps.sps_list[i]->data;
            export_stream_params(s->avctx, &s->ps, sps);
            break;
        }
    }

    return 0;
}

static int ss_hevc_decode_vps_sps(SsHevcContext *s, H2645NAL *nal)
{
    int ret, i;

    if (nal->type == HEVC_NAL_VPS)
    {
        ret = ff_hevc_decode_nal_vps(&nal->gb, s->avctx, &s->ps);
        if (ret < 0)
            return ret;
    }
    else if (nal->type == HEVC_NAL_SPS)
    {
        ret = ff_hevc_decode_nal_sps(&nal->gb, s->avctx, &s->ps, 0);
        if (ret < 0)
            return ret;

        /* export stream parameters from the first SPS */
        for (i = 0; i < FF_ARRAY_ELEMS(s->ps.sps_list); i++)
        {
            if (s->ps.sps_list[i])
            {
                const HEVCSPS *sps = (const HEVCSPS *)s->ps.sps_list[i]->data;
                export_stream_params(s->avctx, &s->ps, sps);
                break;
            }
        }
    }

    return 0;
}

static int ss_hevc_get_frame(SsHevcContext *s, AVFrame *frame)
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

static MI_U32 ss_hevc_vdec_init(SsHevcContext *s, AVCodecContext *avctx)
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
    stVdecChnAttr.eCodecType                     = E_MI_VDEC_CODEC_TYPE_H265;
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

    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
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

static MI_U32 ss_hevc_send_stream(SsHevcContext *s, MI_U8 *data, MI_U32 size, int64_t pts, MI_BOOL bFrame)
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

static int ss_hevc_decode_nalu(SsHevcContext *s, AVPacket *avpkt)
{
    int   i, ret;
    MI_U8 start_code[4] = {0, 0, 0, 1};

    if (s->decoder_initialized && s->avctx->extradata_size == 0)
    {
        return ss_hevc_send_stream(s, avpkt->data, avpkt->size, avpkt->pts, TRUE);
    }

    ret = ff_h2645_packet_split(&s->pkt, avpkt->data, avpkt->size, s->avctx, s->is_nalff, s->nal_length_size,
                                s->avctx->codec_id, 1);
    if (ret < 0)
    {
        av_log(s->avctx, AV_LOG_ERROR, "Error splitting the input into NAL units.\n");
        return ret;
    }

    /* decode the VPS/SPS to init decoder */
    for (i = 0; i < s->pkt.nb_nals && s->decoder_initialized == 0; i++)
    {
        H2645NAL *nal = &s->pkt.nals[i];
        switch (nal->type)
        {
            case HEVC_NAL_VPS:
            case HEVC_NAL_SPS:
                ret = ss_hevc_decode_vps_sps(s, nal);
                if (ret < 0)
                    return ret;

                if (s->avctx->width != 0 && s->avctx->height != 0)
                {
                    decoder_type      = HARD_DECODING;
                    s->avctx->pix_fmt = AV_PIX_FMT_NV12;
                    // Init vdec module
                    if (MI_SUCCESS != (ret = ss_hevc_vdec_init(s, s->avctx)))
                    {
                        return ret;
                    }
                    s->decoder_initialized = 1;
                }
                break;
            default:
                break;
        }
    }

    /* decode the NAL units */
    for (i = 0; i < s->pkt.nb_nals; i++)
    {
        H2645NAL *nal           = &s->pkt.nals[i];
        MI_U8 *   extradata_buf = av_mallocz(nal->size + sizeof(start_code));
        if (!extradata_buf)
            return AVERROR(ENOMEM);

        switch (nal->type)
        {
            case HEVC_NAL_VPS:
            case HEVC_NAL_SPS:
            case HEVC_NAL_PPS:
            case HEVC_NAL_SEI_PREFIX:
            case HEVC_NAL_SEI_SUFFIX:
                memcpy(extradata_buf, start_code, sizeof(start_code));
                memcpy(extradata_buf + sizeof(start_code), nal->data, nal->size);
                ss_hevc_send_stream(s, extradata_buf, nal->size + 4, 0, FALSE);
                break;

            case HEVC_NAL_TRAIL_R:
            case HEVC_NAL_TRAIL_N:
            case HEVC_NAL_TSA_N:
            case HEVC_NAL_TSA_R:
            case HEVC_NAL_STSA_N:
            case HEVC_NAL_STSA_R:
            case HEVC_NAL_BLA_W_LP:
            case HEVC_NAL_BLA_W_RADL:
            case HEVC_NAL_BLA_N_LP:
            case HEVC_NAL_IDR_W_RADL:
            case HEVC_NAL_IDR_N_LP:
            case HEVC_NAL_CRA_NUT:
            case HEVC_NAL_RADL_N:
            case HEVC_NAL_RADL_R:
            case HEVC_NAL_RASL_N:
            case HEVC_NAL_RASL_R:
                memcpy(extradata_buf, start_code, sizeof(start_code));
                memcpy(extradata_buf + sizeof(start_code), nal->data, nal->size);
                ss_hevc_send_stream(s, extradata_buf, nal->size + 4, avpkt->pts, TRUE);
                break;

            case HEVC_NAL_EOS_NUT:
            case HEVC_NAL_EOB_NUT:
                break;
            case HEVC_NAL_AUD:
            case HEVC_NAL_FD_NUT:
                break;
            default:
                av_log(s->avctx, AV_LOG_INFO, "Skipping NAL unit %d\n", nal->type);
                break;
        }
        av_freep(&extradata_buf);
    }

    return MI_SUCCESS;
}

static av_cold int ss_hevc_decode_free(AVCodecContext *avctx)
{
    SsHevcContext *s = avctx->priv_data;
    ff_hevc_ps_uninit(&s->ps);
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

static av_cold int ss_hevc_decode_init(AVCodecContext *avctx)
{
    MI_S32         s32Ret;
    SsHevcContext *s = avctx->priv_data;

    s->u32SendCnt = 0;
    s->u32RecvCnt = 0;

    s->avctx = avctx;
    ff_hevc_reset_sei(&s->sei);

    if (avctx->extradata_size > 0 && avctx->extradata)
    {
        // parse nal data from packet
        s32Ret = ss_hevc_decode_extradata(s, avctx->extradata, avctx->extradata_size);
        if (s32Ret < 0)
        {
            ss_hevc_decode_free(avctx);
            return s32Ret;
        }

        avctx->pix_fmt = AV_PIX_FMT_NV12;
        if (!s->decoder_initialized)
        {
            decoder_type = HARD_DECODING;
            // Init vdec module
            if (MI_SUCCESS != (s32Ret = ss_hevc_vdec_init(s, avctx)))
            {
                ss_hevc_decode_free(avctx);
                return s32Ret;
            }
            s->decoder_initialized = 1;
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
                if (MI_SUCCESS != (s32Ret = ss_hevc_vdec_init(s, avctx)))
                {
                    ss_hevc_decode_free(avctx);
                    return s32Ret;
                }
                s->decoder_initialized = 1;
            }
        }
    }

    return 0;
}

static int ss_hevc_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    MI_S32         s32Ret = 0;
    SsHevcContext *s      = avctx->priv_data;

    // end of stream and vdec buf is null
    if (!avpkt->size)
    {
        av_log(avctx, AV_LOG_INFO, "packet size is 0\n");
        return AVERROR_EOF;
    }
    else
    {
        // continue to send stream to vdec
        s32Ret = ss_hevc_decode_nalu(s, avpkt);
        if (MI_SUCCESS != s32Ret)
        {
            av_log(avctx, AV_LOG_ERROR, "ss_hevc_decode_nalu ret [%x]\n", s32Ret);
            return s32Ret;
        }
    }

    return avpkt->size;
}

static int ss_hevc_receive_frame(AVCodecContext *avctx, AVFrame *frame)
{
    int ret, got_frame;

    SsHevcContext *      s     = avctx->priv_data;
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
        if (MI_SUCCESS != (ret = ss_hevc_get_frame(s, frame)))
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
            ret = ss_hevc_decode_nalu(s, avpkt);
            if (ret < 0)
                av_log(avctx, AV_LOG_ERROR, "ss_hevc_decode_nalu ret [%x]\n", ret);

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
        {
            return ret;
        }
    }

    return avpkt->size;
}

AVCodec ff_sshevc_decoder = {
    .name           = "sshevc",
    .long_name      = NULL_IF_CONFIG_SMALL("HEVC (High Efficiency Video Coding)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_HEVC,
    .priv_data_size = sizeof(SsHevcContext),
    .init           = ss_hevc_decode_init,
    .close          = ss_hevc_decode_free,
    .decode         = ss_hevc_decode_frame,
    .receive_frame  = ss_hevc_receive_frame,
    .capabilities = /*AV_CODEC_CAP_DR1 |*/ AV_CODEC_CAP_DELAY | AV_CODEC_CAP_SLICE_THREADS | AV_CODEC_CAP_FRAME_THREADS,
};