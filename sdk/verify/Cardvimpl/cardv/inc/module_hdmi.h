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
#ifndef _MODULE_HDMI_H_
#define _MODULE_HDMI_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_hdmi.h"

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_HdmiModuleInit(MI_HDMI_TimingType_e eHdmiTiming);
MI_S32 CarDV_HdmiModuleUnInit(void);
MI_S32 CarDV_HdmiModuleSwitchTiming(MI_HDMI_TimingType_e eHdmiTiming);
#endif //#define _MODULE_HDMI_H_