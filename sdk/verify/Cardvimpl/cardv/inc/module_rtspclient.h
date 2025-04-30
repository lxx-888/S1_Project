/*
 * module_rtspclient.h- Sigmastar
 *
 * Copyright (C) 2021 Sigmastar Technology Corp.
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

#ifndef _MODULE_RTSPCLIENT_H_
#define _MODULE_RTSPCLIENT_H_

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

#define RTSP_MAX_STREAM_CNT (2)
#define RTSP_URL_LENGTH     (128)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct
{
    MI_BOOL bUsed;
    MI_BOOL bEnable;
    char    cUrl[RTSP_URL_LENGTH];
#if (CARDV_VDEC_ENABLE)
    MI_BOOL     bUseVdec;
    MI_VDEC_CHN u32VdecChn;
#endif
    MI_U32            u32EsBufSize;
    stream_Link_head *linkHead;
} CarDV_RtspClientAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_RtspClientAttr_t g_stRtspAttr[];

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_RtspClientInit(MI_U8 u8RtspChn);
MI_S32 CarDV_RtspClientUnInit(MI_U8 u8RtspChn);
MI_S32 CarDV_RtspClientAddLink(CarDV_StreamLinkNode_t *pLinkNode);
MI_S32 CarDV_RtspClientRemoveLink(CarDV_StreamLinkNode_t *pLinkNode);
MI_S32 CarDV_RtspClientGetStream(MI_U8 u8RtspChn, MI_S32 s32Sock, MI_VENC_Stream_t *pstStream);
MI_S32 CarDV_RtspClientStartEncoder(MI_U8 u8RtspChn, MI_VideoEncoder *pEncoder);
MI_S32 CarDV_RtspClientStopEncoder(MI_U8 u8RtspChn, MI_VideoEncoder *pEncoder);
MI_S32 CarDV_RtspClientRequestIDR(MI_U8 u8RtspChn, MI_VideoEncoder *pEncoder);

#endif // #define _MODULE_RTSPCLIENT_H_