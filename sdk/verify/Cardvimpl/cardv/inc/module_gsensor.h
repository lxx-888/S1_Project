/*
 * module_gsensor.h- Sigmastar
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
#ifndef _MODULE_GSENSOR_H_
#define _MODULE_GSENSOR_H_

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define AXIS_INIT_VALUE 0xFFFFFFF

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct GSNRINFOCHUCK_s
{
    int x;
    int y;
    int z;
} GSNRINFOCHUCK;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void GsensorStartThread(void);
void GsensorStopThread(void);
void GsensorSetPowerOnByInt(void);
BOOL GsensorIsPowerOnByInt(void);
void GsensorSetSensitivity(MI_S8 level);
void GsensorParkingMonitor(int param);
void GsensorStartRec(MI_BOOL bStart);

#endif //#define _MODULE_GSENSOR_H_
