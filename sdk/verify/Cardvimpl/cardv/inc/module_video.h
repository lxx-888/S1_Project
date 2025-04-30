/*
 * module_video.h- Sigmastar
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
#ifndef _MODULE_VIDEO_H_
#define _MODULE_VIDEO_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mid_VideoEncoder.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define MAX_VIDEO_NUMBER (32)

#define H26X_MAX_ENCODE_WIDTH  (2560)
#define H26X_MAX_ENCODE_HEIGHT (2560)
#define JPEG_MAX_ENCODE_WIDTH  (2560)
#define JPEG_MAX_ENCODE_HEIGHT (2560)

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_VencAttr_t        gstVencAttr[];
extern CarDV_VencPrivPoolCfg_t gstVencPrivPoolCfg;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 VideoMapToIndex(MI_U8 u8CamId, VideoStreamType_e eStreamType);
MI_S32 VideoInitResolution(MI_SYS_ChnPort_t *pstChnPort, MI_U32 *pu32Width, MI_U32 *pu32Height);
MI_S32 VideoSetInputResolution(MI_SYS_ChnPort_t *pstChnPort, MI_U32 u32Width, MI_U32 u32Height);
MI_S32 VideoEnableInput(MI_SYS_ChnPort_t *pstChnPort, MI_BOOL bEnable);

#endif //#define _MODULE_VIDEO_H_