/*
 * module_vif.h- Sigmastar
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
#ifndef _MODULE_VIF_H_
#define _MODULE_VIF_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_vif.h"
#include "mi_sensor.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define CARDV_MAX_VIF_DEV_PERGROUP (4)
#define CARDV_MAX_VIF_GROUP_NUM    (4)
#define CARDV_MAX_VIF_DEV_NUM      (CARDV_MAX_VIF_GROUP_NUM * CARDV_MAX_VIF_DEV_PERGROUP)
#define CARDV_MAX_VIF_OUTPORT_NUM  (2)

#define CARDV_AUTO_DETECT_ANADEC (1)
// 0 : only check plug in or out. 1: check analog-decoder output format, need sensor driver support.

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct CarDV_VifPortAttr_s
{
    MI_BOOL               bCreate;
    MI_BOOL               bUsed;
    MI_SYS_WindowRect_t   stCapRect;
    MI_SYS_WindowSize_t   stDestSize;
    MI_SYS_PixelFormat_e  ePixFormat;
    MI_SYS_CompressMode_e eCompressMode;
    MI_U16                u16Depth;
    MI_U16                u16UserDepth;
} CarDV_VifPortAttr_t;

typedef struct CarDV_VifDevAttr_s
{
    MI_BOOL             bCreate;
    MI_BOOL             bUsed;
    MI_SYS_FieldType_e  eField;
    MI_BOOL             bEnH2T1PMode;
    CarDV_VifPortAttr_t stVifOutPortAttr[CARDV_MAX_VIF_OUTPORT_NUM];
    // Follow code for AHD signal detection
    MI_U8                   u8CamId;
    MI_SNR_Anadec_SrcAttr_t stAnadecSrcAttr;
} CarDV_VifDevAttr_t;

typedef struct CarDV_VifGroupExtAttr
{
    MI_VIF_SNRPad_e eSensorPadID;                         // sensor Pad id
    MI_U32          u32PlaneID[MI_VIF_MAX_GROUP_DEV_CNT]; // For HDR,0 is short exposure, 1 is long exposure
} CarDV_VifGroupExtAttr_t;

typedef struct CarDV_VifGroupAttr_s
{
    MI_BOOL                 bCreate;
    MI_BOOL                 bUsed;
    CarDV_VifGroupExtAttr_t stGroupExtAttr;
    MI_BOOL                 bNeedSetGroupExtAttr;
    MI_VIF_WorkMode_e       eWorkMode;
    MI_VIF_HDRType_e        eHDRType;
    CarDV_VifDevAttr_t      stVifDevAttr[CARDV_MAX_VIF_DEV_PERGROUP];
    // Follow code for AHD signal detection
    MI_BOOL bCheckSignal;
} CarDV_VifGroupAttr_t;

typedef struct CarDV_VifModeAttr_s
{
    CarDV_VifGroupAttr_t stVifGroupAttr[CARDV_MAX_VIF_GROUP_NUM];
} CarDV_VifModAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_VifModAttr_t gstVifModule;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_VifModuleInit(MI_VIF_GROUP GroupId);
MI_S32 CarDV_VifModuleUnInit(MI_VIF_GROUP GroupId);
MI_S32 CarDV_VifDevInit(MI_VIF_DEV vifDev);
MI_S32 CarDV_VifDevUnInit(MI_VIF_DEV vifDev);
MI_S32 CarDV_StartAnaDecDetectThread(void);
MI_S32 CarDV_StopAnaDecDetectThread(void);
MI_S32 CarDv_VifSetAnadecSrcAttr(MI_VIF_DEV vifDev, MI_SNR_Anadec_SrcAttr_t *pstAnadecSrcAttr);

#endif //#define _MODULE_VIF_H_