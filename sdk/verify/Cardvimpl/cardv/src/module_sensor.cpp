/*
 * module_sensor.cpp - Sigmastar
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

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mid_common.h"
#include "module_sensor.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

CarDV_Sensor_Attr_t gstSensorAttr[CARDV_MAX_SENSOR_NUM] = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_SensorModuleInit(MI_SNR_PADID u32SnrPad)
{
    MI_SNR_PADInfo_t     stPad0Info;
    MI_SNR_PlaneInfo_t   stSnrPlane0Info;
    MI_U32               u32ResCount = 0;
    MI_U8                u8ResIndex  = 0;
    MI_U8                u8ChocieRes = 0;
    MI_SNR_Res_t         stRes;
    CarDV_Sensor_Attr_t *pstSensorAttr = NULL;

    if (u32SnrPad >= CARDV_MAX_SENSOR_NUM)
        return -1;

    pstSensorAttr = &gstSensorAttr[u32SnrPad];
    if (pstSensorAttr->bUsed == TRUE && pstSensorAttr->bCreate == FALSE)
    {
        memset(&stRes, 0x0, sizeof(MI_SNR_Res_t));
        memset(&stPad0Info, 0x0, sizeof(MI_SNR_PADInfo_t));
        memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t));
        CARDVCHECKRESULT_OS(MI_SNR_SetPlaneMode(u32SnrPad, pstSensorAttr->bPlaneMode));
        CARDVCHECKRESULT(MI_SNR_QueryResCount(u32SnrPad, &u32ResCount));
        if (pstSensorAttr->u8ResIndex < u32ResCount)
        {
            u8ChocieRes = pstSensorAttr->u8ResIndex;
        }
        else
        {
            MI_U32 u32MinResFps  = 0;
            MI_U32 u32RequestFps = pstSensorAttr->u32RequestFps >= 1000 ? pstSensorAttr->u32RequestFps / 1000
                                                                        : pstSensorAttr->u32RequestFps;
            if (u32RequestFps == 0)
                u32RequestFps = 30;
            for (u8ResIndex = 0; u8ResIndex < u32ResCount; u8ResIndex++)
            {
                CARDVCHECKRESULT(MI_SNR_GetRes(u32SnrPad, u8ResIndex, &stRes));
                printf("list index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n",
                       u8ResIndex, stRes.stCropRect.u16X, stRes.stCropRect.u16Y, stRes.stCropRect.u16Width,
                       stRes.stCropRect.u16Height, stRes.stOutputSize.u16Width, stRes.stOutputSize.u16Height,
                       stRes.u32MaxFps, stRes.u32MinFps, stRes.strResDesc);

                MI_U16 u16SnrOutputWidth  = stRes.stOutputSize.u16Width;
                MI_U16 u16SnrOutputHeight = stRes.stOutputSize.u16Height;
                MI_U32 u32SnrOutputResFps = u16SnrOutputWidth * u16SnrOutputHeight * stRes.u32MaxFps;
                if (u32RequestFps <= stRes.u32MaxFps && pstSensorAttr->u16RequestWidth <= u16SnrOutputWidth
                    && pstSensorAttr->u16RequestHeight <= u16SnrOutputHeight)
                {
                    if (u32MinResFps == 0)
                    {
                        u8ChocieRes  = u8ResIndex;
                        u32MinResFps = u32SnrOutputResFps;
                    }
                    else if (u32SnrOutputResFps < u32MinResFps)
                    {
                        u8ChocieRes  = u8ResIndex;
                        u32MinResFps = u32SnrOutputResFps;
                    }
                }
            }
        }

        CARDVCHECKRESULT(MI_SNR_GetRes(u32SnrPad, u8ChocieRes, &stRes));
        printf("select index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n", u8ChocieRes,
               stRes.stCropRect.u16X, stRes.stCropRect.u16Y, stRes.stCropRect.u16Width, stRes.stCropRect.u16Height,
               stRes.stOutputSize.u16Width, stRes.stOutputSize.u16Height, stRes.u32MaxFps, stRes.u32MinFps,
               stRes.strResDesc);

        CARDVCHECKRESULT_OS(MI_SNR_SetRes(u32SnrPad, u8ChocieRes));
        CARDVCHECKRESULT_OS(MI_SNR_Enable(u32SnrPad));
        pstSensorAttr->bCreate = TRUE;

        if (pstSensorAttr->u32RequestFps)
            CARDVCHECKRESULT_OS(MI_SNR_SetFps(u32SnrPad, pstSensorAttr->u32RequestFps));
    }

    return 0;
}

MI_S32 CarDV_SensorModuleUnInit(MI_SNR_PADID u32SnrPad)
{
    CarDV_Sensor_Attr_t *pstSensorAttr = NULL;

    if (u32SnrPad >= CARDV_MAX_SENSOR_NUM)
        return -1;

    pstSensorAttr = &gstSensorAttr[u32SnrPad];
    if (pstSensorAttr->bUsed == TRUE && pstSensorAttr->bCreate == TRUE)
    {
        CARDVCHECKRESULT(MI_SNR_Disable(u32SnrPad));
        pstSensorAttr->bCreate = FALSE;
    }
    return 0;
}