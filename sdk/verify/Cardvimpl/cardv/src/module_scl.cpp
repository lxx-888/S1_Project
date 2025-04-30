/*
 * module_scl.cpp- Sigmastar
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
#include "module_common.h"
#include "module_scl.h"
#include "module_isp.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

CarDV_SclModAttr_t gstSclModule = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_SclModuleInit(MI_SCL_DEV u32SclDevId)
{
    CarDV_SclDevAttr_t *pstSclDevAttr = &gstSclModule.stSclDevAttr[u32SclDevId];
    MI_SCL_CHANNEL      u32SclChnId   = 0;
    MI_SCL_PORT         SclOutPortId  = 0;

    if (pstSclDevAttr->bUsed == TRUE && pstSclDevAttr->bCreate == FALSE)
    {
        MI_SCL_DevAttr_t stCreateDevAttr;
        memset(&stCreateDevAttr, 0x0, sizeof(MI_SCL_DevAttr_t));
        stCreateDevAttr.u32NeedUseHWOutPortMask = pstSclDevAttr->u32UseHwSclMask;
        CARDVCHECKRESULT_OS(MI_SCL_CreateDevice(u32SclDevId, &stCreateDevAttr));
        pstSclDevAttr->bCreate = TRUE;

        if (pstSclDevAttr->bConfigPrivPool)
        {
            MI_SYS_GlobalPrivPoolConfig_t stPrivPoolCfg;
            memset(&stPrivPoolCfg, 0x0, sizeof(MI_SYS_GlobalPrivPoolConfig_t));
            stPrivPoolCfg.bCreate                                         = TRUE;
            stPrivPoolCfg.eConfigType                                     = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.eModule      = E_MI_MODULE_ID_SCL;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u32Devid     = u32SclDevId;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth  = pstSclDevAttr->u16PrivPoolMaxWidth;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = pstSclDevAttr->u16PrivPoolMaxHeight;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16RingLine  = pstSclDevAttr->u16PrivPoolRingLine;
            CARDVCHECKRESULT_OS(MI_SYS_ConfigPrivateMMAPool(0, &stPrivPoolCfg));
        }
    }

    for (u32SclChnId = 0; u32SclChnId < CARDV_MAX_SCL_CHN_NUM; u32SclChnId++)
    {
        CarDV_SclChannelAttr_t *pstSclChnAttr = &pstSclDevAttr->stSclChnlAttr[u32SclChnId];
        if (pstSclChnAttr->bUsed == TRUE && pstSclChnAttr->bCreate == FALSE)
        {
            MI_SCL_ChannelAttr_t stSclChnAttr;
            memset(&stSclChnAttr, 0x0, sizeof(MI_SCL_ChannelAttr_t));
            CARDVCHECKRESULT_OS(MI_SCL_CreateChannel(u32SclDevId, u32SclChnId, &stSclChnAttr));
            pstSclChnAttr->bCreate = TRUE;

            CarDV_InPortAttr_t *pstSclInPortAttr = &pstSclChnAttr->stSclInPortAttr[0];
            if (pstSclInPortAttr->stInputCropWin.u16Width != 0 && pstSclInPortAttr->stInputCropWin.u16Height != 0)
            {
                CARDVCHECKRESULT_OS(
                    MI_SCL_SetInputPortCrop(u32SclDevId, u32SclChnId, &pstSclInPortAttr->stInputCropWin));
            }

            CARDVCHECKRESULT_OS(MI_SCL_StartChannel(u32SclDevId, u32SclChnId));

            for (SclOutPortId = 0; SclOutPortId < CARDV_MAX_SCL_OUTPORT_NUM; SclOutPortId++)
            {
                CarDV_PortAttr_t *pstSclOutputAttr = &pstSclChnAttr->stSclOutPortAttr[SclOutPortId];
                if (pstSclOutputAttr->bUsed == TRUE && pstSclOutputAttr->bCreate == FALSE)
                {
                    MI_SCL_OutPortParam_t stSclOutputParam;
                    memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
                    memcpy(&stSclOutputParam.stSCLOutCropRect, &pstSclOutputAttr->stOrigPortCrop,
                           sizeof(MI_SYS_WindowRect_t));
                    memcpy(&stSclOutputParam.stSCLOutputSize, &pstSclOutputAttr->stOrigPortSize,
                           sizeof(MI_SYS_WindowSize_t));
                    stSclOutputParam.ePixelFormat  = pstSclOutputAttr->ePixelFormat;
                    stSclOutputParam.bMirror       = pstSclOutputAttr->bMirror;
                    stSclOutputParam.bFlip         = pstSclOutputAttr->bFlip;
                    stSclOutputParam.eCompressMode = pstSclOutputAttr->eCompressMode;

                    CARDVCHECKRESULT_OS(
                        MI_SCL_SetOutputPortParam(u32SclDevId, u32SclChnId, SclOutPortId, &stSclOutputParam));
                    CARDVCHECKRESULT_OS(MI_SCL_EnableOutputPort(u32SclDevId, u32SclChnId, SclOutPortId));
                    pstSclOutputAttr->bEnable = TRUE;

                    MI_SYS_ChnPort_t stChnPort;
                    stChnPort.eModId    = E_MI_MODULE_ID_SCL;
                    stChnPort.u32DevId  = u32SclDevId;
                    stChnPort.u32ChnId  = u32SclChnId;
                    stChnPort.u32PortId = SclOutPortId;
                    CARDVCHECKRESULT_OS(MI_SYS_SetChnOutputPortDepth(0, &stChnPort, pstSclOutputAttr->u16UserDepth,
                                                                     pstSclOutputAttr->u16Depth));
                    // printf("scl module Dev%d, chn%d, port%d depth(%d,%d)\n", u32SclDevId, stChnPort.u32ChnId,
                    // stChnPort.u32PortId,
                    //     pstSclOutputAttr->u16UserDepth, pstSclOutputAttr->u16Depth);
                    pstSclOutputAttr->bCreate = TRUE;
                }
            }
        }
    }

    return 0;
}

MI_S32 CarDV_SclModuleUnInit(MI_SCL_DEV u32SclDevId)
{
    CarDV_SclDevAttr_t *pstSclDevAttr = &gstSclModule.stSclDevAttr[u32SclDevId];
    MI_SCL_CHANNEL      u32SclChnId   = 0;
    MI_SCL_PORT         SclOutPortId  = 0;

    for (u32SclChnId = 0; u32SclChnId < CARDV_MAX_SCL_CHN_NUM; u32SclChnId++)
    {
        CarDV_SclChannelAttr_t *pstSclChnAttr = &pstSclDevAttr->stSclChnlAttr[u32SclChnId];
        if (pstSclChnAttr->bUsed == TRUE && pstSclChnAttr->bCreate == TRUE)
        {
            for (SclOutPortId = 0; SclOutPortId < CARDV_MAX_SCL_OUTPORT_NUM; SclOutPortId++)
            {
                CarDV_PortAttr_t *pstSclOutputAttr = &pstSclChnAttr->stSclOutPortAttr[SclOutPortId];
                if (pstSclOutputAttr->bUsed == TRUE && pstSclOutputAttr->bCreate == TRUE)
                {
                    if (pstSclOutputAttr->bEnable == TRUE)
                    {
                        CARDVCHECKRESULT(MI_SCL_DisableOutputPort(u32SclDevId, u32SclChnId, SclOutPortId));
                        pstSclOutputAttr->bEnable = FALSE;
                    }
                    pstSclOutputAttr->bCreate = FALSE;
                }
            }

            CARDVCHECKRESULT(MI_SCL_StopChannel(u32SclDevId, u32SclChnId));
            CARDVCHECKRESULT(MI_SCL_DestroyChannel(u32SclDevId, u32SclChnId));
            pstSclChnAttr->bCreate = FALSE;
        }
    }

    if (pstSclDevAttr->bUsed == TRUE && pstSclDevAttr->bCreate == TRUE)
    {
        if (pstSclDevAttr->bConfigPrivPool)
        {
            MI_SYS_GlobalPrivPoolConfig_t stPrivPoolCfg;
            memset(&stPrivPoolCfg, 0x0, sizeof(MI_SYS_GlobalPrivPoolConfig_t));
            stPrivPoolCfg.bCreate                                     = FALSE;
            stPrivPoolCfg.eConfigType                                 = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.eModule  = E_MI_MODULE_ID_SCL;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u32Devid = u32SclDevId;
            CARDVCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stPrivPoolCfg));
        }

        CARDVCHECKRESULT(MI_SCL_DestroyDevice(u32SclDevId));
        pstSclDevAttr->bCreate = FALSE;
    }

    return 0;
}

MI_S32 CarDV_SclModuleBind(MI_SCL_DEV u32SclDevId, MI_BOOL bBind)
{
    MI_SCL_CHANNEL u32SclChnId = 0;

    for (u32SclChnId = 0; u32SclChnId < CARDV_MAX_SCL_CHN_NUM; u32SclChnId++)
    {
        CARDVCHECKRESULT(CarDV_SclChannelBind(u32SclDevId, u32SclChnId, bBind));
    }

    return 0;
}

MI_S32 CarDV_SclChannelBind(MI_SCL_DEV u32SclDevId, MI_SCL_CHANNEL u32SclChnId, MI_BOOL bBind)
{
    CarDV_SclDevAttr_t *    pstSclDevAttr = &gstSclModule.stSclDevAttr[u32SclDevId];
    CarDV_SclChannelAttr_t *pstSclChnAttr = &pstSclDevAttr->stSclChnlAttr[u32SclChnId];

    if (pstSclChnAttr->bUsed == TRUE && pstSclChnAttr->bCreate == TRUE)
    {
        CarDV_InPortAttr_t *pstSclInPortAttr = &pstSclChnAttr->stSclInPortAttr[0];
        if (pstSclInPortAttr->stBindParam.stChnPort.eModId < E_MI_MODULE_ID_MAX)
        {
            CarDV_Sys_BindInfo_T stBindInfo;
            memset(&stBindInfo, 0x0, sizeof(CarDV_Sys_BindInfo_T));
            stBindInfo.stSrcChnPort.eModId    = pstSclInPortAttr->stBindParam.stChnPort.eModId;
            stBindInfo.stSrcChnPort.u32DevId  = pstSclInPortAttr->stBindParam.stChnPort.u32DevId;
            stBindInfo.stSrcChnPort.u32ChnId  = pstSclInPortAttr->stBindParam.stChnPort.u32ChnId;
            stBindInfo.stSrcChnPort.u32PortId = pstSclInPortAttr->stBindParam.stChnPort.u32PortId;
            stBindInfo.stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
            stBindInfo.stDstChnPort.u32DevId  = u32SclDevId;
            stBindInfo.stDstChnPort.u32ChnId  = u32SclChnId;
            stBindInfo.stDstChnPort.u32PortId = 0;
            stBindInfo.u32SrcFrmrate          = pstSclInPortAttr->stBindParam.u32SrcFrmrate;
            stBindInfo.u32DstFrmrate          = pstSclInPortAttr->stBindParam.u32DstFrmrate;
            stBindInfo.eBindType              = pstSclInPortAttr->stBindParam.eBindType;
            if (bBind)
            {
                CARDVCHECKRESULT(CarDV_Sys_Bind(&stBindInfo));
                pstSclChnAttr->bBind = TRUE;
            }
            else if (bBind == FALSE && pstSclChnAttr->bBind)
            {
                CARDVCHECKRESULT(CarDV_Sys_UnBind(&stBindInfo));
                pstSclChnAttr->bBind = FALSE;
            }
        }
    }

    return 0;
}

MI_S32 CarDV_SclGetDevChnByVifDev(MI_VIF_DEV u32VifDev, MI_SCL_DEV *pu32SclDevId, MI_SCL_CHANNEL *pu32SclChnId)
{
    MI_SCL_DEV     u32SclDevId = 0;
    MI_SCL_CHANNEL u32SclChnId = 0;

    if (pu32SclDevId == NULL || pu32SclChnId == NULL)
        return -1;

    for (u32SclDevId = 0; u32SclDevId < CARDV_MAX_SCL_DEV_NUM; u32SclDevId++)
    {
        CarDV_SclDevAttr_t *pstSclDevAttr = &gstSclModule.stSclDevAttr[u32SclDevId];
        if (pstSclDevAttr->bUsed == TRUE)
        {
            for (u32SclChnId = 0; u32SclChnId < CARDV_MAX_SCL_CHN_NUM; u32SclChnId++)
            {
                CarDV_SclChannelAttr_t *pstSclChnAttr    = &pstSclDevAttr->stSclChnlAttr[u32SclChnId];
                CarDV_InPortAttr_t *    pstSclInPortAttr = &pstSclChnAttr->stSclInPortAttr[0];
                if (pstSclChnAttr->bUsed == TRUE && pstSclInPortAttr->stBindParam.stChnPort.eModId == E_MI_MODULE_ID_VIF
                    && pstSclInPortAttr->stBindParam.stChnPort.u32DevId == u32VifDev)
                {
                    *pu32SclDevId = u32SclDevId;
                    *pu32SclChnId = u32SclChnId;
                    return 0;
                }
            }
        }
    }

    return -1;
}

MI_S32 CarDV_SclGetVifDev(MI_VIF_DEV *pu32VifDev, MI_SCL_DEV u32SclDevId, MI_SCL_CHANNEL u32SclChnId)
{
    CarDV_SclDevAttr_t *    pstSclDevAttr    = &gstSclModule.stSclDevAttr[u32SclDevId];
    CarDV_SclChannelAttr_t *pstSclChnAttr    = &pstSclDevAttr->stSclChnlAttr[u32SclChnId];
    CarDV_InPortAttr_t *    pstSclInPortAttr = &pstSclChnAttr->stSclInPortAttr[0];

    if (pu32VifDev == NULL)
        return -1;

    if (pstSclDevAttr->bUsed == TRUE && pstSclChnAttr->bUsed == TRUE
        && pstSclInPortAttr->stBindParam.stChnPort.eModId == E_MI_MODULE_ID_VIF)
    {
        *pu32VifDev = pstSclInPortAttr->stBindParam.stChnPort.u32DevId;
        return 0;
    }

    return -1;
}

MI_S32 CarDV_SclChangeOutputPortSize(MI_SCL_DEV u32SclDevId, MI_SCL_CHANNEL u32SclChnId, MI_SCL_PORT SclOutPortId,
                                     MI_SYS_WindowSize_t *pstPortSize)
{
    CarDV_SclDevAttr_t *    pstSclDevAttr;
    CarDV_SclChannelAttr_t *pstSclChnAttr;
    CarDV_PortAttr_t *      pstSclOutputAttr;
    MI_SCL_OutPortParam_t   stSclOutputParam;

    if (u32SclDevId >= CARDV_MAX_SCL_DEV_NUM || u32SclChnId >= CARDV_MAX_SCL_CHN_NUM
        || SclOutPortId >= CARDV_MAX_SCL_OUTPORT_NUM || pstPortSize == NULL)
        return -1;

    pstSclDevAttr    = &gstSclModule.stSclDevAttr[u32SclDevId];
    pstSclChnAttr    = &pstSclDevAttr->stSclChnlAttr[u32SclChnId];
    pstSclOutputAttr = &pstSclChnAttr->stSclOutPortAttr[SclOutPortId];
    memcpy(&pstSclOutputAttr->stOrigPortSize, pstPortSize, sizeof(MI_SYS_WindowSize_t));

    if (pstSclOutputAttr->bCreate == TRUE)
    {
        if (pstSclOutputAttr->bEnable == TRUE)
            CARDVCHECKRESULT(MI_SCL_DisableOutputPort(u32SclDevId, u32SclChnId, SclOutPortId));

        CARDVCHECKRESULT(MI_SCL_GetOutputPortParam(u32SclDevId, u32SclChnId, SclOutPortId, &stSclOutputParam));
        memset(&stSclOutputParam.stSCLOutCropRect, 0, sizeof(MI_SYS_WindowRect_t));
        memcpy(&stSclOutputParam.stSCLOutputSize, &pstSclOutputAttr->stOrigPortSize, sizeof(MI_SYS_WindowSize_t));
        CARDVCHECKRESULT(MI_SCL_SetOutputPortParam(u32SclDevId, u32SclChnId, SclOutPortId, &stSclOutputParam));

        if (pstSclOutputAttr->bEnable == TRUE)
            CARDVCHECKRESULT(MI_SCL_EnableOutputPort(u32SclDevId, u32SclChnId, SclOutPortId));
    }

    return 0;
}

MI_S32 CarDV_SclEnableOutputPort(MI_SCL_DEV u32SclDevId, MI_SCL_CHANNEL u32SclChnId, MI_SCL_PORT SclOutPortId,
                                 MI_BOOL bEnable)
{
    CarDV_SclDevAttr_t *    pstSclDevAttr;
    CarDV_SclChannelAttr_t *pstSclChnAttr;
    CarDV_PortAttr_t *      pstSclOutputAttr;

    if (u32SclDevId >= CARDV_MAX_SCL_DEV_NUM || u32SclChnId >= CARDV_MAX_SCL_CHN_NUM
        || SclOutPortId >= CARDV_MAX_SCL_OUTPORT_NUM)
        return -1;

    pstSclDevAttr    = &gstSclModule.stSclDevAttr[u32SclDevId];
    pstSclChnAttr    = &pstSclDevAttr->stSclChnlAttr[u32SclChnId];
    pstSclOutputAttr = &pstSclChnAttr->stSclOutPortAttr[SclOutPortId];

    if (pstSclOutputAttr->bCreate == TRUE)
    {
        if (bEnable && pstSclOutputAttr->bEnable == FALSE)
        {
            CARDVCHECKRESULT_OS(MI_SCL_EnableOutputPort(u32SclDevId, u32SclChnId, SclOutPortId));
            pstSclOutputAttr->bEnable = TRUE;
        }
        else if (bEnable == FALSE && pstSclOutputAttr->bEnable)
        {
            CARDVCHECKRESULT_OS(MI_SCL_DisableOutputPort(u32SclDevId, u32SclChnId, SclOutPortId));
            pstSclOutputAttr->bEnable = FALSE;
        }
    }

    return 0;
}
