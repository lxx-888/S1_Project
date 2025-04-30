/*
 * module_disalgo.h- Sigmastar
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
#ifndef _MODULE_DIS_ALGO_H_
#define _MODULE_DIS_ALGO_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_sys.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define DIS_MAGIC             'd'
#define DIS_CMD_WRITE_DISPMAP _IOWR(DIS_MAGIC, 0, Dis_DispMap_t *)
#define DIS_CMD_GET_FRAMEINFO _IOWR(DIS_MAGIC, 1, Dis_FrameInfo_t *)
#define DIS_CMD_GET_IMUINFO   _IOWR(DIS_MAGIC, 2, Dis_ImuInfo_t *)

#define IMU_MODULE   (IMU_ICM40607)
#define IMU_ICM40607 (1)
#define IMU_QMI8658  (2)

#if (IMU_MODULE == IMU_ICM40607)
#define IMU_PER_SIZE (16)
#elif (IMU_MODULE == IMU_QMI8658)
#define IMU_PER_SIZE (12)
#endif

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct
{
    void * pDispMapX;
    void * pDispMapY;
    MI_U32 u32MapXSize;
    MI_U32 u32MapYSize;
} Dis_DispMap_t;

typedef struct
{
    MI_U64 u64FramePts;
    MI_U32 u32ShutterDuration;
} Dis_FrameInfo_t;

typedef struct
{
    MI_U64 u64FrameLastPts;
    MI_U16 u16SampleRate;
    MI_U32 u32FifoSize;
    MI_U8 *pu8FifoBuf;
} Dis_ImuInfo_t;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_DisAlgoInit(MI_U16 u16Width, MI_U16 u16Height);
MI_S32 CarDV_DisAlgoUnInit(void);

#endif //#define _MODULE_DIS_ALGO_H_