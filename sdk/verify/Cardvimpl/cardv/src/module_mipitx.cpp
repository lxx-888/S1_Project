/*
 * module_mipitx.cpp- Sigmastar
 *
 * Copyright (C) 2020 Sigmastar Technology Corp.
 *
 * Author: gene.ma <gene.ma@sigmastar.com.cn>
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
#include "module_config.h"
#include "module_mipitx.h"

#if (CARDV_MIPITX_ENABLE)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

CarDV_MipiTxModAttr_t gstMipiTxModule = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_MipiTxModuleInit(void)
{
    CarDV_MipiTxChnAttr_t *pstMipiTxChnAttr = &gstMipiTxModule.stMipiTxChnAttr;

    if (pstMipiTxChnAttr->bUsed)
    {
        MI_MipiTx_ChannelAttr_t stMipiTxChAttr;
        memset(&stMipiTxChAttr, 0x00, sizeof(MI_MipiTx_ChannelAttr_t));
        stMipiTxChAttr.u32Width    = pstMipiTxChnAttr->u32Width;
        stMipiTxChAttr.u32Height   = pstMipiTxChnAttr->u32Height;
        stMipiTxChAttr.ePixFormat  = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        stMipiTxChAttr.eLaneNum    = E_MI_MIPITX_LANE_NUM_4;
        stMipiTxChAttr.u8DCLKDelay = 0;
        // convertboard max:540000000, 4lane1080p30: 445500000 4lane4k30:1188000000 2lane1080p30:720000000, 2lane4k30:
        stMipiTxChAttr.u32Dclk = 445500000;

        MI_MipiTx_TimingConfig_t stMipiTimingCfg;
        memset(&stMipiTimingCfg, 0x00, sizeof(MI_MipiTx_TimingConfig_t));
        stMipiTimingCfg.u8Lpx       = 0x07;
        stMipiTimingCfg.u8ClkHsPrpr = 0x08;
        stMipiTimingCfg.u8ClkZero   = 0x20;
        stMipiTimingCfg.u8ClkHsPre  = 0x01;
        stMipiTimingCfg.u8ClkHsPost = 0x10;
        stMipiTimingCfg.u8ClkTrail  = 0x0A;
        stMipiTimingCfg.u8HsPrpr    = 0x01;
        stMipiTimingCfg.u8HsZero    = 0x0F;
        stMipiTimingCfg.u8HsTrail   = 0x0A;

        CARDVCHECKRESULT(MI_MipiTx_CreateChannel(pstMipiTxChnAttr->u32ChnId, &stMipiTxChAttr));
        CARDVCHECKRESULT(MI_MipiTx_SetTimingConfig(pstMipiTxChnAttr->u32ChnId, &stMipiTimingCfg));
        CARDVCHECKRESULT(MI_MipiTx_StartChannel(pstMipiTxChnAttr->u32ChnId));
        pstMipiTxChnAttr->bCreate = TRUE;
    }

    return 0;
}

MI_S32 CarDV_MipiTxModuleUnInit(void)
{
    CarDV_MipiTxChnAttr_t *pstMipiTxChnAttr = &gstMipiTxModule.stMipiTxChnAttr;

    if (pstMipiTxChnAttr->bUsed && pstMipiTxChnAttr->bCreate)
    {
        CARDVCHECKRESULT(MI_MipiTx_StopChannel(pstMipiTxChnAttr->u32ChnId));
        CARDVCHECKRESULT(MI_MipiTx_DestroyChannel(pstMipiTxChnAttr->u32ChnId));
        pstMipiTxChnAttr->bCreate = FALSE;
    }

    return 0;
}

MI_S32 CarDV_MipiTxModuleBind(MI_BOOL bBind)
{
    CarDV_MipiTxChnAttr_t *pstMipiTxChnAttr = &gstMipiTxModule.stMipiTxChnAttr;

    if (pstMipiTxChnAttr->bUsed && pstMipiTxChnAttr->bCreate)
    {
        CarDV_Sys_BindInfo_T stBindInfo;
        memset(&stBindInfo, 0x0, sizeof(CarDV_Sys_BindInfo_T));
        stBindInfo.stSrcChnPort.eModId    = pstMipiTxChnAttr->stSrcChnPort.eModId;
        stBindInfo.stSrcChnPort.u32DevId  = pstMipiTxChnAttr->stSrcChnPort.u32DevId;
        stBindInfo.stSrcChnPort.u32ChnId  = pstMipiTxChnAttr->stSrcChnPort.u32ChnId;
        stBindInfo.stSrcChnPort.u32PortId = pstMipiTxChnAttr->stSrcChnPort.u32PortId;
        stBindInfo.stDstChnPort.eModId    = E_MI_MODULE_ID_MIPITX;
        stBindInfo.stDstChnPort.u32DevId  = 0;
        stBindInfo.stDstChnPort.u32ChnId  = pstMipiTxChnAttr->u32ChnId;
        stBindInfo.stDstChnPort.u32PortId = 0;
        stBindInfo.u32SrcFrmrate          = 30;
        stBindInfo.u32DstFrmrate          = 30;
        stBindInfo.eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
        if (bBind)
        {
            CARDVCHECKRESULT(CarDV_Sys_Bind(&stBindInfo));
            pstMipiTxChnAttr->bBind = TRUE;
        }
        else if (bBind == FALSE && pstMipiTxChnAttr->bBind)
        {
            CARDVCHECKRESULT(CarDV_Sys_UnBind(&stBindInfo));
            pstMipiTxChnAttr->bBind = FALSE;
        }
    }

    return 0;
}
#endif