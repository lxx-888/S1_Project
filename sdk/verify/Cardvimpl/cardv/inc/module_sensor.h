/*
 * module_sensor.h- Sigmastar
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
#ifndef _MODULE_SENSOR_H_
#define _MODULE_SENSOR_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_sensor.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define CARDV_MAX_SENSOR_NUM (4)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct CarDV_Sensor_Attr_s
{
    MI_BOOL bUsed;
    MI_BOOL bCreate;
    MI_BOOL bPlaneMode;
    MI_U8   u8ResIndex;
    MI_U16  u16RequestWidth;
    MI_U16  u16RequestHeight;
    MI_U32  u32RequestFps;
} CarDV_Sensor_Attr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_Sensor_Attr_t gstSensorAttr[];

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_SensorModuleInit(MI_SNR_PADID u32SnrPad);
MI_S32 CarDV_SensorModuleUnInit(MI_SNR_PADID u32SnrPad);

#endif //#define _MODULE_SENSOR_H_