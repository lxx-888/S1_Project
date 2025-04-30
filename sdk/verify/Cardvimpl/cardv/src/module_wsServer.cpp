/*
 * module_wsServer.cpp- Sigmastar
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
#include "module_config.h"
#include "module_ws.h"
#include "module_video.h"
#include "civetweb.h"

#if (CARDV_WS_OUTPUT_ENABLE)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct
{
    MI_BOOL               bEnable;
    MI_VideoEncoder *     pEncoder;
    struct mg_context *   pCtx;
    struct mg_connection *pConn;
    char *                ps8StreamBuf;
} CarDV_WsServer_t;

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

extern MI_S32    g_s32SocId;
CarDV_WsServer_t g_stWsServer[MAX_VIDEO_NUMBER] = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

/* Handler for new websocket connections. */
static int ws_connect_handler(const struct mg_connection *pConn, void *user_data)
{
    CarDV_WsServer_t *pstWsServer = (CarDV_WsServer_t *)user_data;
    MI_VideoEncoder * pEncoder    = pstWsServer->pEncoder;
    pstWsServer->pConn            = (struct mg_connection *)pConn;

    if (pEncoder->isEncoding())
    {
        /* reject client */
        return 1;
    }

    pstWsServer->ps8StreamBuf = (char *)MALLOC(2 * 1024 * 1024);
    if (pstWsServer->ps8StreamBuf == NULL)
    {
        /* reject client */
        return 1;
    }

    printf("connect = %p\n", pConn);
    return 0;
}

/* Handler indicating the client is ready to receive data. */
static void ws_ready_handler(struct mg_connection *pConn, void *user_data) {}

/* Handler indicating the client sent data to the server. */
static int ws_data_handler(struct mg_connection *pConn, int opcode, char *data, size_t datasize, void *user_data)
{
    CarDV_WsServer_t *pstWsServer = (CarDV_WsServer_t *)user_data;
    MI_VideoEncoder * pEncoder    = pstWsServer->pEncoder;
    int               ret         = 0;

    if ((opcode & MG_WEBSOCKET_OPCODE_BINARY) == 0)
    {
        printf("conn = %p, opcode = %x, len = %d, close connnect\n", pConn, opcode, datasize);
        return 0;
    }

    CarDV_WsRequestHeader_t *pstRequestHeader = (CarDV_WsRequestHeader_t *)data;
    switch (pstRequestHeader->u32RequestType)
    {
    case E_REQ_TYPE_REQ_IDR: {
        pEncoder->requestIDR(TRUE);
        printf("venc%u req IDR\n", pEncoder->getVencChn());
        goto SEND_PING;
    }
    case E_REQ_TYPE_START_ENCODER: {
        pEncoder->startVideoEncoder(TRUE);
        if (pEncoder->getCodec() != VE_JPG)
        {
            pEncoder->requestIDR(TRUE);
        }
        printf("venc%u req start encoder\n", pEncoder->getVencChn());
        goto SEND_PING;
    }
    case E_REQ_TYPE_STOP_ENCODER: {
        pEncoder->stopVideoEncoder();
        printf("venc%u req stop encoder\n", pEncoder->getVencChn());
        goto SEND_PING;
    }
    case E_REQ_TYPE_SYNC_PTS: {
        MI_U64 *pu64PtsBase = (MI_U64 *)(data + sizeof(CarDV_WsRequestHeader_t));
        MI_SYS_InitPtsBase(g_s32SocId, *pu64PtsBase);
        printf("venc%u req sync pts = %llu\n", pEncoder->getVencChn(), *pu64PtsBase);
        goto SEND_PING;
    }
    case E_REQ_TYPE_SYNC_DATE: {
        struct timeval * tv = (struct timeval *)(data + sizeof(CarDV_WsRequestHeader_t));
        struct timezone *tz = (struct timezone *)(data + sizeof(CarDV_WsRequestHeader_t) + sizeof(struct timeval));
        settimeofday(tv, tz);
        goto SEND_PING;
    }
    default:
        return 1;
    }

SEND_PING:
    // When the server receives the request, the ping-pong packet will be retimed. Therefore, it is necessary to
    // actively initiate ping-pong packet transmission here.
    ret = mg_websocket_write(pConn, MG_WEBSOCKET_OPCODE_PING, NULL, 0);
    if (ret <= 0)
    {
        printf("conn = %p, send PING fail, close connnect\n", pConn);
        return 0;
    }

    return 1;
}

/* Handler indicating the connection to the client is closing. */
static void ws_close_handler(const struct mg_connection *pConn, void *user_data)
{
    CarDV_WsServer_t *pstWsServer = (CarDV_WsServer_t *)user_data;
    MI_VideoEncoder * pEncoder    = pstWsServer->pEncoder;
    pEncoder->stopVideoEncoder();
    FREEIF(pstWsServer->ps8StreamBuf);
    printf("disconnect = %p\n", pConn);
}

MI_S32 CarDV_WsServerStart(MI_U32 u32Chn, MI_VideoEncoder *pEncoder)
{
    CarDV_WsServer_t *pstWsServer = &g_stWsServer[u32Chn];
    if (pstWsServer->bEnable)
    {
        return 0;
    }

    char port[16]    = {0};
    char timeout[16] = {0};
    sprintf(port, "%d", BASE_LISTEN_PORT + u32Chn);
    sprintf(timeout, "%d", PING_PONG_TIMEOUT_MS);
    const char *server_options[] = {"listening_ports",
                                    port,
                                    "num_threads",
                                    "1",
                                    "enable_websocket_ping_pong",
                                    "yes",
                                    "websocket_timeout_ms",
                                    timeout,
                                    "request_timeout_ms",
                                    "2000",
                                    NULL,
                                    NULL};

    /* Initialize CivetWeb library without OpenSSL/TLS support. */
    mg_init_library(0);

    /* Start the server using the advanced API. */
    struct mg_callbacks callbacks = {0};
    void *              user_data = pstWsServer;

    struct mg_init_data mg_start_init_data   = {0};
    mg_start_init_data.callbacks             = &callbacks;
    mg_start_init_data.user_data             = user_data;
    mg_start_init_data.configuration_options = server_options;

    struct mg_context *pCtx = mg_start2(&mg_start_init_data, NULL);
    if (!pCtx)
    {
        printf("Cannot start server\n");
        mg_exit_library();
        return -1;
    }

    pstWsServer->pCtx     = pCtx;
    pstWsServer->pEncoder = pEncoder;

    /* Register the websocket callback functions. */
    char url[16];
    sprintf(url, "/video%d", pEncoder->getVencChn());
    mg_set_websocket_handler(pCtx, url, ws_connect_handler, ws_ready_handler, ws_data_handler, ws_close_handler,
                             user_data);
    printf("ws server start ctx = %p, encoder = %p, url: %s\n", pCtx, pEncoder, url);
    pstWsServer->bEnable = TRUE;
    return 0;
}

int CarDV_WsServerStop(MI_U32 u32Chn)
{
    CarDV_WsServer_t * pstWsServer = &g_stWsServer[u32Chn];
    struct mg_context *pCtx        = pstWsServer->pCtx;
    MI_VideoEncoder *  pEncoder    = pstWsServer->pEncoder;
    pEncoder->stopVideoEncoder();
    FREEIF(pstWsServer->ps8StreamBuf);

    /* Stop server, disconnect all clients. Then deinitialize CivetWeb library.
     */
    printf("ws server stop %p\n", pCtx);
    mg_stop(pCtx);
    mg_exit_library();
    pstWsServer->bEnable = FALSE;
    return 0;
}

MI_S32 CarDV_WsServerSendStream(MI_U32 u32Chn, MI_VENC_Stream_t *pstStream, MI_U32 u32FrameSize)
{
    CarDV_WsServer_t *      pstWsServer   = &g_stWsServer[u32Chn];
    struct mg_connection *  pConn         = pstWsServer->pConn;
    CarDV_WsStreamHeader_t *pstStreamHead = (CarDV_WsStreamHeader_t *)pstWsServer->ps8StreamBuf;
    char *                  ps8Buf        = pstWsServer->ps8StreamBuf + sizeof(CarDV_WsStreamHeader_t);
    MI_VENC_Pack_t *        pstPack       = pstStream->pstPack;
    MI_U32                  u32Offset     = 0;

    pstStreamHead->u32FrameSize = u32FrameSize;
    pstStreamHead->u64Pts       = pstPack[pstStream->u32PackCount - 1].u64PTS;
    pstStreamHead->u32Poc       = pstPack[pstStream->u32PackCount - 1].s32PocNum;
    pstStreamHead->stDataType   = pstPack[0].stDataType;
    pstStreamHead->u32Seq       = pstStream->u32Seq;

    for (MI_U32 i = 0; i < STREAM_PACK_CNT && pstPack[i].u32Len != 0; i++)
    {
        memcpy(ps8Buf + u32Offset, pstPack[i].pu8Addr, pstPack[i].u32Len);
        u32Offset += pstPack[i].u32Len;
    }

    mg_websocket_write(pConn, MG_WEBSOCKET_OPCODE_BINARY, pstWsServer->ps8StreamBuf,
                       u32FrameSize + sizeof(CarDV_WsStreamHeader_t));
    return 0;
}

#endif