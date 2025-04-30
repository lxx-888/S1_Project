/*
 * module_mux.h- Sigmastar
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
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
#ifndef _MODULE_MUX_H_
#define _MODULE_MUX_H_

#include "mid_mux.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

// muxer fps = num / den, Ex: 27.5fps : 55 / 2.
// if both value is 0, means muxer fps follow venc fps. otherwise, constant muxer fps.
// Note : muxer fps must larger than venc fps (because only support insert frame, and can't drop frame), or it cause AV
// sync abnormal.
#if (0) // Ex : venc 25fps -> muxer 30fps
#define MUXER_FPS_NUM (30)
#define MUXER_FPS_DEN (1)
#else // follow venc fps
#define MUXER_FPS_NUM (0)
#define MUXER_FPS_DEN (0)
#endif

//==============================================================================
//
//                              ENUMERATION
//
//==============================================================================

typedef enum _MuxerStatus_e
{
    MUXER_STATUS_IDLE,
    MUXER_STATUS_PRE_RECORD,
    MUXER_STATUS_RECORD,
    MUXER_STATUS_PAUSE,
} MuxerStatus_e;

//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef struct CarDV_MuxerAttr_s
{
    MI_BOOL     bUsed;
    MI_U8       u8CamId;
    MI_U8       u8VidTrackNum;
    MI_U8       u8VencChn[MUXER_MAX_VIDEO_TRACK];
    MuxerType_e eMuxerType;
    MuxerFile_e eMuxerFile;
    MI_U64      u64LimitTime;
    BOOL        bGpsTrack;
    BOOL        bGsnrTrack;
    BOOL        bThumbTrack;
} CarDV_MuxerAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_MuxerAttr_t gstMuxerAttr[];

#endif //#define _MODULE_MUX_H_
