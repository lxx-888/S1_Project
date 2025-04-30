/*
 * module_ldc.h- Sigmastar
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
#ifndef _MODULE_LDC_H_
#define _MODULE_LDC_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_ldc.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define CARDV_MAX_LDC_DEV_NUM (1)
#define CARDV_MAX_LDC_CHN_NUM (16)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct CarDV_LdcChannelAttr_s
{
    MI_U8   u8ChnId;
    MI_BOOL bUsed;
    MI_BOOL bCreate;
    MI_BOOL bBypass;
    MI_BOOL bBind;
    MI_BOOL bUseProjection3x3Matrix;
    float   afProjection3x3Matrix[LDC_MAXTRIX_NUM];
    MI_U16  u16UserSliceNum;
    MI_U16  u16FocalLength;
    char    CfgFilePath[256];
    union
    {
        char SensorCalibBinPath[256];
        struct
        {
            char Xmap[256];
            char Ymap[256];
        };
    };
    MI_LDC_WorkMode_e    eWorkMode;
    MI_LDC_MapInfoType_e eMapMode;
    CarDV_BindParam_t    stBindParam;
} CarDV_LdcChannelAttr_t;

typedef struct CarDV_LdcDevAttr_s
{
    MI_U8                  u8DevId;
    MI_BOOL                bUsed;
    MI_BOOL                bCreate;
    CarDV_LdcChannelAttr_t stLdcChnlAttr[CARDV_MAX_LDC_CHN_NUM];
} CarDV_LdcDevAttr_t;

typedef struct CarDV_LdcModAttr_s
{
    CarDV_LdcDevAttr_t stLdcDevAttr[CARDV_MAX_LDC_DEV_NUM];
} CarDV_LdcModAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_LdcModAttr_t gstLdcModule;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_LdcModuleInit(MI_LDC_DEV u32LdcDevId);
MI_S32 CarDV_LdcModuleUnInit(MI_LDC_DEV u32LdcDevId);
MI_S32 CarDV_LdcModuleBind(MI_LDC_DEV u32LdcDevId, MI_BOOL bBind);
MI_S32 CarDV_LdcChannelInit(MI_LDC_DEV u32LdcDevId, MI_LDC_CHN u32LdcChnId);
MI_S32 CarDV_LdcChannelUnInit(MI_LDC_DEV u32LdcDevId, MI_LDC_CHN u32LdcChnId);
MI_S32 CarDV_LdcChannelBind(MI_LDC_DEV u32LdcDevId, MI_LDC_CHN u32LdcChnId, MI_BOOL bBind);
MI_S32 CarDV_LdcChannelSetCfg(MI_LDC_DEV u32LdcDevId, MI_LDC_CHN u32LdcChnId, const char *CfgFilePath);
MI_S32 CarDV_LdcGetDevChnByVifDev(MI_VIF_DEV u32VifDev, MI_LDC_DEV *pu32LdcDevId, MI_LDC_CHN *pu32LdcChnId);

#endif //#define _MODULE_LDC_H_