/*
 * module_hdmi.h- Sigmastar
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
#ifndef _MODULE_MIPITX_H_
#define _MODULE_MIPITX_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_mipitx.h"

//==============================================================================
//
//                              STRUCT & ENUM DEFINES
//
//==============================================================================

typedef struct CarDV_MipiTxChnAttr_s
{
    MI_BOOL          bCreate;
    MI_BOOL          bUsed;
    MI_BOOL          bBind;
    MI_U32           u32ChnId;
    MI_U32           u32Width;
    MI_U32           u32Height;
    MI_SYS_ChnPort_t stSrcChnPort;
} CarDV_MipiTxChnAttr_t;

typedef struct CarDV_MipiTxModAttr_s
{
    CarDV_MipiTxChnAttr_t stMipiTxChnAttr;
} CarDV_MipiTxModAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VALUE
//
//==============================================================================

extern CarDV_MipiTxModAttr_t gstMipiTxModule;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_MipiTxModuleInit(void);
MI_S32 CarDV_MipiTxModuleUnInit(void);
MI_S32 CarDV_MipiTxModuleBind(MI_BOOL bBind);

#endif //#define _MODULE_HDMI_H_