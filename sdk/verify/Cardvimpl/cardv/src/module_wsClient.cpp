/*
 * module_wsClient.cpp- Sigmastar
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
#include "module_ws.h"
#include "civetweb.h"

#if (CARDV_WS_INPUT_ENABLE)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct
{
    pthread_t             pid;
    MI_BOOL               bThreadExit;
    struct mg_connection *pClientConn;
    CarDV_WsConnStatus_e  eConnStatus;
    MI_U64                u64LastPingPts;
    CarDV_WsClientAttr_t *pstWsAttr;
    MI_U8 *               pu8EsBuffer;
    MI_PHY                u64EsPhyBuffer;
    MI_U32                u32WriteOffset;
} CarDV_WsInternalAttr_t;

typedef struct
{
    MI_U32             u32WriteOffset;
    MI_U32             u32FrameSize;
    MI_U64             u64Pts;
    MI_U32             u32Poc;
    MI_U32             u32Seq;
    MI_VENC_DataType_t stDataType;
} CarDV_WsStream_t;

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

extern MI_S32 g_s32SocId;

CarDV_WsClientAttr_t   g_stWsAttr[WS_MAX_STREAM_CNT]         = {0};
CarDV_WsInternalAttr_t g_stWsInternalAttr[WS_MAX_STREAM_CNT] = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

static MI_S32 _AllocEsBuffer(CarDV_WsInternalAttr_t *pstWsInternalAttr)
{
    MI_U8  u8MMAHeapName[32];
    MI_U32 u32EsBufSize = pstWsInternalAttr->pstWsAttr->u32EsBufSize;
    sprintf((char *)u8MMAHeapName, "#ws_es_%d", pstWsInternalAttr - g_stWsInternalAttr);
    ExecFunc(MI_SYS_MMA_Alloc(g_s32SocId, u8MMAHeapName, u32EsBufSize, &pstWsInternalAttr->u64EsPhyBuffer), MI_SUCCESS);
    ExecFunc(
        MI_SYS_Mmap(pstWsInternalAttr->u64EsPhyBuffer, u32EsBufSize, (void **)&pstWsInternalAttr->pu8EsBuffer, TRUE),
        MI_SUCCESS);

    return 0;
}

static MI_S32 _FreeEsBuffer(CarDV_WsInternalAttr_t *pstWsInternalAttr)
{
    if (pstWsInternalAttr->pu8EsBuffer)
    {
        ExecFunc(MI_SYS_Munmap(pstWsInternalAttr->pu8EsBuffer, pstWsInternalAttr->pstWsAttr->u32EsBufSize), MI_SUCCESS);
        pstWsInternalAttr->pu8EsBuffer = NULL;
    }
    if (pstWsInternalAttr->u64EsPhyBuffer)
    {
        ExecFunc(MI_SYS_MMA_Free(g_s32SocId, pstWsInternalAttr->u64EsPhyBuffer), MI_SUCCESS);
        pstWsInternalAttr->u64EsPhyBuffer = 0;
    }
    return 0;
}

static MI_S32 _SendStream(CarDV_WsInternalAttr_t *pstWsInternalAttr, char *data, MI_U32 u32dataLen)
{
    MI_S32                  s32Ret = 0;
    MI_U8 *                 puBuf  = NULL;
    CarDV_WsStream_t        stWsStream;
    CarDV_StreamLinkNode_t *pLinkNode;
    CarDV_WsClientAttr_t *  pstWsAttr       = pstWsInternalAttr->pstWsAttr;
    CarDV_WsStreamHeader_t *pstStreamHeader = (CarDV_WsStreamHeader_t *)data;
    char *                  ps8Buf          = data + sizeof(CarDV_WsStreamHeader_t);

    if (pstWsInternalAttr->pu8EsBuffer == NULL)
    {
        _AllocEsBuffer(pstWsInternalAttr);
    }

    puBuf = pstWsInternalAttr->pu8EsBuffer;

    // Out of buffer size
    if (pstWsInternalAttr->u32WriteOffset + pstStreamHeader->u32FrameSize > pstWsAttr->u32EsBufSize)
    {
        pstWsInternalAttr->u32WriteOffset = 0;
    }

    memcpy(puBuf + pstWsInternalAttr->u32WriteOffset, ps8Buf, pstStreamHeader->u32FrameSize);
    MI_SYS_FlushInvCache(puBuf + pstWsInternalAttr->u32WriteOffset, pstStreamHeader->u32FrameSize);

    stWsStream.stDataType     = pstStreamHeader->stDataType;
    stWsStream.u32FrameSize   = pstStreamHeader->u32FrameSize;
    stWsStream.u32WriteOffset = pstWsInternalAttr->u32WriteOffset;
    stWsStream.u64Pts         = pstStreamHeader->u64Pts;
    stWsStream.u32Poc         = pstStreamHeader->u32Poc;
    stWsStream.u32Seq         = pstStreamHeader->u32Seq;
    SLIST_FOREACH(pLinkNode, pstWsAttr->linkHead, next)
    {
        if (pLinkNode->bStart)
        {
            if (pLinkNode->bRequstIDR
                && !(stWsStream.stDataType.eH264EType == E_MI_VENC_H264E_NALU_ISLICE
                     || stWsStream.stDataType.eH265EType == E_MI_VENC_H265E_NALU_ISLICE))
            {
                continue;
            }

            pLinkNode->bRequstIDR = FALSE;
            s32Ret                = write(pLinkNode->s32Sock, &stWsStream, sizeof(CarDV_WsStream_t));
            if (s32Ret != sizeof(CarDV_WsStream_t))
            {
                perror("write frame error: ");
                return -2;
            }
        }
    }
    pstWsInternalAttr->u32WriteOffset += pstStreamHeader->u32FrameSize;
    return s32Ret;
}

/* Callback for handling data received from the server */
static int websocket_client_data_handler(struct mg_connection *conn, int flags, char *data, size_t data_len,
                                         void *user_data)
{
    /* We may get some different message types (websocket opcodes).
     * We will handle these messages differently. */
    int is_bin   = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_BINARY);
    int is_ping  = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_PING);
    int is_pong  = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_PONG);
    int is_close = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE);

    /* Check if we got a websocket PING request */
    if (is_ping)
    {
        /* PING requests are to check if the connection is broken.
         * They should be replied with a PONG with the same data.
         */
        // printf("handler ping conn = %p\n", conn);
        CarDV_WsInternalAttr_t *pstWsInternalAttr = (CarDV_WsInternalAttr_t *)user_data;
        MI_SYS_GetCurPts(g_s32SocId, &pstWsInternalAttr->u64LastPingPts);
        mg_websocket_client_write(conn, MG_WEBSOCKET_OPCODE_PONG, data, data_len);
        return 1;
    }

    /* Check if we got a websocket PONG message */
    if (is_pong)
    {
        /* A PONG message may be a response to our PING, but
         * it is also allowed to send unsolicited PONG messages
         * send by the server to check some lower level TCP
         * connections. Just ignore all kinds of PONGs. */
        // printf("handler pong conn = %p\n", conn);
        return 1;
    }

    /* Another option would be BINARY data. */
    if (is_bin)
    {
        CarDV_WsInternalAttr_t *pstWsInternalAttr = (CarDV_WsInternalAttr_t *)user_data;
        if (!SLIST_EMPTY(pstWsInternalAttr->pstWsAttr->linkHead))
        {
            _SendStream(pstWsInternalAttr, data, data_len);
        }

#if (CARDV_VDEC_ENABLE)
        if (pstWsInternalAttr->pstWsAttr->bUseVdec)
        {
            CarDV_WsStreamHeader_t *pstStreamHeader = (CarDV_WsStreamHeader_t *)data;
            MI_U8 *                 pu8Buf          = (MI_U8 *)data + sizeof(CarDV_WsStreamHeader_t);
            CarDV_VdecDecodeStream(pstWsInternalAttr->pstWsAttr->u32VdecChn, pu8Buf, pstStreamHeader->u32FrameSize,
                                   pstStreamHeader->u64Pts);
        }
#endif
    }

    /* It could be a CLOSE message as well. */
    if (is_close)
    {
        printf("close conn = %p\n", conn);
        return 0;
    }

    /* Return 1 to keep the connection open */
    return 1;
}

/* Callback for handling a close message received from the server */
static void websocket_client_close_handler(const struct mg_connection *conn, void *user_data)
{
    CarDV_WsInternalAttr_t *pstWsInternalAttr = (CarDV_WsInternalAttr_t *)user_data;
    _FreeEsBuffer(pstWsInternalAttr);
    pstWsInternalAttr->eConnStatus = E_CONN_STATUS_CLOSED;
    printf("disconnect = %p\n", conn);
}

static void *_WsConnectThread(void *arg)
{
    CarDV_WsInternalAttr_t *pstWsInternalAttr = (CarDV_WsInternalAttr_t *)arg;
    CarDV_WsClientAttr_t *  pstWsAttr         = pstWsInternalAttr->pstWsAttr;
    MI_U8                   u8WsChn           = pstWsInternalAttr - &g_stWsInternalAttr[0];
    MI_U64                  u64Pts;
    char                    err_buf[128] = {0};
    char *                  host         = pstWsAttr->cHost;
    char *                  path         = pstWsAttr->cPath;
    int                     port         = pstWsAttr->u32Port;
    const int               secure       = 0;

RETRY_CONNECT:
    pstWsInternalAttr->eConnStatus = E_CONN_STATUS_INVAILD;
    /* Connect to the given WS or WSS (WS secure) server */
    pstWsInternalAttr->pClientConn =
        mg_connect_websocket_client(host, port, secure, err_buf, sizeof(err_buf), path, NULL,
                                    websocket_client_data_handler, websocket_client_close_handler, pstWsInternalAttr);
    if (pstWsInternalAttr->pClientConn == NULL)
    {
        if (pstWsInternalAttr->bThreadExit)
        {
            goto EXIT;
        }
        sleep(1);
        goto RETRY_CONNECT;
    }

    printf("connect = %p, chn = %d\n", pstWsInternalAttr->pClientConn, u8WsChn);
    pstWsInternalAttr->eConnStatus = E_CONN_STATUS_CONNECTED;
    MI_SYS_GetCurPts(g_s32SocId, &pstWsInternalAttr->u64LastPingPts);

    CarDV_WsClientSyncBasePts(u8WsChn);
    CarDV_WsClientSyncDate(u8WsChn);

#if (CARDV_VDEC_ENABLE)
    if (pstWsAttr->bUseVdec)
    {
        CarDV_WsClientStartEncoder(u8WsChn, NULL);
    }
    else
#endif
    {
        CarDV_StreamLinkNode_t *pLinkNode;
        SLIST_FOREACH(pLinkNode, pstWsAttr->linkHead, next)
        {
            if (pLinkNode && pLinkNode->bStart)
            {
                CarDV_WsClientStartEncoder(u8WsChn, pLinkNode->pEncoder);
                break;
            }
        }
    }

    while (!pstWsInternalAttr->bThreadExit)
    {
        sleep(1);
        if (pstWsInternalAttr->eConnStatus == E_CONN_STATUS_CLOSED)
        {
            mg_close_connection(pstWsInternalAttr->pClientConn);
            pstWsInternalAttr->pClientConn = NULL;
            goto RETRY_CONNECT;
        }

        MI_SYS_GetCurPts(g_s32SocId, &u64Pts);
        if (u64Pts - pstWsInternalAttr->u64LastPingPts > PING_PONG_TIMEOUT_MS * 2000)
        {
            mg_close_connection(pstWsInternalAttr->pClientConn);
            pstWsInternalAttr->pClientConn = NULL;
            goto RETRY_CONNECT;
        }
    }

EXIT:
    if (pstWsInternalAttr->pClientConn)
    {
        /* Close client connection */
        mg_close_connection(pstWsInternalAttr->pClientConn);
        pstWsInternalAttr->pClientConn = NULL;
    }

    _FreeEsBuffer(pstWsInternalAttr);
    return 0;
}

MI_S32 CarDV_WsClientSyncBasePts(MI_U8 u8WsChn)
{
    CarDV_WsInternalAttr_t *pstWsInternalAttr = &g_stWsInternalAttr[u8WsChn];
    if (pstWsInternalAttr->pClientConn)
    {
        char                     data[32];
        CarDV_WsRequestHeader_t *pstRequestHeader = (CarDV_WsRequestHeader_t *)data;
        MI_U64                   u64BasePts;
        MI_SYS_GetCurPts(g_s32SocId, &u64BasePts);
        pstRequestHeader->u32RequestType = E_REQ_TYPE_SYNC_PTS;
        memcpy(data + sizeof(CarDV_WsRequestHeader_t), &u64BasePts, sizeof(MI_U64));
        mg_websocket_client_write(pstWsInternalAttr->pClientConn, MG_WEBSOCKET_OPCODE_BINARY, data,
                                  sizeof(CarDV_WsRequestHeader_t) + sizeof(MI_U64));
    }

    return 0;
}

MI_S32 CarDV_WsClientSyncDate(MI_U8 u8WsChn)
{
    CarDV_WsInternalAttr_t *pstWsInternalAttr = &g_stWsInternalAttr[u8WsChn];
    if (pstWsInternalAttr->pClientConn)
    {
        char                     data[32]         = {0};
        CarDV_WsRequestHeader_t *pstRequestHeader = (CarDV_WsRequestHeader_t *)data;
        pstRequestHeader->u32RequestType          = E_REQ_TYPE_SYNC_DATE;

        struct timeval * tv = (struct timeval *)(data + sizeof(CarDV_WsRequestHeader_t));
        struct timezone *tz = (struct timezone *)(data + sizeof(CarDV_WsRequestHeader_t) + sizeof(struct timeval));
        gettimeofday(tv, tz);
        mg_websocket_client_write(pstWsInternalAttr->pClientConn, MG_WEBSOCKET_OPCODE_BINARY, data,
                                  sizeof(CarDV_WsRequestHeader_t) + sizeof(struct timeval) + sizeof(struct timezone));
    }

    return 0;
}

MI_S32 CarDV_WsClientStartEncoder(MI_U8 u8WsChn, MI_VideoEncoder *pEncoder)
{
    CarDV_WsInternalAttr_t *pstWsInternalAttr = &g_stWsInternalAttr[u8WsChn];
    CarDV_WsClientAttr_t *  pstWsAttr         = &g_stWsAttr[u8WsChn];
    CarDV_StreamLinkNode_t *pLinkNode;
    SLIST_FOREACH(pLinkNode, pstWsAttr->linkHead, next)
    {
        if (pLinkNode->pEncoder == pEncoder)
        {
            pLinkNode->bRequstIDR = pEncoder->getCodec() == VE_JPG ? FALSE : TRUE;
            pLinkNode->bStart     = TRUE;
            break;
        }
    }

    if (pstWsInternalAttr->pClientConn)
    {
        CarDV_WsRequestHeader_t stRequestHeader;
        stRequestHeader.u32RequestType = E_REQ_TYPE_START_ENCODER;
        mg_websocket_client_write(pstWsInternalAttr->pClientConn, MG_WEBSOCKET_OPCODE_BINARY, (char *)&stRequestHeader,
                                  sizeof(CarDV_WsRequestHeader_t));
    }

    return 0;
}

MI_S32 CarDV_WsClientStopEncoder(MI_U8 u8WsChn, MI_VideoEncoder *pEncoder)
{
    CarDV_WsInternalAttr_t *pstWsInternalAttr = &g_stWsInternalAttr[u8WsChn];
    CarDV_WsClientAttr_t *  pstWsAttr         = &g_stWsAttr[u8WsChn];
    MI_BOOL                 bNeedStopEncoder  = TRUE;
    CarDV_StreamLinkNode_t *pLinkNode;
    SLIST_FOREACH(pLinkNode, pstWsAttr->linkHead, next)
    {
        if (pLinkNode->pEncoder == pEncoder)
        {
            pLinkNode->bStart = FALSE;
            break;
        }
    }

#if (CARDV_VDEC_ENABLE)
    if (pstWsInternalAttr->pstWsAttr->bUseVdec)
    {
        bNeedStopEncoder = FALSE;
    }
#endif
    else
    {
        // All linked encoder status is stop, than send stop cmd to server.
        SLIST_FOREACH(pLinkNode, pstWsAttr->linkHead, next)
        {
            if (pLinkNode->bStart)
            {
                bNeedStopEncoder = FALSE;
                break;
            }
        }
    }

    if (bNeedStopEncoder && pstWsInternalAttr->pClientConn)
    {
        CarDV_WsRequestHeader_t stRequestHeader;
        stRequestHeader.u32RequestType = E_REQ_TYPE_STOP_ENCODER;
        mg_websocket_client_write(pstWsInternalAttr->pClientConn, MG_WEBSOCKET_OPCODE_BINARY, (char *)&stRequestHeader,
                                  sizeof(CarDV_WsRequestHeader_t));
    }

    return 0;
}

MI_S32 CarDV_WsClientRequestIDR(MI_U8 u8WsChn, MI_VideoEncoder *pEncoder)
{
    CarDV_StreamLinkNode_t *pLinkNode;
    CarDV_WsClientAttr_t *  pstWsAttr         = &g_stWsAttr[u8WsChn];
    CarDV_WsInternalAttr_t *pstWsInternalAttr = &g_stWsInternalAttr[u8WsChn];

    SLIST_FOREACH(pLinkNode, pstWsAttr->linkHead, next)
    {
        if (pLinkNode->pEncoder == pEncoder)
        {
            pLinkNode->bRequstIDR = TRUE;
            if (pstWsInternalAttr->pClientConn)
            {
                CarDV_WsRequestHeader_t stRequestHeader;
                stRequestHeader.u32RequestType = E_REQ_TYPE_REQ_IDR;
                mg_websocket_client_write(pstWsInternalAttr->pClientConn, MG_WEBSOCKET_OPCODE_BINARY,
                                          (char *)&stRequestHeader, sizeof(CarDV_WsRequestHeader_t));
            }
            return 0;
        }
    }

    return -1;
}

MI_S32 CarDV_WsClientInit(MI_U8 u8WsChn)
{
    CarDV_WsClientAttr_t *  pstWsAttr;
    CarDV_WsInternalAttr_t *pstWsInternalAttr;
    char                    taskName[16];

    if (u8WsChn >= WS_MAX_STREAM_CNT)
    {
        return -1;
    }

    pstWsAttr         = &g_stWsAttr[u8WsChn];
    pstWsInternalAttr = &g_stWsInternalAttr[u8WsChn];
    if (pstWsAttr->bUsed && pstWsAttr->bEnable == FALSE)
    {
        mg_init_library(0);
        pstWsInternalAttr->pstWsAttr   = pstWsAttr;
        pstWsInternalAttr->bThreadExit = FALSE;
        pthread_create(&pstWsInternalAttr->pid, NULL, _WsConnectThread, (void *)pstWsInternalAttr);
        sprintf(taskName, "wsC%d", u8WsChn);
        pthread_setname_np(pstWsInternalAttr->pid, taskName);
        pstWsAttr->bEnable = TRUE;
    }

    return 0;
}

MI_S32 CarDV_WsClientUnInit(MI_U8 u8WsChn)
{
    CarDV_WsClientAttr_t *  pstWsAttr;
    CarDV_WsInternalAttr_t *pstWsInternalAttr;

    if (u8WsChn >= WS_MAX_STREAM_CNT)
    {
        return -1;
    }

    pstWsAttr         = &g_stWsAttr[u8WsChn];
    pstWsInternalAttr = &g_stWsInternalAttr[u8WsChn];
    if (pstWsAttr->bUsed && pstWsAttr->bEnable)
    {
        if (pstWsInternalAttr->pid)
        {
            pstWsInternalAttr->bThreadExit = TRUE;
            pthread_join(pstWsInternalAttr->pid, NULL);
            pstWsInternalAttr->pid = 0;
        }
        mg_exit_library();
        pstWsAttr->bEnable = FALSE;
    }

    return 0;
}

MI_S32 CarDV_WsClientAddLink(CarDV_StreamLinkNode_t *pLinkNode)
{
    if (pLinkNode)
    {
        MI_U8                 u8WsChn   = pLinkNode->u8Chn;
        CarDV_WsClientAttr_t *pstWsAttr = &g_stWsAttr[u8WsChn];

        SLIST_INSERT_HEAD(pstWsAttr->linkHead, pLinkNode, next);
        return 0;
    }

    return -1;
}

MI_S32 CarDV_WsClientRemoveLink(CarDV_StreamLinkNode_t *pLinkNode)
{
    if (pLinkNode)
    {
        MI_U8                 u8WsChn   = pLinkNode->u8Chn;
        CarDV_WsClientAttr_t *pstWsAttr = &g_stWsAttr[u8WsChn];

        SLIST_REMOVE(pstWsAttr->linkHead, pLinkNode, CarDV_StreamLinkNode_t, next);
        return 0;
    }

    return -1;
}

MI_S32 CarDV_WsClientGetStream(MI_U8 u8WsChn, MI_S32 s32Sock, MI_VENC_Stream_t *pstStream)
{
    MI_S32                  s32Ret;
    CarDV_WsStream_t        stWsStream;
    CarDV_WsInternalAttr_t *pstWsInternalAttr = &g_stWsInternalAttr[u8WsChn];

    s32Ret = read(s32Sock, &stWsStream, sizeof(CarDV_WsStream_t));
    if (s32Ret != sizeof(CarDV_WsStream_t))
    {
        return -1;
    }

    pstStream->u32PackCount          = 1;
    pstStream->u32Seq                = stWsStream.u32Seq;
    pstStream->pstPack[0].pu8Addr    = pstWsInternalAttr->pu8EsBuffer + stWsStream.u32WriteOffset;
    pstStream->pstPack[0].phyAddr    = pstWsInternalAttr->u64EsPhyBuffer + stWsStream.u32WriteOffset;
    pstStream->pstPack[0].u32Len     = stWsStream.u32FrameSize;
    pstStream->pstPack[0].u64PTS     = stWsStream.u64Pts;
    pstStream->pstPack[0].s32PocNum  = stWsStream.u32Poc;
    pstStream->pstPack[0].stDataType = stWsStream.stDataType;
    pstStream->pstPack[0].bFrameEnd  = TRUE;

    return 0;
}
#endif