/*
 * module_vdisp.cpp- Sigmastar
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
#include "module_vdisp.h"

#if (CARDV_VDISP_ENABLE)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

CarDV_VdispModAttr_t gstVdispModule = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_VdispModuleInit(MI_VDISP_DEV VdispDevId)
{
    CarDV_VdispDevAttr_t *pstVdispDevAttr = &gstVdispModule.stVdispDevAttr[VdispDevId];
    MI_VDISP_CHN          VdispChnId      = 0;
    MI_VDISP_PORT         VdispOutPortId  = 0;
    MI_SYS_ChnPort_t      stChnPort;

    if (pstVdispDevAttr->bUsed == TRUE && pstVdispDevAttr->bCreate == FALSE)
    {
        MI_VDISP_OutputPortAttr_t *pstVdispOutAttr = &pstVdispDevAttr->stVdispOutAttr[VdispOutPortId];
        CARDVCHECKRESULT(MI_VDISP_OpenDevice(VdispDevId));
        pstVdispDevAttr->bCreate = TRUE;

        for (VdispChnId = 0; VdispChnId < VDISP_MAX_CHN_NUM_PER_DEV + VDISP_OVERLAY_CHN_NUM; VdispChnId++)
        {
            CarDV_VdispChannelAttr_t *pstVdispChnAttr = &pstVdispDevAttr->stVdispChnlAttr[VdispChnId];
            if (pstVdispChnAttr->bUsed == TRUE && pstVdispChnAttr->bCreate == FALSE)
            {
                CARDVCHECKRESULT(
                    MI_VDISP_SetInputChannelAttr(VdispDevId, VdispChnId, &pstVdispChnAttr->stVdispInPortAttr[0]));
                CARDVCHECKRESULT(MI_VDISP_EnableInputChannel(VdispDevId, VdispChnId));
                pstVdispChnAttr->bCreate = TRUE;
            }
        }

        CARDVCHECKRESULT(MI_VDISP_SetOutputPortAttr(VdispDevId, VdispOutPortId, pstVdispOutAttr));
        CARDVCHECKRESULT(MI_VDISP_StartDev(VdispDevId));

        stChnPort.eModId    = E_MI_MODULE_ID_VDISP;
        stChnPort.u32DevId  = VdispDevId;
        stChnPort.u32ChnId  = 0;
        stChnPort.u32PortId = VdispOutPortId;
        CARDVCHECKRESULT(
            MI_SYS_SetChnOutputPortDepth(0, &stChnPort, pstVdispDevAttr->u16UserDepth, pstVdispDevAttr->u16Depth));
        // printf("vdisp module Dev%d, chn%d, port%d depth(%d,%d)\n", stChnPort.u32DevId, stChnPort.u32ChnId,
        // stChnPort.u32PortId,
        //     pstVdispDevAttr->u16UserDepth, pstVdispDevAttr->u16Depth);
    }

    return 0;
}

MI_S32 CarDV_VdispModuleUnInit(MI_VDISP_DEV VdispDevId)
{
    CarDV_VdispDevAttr_t *pstVdispDevAttr = &gstVdispModule.stVdispDevAttr[VdispDevId];
    MI_VDISP_CHN          VdispChnId      = 0;

    if (pstVdispDevAttr->bUsed == TRUE && pstVdispDevAttr->bCreate == TRUE)
    {
        CARDVCHECKRESULT(MI_VDISP_StopDev(VdispDevId));

        for (VdispChnId = 0; VdispChnId < VDISP_MAX_CHN_NUM_PER_DEV + VDISP_OVERLAY_CHN_NUM; VdispChnId++)
        {
            CarDV_VdispChannelAttr_t *pstVdispChnAttr = &pstVdispDevAttr->stVdispChnlAttr[VdispChnId];
            if (pstVdispChnAttr->bUsed == TRUE && pstVdispChnAttr->bCreate == TRUE)
            {
                CARDVCHECKRESULT(MI_VDISP_DisableInputChannel(VdispDevId, VdispChnId));
                pstVdispChnAttr->bCreate = TRUE;
            }
        }

        CARDVCHECKRESULT(MI_VDISP_CloseDevice(VdispDevId));
        pstVdispDevAttr->bCreate = FALSE;
    }

    return 0;
}

MI_S32 CarDV_VdispModuleBind(MI_VDISP_DEV VdispDevId, MI_BOOL bBind)
{
    CarDV_VdispDevAttr_t *pstVdispDevAttr = &gstVdispModule.stVdispDevAttr[VdispDevId];
    MI_VDISP_CHN          VdispChnId      = 0;

    for (VdispChnId = 0; VdispChnId < VDISP_MAX_CHN_NUM_PER_DEV + VDISP_OVERLAY_CHN_NUM; VdispChnId++)
    {
        CarDV_VdispChannelAttr_t *pstVdispChnAttr = &pstVdispDevAttr->stVdispChnlAttr[VdispChnId];
        if (pstVdispChnAttr->bUsed == TRUE && pstVdispChnAttr->bCreate == TRUE)
        {
            CarDV_Sys_BindInfo_T stBindInfo;
            memset(&stBindInfo, 0x0, sizeof(CarDV_Sys_BindInfo_T));
            stBindInfo.stSrcChnPort.eModId    = pstVdispChnAttr->stBindParam[0].stChnPort.eModId;
            stBindInfo.stSrcChnPort.u32DevId  = pstVdispChnAttr->stBindParam[0].stChnPort.u32DevId;
            stBindInfo.stSrcChnPort.u32ChnId  = pstVdispChnAttr->stBindParam[0].stChnPort.u32ChnId;
            stBindInfo.stSrcChnPort.u32PortId = pstVdispChnAttr->stBindParam[0].stChnPort.u32PortId;
            stBindInfo.stDstChnPort.eModId    = E_MI_MODULE_ID_VDISP;
            stBindInfo.stDstChnPort.u32DevId  = VdispDevId;
            stBindInfo.stDstChnPort.u32ChnId  = VdispChnId;
            stBindInfo.stDstChnPort.u32PortId = 0;
            stBindInfo.u32SrcFrmrate          = pstVdispChnAttr->stBindParam[0].u32SrcFrmrate;
            stBindInfo.u32DstFrmrate          = pstVdispChnAttr->stBindParam[0].u32DstFrmrate;
            stBindInfo.eBindType              = pstVdispChnAttr->stBindParam[0].eBindType;
            if (bBind && pstVdispChnAttr->bBind == FALSE)
            {
                CARDVCHECKRESULT(CarDV_Sys_Bind(&stBindInfo));
                pstVdispChnAttr->bBind = TRUE;
            }
            else if (bBind == FALSE && pstVdispChnAttr->bBind)
            {
                CARDVCHECKRESULT(CarDV_Sys_UnBind(&stBindInfo));
                pstVdispChnAttr->bBind = FALSE;
            }
        }
    }

    return 0;
}
#endif