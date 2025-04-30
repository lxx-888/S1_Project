/*
 * module_rtspclient.cpp- Sigmastar
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
#include <sys/queue.h>
#include <pthread.h>
#include "module_config.h"
#include "module_common.h"
#include "module_vdec.h"
#include "module_rtspclient.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
#include "libavformat/avformat.h"
#ifdef __cplusplus
}
#endif //__cplusplus

#if (CARDV_RTSP_INPUT_ENABLE)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct
{
    pthread_t               pid;
    MI_BOOL                 bThreadExit;
    CarDV_RtspClientAttr_t *pstRtspAttr;
    AVCodecID               eEncodeType;
    MI_U8 *                 pu8EsBuffer;
    MI_PHY                  u64EsPhyBuffer;
    MI_U32                  u32WriteOffset;
} CarDV_RtspInternalAttr_t;

typedef struct
{
    MI_U32             u32WriteOffset;
    MI_U32             u32FrameSize;
    MI_U64             u64Pts;
    MI_U32             u32Poc;
    MI_VENC_DataType_t stDataType;
} CarDV_RtspStream_t;

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

extern MI_S32 g_s32SocId;

CarDV_RtspClientAttr_t   g_stRtspAttr[RTSP_MAX_STREAM_CNT]         = {0};
CarDV_RtspInternalAttr_t g_stRtspInternalAttr[RTSP_MAX_STREAM_CNT] = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

static MI_S32 _AllocEsBuffer(CarDV_RtspInternalAttr_t *pstRtspInternalAttr)
{
    MI_U8  u8MMAHeapName[32];
    MI_U32 u32EsBufSize = pstRtspInternalAttr->pstRtspAttr->u32EsBufSize;
    sprintf((char *)u8MMAHeapName, "#rtsp_es_%d", pstRtspInternalAttr - g_stRtspInternalAttr);
    ExecFunc(MI_SYS_MMA_Alloc(g_s32SocId, u8MMAHeapName, u32EsBufSize, &pstRtspInternalAttr->u64EsPhyBuffer),
             MI_SUCCESS);
    ExecFunc(MI_SYS_Mmap(pstRtspInternalAttr->u64EsPhyBuffer, u32EsBufSize, (void **)&pstRtspInternalAttr->pu8EsBuffer,
                         TRUE),
             MI_SUCCESS);

    return 0;
}

static MI_S32 _FreeEsBuffer(CarDV_RtspInternalAttr_t *pstRtspInternalAttr)
{
    if (pstRtspInternalAttr->pu8EsBuffer)
    {
        ExecFunc(MI_SYS_Munmap(pstRtspInternalAttr->pu8EsBuffer, pstRtspInternalAttr->pstRtspAttr->u32EsBufSize),
                 MI_SUCCESS);
        pstRtspInternalAttr->pu8EsBuffer = NULL;
    }
    if (pstRtspInternalAttr->u64EsPhyBuffer)
    {
        ExecFunc(MI_SYS_MMA_Free(g_s32SocId, pstRtspInternalAttr->u64EsPhyBuffer), MI_SUCCESS);
        pstRtspInternalAttr->u64EsPhyBuffer = 0;
    }
    return 0;
}

static MI_S32 _SendStream(CarDV_RtspInternalAttr_t *pstRtspInternalAttr, AVPacket *pAvPkt, MI_U32 u32Poc)
{
    MI_S32                  s32Ret = 0;
    MI_U8 *                 puBuf  = NULL;
    CarDV_RtspStream_t      stRtspStream;
    CarDV_StreamLinkNode_t *pLinkNode;
    CarDV_RtspClientAttr_t *pstRtspAttr = pstRtspInternalAttr->pstRtspAttr;

    if (pstRtspInternalAttr->pu8EsBuffer == NULL)
    {
        _AllocEsBuffer(pstRtspInternalAttr);
    }

    puBuf = pstRtspInternalAttr->pu8EsBuffer;
    if (pAvPkt->data == NULL || pAvPkt->size == 0 || puBuf == NULL)
    {
        printf("send stream fail\n");
        return -1;
    }

    // Out of buffer size
    if (pstRtspInternalAttr->u32WriteOffset + pAvPkt->size > pstRtspAttr->u32EsBufSize)
    {
        pstRtspInternalAttr->u32WriteOffset = 0;
    }

    memcpy(puBuf + pstRtspInternalAttr->u32WriteOffset, pAvPkt->data, pAvPkt->size);
    MI_SYS_FlushInvCache(puBuf + pstRtspInternalAttr->u32WriteOffset, pAvPkt->size);

    switch (pstRtspInternalAttr->eEncodeType)
    {
    case AV_CODEC_ID_HEVC:
        stRtspStream.stDataType.eH265EType =
            (pAvPkt->flags & AV_PKT_FLAG_KEY) ? E_MI_VENC_H265E_NALU_ISLICE : E_MI_VENC_H265E_NALU_PSLICE;
        break;
    case AV_CODEC_ID_H264:
        stRtspStream.stDataType.eH264EType =
            (pAvPkt->flags & AV_PKT_FLAG_KEY) ? E_MI_VENC_H264E_NALU_ISLICE : E_MI_VENC_H264E_NALU_PSLICE;
        break;
    default:
        printf("Not match CODEC type\n");
        return -1;
    }

    stRtspStream.u32FrameSize   = pAvPkt->size;
    stRtspStream.u32WriteOffset = pstRtspInternalAttr->u32WriteOffset;
    stRtspStream.u64Pts         = pAvPkt->pts;
    stRtspStream.u32Poc         = u32Poc;
    SLIST_FOREACH(pLinkNode, pstRtspAttr->linkHead, next)
    {
        if (pLinkNode->bStart)
        {
            if (pLinkNode->bRequstIDR
                && !(stRtspStream.stDataType.eH264EType == E_MI_VENC_H264E_NALU_ISLICE
                     || stRtspStream.stDataType.eH265EType == E_MI_VENC_H265E_NALU_ISLICE))
            {
                continue;
            }

            pLinkNode->bRequstIDR = FALSE;
            s32Ret                = write(pLinkNode->s32Sock, &stRtspStream, sizeof(CarDV_RtspStream_t));
            if (s32Ret != sizeof(CarDV_RtspStream_t))
            {
                perror("write frame error: ");
                return -2;
            }
        }
    }
    pstRtspInternalAttr->u32WriteOffset += pAvPkt->size;
    return s32Ret;
}

static void *_RtspConnectThread(void *arg)
{
    MI_S32                    i;
    MI_S32                    s32Ret              = 0;
    MI_S32                    s32VideoIdx         = -1;
    AVDictionary *            opts                = NULL;
    AVFormatContext *         pFormatCtx          = NULL;
    AVRational                src_tb              = {1, 90000};
    AVRational                dst_tb              = {1, 1000000};
    MI_U64                    u64BasePts          = 0;
    MI_U32                    u32Poc              = 0;
    CarDV_RtspInternalAttr_t *pstRtspInternalAttr = (CarDV_RtspInternalAttr_t *)arg;

RETRY_CONNECT:
    pFormatCtx = avformat_alloc_context();
    if (pFormatCtx == NULL)
    {
        printf("alloc context pointer failed\n");
        goto out;
    }

    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "stimeout", "3000000", 0);
    s32Ret = avformat_open_input(&pFormatCtx, pstRtspInternalAttr->pstRtspAttr->cUrl, NULL, &opts);
    if (s32Ret != 0)
    {
        printf("[%s] open error :%d\n", pstRtspInternalAttr->pstRtspAttr->cUrl, s32Ret);
        goto out;
    }

    MI_SYS_GetCurPts(g_s32SocId, &u64BasePts);
    av_dump_format(pFormatCtx, 0, pstRtspInternalAttr->pstRtspAttr->cUrl, 0);

    for (i = 0; i < (MI_S32)pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            s32VideoIdx                      = i;
            pstRtspInternalAttr->eEncodeType = pFormatCtx->streams[i]->codecpar->codec_id;
            break;
        }
    }

    while (!pstRtspInternalAttr->bThreadExit)
    {
        AVPacket pkt1, *pkt = &pkt1;
        s32Ret = av_read_frame(pFormatCtx, pkt);
        if (s32Ret < 0)
        {
            printf("[%s] read error :%d\n", pstRtspInternalAttr->pstRtspAttr->cUrl, s32Ret);
            break;
        }

        if (pkt->stream_index == s32VideoIdx)
        {
            if (pkt->pts == AV_NOPTS_VALUE)
            {
                pkt->pts = 0;
            }

            /* rescale output packet timestamp values from stream timebase to MI_SYS */
            av_packet_rescale_ts(pkt, src_tb, dst_tb);
            pkt->pts += u64BasePts;

            // Calc POC
            if (pkt->flags & AV_PKT_FLAG_KEY)
            {
                u32Poc = 0;
            }
            else
            {
                u32Poc++;
            }

            if (!SLIST_EMPTY(pstRtspInternalAttr->pstRtspAttr->linkHead))
            {
                _SendStream(pstRtspInternalAttr, pkt, u32Poc);
            }

#if (CARDV_VDEC_ENABLE)
            if (pstRtspInternalAttr->pstRtspAttr->bUseVdec)
            {
                CarDV_VdecDecodeStream(pstRtspInternalAttr->pstRtspAttr->u32VdecChn, pkt->data, pkt->size, pkt->pts);
            }
#endif
        }

        av_packet_unref(pkt);
    }

out:
    _FreeEsBuffer(pstRtspInternalAttr);

    if (opts)
    {
        av_dict_free(&opts);
    }

    if (pFormatCtx)
    {
        avformat_close_input(&pFormatCtx);
    }

    if (s32Ret < 0 && !pstRtspInternalAttr->bThreadExit)
    {
        goto RETRY_CONNECT;
    }

    return 0;
}

MI_S32 CarDV_RtspClientInit(MI_U8 u8RtspChn)
{
    CarDV_RtspClientAttr_t *  pstRtspAttr;
    CarDV_RtspInternalAttr_t *pstRtspInternalAttr;
    char                      taskName[16];

    if (u8RtspChn >= RTSP_MAX_STREAM_CNT)
    {
        return -1;
    }

    pstRtspAttr         = &g_stRtspAttr[u8RtspChn];
    pstRtspInternalAttr = &g_stRtspInternalAttr[u8RtspChn];
    if (pstRtspAttr->bUsed)
    {
        CarDV_RtspClientUnInit(u8RtspChn);

        pstRtspInternalAttr->pstRtspAttr = pstRtspAttr;
        pstRtspInternalAttr->bThreadExit = FALSE;
        pthread_create(&pstRtspInternalAttr->pid, NULL, _RtspConnectThread,
                       (CarDV_RtspInternalAttr_t *)pstRtspInternalAttr);
        sprintf(taskName, "rtspC%d", u8RtspChn);
        pthread_setname_np(pstRtspInternalAttr->pid, taskName);
        pstRtspAttr->bEnable = TRUE;
    }

    return 0;
}

MI_S32 CarDV_RtspClientUnInit(MI_U8 u8RtspChn)
{
    CarDV_RtspClientAttr_t *  pstRtspAttr;
    CarDV_RtspInternalAttr_t *pstRtspInternalAttr;

    if (u8RtspChn >= RTSP_MAX_STREAM_CNT)
    {
        return -1;
    }

    pstRtspAttr         = &g_stRtspAttr[u8RtspChn];
    pstRtspInternalAttr = &g_stRtspInternalAttr[u8RtspChn];
    if (pstRtspAttr->bUsed && pstRtspAttr->bEnable)
    {
        if (pstRtspInternalAttr->pid)
        {
            pstRtspInternalAttr->bThreadExit = TRUE;
            pthread_join(pstRtspInternalAttr->pid, NULL);
            pstRtspInternalAttr->pid = 0;
        }
        pstRtspAttr->bEnable = FALSE;
    }

    return 0;
}

MI_S32 CarDV_RtspClientAddLink(CarDV_StreamLinkNode_t *pLinkNode)
{
    if (pLinkNode)
    {
        MI_U8                   u8RtspChn   = pLinkNode->u8Chn;
        CarDV_RtspClientAttr_t *pstRtspAttr = &g_stRtspAttr[u8RtspChn];

        SLIST_INSERT_HEAD(pstRtspAttr->linkHead, pLinkNode, next);
        if (pstRtspAttr->bEnable)
        {
            CarDV_RtspInternalAttr_t *pstRtspInternalAttr = &g_stRtspInternalAttr[u8RtspChn];
            switch (pstRtspInternalAttr->eEncodeType)
            {
            case AV_CODEC_ID_HEVC:
                pLinkNode->pEncoder->setCodec(VE_H265);
                break;
            case AV_CODEC_ID_H264:
                pLinkNode->pEncoder->setCodec(VE_H264);
                break;
            default:
                printf("Not match CODEC type\n");
                break;
            }
        }

        return 0;
    }

    return -1;
}

MI_S32 CarDV_RtspClientRemoveLink(CarDV_StreamLinkNode_t *pLinkNode)
{
    if (pLinkNode)
    {
        MI_U8                   u8RtspChn   = pLinkNode->u8Chn;
        CarDV_RtspClientAttr_t *pstRtspAttr = &g_stRtspAttr[u8RtspChn];

        SLIST_REMOVE(pstRtspAttr->linkHead, pLinkNode, CarDV_StreamLinkNode_t, next);
        return 0;
    }

    return -1;
}

MI_S32 CarDV_RtspClientGetStream(MI_U8 u8RtspChn, MI_S32 s32Sock, MI_VENC_Stream_t *pstStream)
{
    MI_S32                    s32Ret;
    CarDV_RtspStream_t        stRtspStream;
    CarDV_RtspInternalAttr_t *pstRtspInternalAttr = &g_stRtspInternalAttr[u8RtspChn];

    s32Ret = read(s32Sock, &stRtspStream, sizeof(CarDV_RtspStream_t));
    if (s32Ret != sizeof(CarDV_RtspStream_t))
    {
        return -1;
    }

    pstStream->u32PackCount          = 1;
    pstStream->pstPack[0].pu8Addr    = pstRtspInternalAttr->pu8EsBuffer + stRtspStream.u32WriteOffset;
    pstStream->pstPack[0].phyAddr    = pstRtspInternalAttr->u64EsPhyBuffer + stRtspStream.u32WriteOffset;
    pstStream->pstPack[0].u32Len     = stRtspStream.u32FrameSize;
    pstStream->pstPack[0].u64PTS     = stRtspStream.u64Pts;
    pstStream->pstPack[0].s32PocNum  = stRtspStream.u32Poc;
    pstStream->pstPack[0].stDataType = stRtspStream.stDataType;
    pstStream->pstPack[0].bFrameEnd  = TRUE;

    return 0;
}

MI_S32 CarDV_RtspClientStartEncoder(MI_U8 u8RtspChn, MI_VideoEncoder *pEncoder)
{
    CarDV_StreamLinkNode_t *pLinkNode;
    CarDV_RtspClientAttr_t *pstRtspAttr = &g_stRtspAttr[u8RtspChn];

    SLIST_FOREACH(pLinkNode, pstRtspAttr->linkHead, next)
    {
        if (pLinkNode->pEncoder == pEncoder)
        {
            pLinkNode->bRequstIDR = pEncoder->getCodec() == VE_JPG ? FALSE : TRUE;
            pLinkNode->bStart     = TRUE;
            break;
        }
    }

    return 0;
}

MI_S32 CarDV_RtspClientStopEncoder(MI_U8 u8RtspChn, MI_VideoEncoder *pEncoder)
{
    CarDV_StreamLinkNode_t *pLinkNode;
    CarDV_RtspClientAttr_t *pstRtspAttr = &g_stRtspAttr[u8RtspChn];

    SLIST_FOREACH(pLinkNode, pstRtspAttr->linkHead, next)
    {
        if (pLinkNode->pEncoder == pEncoder)
        {
            pLinkNode->bStart = FALSE;
            break;
        }
    }

    return 0;
}

MI_S32 CarDV_RtspClientRequestIDR(MI_U8 u8RtspChn, MI_VideoEncoder *pEncoder)
{
    CarDV_StreamLinkNode_t *pLinkNode;
    CarDV_RtspClientAttr_t *pstRtspAttr = &g_stRtspAttr[u8RtspChn];

    SLIST_FOREACH(pLinkNode, pstRtspAttr->linkHead, next)
    {
        if (pLinkNode->pEncoder == pEncoder)
        {
            pLinkNode->bRequstIDR = TRUE;
            return 0;
        }
    }

    return -1;
}
#endif