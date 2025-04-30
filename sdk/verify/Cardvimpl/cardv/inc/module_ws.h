/*
 * module_ws.h- Sigmastar
 *
 * Copyright (C) 2021 Sigmastar Technology Corp.
 *
 * Author: jeff.li <jeff.li@sigmastar.com.tw>
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

#ifndef _MODULE_WS_H_
#define _MODULE_WS_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "module_common.h"
#include "module_config.h"
#include "mid_VideoEncoder.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define WS_MAX_STREAM_CNT  (4)
#define WS_URL_HOST_LENGTH (64)
#define WS_URL_PATH_LENGTH (32)

#define BASE_LISTEN_PORT     (8080)
#define PING_PONG_TIMEOUT_MS (3000)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef enum
{
    E_CONN_STATUS_INVAILD = 0,
    E_CONN_STATUS_CONNECTED,
    E_CONN_STATUS_CLOSED,
} CarDV_WsConnStatus_e;

typedef enum
{
    E_REQ_TYPE_SYNC_PTS = 1,
    E_REQ_TYPE_SYNC_DATE,
    E_REQ_TYPE_REQ_IDR,
    E_REQ_TYPE_START_ENCODER,
    E_REQ_TYPE_STOP_ENCODER
} CarDV_WsReqType_e;

typedef struct
{
    MI_U32             u32FrameSize;
    MI_U64             u64Pts;
    MI_U32             u32Poc;
    MI_U32             u32Seq;
    MI_VENC_DataType_t stDataType;
} CarDV_WsStreamHeader_t;

typedef struct
{
    MI_U32 u32RequestType;
} CarDV_WsRequestHeader_t;

typedef struct
{
    MI_BOOL bUsed;
    MI_BOOL bEnable;
    char    cHost[WS_URL_HOST_LENGTH];
    char    cPath[WS_URL_PATH_LENGTH];
    MI_U32  u32Port;
#if (CARDV_VDEC_ENABLE)
    MI_BOOL     bUseVdec;
    MI_VDEC_CHN u32VdecChn;
#endif
    MI_U32            u32EsBufSize;
    stream_Link_head *linkHead;
} CarDV_WsClientAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_WsClientAttr_t g_stWsAttr[];

//==============================================================================
//
//                              WS CLIENT FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_WsClientInit(MI_U8 u8WsChn);
MI_S32 CarDV_WsClientUnInit(MI_U8 u8WsChn);
MI_S32 CarDV_WsClientAddLink(CarDV_StreamLinkNode_t *pLinkNode);
MI_S32 CarDV_WsClientRemoveLink(CarDV_StreamLinkNode_t *pLinkNode);
MI_S32 CarDV_WsClientGetStream(MI_U8 u8WsChn, MI_S32 s32Sock, MI_VENC_Stream_t *pstStream);
MI_S32 CarDV_WsClientRequestIDR(MI_U8 u8WsChn, MI_VideoEncoder *pEncoder);
MI_S32 CarDV_WsClientSyncBasePts(MI_U8 u8WsChn);
MI_S32 CarDV_WsClientSyncDate(MI_U8 u8WsChn);
MI_S32 CarDV_WsClientStartEncoder(MI_U8 u8WsChn, MI_VideoEncoder *pEncoder);
MI_S32 CarDV_WsClientStopEncoder(MI_U8 u8WsChn, MI_VideoEncoder *pEncoder);

//==============================================================================
//
//                              WS SERVER FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_WsServerStart(MI_U32 u32Chn, MI_VideoEncoder *pEncoder);
MI_S32 CarDV_WsServerStop(MI_U32 u32Chn);
MI_S32 CarDV_WsServerSendStream(MI_U32 u32Chn, MI_VENC_Stream_t *pstStream, MI_U32 u32FrameSize);

#endif // #define _MODULE_WS_H_