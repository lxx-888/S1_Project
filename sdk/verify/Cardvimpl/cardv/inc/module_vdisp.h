/*
 * module_vdisp.h- Sigmastar
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
#ifndef _MODULE_VDISP_H_
#define _MODULE_VDISP_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_vdisp.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define VDISP_OVERLAY_CHN_NUM (1)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct CarDV_VdispChannelAttr_s
{
    MI_U8                   u8ChnId;
    MI_BOOL                 bUsed;
    MI_BOOL                 bCreate;
    MI_BOOL                 bBind;
    MI_VDISP_InputChnAttr_t stVdispInPortAttr[VDISP_MAX_INPUTPORT_NUM];
    CarDV_BindParam_t       stBindParam[VDISP_MAX_INPUTPORT_NUM];
} CarDV_VdispChannelAttr_t;

typedef struct CarDV_VdispDevAttr_s
{
    MI_U8                     u8DevId;
    MI_BOOL                   bUsed;
    MI_BOOL                   bCreate;
    MI_U16                    u16Depth;
    MI_U16                    u16UserDepth;
    CarDV_VdispChannelAttr_t  stVdispChnlAttr[VDISP_MAX_CHN_NUM_PER_DEV + VDISP_OVERLAY_CHN_NUM];
    MI_VDISP_OutputPortAttr_t stVdispOutAttr[VDISP_MAX_OUTPUTPORT_NUM];
} CarDV_VdispDevAttr_t;

typedef struct CarDV_VdispModAttr_s
{
    CarDV_VdispDevAttr_t stVdispDevAttr[VDISP_MAX_DEVICE_NUM];
} CarDV_VdispModAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_VdispModAttr_t gstVdispModule;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_VdispModuleInit(MI_VDISP_DEV VdispDevId);
MI_S32 CarDV_VdispModuleUnInit(MI_VDISP_DEV VdispDevId);
MI_S32 CarDV_VdispModuleBind(MI_VDISP_DEV VdispDevId, MI_BOOL bBind);

#endif //#define _MODULE_VDISP_H_