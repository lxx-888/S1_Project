/*
 * module_common.cpp - Sigmastar
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
#include "module_sensor.h"
#include "module_vif.h"
#include "module_isp.h"
#if (CARDV_LDC_ENABLE)
#include "module_ldc.h"
#endif
#include "module_scl.h"
#include "module_vdisp.h"
#include "module_jpd.h"
#include "module_vdec.h"
#if (CARDV_DISPLAY_ENABLE)
#include "module_display.h"
#endif
#include "module_mipitx.h"
#include "module_rtspclient.h"
#include "module_ws.h"

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_VideoPipeInit()
{
    // Set Anadec src attr before sensor init.
    for (MI_VIF_DEV u32VifDev = 0; u32VifDev < CARDV_MAX_VIF_DEV_NUM; u32VifDev++)
    {
        CARDVCHECKRESULT(CarDv_VifSetAnadecSrcAttr(u32VifDev, NULL));
    }

    CARDVCHECKRESULT(MI_SNR_InitDev(NULL));
    for (MI_SNR_PADID u32SnrPad = 0; u32SnrPad < CARDV_MAX_SENSOR_NUM; u32SnrPad++)
    {
        CARDVCHECKRESULT(CarDV_SensorModuleInit(u32SnrPad));
    }

    for (MI_VIF_GROUP u32VifGroup = 0; u32VifGroup < CARDV_MAX_VIF_GROUP_NUM; u32VifGroup++)
    {
        CARDVCHECKRESULT(CarDV_VifModuleInit(u32VifGroup));
    }

#if (CARDV_JPD_ENABLE)
    for (MI_JPD_CHN u32JpdChn = 0; u32JpdChn < CARDV_MAX_JPD_CHN_NUM; u32JpdChn++)
    {
        CARDVCHECKRESULT(CarDV_JpdModuleInit(u32JpdChn));
    }
#endif

#if (CARDV_VDEC_ENABLE)
    for (MI_VDEC_CHN u32VdecChn = 0; u32VdecChn < CARDV_MAX_VDEC_CHN_NUM; u32VdecChn++)
    {
        CARDVCHECKRESULT(CarDV_VdecModuleInit(u32VdecChn));
    }
#endif

    for (MI_ISP_DEV u32IspDevId = 0; u32IspDevId < CARDV_MAX_ISP_DEV_NUM; u32IspDevId++)
    {
        CARDVCHECKRESULT(CarDV_IspModuleInit(u32IspDevId));
    }

#if (CARDV_LDC_ENABLE)
    for (MI_LDC_DEV u32LdcDevId = 0; u32LdcDevId < CARDV_MAX_LDC_DEV_NUM; u32LdcDevId++)
    {
        CARDVCHECKRESULT(CarDV_LdcModuleInit(u32LdcDevId));
    }
#endif

    for (MI_SCL_DEV u32SclDevId = 0; u32SclDevId < CARDV_MAX_SCL_DEV_NUM; u32SclDevId++)
    {
        CARDVCHECKRESULT(CarDV_SclModuleInit(u32SclDevId));
    }

#if (CARDV_VDISP_ENABLE)
    CARDVCHECKRESULT_OS(MI_VDISP_InitDev(NULL));
    for (MI_VDISP_DEV s32VdispDevId = 0; s32VdispDevId < VDISP_MAX_DEVICE_NUM; s32VdispDevId++)
    {
        CARDVCHECKRESULT(CarDV_VdispModuleInit(s32VdispDevId));
    }
#endif

#if (CARDV_DISPLAY_ENABLE)
    CARDVCHECKRESULT(CarDV_DispModuleInit());
#endif

#if (CARDV_MIPITX_ENABLE)
    CARDVCHECKRESULT(CarDV_MipiTxModuleInit());
#endif

#if (CARDV_RTSP_INPUT_ENABLE)
    for (MI_U8 u8RtspChn = 0; u8RtspChn < RTSP_MAX_STREAM_CNT; u8RtspChn++)
    {
        CARDVCHECKRESULT(CarDV_RtspClientInit(u8RtspChn));
    }
#endif

#if (CARDV_WS_INPUT_ENABLE)
    for (MI_U8 u8WsChn = 0; u8WsChn < WS_MAX_STREAM_CNT; u8WsChn++)
    {
        CARDVCHECKRESULT(CarDV_WsClientInit(u8WsChn));
    }
#endif

    return 0;
}

MI_S32 CarDV_VideoPipeUnInit()
{
#if (CARDV_WS_INPUT_ENABLE)
    for (MI_U8 u8WsChn = 0; u8WsChn < WS_MAX_STREAM_CNT; u8WsChn++)
    {
        CARDVCHECKRESULT(CarDV_WsClientUnInit(u8WsChn));
    }
#endif

#if (CARDV_RTSP_INPUT_ENABLE)
    for (MI_U8 u8RtspChn = 0; u8RtspChn < RTSP_MAX_STREAM_CNT; u8RtspChn++)
    {
        CARDVCHECKRESULT(CarDV_RtspClientUnInit(u8RtspChn));
    }
#endif

#if (CARDV_MIPITX_ENABLE)
    CARDVCHECKRESULT(CarDV_MipiTxModuleUnInit());
#endif

#if (CARDV_DISPLAY_ENABLE)
    CARDVCHECKRESULT(CarDV_DispModuleUnInit());
#endif

#if (CARDV_VDISP_ENABLE)
    for (MI_VDISP_DEV s32VdispDevId = 0; s32VdispDevId < VDISP_MAX_DEVICE_NUM; s32VdispDevId++)
    {
        CARDVCHECKRESULT(CarDV_VdispModuleUnInit(s32VdispDevId));
    }
    CARDVCHECKRESULT(MI_VDISP_DeInitDev());
#endif

    for (MI_SCL_DEV u32SclDevId = 0; u32SclDevId < CARDV_MAX_SCL_DEV_NUM; u32SclDevId++)
    {
        CARDVCHECKRESULT(CarDV_SclModuleUnInit(u32SclDevId));
    }

#if (CARDV_LDC_ENABLE)
    for (MI_LDC_DEV u32LdcDevId = 0; u32LdcDevId < CARDV_MAX_LDC_DEV_NUM; u32LdcDevId++)
    {
        CARDVCHECKRESULT(CarDV_LdcModuleUnInit(u32LdcDevId));
    }
#endif

    for (MI_ISP_DEV u32IspDevId = 0; u32IspDevId < CARDV_MAX_ISP_DEV_NUM; u32IspDevId++)
    {
        CARDVCHECKRESULT(CarDV_IspModuleUnInit(u32IspDevId));
    }

#if (CARDV_VDEC_ENABLE)
    for (MI_VDEC_CHN u32VdecChn = 0; u32VdecChn < CARDV_MAX_VDEC_CHN_NUM; u32VdecChn++)
    {
        CARDVCHECKRESULT(CarDV_VdecModuleUnInit(u32VdecChn));
    }
#endif

#if (CARDV_JPD_ENABLE)
    for (MI_JPD_CHN u32JpdChn = 0; u32JpdChn < CARDV_MAX_JPD_CHN_NUM; u32JpdChn++)
    {
        CARDVCHECKRESULT(CarDV_JpdModuleUnInit(u32JpdChn));
    }
#endif

    for (MI_VIF_GROUP u32VifGroup = 0; u32VifGroup < CARDV_MAX_VIF_GROUP_NUM; u32VifGroup++)
    {
        CARDVCHECKRESULT(CarDV_VifModuleUnInit(u32VifGroup));
    }

    for (MI_SNR_PADID u32SnrPad = 0; u32SnrPad < CARDV_MAX_SENSOR_NUM; u32SnrPad++)
    {
        CARDVCHECKRESULT(CarDV_SensorModuleUnInit(u32SnrPad));
    }
    CARDVCHECKRESULT(MI_SNR_DeInitDev());

    return 0;
}

MI_S32 CarDV_VideoPipeBind(MI_BOOL bBind)
{
    for (MI_ISP_DEV u32IspDevId = 0; u32IspDevId < CARDV_MAX_ISP_DEV_NUM; u32IspDevId++)
    {
        CARDVCHECKRESULT(CarDV_IspModuleBind(u32IspDevId, bBind));
    }

    for (MI_SCL_DEV u32SclDevId = 0; u32SclDevId < CARDV_MAX_SCL_DEV_NUM; u32SclDevId++)
    {
        CARDVCHECKRESULT(CarDV_SclModuleBind(u32SclDevId, bBind));
    }

#if (CARDV_VDISP_ENABLE)
    for (MI_VDISP_DEV s32VdispDevId = 0; s32VdispDevId < VDISP_MAX_DEVICE_NUM; s32VdispDevId++)
    {
        CARDVCHECKRESULT(CarDV_VdispModuleBind(s32VdispDevId, bBind));
    }
#endif

#if (CARDV_LDC_ENABLE)
    for (MI_LDC_DEV u32LdcDevId = 0; u32LdcDevId < CARDV_MAX_LDC_DEV_NUM; u32LdcDevId++)
    {
        CARDVCHECKRESULT(CarDV_LdcModuleBind(u32LdcDevId, bBind));
    }
#endif

#if (CARDV_DISPLAY_ENABLE)
    CARDVCHECKRESULT(CarDV_DispModuleBind(bBind));
#endif

#if (CARDV_MIPITX_ENABLE)
    CARDVCHECKRESULT(CarDV_MipiTxModuleBind(bBind));
#endif

    return 0;
}

MI_S32 CarDV_VideoPipeSwitchHDR(MI_BOOL bHdrEnable)
{
    // hard code test.
    // TODO : switch according to pipe line.
    MI_SNR_PADID            u32SnrPad       = 0;
    MI_VIF_GROUP            u32VifGroup     = 0;
    MI_ISP_DEV              u32IspDevId     = 0;
    MI_ISP_CHANNEL          u32IspChnId     = 0;
    CarDV_Sensor_Attr_t *   pstSensorAttr   = &gstSensorAttr[u32SnrPad];
    CarDV_VifGroupAttr_t *  pstVifGroupAttr = &gstVifModule.stVifGroupAttr[u32VifGroup];
    CarDV_IspDevAttr_t *    pstIspDevAttr   = &gstIspModule.stIspDevAttr[u32IspDevId];
    CarDV_IspChannelAttr_t *pstIspChnAttr   = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];

    if (pstVifGroupAttr->eHDRType == E_MI_VIF_HDR_TYPE_OFF && bHdrEnable)
    {
        printf("HDR OFF -> DOL\n");
        pstVifGroupAttr->eHDRType             = E_MI_VIF_HDR_TYPE_DOL;
        pstIspChnAttr->stIspChnParam.eHDRType = E_MI_ISP_HDR_TYPE_DOL;
    }
    else if (pstVifGroupAttr->eHDRType != E_MI_VIF_HDR_TYPE_OFF && !bHdrEnable)
    {
        printf("HDR DOL -> OFF\n");
        pstVifGroupAttr->eHDRType             = E_MI_VIF_HDR_TYPE_OFF;
        pstIspChnAttr->stIspChnParam.eHDRType = E_MI_ISP_HDR_TYPE_OFF;
    }
    else
    {
        printf("HDR NOT change\n");
        return 0;
    }

    if (E_MI_VIF_HDR_TYPE_VC == pstVifGroupAttr->eHDRType || E_MI_VIF_HDR_TYPE_DOL == pstVifGroupAttr->eHDRType)
    {
        pstSensorAttr->bPlaneMode = TRUE;
    }
    else
    {
        pstSensorAttr->bPlaneMode = FALSE;
    }

    CARDVCHECKRESULT(CarDV_IspChannelBind(u32IspDevId, u32IspChnId, FALSE));
    CARDVCHECKRESULT(MI_ISP_StopChannel(u32IspDevId, u32IspChnId));
    CARDVCHECKRESULT(CarDV_VifModuleUnInit(u32VifGroup));
    CARDVCHECKRESULT(CarDV_SensorModuleUnInit(u32SnrPad));
    CARDVCHECKRESULT(CarDV_SensorModuleInit(u32SnrPad));
    CARDVCHECKRESULT(CarDV_VifModuleInit(u32VifGroup));
    CARDVCHECKRESULT(MI_ISP_SetChnParam(u32IspDevId, u32IspChnId, &pstIspChnAttr->stIspChnParam));
    CARDVCHECKRESULT(MI_ISP_StartChannel(u32IspDevId, u32IspChnId));
    CARDVCHECKRESULT(CarDV_IspChannelBind(u32IspDevId, u32IspChnId, TRUE));
    return 0;
}

MI_S32 CarDV_VideoPipeEnableSensor(MI_SNR_PADID u32SnrPad)
{
    MI_VIF_GROUP   u32VifGroup      = 0;
    MI_VIF_DEV     u32VifDev        = 0;
    MI_S8          vifDevIdPerGroup = 0;
    MI_ISP_DEV     u32IspDevId      = 0;
    MI_ISP_CHANNEL u32IspChnId      = 0;
    MI_SCL_DEV     u32SclDevId      = 0;
    MI_SCL_CHANNEL u32SclChnId      = 0;
#if (CARDV_LDC_ENABLE)
    MI_LDC_DEV              u32LdcDevId = 0;
    MI_LDC_CHN              u32LdcChnId = 0;
    MI_VIF_OutputPortAttr_t stVifOutAttr;
#endif
    CarDV_VifGroupAttr_t *pstVifGroupAttr;
    CarDV_VifDevAttr_t *  pstVifDevAttr;

    for (u32VifGroup = 0; u32VifGroup < CARDV_MAX_VIF_GROUP_NUM; u32VifGroup++)
    {
        pstVifGroupAttr = &gstVifModule.stVifGroupAttr[u32VifGroup];
        if (pstVifGroupAttr->bUsed == TRUE && u32SnrPad == (MI_SNR_PADID)pstVifGroupAttr->stGroupExtAttr.eSensorPadID)
        {
            // Found VIF group
            break;
        }
    }

    if (u32VifGroup == CARDV_MAX_VIF_GROUP_NUM)
        return 0;

    CARDVCHECKRESULT(CarDV_SensorModuleInit(u32SnrPad));
    CARDVCHECKRESULT(CarDV_VifModuleInit(u32VifGroup));

    // Bind SCL or ISP according to pipe line
    for (vifDevIdPerGroup = 0; vifDevIdPerGroup < CARDV_MAX_VIF_DEV_PERGROUP; vifDevIdPerGroup++)
    {
        pstVifDevAttr = &pstVifGroupAttr->stVifDevAttr[vifDevIdPerGroup];
        if (pstVifDevAttr->bUsed == TRUE && pstVifDevAttr->bCreate == TRUE)
        {
            u32VifDev = u32VifGroup * CARDV_MAX_VIF_DEV_PERGROUP + vifDevIdPerGroup;

#if (CARDV_LDC_ENABLE)
            if (CarDV_LdcGetDevChnByVifDev(u32VifDev, &u32LdcDevId, &u32LdcChnId) == 0)
            {
                // TBD : Use LDC source output attr is better, but LDC source module maybe disabled.
                CARDVCHECKRESULT(MI_VIF_GetOutputPortAttr(u32VifDev, 0, &stVifOutAttr));
                if (stVifOutAttr.stDestSize.u16Width == 1920 && stVifOutAttr.stDestSize.u16Height == 1080)
                {
                    CarDV_LdcChannelSetCfg(u32LdcDevId, u32LdcChnId, "/bootconfig/bin/AMTK_RSC_1920x1080.json");
                }
                else if (stVifOutAttr.stDestSize.u16Width == 3840 && stVifOutAttr.stDestSize.u16Height == 2160)
                {
                    CarDV_LdcChannelSetCfg(u32LdcDevId, u32LdcChnId, "/bootconfig/bin/AMTK_RSC_3840x2160.json");
                }

                CARDVCHECKRESULT(CarDV_LdcChannelInit(u32LdcDevId, u32LdcChnId));
                CARDVCHECKRESULT(CarDV_LdcChannelBind(u32LdcDevId, u32LdcChnId, TRUE));
            }
#endif

            if (CarDV_IspGetDevChnByVifDev(u32VifDev, &u32IspDevId, &u32IspChnId) == 0)
            {
                CARDVCHECKRESULT(CarDV_IspChannelBind(u32IspDevId, u32IspChnId, TRUE));
            }

            if (CarDV_SclGetDevChnByVifDev(u32VifDev, &u32SclDevId, &u32SclChnId) == 0)
            {
                CARDVCHECKRESULT(CarDV_SclChannelBind(u32SclDevId, u32SclChnId, TRUE));
            }
        }
    }

    return 0;
}

MI_S32 CarDV_VideoPipeDisableSensor(MI_SNR_PADID u32SnrPad)
{
    MI_VIF_GROUP   u32VifGroup      = 0;
    MI_VIF_DEV     u32VifDev        = 0;
    MI_S8          vifDevIdPerGroup = 0;
    MI_ISP_DEV     u32IspDevId      = 0;
    MI_ISP_CHANNEL u32IspChnId      = 0;
    MI_SCL_DEV     u32SclDevId      = 0;
    MI_SCL_CHANNEL u32SclChnId      = 0;
#if (CARDV_LDC_ENABLE)
    MI_LDC_DEV u32LdcDevId = 0;
    MI_LDC_CHN u32LdcChnId = 0;
#endif
    CarDV_VifGroupAttr_t *pstVifGroupAttr;
    CarDV_VifDevAttr_t *  pstVifDevAttr;

    for (u32VifGroup = 0; u32VifGroup < CARDV_MAX_VIF_GROUP_NUM; u32VifGroup++)
    {
        pstVifGroupAttr = &gstVifModule.stVifGroupAttr[u32VifGroup];
        if (pstVifGroupAttr->bUsed == TRUE && pstVifGroupAttr->bCreate == TRUE
            && u32SnrPad == (MI_SNR_PADID)pstVifGroupAttr->stGroupExtAttr.eSensorPadID)
        {
            // Found VIF group
            break;
        }
    }

    if (u32VifGroup == CARDV_MAX_VIF_GROUP_NUM)
        return 0;

    // Unbind SCL or ISP according to pipe line
    pstVifGroupAttr = &gstVifModule.stVifGroupAttr[u32VifGroup];
    for (vifDevIdPerGroup = 0; vifDevIdPerGroup < CARDV_MAX_VIF_DEV_PERGROUP; vifDevIdPerGroup++)
    {
        pstVifDevAttr = &pstVifGroupAttr->stVifDevAttr[vifDevIdPerGroup];
        if (pstVifDevAttr->bUsed == TRUE && pstVifDevAttr->bCreate == TRUE)
        {
            u32VifDev = u32VifGroup * CARDV_MAX_VIF_DEV_PERGROUP + vifDevIdPerGroup;

            if (CarDV_IspGetDevChnByVifDev(u32VifDev, &u32IspDevId, &u32IspChnId) == 0)
            {
                CARDVCHECKRESULT(CarDV_IspChannelBind(u32IspDevId, u32IspChnId, FALSE));
            }

            if (CarDV_SclGetDevChnByVifDev(u32VifDev, &u32SclDevId, &u32SclChnId) == 0)
            {
                CARDVCHECKRESULT(CarDV_SclChannelBind(u32SclDevId, u32SclChnId, FALSE));
            }

#if (CARDV_LDC_ENABLE)
            if (CarDV_LdcGetDevChnByVifDev(u32VifDev, &u32LdcDevId, &u32LdcChnId) == 0)
            {
                CARDVCHECKRESULT(CarDV_LdcChannelBind(u32LdcDevId, u32LdcChnId, FALSE));
                CARDVCHECKRESULT(CarDV_LdcChannelUnInit(u32LdcDevId, u32LdcChnId));
            }
#endif
        }
    }

    CARDVCHECKRESULT(CarDV_VifModuleUnInit(u32VifGroup));
    CARDVCHECKRESULT(CarDV_SensorModuleUnInit(u32SnrPad));
    return 0;
}

#if (CARDV_LDC_ENABLE)
MI_S32 CarDV_VideoPipeEnableLdc(void)
{
    MI_SCL_DEV              u32SclDevId = 0;
    MI_SCL_CHANNEL          u32SclChnId = 0;
    MI_LDC_DEV              u32LdcDevId = 0;
    MI_LDC_CHN              u32LdcChnId = 0;
    CarDV_Sys_BindInfo_T    stBindInfo;
    CarDV_LdcDevAttr_t *    pstLdcDevAttr;
    CarDV_LdcChannelAttr_t *pstLdcChnAttr;

    for (u32SclDevId = 0; u32SclDevId < CARDV_MAX_SCL_DEV_NUM; u32SclDevId++)
    {
        CarDV_SclDevAttr_t *pstSclDevAttr = &gstSclModule.stSclDevAttr[u32SclDevId];
        if (pstSclDevAttr->bUsed == TRUE)
        {
            for (u32SclChnId = 0; u32SclChnId < CARDV_MAX_SCL_CHN_NUM; u32SclChnId++)
            {
                CarDV_SclChannelAttr_t *pstSclChnAttr    = &pstSclDevAttr->stSclChnlAttr[u32SclChnId];
                CarDV_InPortAttr_t *    pstSclInPortAttr = &pstSclChnAttr->stSclInPortAttr[0];
                if (pstSclChnAttr->bUsed == TRUE
                    && pstSclInPortAttr->stBindParam.stChnPort.eModId == E_MI_MODULE_ID_LDC)
                {
                    u32LdcDevId   = pstSclInPortAttr->stBindParam.stChnPort.u32DevId;
                    u32LdcChnId   = pstSclInPortAttr->stBindParam.stChnPort.u32ChnId;
                    pstLdcDevAttr = &gstLdcModule.stLdcDevAttr[u32LdcDevId];
                    pstLdcChnAttr = &pstLdcDevAttr->stLdcChnlAttr[u32LdcChnId];

                    if (pstLdcChnAttr->bCreate == FALSE)
                    {
                        // unbind LDC input module -> SCL
                        memset(&stBindInfo, 0x0, sizeof(CarDV_Sys_BindInfo_T));
                        stBindInfo.stSrcChnPort.eModId    = pstLdcChnAttr->stBindParam.stChnPort.eModId;
                        stBindInfo.stSrcChnPort.u32DevId  = pstLdcChnAttr->stBindParam.stChnPort.u32DevId;
                        stBindInfo.stSrcChnPort.u32ChnId  = pstLdcChnAttr->stBindParam.stChnPort.u32ChnId;
                        stBindInfo.stSrcChnPort.u32PortId = pstLdcChnAttr->stBindParam.stChnPort.u32PortId;
                        stBindInfo.stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
                        stBindInfo.stDstChnPort.u32DevId  = u32SclDevId;
                        stBindInfo.stDstChnPort.u32ChnId  = u32SclChnId;
                        CARDVCHECKRESULT(CarDV_Sys_UnBind(&stBindInfo));

                        pstLdcChnAttr->bBypass = FALSE;

                        // init LDC channel
                        CARDVCHECKRESULT(CarDV_LdcChannelInit(u32LdcDevId, u32LdcChnId));

                        // bind LDC input module -> LDC
                        CARDVCHECKRESULT(CarDV_LdcChannelBind(u32LdcDevId, u32LdcChnId, TRUE));

                        // bind LDC -> SCL
                        CARDVCHECKRESULT(CarDV_SclChannelBind(u32SclDevId, u32SclChnId, TRUE));

                        if (pstLdcChnAttr->stBindParam.stChnPort.eModId == E_MI_MODULE_ID_ISP)
                        {
                            CARDVCHECKRESULT(CarDV_IspZoomReset(pstLdcChnAttr->stBindParam.stChnPort.u32DevId,
                                                                pstLdcChnAttr->stBindParam.stChnPort.u32ChnId));
                        }
                    }
                }
            }
        }
    }

    return 0;
}

MI_S32 CarDV_VideoPipeDisableLdc(void)
{
    MI_SCL_DEV              u32SclDevId = 0;
    MI_SCL_CHANNEL          u32SclChnId = 0;
    MI_LDC_DEV              u32LdcDevId = 0;
    MI_LDC_CHN              u32LdcChnId = 0;
    CarDV_Sys_BindInfo_T    stBindInfo;
    CarDV_LdcDevAttr_t *    pstLdcDevAttr;
    CarDV_LdcChannelAttr_t *pstLdcChnAttr;

    for (u32SclDevId = 0; u32SclDevId < CARDV_MAX_SCL_DEV_NUM; u32SclDevId++)
    {
        CarDV_SclDevAttr_t *pstSclDevAttr = &gstSclModule.stSclDevAttr[u32SclDevId];
        if (pstSclDevAttr->bUsed == TRUE)
        {
            for (u32SclChnId = 0; u32SclChnId < CARDV_MAX_SCL_CHN_NUM; u32SclChnId++)
            {
                CarDV_SclChannelAttr_t *pstSclChnAttr    = &pstSclDevAttr->stSclChnlAttr[u32SclChnId];
                CarDV_InPortAttr_t *    pstSclInPortAttr = &pstSclChnAttr->stSclInPortAttr[0];
                if (pstSclChnAttr->bUsed == TRUE
                    && pstSclInPortAttr->stBindParam.stChnPort.eModId == E_MI_MODULE_ID_LDC)
                {
                    u32LdcDevId   = pstSclInPortAttr->stBindParam.stChnPort.u32DevId;
                    u32LdcChnId   = pstSclInPortAttr->stBindParam.stChnPort.u32ChnId;
                    pstLdcDevAttr = &gstLdcModule.stLdcDevAttr[u32LdcDevId];
                    pstLdcChnAttr = &pstLdcDevAttr->stLdcChnlAttr[u32LdcChnId];

                    if (pstLdcChnAttr->bCreate == TRUE)
                    {
                        // unbind LDC -> SCL
                        CARDVCHECKRESULT(CarDV_SclChannelBind(u32SclDevId, u32SclChnId, FALSE));

                        // unbind LDC input module -> LDC
                        CARDVCHECKRESULT(CarDV_LdcChannelBind(u32LdcDevId, u32LdcChnId, FALSE));

                        // deinit LDC channel
                        CARDVCHECKRESULT(CarDV_LdcChannelUnInit(u32LdcDevId, u32LdcChnId));

                        pstLdcChnAttr->bBypass = TRUE;

                        // bind LDC input module -> SCL
                        memset(&stBindInfo, 0x0, sizeof(CarDV_Sys_BindInfo_T));
                        stBindInfo.stSrcChnPort.eModId    = pstLdcChnAttr->stBindParam.stChnPort.eModId;
                        stBindInfo.stSrcChnPort.u32DevId  = pstLdcChnAttr->stBindParam.stChnPort.u32DevId;
                        stBindInfo.stSrcChnPort.u32ChnId  = pstLdcChnAttr->stBindParam.stChnPort.u32ChnId;
                        stBindInfo.stSrcChnPort.u32PortId = pstLdcChnAttr->stBindParam.stChnPort.u32PortId;
                        stBindInfo.stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
                        stBindInfo.stDstChnPort.u32DevId  = u32SclDevId;
                        stBindInfo.stDstChnPort.u32ChnId  = u32SclChnId;
                        stBindInfo.stDstChnPort.u32PortId = 0;
                        stBindInfo.u32SrcFrmrate          = 30;
                        stBindInfo.u32DstFrmrate          = 30;
                        stBindInfo.eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
                        CARDVCHECKRESULT(CarDV_Sys_Bind(&stBindInfo));

                        if (pstLdcChnAttr->stBindParam.stChnPort.eModId == E_MI_MODULE_ID_ISP)
                        {
                            CARDVCHECKRESULT(CarDV_IspZoomReset(pstLdcChnAttr->stBindParam.stChnPort.u32DevId,
                                                                pstLdcChnAttr->stBindParam.stChnPort.u32ChnId));
                        }
                    }
                }
            }
        }
    }

    return 0;
}
#endif

MI_S32 CarDV_VideoPipeSwitchSensorResIdx(MI_SNR_PADID u32SnrPad, MI_U8 u8ResIndex)
{
    CarDV_Sensor_Attr_t *pstSensorAttr = &gstSensorAttr[u32SnrPad];

    CARDVCHECKRESULT(CarDV_VideoPipeDisableSensor(u32SnrPad));
    pstSensorAttr->u8ResIndex = u8ResIndex;
    CARDVCHECKRESULT(CarDV_VideoPipeEnableSensor(u32SnrPad));

    return 0;
}

MI_S32 CarDV_VideoPipeSetAnadecSrcAttr(MI_U8 u8CamID, MI_SNR_Anadec_SrcAttr_t *pstAnadecSrcAttr)
{
    MI_SNR_PADID u32SnrPad        = -1;
    MI_VIF_DEV   u32VifDev        = -1;
    MI_VIF_GROUP u32VifGroup      = 0;
    MI_S8        vifDevIdPerGroup = 0;

    for (u32VifGroup = 0; u32VifGroup < CARDV_MAX_VIF_GROUP_NUM; u32VifGroup++)
    {
        CarDV_VifGroupAttr_t *pstVifGroupAttr = &gstVifModule.stVifGroupAttr[u32VifGroup];
        if (pstVifGroupAttr->bUsed == TRUE && pstVifGroupAttr->bCreate == TRUE)
        {
            for (vifDevIdPerGroup = 0; vifDevIdPerGroup < CARDV_MAX_VIF_DEV_PERGROUP; vifDevIdPerGroup++)
            {
                CarDV_VifDevAttr_t *pstVifDevAttr = &pstVifGroupAttr->stVifDevAttr[vifDevIdPerGroup];
                if (pstVifDevAttr->bUsed == TRUE && pstVifDevAttr->bCreate == TRUE && pstVifDevAttr->u8CamId == u8CamID)
                {
                    u32SnrPad = (MI_SNR_PADID)pstVifGroupAttr->stGroupExtAttr.eSensorPadID;
                    u32VifDev = u32VifGroup * CARDV_MAX_VIF_DEV_PERGROUP + vifDevIdPerGroup;
                    // Found sensor pad and VIF dev
                    break;
                }
            }

            if (vifDevIdPerGroup != CARDV_MAX_VIF_DEV_PERGROUP)
                break;
        }
    }

    if (u32VifDev >= CARDV_MAX_VIF_DEV_NUM || u32SnrPad >= CARDV_MAX_SENSOR_NUM)
        return 0;

    CARDVCHECKRESULT(CarDv_VifSetAnadecSrcAttr(u32VifDev, pstAnadecSrcAttr));
    CARDVCHECKRESULT(CarDV_VideoPipeSwitchSensorResIdx(u32SnrPad, 0));

    return 0;
}

MI_S32 video_pipe_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    switch (id)
    {
    case CMD_PIPE_INIT:
        CARDVCHECKRESULT(CarDV_VideoPipeInit());
        CARDVCHECKRESULT(CarDV_VideoPipeBind(TRUE));
        CARDVCHECKRESULT(CarDV_StartAnaDecDetectThread());
        break;

    case CMD_PIPE_UNINIT:
        CARDVCHECKRESULT(CarDV_StopAnaDecDetectThread());
        CARDVCHECKRESULT(CarDV_VideoPipeBind(FALSE));
        CARDVCHECKRESULT(CarDV_VideoPipeUnInit());
        break;

    case CMD_PIPE_SENSOR_ENABLE:
        for (MI_SNR_PADID u32SnrPad = 0; u32SnrPad < CARDV_MAX_SENSOR_NUM; u32SnrPad++)
        {
            CARDVCHECKRESULT(CarDV_VideoPipeEnableSensor(u32SnrPad));
        }
        break;

    case CMD_PIPE_SENSOR_DISABLE:
        for (MI_SNR_PADID u32SnrPad = 0; u32SnrPad < CARDV_MAX_SENSOR_NUM; u32SnrPad++)
        {
            CARDVCHECKRESULT(CarDV_VideoPipeDisableSensor(u32SnrPad));
        }
        break;

    case CMD_PIPE_SWITCH_HDR: {
        MI_U32  pipeParam[1];
        MI_BOOL bHdrEnable;
        memcpy(pipeParam, param, sizeof(pipeParam));
        bHdrEnable = pipeParam[0];
        CARDVCHECKRESULT(CarDV_VideoPipeSwitchHDR(bHdrEnable));
    }
    break;

    case CMD_PIPE_SWITCH_SENSOR_RES_IDX: {
        MI_U32       pipeParam[2];
        MI_U8        u8ResIdx;
        MI_SNR_PADID u32SnrPad;

        memcpy(pipeParam, param, sizeof(pipeParam));
        u32SnrPad = pipeParam[0];
        u8ResIdx  = pipeParam[1];
        CARDVCHECKRESULT(CarDV_VideoPipeSwitchSensorResIdx(u32SnrPad, u8ResIdx));
    }
    break;

    case CMD_PIPE_SWITCH_ANADEC_RES: {
        MI_U32                  pipeParam[5];
        MI_U8                   u8CamId;
        MI_SNR_Anadec_SrcAttr_t stAnadecSrcAttr;

        memcpy(pipeParam, param, sizeof(pipeParam));
        u8CamId = pipeParam[0];
        memset(&stAnadecSrcAttr, 0, sizeof(MI_SNR_Anadec_SrcAttr_t));
        stAnadecSrcAttr.stRes.u16Width  = pipeParam[1];
        stAnadecSrcAttr.stRes.u16Height = pipeParam[2];
        stAnadecSrcAttr.u32Fps          = pipeParam[3];
        stAnadecSrcAttr.eTransferMode   = (MI_SNR_Anadec_TransferMode_e)pipeParam[4];
        CARDVCHECKRESULT(CarDV_VideoPipeSetAnadecSrcAttr(u8CamId, &stAnadecSrcAttr));
    }
    break;

    case CMD_PIPE_ISP_ZOOM: {
        MI_U32 zoomParam[3];
        MI_U32 u32IspDev;
        MI_U32 u32IspChn;
        MI_U32 u32ZoomIn;
        memcpy(zoomParam, param, sizeof(zoomParam));
        u32IspDev = zoomParam[0];
        u32IspChn = zoomParam[1];
        u32ZoomIn = zoomParam[2]; // 0 : Zoom Out 1 : Zoom In 2 : Zoom Stop
        if (u32ZoomIn >= 2)
            CARDVCHECKRESULT(CarDV_IspZoomStop(u32IspDev, u32IspChn));
        else
            CARDVCHECKRESULT(CarDV_IspZoomStart(u32IspDev, u32IspChn, u32ZoomIn));
    }
    break;

#if (CARDV_LDC_ENABLE)
    case CMD_PIPE_LDC_ENABLE: {
        CARDVCHECKRESULT(CarDV_VideoPipeEnableLdc());
    }
    break;

    case CMD_PIPE_LDC_DISABLE: {
        CARDVCHECKRESULT(CarDV_VideoPipeDisableLdc());
    }
    break;
#endif

    default:
        break;
    }

    return 0;
}
