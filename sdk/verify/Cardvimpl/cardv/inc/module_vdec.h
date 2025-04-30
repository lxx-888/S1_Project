/*
 * module_vdec.h- Sigmastar
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
#ifndef _MODULE_VDEC_H_
#define _MODULE_VDEC_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_vdec.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define CARDV_MAX_VDEC_CHN_NUM (4)
#define CARDV_VDEC_DEV_ID      (0)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct
{
    int            startcodeprefix_len;
    unsigned int   len;
    unsigned int   max_size;
    char*          buf;
    unsigned short lost_packets;
} NALU_t;

typedef struct CarDV_VdecChnAttr_s
{
    MI_VDEC_CHN         u32ChnId;
    MI_BOOL             bUsed;
    MI_BOOL             bCreate;
    MI_U32              u32MaxW;
    MI_U32              u32MaxH;
    MI_U32              u32StreamBufSize;
    MI_U32              u32Fps;
    MI_VDEC_CodecType_e eCodecType;
    MI_BOOL             bThreadExit;
    pthread_t           pPutDataThread;
    char                szFileName[64];
} CarDV_VdecChnAttr_t;

typedef struct CarDV_VdecModAttr_s
{
    CarDV_VdecChnAttr_t stVdecChnAttr[CARDV_MAX_VDEC_CHN_NUM];
} CarDV_VdecModAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_VdecModAttr_t gstVdecModule;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_VdecModuleInit(MI_VDEC_CHN u32VdecChn);
MI_S32 CarDV_VdecModuleUnInit(MI_VDEC_CHN u32VdecChn);
MI_S32 CarDV_VdecDecodeStream(MI_VDEC_CHN u32VdecChn, MI_U8* pu8EsBuf, MI_U32 u32EsSize, MI_U64 u64Pts);

#endif //#define _MODULE_VDEC_H_