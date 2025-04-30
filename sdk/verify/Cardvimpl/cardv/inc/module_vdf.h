/*
 * module_vdf.h- Sigmastar
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

#ifndef __MODULE_VDF_H__
#define __MODULE_VDF_H__

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_vdf.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define VDF_CHANNEL_NUM (2)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct CarDV_VdfChannelAttr_s
{
    MI_U8               u8ChnId;
    MI_BOOL             bUsed;
    MI_BOOL             bCreate;
    MI_BOOL             bBind;
    MI_VDF_ChnAttr_t    stChnAttr;
    MI_SYS_WindowSize_t stSize;
    CarDV_BindParam_t   stBindParam;
} CarDV_VdfChannelAttr_t;

typedef struct CarDV_VdfModAttr_s
{
    MI_BOOL                bUsed;
    MI_BOOL                bCreate;
    CarDV_VdfChannelAttr_t stVdfChnAttr[VDF_CHANNEL_NUM];
} CarDV_VdfModAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_VdfModAttr_t gstVdfModule;

#endif /* __MODULE_VDF_H__ */