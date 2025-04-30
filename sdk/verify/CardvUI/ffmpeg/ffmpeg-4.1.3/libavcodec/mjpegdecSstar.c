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

/**
 * @file
 * MJPEG codec.
 * @author jeff.li <jeff.li@sigmastar.com.cn>
 */

#include "libavutil/intreadwrite.h"
#include "mjpeg.h"
#include "mjpegdecSstar.h"
#include <sys/select.h>

#define JPD_MIN(a, b) ((a) < (b) ? (a) : (b))
#define JPD_DIRECT_DECODE (0)

#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return -1;\
    }\

static int get_jpeg_size(AVPacket *avpkt)
{
    const uint8_t *b = avpkt->data;
    int i, state = SOI;

    if (AV_RB16(b) != 0xFFD8 ||
        AV_RB32(b) == 0xFFD8FFF7)
    return 0;

    if (AV_RB16(&b[avpkt->size - 2]) == 0xFFD9)
        return avpkt->size;

    b += 2;
    for (i = 0; i < avpkt->size - 3; i++) {
        int c;
        if (b[i] != 0xFF)
            continue;
        c = b[i + 1];
        switch (c) {
        case SOI:
            return 0;
        case SOF0:
        case SOF1:
        case SOF2:
        case SOF3:
        case SOF5:
        case SOF6:
        case SOF7:
            i += AV_RB16(&b[i + 2]) + 1;
            if (state != SOI)
                return 0;
            state = SOF0;
            break;
        case SOS:
            i += AV_RB16(&b[i + 2]) + 1;
            if (state != SOF0 && state != SOS)
                return 0;
            state = SOS;
            break;
        case EOI:
            if (state != SOS)
                return 0;
            return i + 4;
        case DQT:
        case APP0:
        case APP1:
        case APP2:
        case APP3:
        case APP4:
        case APP5:
        case APP6:
        case APP7:
        case APP8:
        case APP9:
        case APP10:
        case APP11:
        case APP12:
        case APP13:
        case APP14:
        case APP15:
        case COM:
            i += AV_RB16(&b[i + 2]) + 1;
            break;
        default:
            if (  (c > TEM && c < SOF0)
                || c == JPG)
                return 0;
        }
    }

    return 0;
}

#if (JPD_DIRECT_DECODE == 0)
static int ss_mjpeg_scl_init(SsMjpegContext *s)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_SCL_DevAttr_t stCreateDevAttr;
    MI_SCL_ChannelAttr_t stSclChnAttr;
    MI_SCL_OutPortParam_t stSclOutputParam;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    s->SclDev = 3;
    s->SclChn = 16;
    s->SclPort = 0;
    s->s32Fd = -1;

    memset(&stCreateDevAttr, 0x0, sizeof(MI_SCL_DevAttr_t));
    stCreateDevAttr.u32NeedUseHWOutPortMask = 0x20; // HWSCL5
    s32Ret = MI_SCL_CreateDevice(s->SclDev, &stCreateDevAttr);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_SCL_CreateDevice fail [%x]\n", s32Ret);
        return -1;
    }

    memset(&stSclChnAttr, 0x0, sizeof(MI_SCL_ChannelAttr_t));
    s32Ret = MI_SCL_CreateChannel(s->SclDev, s->SclChn, &stSclChnAttr);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_SCL_CreateChannel fail [%x]\n", s32Ret);
        goto ERROR_DESTROY_DEV;
    }

    s32Ret = MI_SCL_StartChannel(s->SclDev, s->SclChn);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_SCL_StartChannel fail [%x]\n", s32Ret);
        goto ERROR_DESTROY_CHN;
    }

    memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));;
    stSclOutputParam.stSCLOutputSize.u16Width = 1280;
    stSclOutputParam.stSCLOutputSize.u16Height = 720;
    stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stSclOutputParam.bMirror = FALSE;
    stSclOutputParam.bFlip = FALSE;
    s32Ret = MI_SCL_SetOutputPortParam(s->SclDev, s->SclChn, s->SclPort, &stSclOutputParam);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_SCL_SetOutputPortParam fail [%x]\n", s32Ret);
        goto ERROR_STOP_CHN;
    }

    s32Ret = MI_SCL_EnableOutputPort(s->SclDev, s->SclChn, s->SclPort);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_SCL_EnableOutputPort fail [%x]\n", s32Ret);
        goto ERROR_STOP_CHN;
    }

    stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId = s->SclDev;
    stDstChnPort.u32ChnId = s->SclChn;
    stDstChnPort.u32PortId = s->SclPort;
    s32Ret = MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 1, 1);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_SYS_SetChnOutputPortDepth fail [%x]\n", s32Ret);
        goto ERROR_DISABLE_PORT;
    }

    stSrcChnPort.eModId = E_MI_MODULE_ID_JPD;
    stSrcChnPort.u32DevId = s->JpdDev;
    stSrcChnPort.u32ChnId = s->JpdChn;
    stSrcChnPort.u32PortId = 0;
    s32Ret = MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, 1, 1, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_SYS_BindChnPort2 fail [%x]\n", s32Ret);
        goto ERROR_DISABLE_PORT;
    }

    s32Ret = MI_SYS_GetFd(&stDstChnPort, &s->s32Fd);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_SYS_GetFd fail [%x]\n", s32Ret);
        goto ERROR_UNBIND;
    }

    return 0;

ERROR_UNBIND:
    MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort);
ERROR_DISABLE_PORT:
    MI_SCL_DisableOutputPort(s->SclDev, s->SclChn, s->SclPort);
ERROR_STOP_CHN:
    MI_SCL_StopChannel(s->SclDev, s->SclChn);
ERROR_DESTROY_CHN:
    MI_SCL_DestroyChannel(s->SclDev, s->SclChn);
ERROR_DESTROY_DEV:
    MI_SCL_DestroyDevice(s->SclDev);
    return -1;
}

static int ss_mjpeg_scl_deinit(SsMjpegContext *s)
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    STCHECKRESULT(MI_SYS_CloseFd(s->s32Fd));

    stSrcChnPort.eModId = E_MI_MODULE_ID_JPD;
    stSrcChnPort.u32DevId = s->JpdDev;
    stSrcChnPort.u32ChnId = s->JpdChn;
    stSrcChnPort.u32PortId = 0;
    stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId = s->SclDev;
    stDstChnPort.u32ChnId = s->SclChn;
    stDstChnPort.u32PortId = s->SclPort;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));
    STCHECKRESULT(MI_SCL_DisableOutputPort(s->SclDev, s->SclChn, s->SclPort));
    STCHECKRESULT(MI_SCL_StopChannel(s->SclDev, s->SclChn));
    STCHECKRESULT(MI_SCL_DestroyChannel(s->SclDev, s->SclChn));
    STCHECKRESULT(MI_SCL_DestroyDevice(s->SclDev));
    return 0;
}

static int ss_mjpeg_polling_scl(SsMjpegContext *s)
{
    MI_S32 s32Ret = 0;
    fd_set read_fds;
    struct timeval TimeoutVal;

    TimeoutVal.tv_sec  = 0;
    TimeoutVal.tv_usec = 500000;

    FD_ZERO(&read_fds);
    FD_SET(s->s32Fd, &read_fds);
    s32Ret = select(s->s32Fd + 1, &read_fds, NULL, NULL, &TimeoutVal);
    if (s32Ret < 0) {
        printf("select failed\n");
        return -1;
    } else if (0 == s32Ret) {
        return -1;
    } else if (!FD_ISSET(s->s32Fd, &read_fds)) {
        printf("not detect\n");
        return -1;
    }

    FD_CLR(s->s32Fd, &read_fds);
    return 0;
}
#endif

static av_cold int ss_mjpeg_decode_free(AVCodecContext *avctx)
{
    SsMjpegContext *s = avctx->priv_data;

#if (JPD_DIRECT_DECODE == 0)
    STCHECKRESULT(ss_mjpeg_scl_deinit(s));
#endif
    STCHECKRESULT(MI_JPD_StopChn(s->JpdDev, s->JpdChn));
    STCHECKRESULT(MI_JPD_DestroyChn(s->JpdDev, s->JpdChn));
    STCHECKRESULT(MI_JPD_DestroyDev(s->JpdDev));

    if (s->picture_ptr)
        av_frame_unref(s->picture_ptr);
    av_frame_free(&s->picture_ptr);
    return 0;
}

static av_cold int ss_mjpeg_decode_init(AVCodecContext *avctx)
{
    SsMjpegContext *s = avctx->priv_data;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_JPD_InitParam_t    stInitParam;
    MI_JPD_ChnCreatConf_t stChnCreatConf;

    avctx->chroma_sample_location = AVCHROMA_LOC_CENTER;
    avctx->colorspace = AVCOL_SPC_BT470BG;

    s->picture_ptr = av_frame_alloc();
    if (!s->picture_ptr) {
        av_log(avctx, AV_LOG_ERROR, "ssmjpeg malloc frame error!\n");
        return -1;
    }

    s->JpdDev = 0;
    s->JpdChn = 0;
    memset(&stChnCreatConf, 0x0, sizeof(MI_JPD_ChnCreatConf_t));
    #if (JPD_DIRECT_DECODE == 1)
    stChnCreatConf.u32MaxPicWidth   = 3840;
    stChnCreatConf.u32MaxPicHeight  = 2160;
    stChnCreatConf.u32StreamBufSize = 0;
    avctx->pix_fmt = AV_PIX_FMT_YUYV422;
    #else
    stChnCreatConf.u32MaxPicWidth   = 8192;
    stChnCreatConf.u32MaxPicHeight  = 8192;
    stChnCreatConf.u32StreamBufSize = 4 * 1024 * 1024;
    avctx->pix_fmt = AV_PIX_FMT_NV12;
    #endif
    s32Ret = MI_JPD_CreateDev(s->JpdDev, &stInitParam);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_JPD_CreateDev fail [%x]\n", s32Ret);
        return -1;
    }
    s32Ret = MI_JPD_CreateChn(s->JpdDev, s->JpdChn, &stChnCreatConf);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_JPD_CreateChn fail [%x]\n", s32Ret);
        goto ERROR_DESTROY_DEV;
    }
    s32Ret = MI_JPD_StartChn(s->JpdDev, s->JpdChn);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_JPD_StartChn fail [%x]\n", s32Ret);
        goto ERROR_DESTROY_CHN;
    }

#if (JPD_DIRECT_DECODE == 0)
    if (ss_mjpeg_scl_init(s) != 0) {
        goto ERROR_STOP;
    }
#endif
    return 0;

#if (JPD_DIRECT_DECODE == 0)
ERROR_STOP:
    MI_JPD_StopChn(s->JpdDev, s->JpdChn);
#endif
ERROR_DESTROY_CHN:
    MI_JPD_DestroyChn(s->JpdDev, s->JpdChn);
ERROR_DESTROY_DEV:
    MI_JPD_DestroyDev(s->JpdDev);
    return -1;
}

#if (JPD_DIRECT_DECODE == 1)
static int ss_mjpeg_decode_frame(AVCodecContext *avctx, void *data,
                                      int *got_frame, AVPacket *avpkt)
{
    AVFrame *frame = data;
    SsMjpegContext *s = avctx->priv_data;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U8 *pu8Buf = avpkt->data;
    MI_U8 *pu8VirBuf = NULL;
    MI_U32 u32LengthCurStream = avpkt->size;
    MI_JPD_DirectInputBuf_t stInputBuf;
    MI_JPD_DirectOutputBuf_t stOutputBuf;
    MI_JPD_StreamInfo_t stStreamInfo;
    MI_U32 u32OutputBufSize = 0;

    memset(&stInputBuf, 0, sizeof(MI_JPD_DirectInputBuf_t));
    memset(&stOutputBuf, 0, sizeof(MI_JPD_DirectOutputBuf_t));
    memset(&stStreamInfo, 0, sizeof(MI_JPD_StreamInfo_t));

    u32LengthCurStream = get_jpeg_size(avpkt);
    if (u32LengthCurStream == 0)
        u32LengthCurStream = avpkt->size;

    stInputBuf.pu8InputJPGRawFrameDataVirtAddr = pu8Buf;
    stInputBuf.phyAddr = 0;
    stInputBuf.u32Length = u32LengthCurStream;
    s32Ret = MI_JPD_QueryStreamInfo(s->JpdDev, s->JpdChn, &stInputBuf, &stStreamInfo);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_JPD_QueryStreamInfo failed, s32Ret: 0x%x.\n", s32Ret);
        return -1;
    }
    // printf("stream info: w:%d h:%d format:%d bufW:%d bufH:%d\n",
    //     stStreamInfo.u32CurPicWidth, stStreamInfo.u32CurPicHeight,
    //     stStreamInfo.ePixelFormat,
    //     stStreamInfo.u32BufWidth, stStreamInfo.u32BufHeight);

    avctx->width = stStreamInfo.u32CurPicWidth;
    avctx->height = stStreamInfo.u32CurPicHeight;

    // only support output pixel format YUYV422
    stOutputBuf.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stOutputBuf.u32Width = stStreamInfo.u32BufWidth;
    stOutputBuf.u32Height = stStreamInfo.u32BufHeight;
    stOutputBuf.u32Stride[0] = stOutputBuf.u32Width * 2;
    u32OutputBufSize = stOutputBuf.u32Stride[0] * stOutputBuf.u32Height;
    s32Ret = MI_SYS_MMA_Alloc(0, (MI_U8*)"#jpd_outbuf", u32OutputBufSize, &stOutputBuf.phyAddr[0]);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_SYS_MMA_Alloc failed, s32Ret: 0x%x.\n", s32Ret);
        return -1;
    }

    s32Ret = MI_JPD_DirectBufDecode(s->JpdDev, s->JpdChn, &stInputBuf, &stOutputBuf);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_JPD_DirectBufDecode failed, s32Ret: 0x%x.\n", s32Ret);
        goto RETURN_FREE;
    }

    av_frame_unref(s->picture_ptr);
    if (ff_get_buffer(avctx, s->picture_ptr, AV_GET_BUFFER_FLAG_REF) < 0)
        goto RETURN_FREE;

    if (av_frame_ref(frame, s->picture_ptr) < 0) {
        printf("av_frame_ref fail\n");
        goto RETURN_FREE;
    }

    s32Ret = MI_SYS_Mmap(stOutputBuf.phyAddr[0], u32OutputBufSize, (void **)&pu8VirBuf, TRUE);
    if (MI_SUCCESS != s32Ret) {
        printf("MI_SYS_Mmap failed, s32Ret: 0x%x.\n", s32Ret);
        goto RETURN_FREE;
    }

    MI_SYS_FlushInvCache(pu8VirBuf, u32OutputBufSize);
    *got_frame = 1;
    memcpy(s->picture_ptr->data[0], pu8VirBuf, avctx->width * avctx->height * 2);
    MI_SYS_Munmap(pu8VirBuf, u32OutputBufSize);
    MI_SYS_MMA_Free(0, stOutputBuf.phyAddr[0]);
    return avpkt->size;
RETURN_FREE:
    MI_SYS_MMA_Free(0, stOutputBuf.phyAddr[0]);
    return -1;
}
#else
static int ss_mjpeg_decode_frame(AVCodecContext *avctx, void *data,
                                      int *got_frame, AVPacket *avpkt)
{
    AVFrame *frame = data;
    SsMjpegContext *s = avctx->priv_data;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U8 *pu8Buf = avpkt->data;
    MI_U32 u32RequiredLength = avpkt->size;
    MI_JPD_StreamBuf_t stRetStreamBuf;
    MI_S32 s32TimeOutMs = 30; // timeout is 30ms
    MI_SYS_BUF_HANDLE bufHandle;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_ChnPort_t stChnPort;

    *got_frame = 0;
    stChnPort.eModId = E_MI_MODULE_ID_SCL;
    stChnPort.u32DevId = s->SclDev;
    stChnPort.u32ChnId = s->SclChn;
    stChnPort.u32PortId = s->SclPort;

    u32RequiredLength = get_jpeg_size(avpkt);
    if (u32RequiredLength == 0)
        return -1;

    while (*got_frame == 0) {
        s32Ret = MI_JPD_GetStreamBuf(s->JpdDev, s->JpdChn, u32RequiredLength, &stRetStreamBuf, s32TimeOutMs);
        if (MI_SUCCESS != s32Ret) {
            // printf("MI_JPD_GetStreamBuf fail [%x]\n", s32Ret);
            continue;
        }

        memcpy(stRetStreamBuf.pu8HeadVirtAddr, pu8Buf, JPD_MIN(stRetStreamBuf.u32HeadLength, u32RequiredLength));

        if (stRetStreamBuf.u32HeadLength + stRetStreamBuf.u32TailLength < u32RequiredLength) {
            // something wrong happen
            printf("MI_JPD_GetStreamBuf return wrong value: HeadLen%u TailLen%u RequiredLength%u\n",
                   stRetStreamBuf.u32HeadLength, stRetStreamBuf.u32TailLength, u32RequiredLength);
            s32Ret = MI_JPD_DropStreamBuf(s->JpdDev, s->JpdChn, &stRetStreamBuf);
            if (MI_SUCCESS != s32Ret)
            {
                printf("chn%u MI_JPD_DropStreamBuf failed, s32Ret: 0x%x.\n", s->JpdChn, s32Ret);
                continue;
            }
        } else if (stRetStreamBuf.u32TailLength > 0) {
            memcpy(stRetStreamBuf.pu8TailVirtAddr, pu8Buf + stRetStreamBuf.u32HeadLength,
                   JPD_MIN(stRetStreamBuf.u32TailLength, u32RequiredLength - stRetStreamBuf.u32HeadLength));
        }

        stRetStreamBuf.u32ContentLength = u32RequiredLength;
        s32Ret = MI_JPD_PutStreamBuf(s->JpdDev,s->JpdChn, &stRetStreamBuf);
        if (MI_SUCCESS != s32Ret) {
            printf("chn%u MI_JPD_PutStreamBuf failed, s32Ret: 0x%x.\n", s->JpdChn, s32Ret);
            continue;
        }

        if (ss_mjpeg_polling_scl(s) < 0) {
            continue;
        }

        s32Ret = MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &bufHandle);
        if (MI_SUCCESS == s32Ret) {
            MI_U32 ysize;

            avctx->width = stBufInfo.stFrameData.u32Stride[0];
            avctx->height = stBufInfo.stFrameData.u16Height;

            av_frame_unref(s->picture_ptr);
            if (ff_get_buffer(avctx, s->picture_ptr, AV_GET_BUFFER_FLAG_REF) < 0)
                goto RETURN_FREE;

            if (av_frame_ref(frame, s->picture_ptr) < 0) {
                printf("av_frame_ref fail\n");
                goto RETURN_FREE;
            }

            ysize  = avctx->width * avctx->height;
            memcpy(s->picture_ptr->data[0], stBufInfo.stFrameData.pVirAddr[0], ysize);
            memcpy(s->picture_ptr->data[1], stBufInfo.stFrameData.pVirAddr[1], ysize / 2);

            *got_frame = 1;
            MI_SYS_ChnOutputPortPutBuf(bufHandle);
        } else {
            return AVERROR(EAGAIN);
        }
    }

    return avpkt->size;

RETURN_FREE:
    MI_SYS_ChnOutputPortPutBuf(bufHandle);
    return -1;
}
#endif

AVCodec ff_ssmjpeg_decoder = {
    .name                  = "ssmjpeg",
    .long_name             = NULL_IF_CONFIG_SMALL("Sstar MJPEG"),
    .type                  = AVMEDIA_TYPE_VIDEO,
    .id                    = AV_CODEC_ID_MJPEG,
    .priv_data_size        = sizeof(SsMjpegContext),
    .init                  = ss_mjpeg_decode_init,
    .close                 = ss_mjpeg_decode_free,
    .decode                = ss_mjpeg_decode_frame,
    .capabilities          = AV_CODEC_CAP_DR1,
};