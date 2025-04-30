/*
 * module_isp.cpp- Sigmastar
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

#include <pthread.h>
#include "mid_common.h"
#include "mid_utils.h"
#include "module_common.h"
#include "module_isp.h"
#include "module_vif.h"
#if (CARDV_LDC_ENABLE)
#include "module_ldc.h"
#endif
#include "mi_isp_cus3a_api.h"
#include "isp_cus3a_if.h"
#include "mi_isp_iq.h"
#include "mi_isp_iq_datatype.h"
#include "mi_isp_ae.h"
#include "mi_isp_ae_datatype.h"
//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

CarDV_IspModAttr_t gstIspModule = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

static MI_S32 _TransMISnrPadToMIIspBindSensorId(MI_SNR_PADID eMiSnrPadId, MI_ISP_BindSnrId_e *peMiIspSnrBindId)
{
    switch (eMiSnrPadId)
    {
    case 0:
        *peMiIspSnrBindId = E_MI_ISP_SENSOR0;
        break;
    case 1:
        *peMiIspSnrBindId = E_MI_ISP_SENSOR1;
        break;
    case 2:
        *peMiIspSnrBindId = E_MI_ISP_SENSOR2;
        break;
    case 3:
        *peMiIspSnrBindId = E_MI_ISP_SENSOR3;
        break;
    default:
        *peMiIspSnrBindId = E_MI_ISP_SENSOR0;
        printf("[%s]%d snrPad%d fail \n", __FUNCTION__, __LINE__, eMiSnrPadId);
        break;
    }

    return MI_SUCCESS;
}

static void *_LoadApiBinThread(void *args)
{
    CarDV_IspChannelAttr_t *      pstIspChnAttr = (CarDV_IspChannelAttr_t *)args;
    MI_ISP_DEV                    u32IspDevId   = 0;
    MI_ISP_CHANNEL                u32IspChnId   = pstIspChnAttr->u8ChnId;
    MI_ISP_IQ_ParamInitInfoType_t stInitInfo;

    while (!pstIspChnAttr->bThreadExit)
    {
        memset(&stInitInfo, 0, sizeof(MI_ISP_IQ_ParamInitInfoType_t));
        MI_ISP_IQ_GetParaInitStatus(u32IspDevId, u32IspChnId, &stInitInfo);
        if (stInitInfo.stParaAPI.bFlag == FALSE)
        {
            usleep(30000);
            continue;
        }
#if !defined(DUAL_OS) && !defined(__RTOS__) // for pure linux
        MI_U32 u32Key          = 1234;
        MI_U8 *bindata_buf     = NULL;
        MI_U32 BIN_BUF_MAX_LEN = 1024;
        FILE * iqfp            = fopen(pstIspChnAttr->szApiBinFilePath, "rb");

        if (iqfp != NULL)
        {
            fseek(iqfp, 0, SEEK_END);
            BIN_BUF_MAX_LEN = ftell(iqfp);
            bindata_buf     = (MI_U8 *)calloc(BIN_BUF_MAX_LEN, sizeof(MI_U8));
            fseek(iqfp, 0, SEEK_SET);

            memset(bindata_buf, 0, BIN_BUF_MAX_LEN);
            fread(bindata_buf, 1, BIN_BUF_MAX_LEN, iqfp);
            MI_ISP_IQ_ApiCmdLoadBinFile(u32IspDevId, u32IspChnId, bindata_buf, u32Key);
            if (bindata_buf != NULL)
            {
                free(bindata_buf);
                bindata_buf = NULL;
            }
        }
        else
        {
            printf("[%s][%s] File path does not exist. %s not found!\n", __FILE__, __FUNCTION__,
                   pstIspChnAttr->szApiBinFilePath);
        }

        if (iqfp != NULL)
        {
            fclose(iqfp);
            iqfp = NULL;
        }
#endif
        break;
    }

    return NULL;
}

static void *_IspFrameSyncHandlerThread(void *args)
{
    CarDV_IspDevAttr_t *pstIspDevAttr = (CarDV_IspDevAttr_t *)args;
    MI_ISP_CHANNEL      u32IspChnId;
    MI_U32              u32FrameCnt[CARDV_MAX_ISP_CHN_NUM] = {0};
    MI_S32              s32Fd;
    MI_U32              u32FrameStatus;

    Cus3AOpenIspFrameSync(&s32Fd);
    if (s32Fd < 0)
    {
        printf("Open Isp Frame Sync Fail\n");
        return NULL;
    }

    while (!pstIspDevAttr->bThreadExit)
    {
        u32FrameStatus = Cus3AWaitIspFrameSync(s32Fd, 1000);
        for (u32IspChnId = 0; u32IspChnId < CARDV_MAX_ISP_CHN_NUM; ++u32IspChnId)
        {
            if ((u32FrameStatus >> u32IspChnId) & 0x1)
            {
                CarDV_IspChannelAttr_t *pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];

                u32FrameCnt[u32IspChnId]++;

                // recover normal exporsure
                if (u32FrameCnt[u32IspChnId] == pstIspChnAttr->u32ExpoRecoverFrameCnt)
                {
                    MI_ISP_AE_ModeType_e      eAeMode = E_SS_AE_MODE_A;
                    MI_ISP_IQ_ApiBypassType_t stParam;

                    stParam.bEnable   = E_SS_BYPASS_ON;
                    stParam.eAPIIndex = E_API20_WDR_LOC;
                    MI_ISP_IQ_SetApiBypassMode(pstIspDevAttr->u8DevId, u32IspChnId, &stParam);

                    usleep(3000); // sleep 3ms for current frame AE process done
                    MI_ISP_AE_SetExpoMode(pstIspDevAttr->u8DevId, u32IspChnId, &eAeMode);

                    pstIspChnAttr->u32ExpoRecoverFrameCnt = 0;
                }

                // handler long exposure
                if (pstIspChnAttr->u32ShutterUs && pstIspChnAttr->u32ExpoFrameCnt)
                {
                    MI_ISP_AE_ModeType_e      eAeMode = E_SS_AE_MODE_M;
                    MI_ISP_AE_ExpoValueType_t stExpoVal;
                    MI_ISP_IQ_ApiBypassType_t stParam;

                    stParam.bEnable   = E_SS_BYPASS_OFF;
                    stParam.eAPIIndex = E_API20_WDR_LOC;
                    MI_ISP_IQ_SetApiBypassMode(pstIspDevAttr->u8DevId, u32IspChnId, &stParam);
                    usleep(3000); // sleep 3ms for current frame AE process done
                    MI_ISP_AE_SetExpoMode(pstIspDevAttr->u8DevId, u32IspChnId, &eAeMode);
                    MI_ISP_AE_GetManualExpo(pstIspDevAttr->u8DevId, u32IspChnId, &stExpoVal);
                    stExpoVal.u32US = pstIspChnAttr->u32ShutterUs;
                    MI_ISP_AE_SetManualExpo(pstIspDevAttr->u8DevId, u32IspChnId, &stExpoVal);

                    pstIspChnAttr->u32ExpoRecoverFrameCnt = pstIspChnAttr->u32ExpoFrameCnt + u32FrameCnt[u32IspChnId];
                    pstIspChnAttr->u32ShutterUs           = 0;
                    pstIspChnAttr->u32ExpoFrameCnt        = 0;
                }
            }
        }
    }

    Cus3ACloseIspFrameSync(s32Fd);
    printf("Exit Isp Frame Sync\n");
    return NULL;
}

MI_S32 CarDV_IspLoadZoomTable(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId, MI_BOOL bForceLoad)
{
    CarDV_IspDevAttr_t *    pstIspDevAttr    = &gstIspModule.stIspDevAttr[u32IspDevId];
    CarDV_IspChannelAttr_t *pstIspChnAttr    = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];
    CarDV_InPortAttr_t *    pstIspInPortAttr = &pstIspChnAttr->stIspInPortAttr[0];
    MI_SYS_ChnPort_t        stSrcChnPort;

    if (pstIspChnAttr->bZoomCreate == FALSE || bForceLoad == TRUE)
    {
        memcpy(&stSrcChnPort, &pstIspInPortAttr->stBindParam.stChnPort, sizeof(MI_SYS_ChnPort_t));
        if (stSrcChnPort.eModId == E_MI_MODULE_ID_VIF)
        {
            MI_VIF_OutputPortAttr_t stVifOutAttr;
            MI_U32                  u32SrcWidth, u32SrcHeight;
            MI_U32                  u32DstWidth, u32DstHeight;
            MI_U32                  u32AspectRatio;
            MI_ISP_ZoomEntry_t *    pstZoomEntry;
            MI_ISP_ZoomTable_t      stZoomTable;

            CARDVCHECKRESULT(MI_VIF_GetOutputPortAttr(stSrcChnPort.u32DevId, stSrcChnPort.u32PortId, &stVifOutAttr));
            u32SrcWidth  = stVifOutAttr.stDestSize.u16Width;
            u32SrcHeight = stVifOutAttr.stDestSize.u16Height;
#if (CARDV_LDC_ENABLE)
            MI_LDC_DEV u32LdcDevId = 0;
            MI_LDC_CHN u32LdcChnId = 0;
            MI_BOOL    bFound      = FALSE;
            for (u32LdcDevId = 0; u32LdcDevId < CARDV_MAX_LDC_DEV_NUM && bFound == FALSE; u32LdcDevId++)
            {
                CarDV_LdcDevAttr_t *pstLdcDevAttr = &gstLdcModule.stLdcDevAttr[u32LdcDevId];
                if (pstLdcDevAttr->bUsed == TRUE)
                {
                    for (u32LdcChnId = 0; u32LdcChnId < CARDV_MAX_LDC_CHN_NUM && bFound == FALSE; u32LdcChnId++)
                    {
                        CarDV_LdcChannelAttr_t *pstLdcChnAttr = &pstLdcDevAttr->stLdcChnlAttr[u32LdcChnId];
                        if (pstLdcChnAttr->bUsed == TRUE
                            && pstLdcChnAttr->stBindParam.stChnPort.eModId == E_MI_MODULE_ID_ISP
                            && pstLdcChnAttr->stBindParam.stChnPort.u32DevId == u32IspDevId
                            && pstLdcChnAttr->stBindParam.stChnPort.u32ChnId == u32IspChnId)
                        {
                            MI_LDC_OutputPortAttr_t stLdcOutAttr;
                            CARDVCHECKRESULT(MI_LDC_GetOutputPortAttr(u32LdcDevId, u32LdcChnId, &stLdcOutAttr));
                            u32SrcWidth  = stLdcOutAttr.u16Width;
                            u32SrcHeight = stLdcOutAttr.u16Height;
                            bFound       = TRUE;
                        }
                    }
                }
            }
#endif
            u32DstWidth                    = u32SrcWidth * CARDV_ISP_ZOOM_MULTIPLE_DEN / CARDV_ISP_ZOOM_MULTIPLE_NUM;
            u32DstHeight                   = u32SrcHeight * CARDV_ISP_ZOOM_MULTIPLE_DEN / CARDV_ISP_ZOOM_MULTIPLE_NUM;
            u32AspectRatio                 = u32DstWidth * 100 / u32DstHeight;
            pstIspChnAttr->u32ZoomEntryNum = (u32SrcWidth - u32DstWidth) / CARDV_ISP_ZOOM_STEP / 2;

            // printf("Ratio [%d] EntryNum [%d] [%dx%d] -> [%dx%d]\n", u32AspectRatio, pstIspChnAttr->u32ZoomEntryNum,
            //     u32SrcWidth, u32SrcHeight, u32DstWidth, u32DstHeight);

            if (pstIspChnAttr->bZoomStart)
                CARDVCHECKRESULT(MI_ISP_StopPortZoom(u32IspDevId, u32IspChnId));

            pstZoomEntry = (MI_ISP_ZoomEntry_t *)MALLOC(sizeof(MI_ISP_ZoomEntry_t) * pstIspChnAttr->u32ZoomEntryNum);
            if (pstZoomEntry)
            {
                for (MI_U32 i = 0; i < pstIspChnAttr->u32ZoomEntryNum; i++)
                {
                    pstZoomEntry[i].u8ZoomSensorId     = 0;
                    pstZoomEntry[i].stCropWin.u16Width = ALIGN_2xUP(u32SrcWidth - CARDV_ISP_ZOOM_STEP * 2 * i);
                    pstZoomEntry[i].stCropWin.u16X =
                        ALIGN_DOWN((u32SrcWidth - pstZoomEntry[i].stCropWin.u16Width) / 2, 2);
                    pstZoomEntry[i].stCropWin.u16Height =
                        ALIGN_2xUP((MI_U32)(pstZoomEntry[i].stCropWin.u16Width) * 100 / u32AspectRatio);
                    pstZoomEntry[i].stCropWin.u16Y =
                        ALIGN_DOWN((u32SrcHeight - pstZoomEntry[i].stCropWin.u16Height) / 2, 2);

                    if (pstZoomEntry[i].stCropWin.u16Width >= u32SrcWidth)
                    {
                        pstZoomEntry[i].stCropWin.u16Width = u32SrcWidth;
                        pstZoomEntry[i].stCropWin.u16X     = 0;
                    }
                    if (pstZoomEntry[i].stCropWin.u16Height >= u32SrcHeight)
                    {
                        pstZoomEntry[i].stCropWin.u16Height = u32SrcHeight;
                        pstZoomEntry[i].stCropWin.u16Y      = 0;
                    }

                    // printf("idx [%d] pos [%d,%d] res [%dx%d]\n", i, pstZoomEntry[i].stCropWin.u16X,
                    // pstZoomEntry[i].stCropWin.u16Y,
                    //     pstZoomEntry[i].stCropWin.u16Width, pstZoomEntry[i].stCropWin.u16Height);
                }

                stZoomTable.u32EntryNum   = pstIspChnAttr->u32ZoomEntryNum;
                stZoomTable.pVirTableAddr = pstZoomEntry;
                CARDVCHECKRESULT(MI_ISP_LoadPortZoomTable(u32IspDevId, u32IspChnId, &stZoomTable));
                pstIspChnAttr->bZoomCreate = TRUE;
                FREEIF(pstZoomEntry);
            }
        }
    }

    return 0;
}

MI_S32 CarDV_IspUnLoadZoomTable(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId)
{
    CarDV_IspDevAttr_t *    pstIspDevAttr = &gstIspModule.stIspDevAttr[u32IspDevId];
    CarDV_IspChannelAttr_t *pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];

    if (pstIspChnAttr->bZoomStart == TRUE)
    {
        CARDVCHECKRESULT(MI_ISP_StopPortZoom(u32IspDevId, u32IspChnId));
        pstIspChnAttr->bZoomStart = FALSE;
    }

    if (pstIspChnAttr->bZoomCreate == TRUE)
        pstIspChnAttr->bZoomCreate = FALSE;

    return 0;
}

MI_S32 CarDV_IspZoomReset(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId)
{
    CarDV_IspDevAttr_t *    pstIspDevAttr = &gstIspModule.stIspDevAttr[u32IspDevId];
    CarDV_IspChannelAttr_t *pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];
    MI_ISP_ZoomAttr_t       stZoomAttr;

    CARDVCHECKRESULT(CarDV_IspLoadZoomTable(u32IspDevId, u32IspChnId, TRUE));
    stZoomAttr.u32FromEntryIndex = 0;
    stZoomAttr.u32ToEntryIndex   = 0;
    CARDVCHECKRESULT(MI_ISP_StartPortZoom(u32IspDevId, u32IspChnId, &stZoomAttr));
    pstIspChnAttr->bZoomStart = TRUE;

    return 0;
}

MI_S32 CarDV_IspZoomStart(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId, MI_BOOL bZoomIn)
{
    CarDV_IspDevAttr_t *    pstIspDevAttr = &gstIspModule.stIspDevAttr[u32IspDevId];
    CarDV_IspChannelAttr_t *pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];
    MI_ISP_ZoomAttr_t       stZoomAttr;

    if (u32IspDevId >= CARDV_MAX_ISP_DEV_NUM || u32IspChnId >= CARDV_MAX_ISP_CHN_NUM)
        return 0;

    CARDVCHECKRESULT(CarDV_IspLoadZoomTable(u32IspDevId, u32IspChnId, FALSE));

    if (pstIspChnAttr->bZoomStart == TRUE)
    {
        CARDVCHECKRESULT(MI_ISP_StopPortZoom(u32IspDevId, u32IspChnId));
        pstIspChnAttr->bZoomStart = FALSE;
    }

    CARDVCHECKRESULT(MI_ISP_GetPortCurZoomAttr(u32IspDevId, u32IspChnId, &stZoomAttr));
    stZoomAttr.u32FromEntryIndex = stZoomAttr.u32CurEntryIndex;
    stZoomAttr.u32ToEntryIndex   = bZoomIn ? pstIspChnAttr->u32ZoomEntryNum - 1 : 0;
    CARDVCHECKRESULT(MI_ISP_StartPortZoom(u32IspDevId, u32IspChnId, &stZoomAttr));
    pstIspChnAttr->bZoomStart = TRUE;

    return 0;
}

MI_S32 CarDV_IspZoomStop(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId)
{
    CarDV_IspDevAttr_t *    pstIspDevAttr = &gstIspModule.stIspDevAttr[u32IspDevId];
    CarDV_IspChannelAttr_t *pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];

    if (pstIspChnAttr->bZoomStart == TRUE)
    {
        CARDVCHECKRESULT(MI_ISP_StopPortZoom(u32IspDevId, u32IspChnId));
        pstIspChnAttr->bZoomStart = FALSE;
    }

    return 0;
}

MI_S32 CarDV_IspChannelInit(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId)
{
    CarDV_IspDevAttr_t *    pstIspDevAttr    = &gstIspModule.stIspDevAttr[u32IspDevId];
    CarDV_IspChannelAttr_t *pstIspChnAttr    = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];
    CarDV_InPortAttr_t *    pstIspInPortAttr = &pstIspChnAttr->stIspInPortAttr[0];
    MI_ISP_PORT             IspOutPortId     = 0;
    MI_ISP_ChannelAttr_t    stIspChnAttr;

    memset(&stIspChnAttr, 0x0, sizeof(MI_ISP_ChannelAttr_t));
    _TransMISnrPadToMIIspBindSensorId(pstIspChnAttr->eBindMiSnrPadId,
                                      (MI_ISP_BindSnrId_e *)&stIspChnAttr.u32SensorBindId);
    CARDVCHECKRESULT_OS(MI_ISP_CreateChannel(u32IspDevId, u32IspChnId, &stIspChnAttr));

    if (pstIspInPortAttr->stInputCropWin.u16Width != 0 && pstIspInPortAttr->stInputCropWin.u16Height != 0)
    {
        CARDVCHECKRESULT_OS(MI_ISP_SetInputPortCrop(u32IspDevId, u32IspChnId, &pstIspInPortAttr->stInputCropWin));
    }

    CARDVCHECKRESULT_OS(MI_ISP_SetChnParam(u32IspDevId, u32IspChnId, &pstIspChnAttr->stIspChnParam));
    CARDVCHECKRESULT_OS(MI_ISP_StartChannel(u32IspDevId, u32IspChnId));
    pstIspChnAttr->bCreate = TRUE;

    if (access(pstIspChnAttr->szApiBinFilePath, F_OK) == 0)
    {
        pstIspChnAttr->bThreadExit = FALSE;
        pthread_create(&pstIspChnAttr->pLoadApiBinThread, NULL, _LoadApiBinThread, (void *)pstIspChnAttr);
    }
    else
    {
        printf("ISP [%s] Not Exist\n", pstIspChnAttr->szApiBinFilePath);
    }

    for (IspOutPortId = 0; IspOutPortId < CARDV_MAX_ISP_OUTPORT_NUM; IspOutPortId++)
    {
        CarDV_PortAttr_t *pstIspOutputAttr = &pstIspChnAttr->stIspOutPortAttr[IspOutPortId];
        if (pstIspOutputAttr->bUsed == TRUE && pstIspOutputAttr->bCreate == FALSE)
        {
            if (IspOutPortId == CARDV_ISP_OUTPORT_BUFFER)
            {
                MI_ISP_OutPortParam_t stIspOutputParam;
                memset(&stIspOutputParam, 0x0, sizeof(MI_ISP_OutPortParam_t));
                memcpy(&stIspOutputParam.stCropRect, &pstIspOutputAttr->stOrigPortCrop, sizeof(MI_SYS_WindowRect_t));
                stIspOutputParam.ePixelFormat = pstIspOutputAttr->ePixelFormat;
                CARDVCHECKRESULT_OS(
                    MI_ISP_SetOutputPortParam(u32IspDevId, u32IspChnId, IspOutPortId, &stIspOutputParam));
            }

            CARDVCHECKRESULT_OS(MI_ISP_EnableOutputPort(u32IspDevId, u32IspChnId, IspOutPortId));

            if (IspOutPortId == CARDV_ISP_OUTPORT_BUFFER || IspOutPortId == CARDV_ISP_OUTPORT_IR)
            {
                MI_SYS_ChnPort_t stChnPort;
                stChnPort.eModId    = E_MI_MODULE_ID_ISP;
                stChnPort.u32DevId  = u32IspDevId;
                stChnPort.u32ChnId  = u32IspChnId;
                stChnPort.u32PortId = IspOutPortId;
                CARDVCHECKRESULT_OS(MI_SYS_SetChnOutputPortDepth(0, &stChnPort, pstIspOutputAttr->u16UserDepth,
                                                                 pstIspOutputAttr->u16Depth));
                // printf("isp module Dev%d, chn%d, port%d depth(%d,%d)\n", stChnPort.u32DevId, stChnPort.u32ChnId,
                // stChnPort.u32PortId,
                //     pstIspOutputAttr->u16UserDepth, pstIspOutputAttr->u16Depth);
            }
            pstIspOutputAttr->bCreate = TRUE;
        }
    }

    return 0;
}

MI_S32 CarDV_IspChannelUnInit(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId)
{
    CarDV_IspDevAttr_t *    pstIspDevAttr = &gstIspModule.stIspDevAttr[u32IspDevId];
    CarDV_IspChannelAttr_t *pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];
    MI_ISP_PORT             IspOutPortId  = 0;

    for (IspOutPortId = 0; IspOutPortId < CARDV_MAX_ISP_OUTPORT_NUM; IspOutPortId++)
    {
        CarDV_PortAttr_t *pstIspOutputAttr = &pstIspChnAttr->stIspOutPortAttr[IspOutPortId];
        if (pstIspOutputAttr->bUsed == TRUE && pstIspOutputAttr->bCreate == TRUE)
        {
            CARDVCHECKRESULT(MI_ISP_DisableOutputPort(u32IspDevId, u32IspChnId, IspOutPortId));
            pstIspOutputAttr->bCreate = FALSE;
        }
    }

    // CARDVCHECKRESULT(CarDV_IspUnLoadZoomTable(u32IspDevId, u32IspChnId));

    if (pstIspChnAttr->pLoadApiBinThread)
    {
        pstIspChnAttr->bThreadExit = TRUE;
        pthread_join(pstIspChnAttr->pLoadApiBinThread, NULL);
        pstIspChnAttr->pLoadApiBinThread = 0;
    }

    CARDVCHECKRESULT(MI_ISP_StopChannel(u32IspDevId, u32IspChnId));
    CARDVCHECKRESULT(MI_ISP_DestroyChannel(u32IspDevId, u32IspChnId));
    pstIspChnAttr->bCreate = FALSE;
    return 0;
}

MI_S32 CarDV_IspModuleInit(MI_ISP_DEV u32IspDevId)
{
    CarDV_IspDevAttr_t *pstIspDevAttr = &gstIspModule.stIspDevAttr[u32IspDevId];
    MI_ISP_CHANNEL      u32IspChnId   = 0;

    if (pstIspDevAttr->bUsed == TRUE && pstIspDevAttr->bCreate == FALSE)
    {
        MI_ISP_DevAttr_t stCreateDevAttr;
        memset(&stCreateDevAttr, 0x0, sizeof(MI_ISP_DevAttr_t));
        stCreateDevAttr.u32DevStitchMask = E_MI_ISP_DEVICEMASK_ID0;
        CARDVCHECKRESULT_OS(MI_ISP_CreateDevice(u32IspDevId, &stCreateDevAttr));
        pstIspDevAttr->bCreate = TRUE;
    }

    for (u32IspChnId = 0; u32IspChnId < CARDV_MAX_ISP_CHN_NUM; u32IspChnId++)
    {
        CarDV_IspChannelAttr_t *pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];
        if (pstIspChnAttr->bUsed == TRUE && pstIspChnAttr->bCreate == FALSE)
        {
            CARDVCHECKRESULT(CarDV_IspChannelInit(u32IspDevId, u32IspChnId));
        }
    }

    pstIspDevAttr->bThreadExit = FALSE;
    pthread_create(&pstIspDevAttr->pIspFrameSyncThread, NULL, _IspFrameSyncHandlerThread, (void *)pstIspDevAttr);

    return MI_SUCCESS;
}

MI_S32 CarDV_IspModuleUnInit(MI_ISP_DEV u32IspDevId)
{
    CarDV_IspDevAttr_t *pstIspDevAttr = &gstIspModule.stIspDevAttr[u32IspDevId];
    MI_ISP_CHANNEL      u32IspChnId   = 0;

    if (pstIspDevAttr->pIspFrameSyncThread)
    {
        pstIspDevAttr->bThreadExit = TRUE;
        pthread_join(pstIspDevAttr->pIspFrameSyncThread, NULL);
        pstIspDevAttr->pIspFrameSyncThread = 0;
    }

    for (u32IspChnId = 0; u32IspChnId < CARDV_MAX_ISP_CHN_NUM; u32IspChnId++)
    {
        CarDV_IspChannelAttr_t *pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];
        if (pstIspChnAttr->bUsed == TRUE && pstIspChnAttr->bCreate == TRUE)
        {
            CARDVCHECKRESULT(CarDV_IspChannelUnInit(u32IspDevId, u32IspChnId));
        }
    }

    if (pstIspDevAttr->bUsed == TRUE && pstIspDevAttr->bCreate == TRUE)
    {
        CARDVCHECKRESULT(MI_ISP_DestoryDevice(u32IspDevId));
        pstIspDevAttr->bCreate = FALSE;
    }

    return MI_SUCCESS;
}

MI_S32 CarDV_IspModuleBind(MI_ISP_DEV u32IspDevId, MI_BOOL bBind)
{
    MI_ISP_CHANNEL u32IspChnId = 0;

    for (u32IspChnId = 0; u32IspChnId < CARDV_MAX_ISP_CHN_NUM; u32IspChnId++)
    {
        CARDVCHECKRESULT(CarDV_IspChannelBind(u32IspDevId, u32IspChnId, bBind));
    }

    return 0;
}

MI_S32 CarDV_IspChannelBind(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId, MI_BOOL bBind)
{
    CarDV_IspDevAttr_t *    pstIspDevAttr = &gstIspModule.stIspDevAttr[u32IspDevId];
    CarDV_IspChannelAttr_t *pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];

    if (pstIspChnAttr->bUsed == TRUE && pstIspChnAttr->bCreate == TRUE)
    {
        CarDV_InPortAttr_t *pstIspInPortAttr = &pstIspChnAttr->stIspInPortAttr[0];
        if (pstIspInPortAttr->stBindParam.stChnPort.eModId < E_MI_MODULE_ID_MAX)
        {
            CarDV_Sys_BindInfo_T stBindInfo;
            memset(&stBindInfo, 0x0, sizeof(CarDV_Sys_BindInfo_T));

            stBindInfo.stSrcChnPort.eModId    = pstIspInPortAttr->stBindParam.stChnPort.eModId;
            stBindInfo.stSrcChnPort.u32DevId  = pstIspInPortAttr->stBindParam.stChnPort.u32DevId;
            stBindInfo.stSrcChnPort.u32ChnId  = pstIspInPortAttr->stBindParam.stChnPort.u32ChnId;
            stBindInfo.stSrcChnPort.u32PortId = pstIspInPortAttr->stBindParam.stChnPort.u32PortId;
            stBindInfo.stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
            stBindInfo.stDstChnPort.u32DevId  = u32IspDevId;
            stBindInfo.stDstChnPort.u32ChnId  = u32IspChnId;
            stBindInfo.stDstChnPort.u32PortId = 0;
            stBindInfo.u32SrcFrmrate          = pstIspInPortAttr->stBindParam.u32SrcFrmrate;
            stBindInfo.u32DstFrmrate          = pstIspInPortAttr->stBindParam.u32DstFrmrate;
            stBindInfo.eBindType              = pstIspInPortAttr->stBindParam.eBindType;
            if (bBind)
            {
                CARDVCHECKRESULT(CarDV_Sys_Bind(&stBindInfo));
                pstIspChnAttr->bBind = TRUE;
                // CARDVCHECKRESULT_OS(CarDV_IspZoomReset(u32IspDevId, u32IspChnId));
            }
            else if (bBind == FALSE && pstIspChnAttr->bBind)
            {
                CARDVCHECKRESULT(CarDV_Sys_UnBind(&stBindInfo));
                pstIspChnAttr->bBind = FALSE;
            }
        }
    }

    return 0;
}

MI_S32 CarDV_IspGetDevChnByVifDev(MI_VIF_DEV u32VifDev, MI_ISP_DEV *pu32IspDevId, MI_ISP_CHANNEL *pu32IspChnId)
{
    MI_ISP_DEV     u32IspDevId = 0;
    MI_ISP_CHANNEL u32IspChnId = 0;

    if (pu32IspDevId == NULL || pu32IspChnId == NULL)
        return -1;

    for (u32IspDevId = 0; u32IspDevId < CARDV_MAX_ISP_DEV_NUM; u32IspDevId++)
    {
        CarDV_IspDevAttr_t *pstIspDevAttr = &gstIspModule.stIspDevAttr[u32IspDevId];
        if (pstIspDevAttr->bUsed == TRUE)
        {
            for (u32IspChnId = 0; u32IspChnId < CARDV_MAX_ISP_CHN_NUM; u32IspChnId++)
            {
                CarDV_IspChannelAttr_t *pstIspChnAttr    = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];
                CarDV_InPortAttr_t *    pstIspInPortAttr = &pstIspChnAttr->stIspInPortAttr[0];
                if (pstIspChnAttr->bUsed == TRUE && pstIspInPortAttr->stBindParam.stChnPort.eModId == E_MI_MODULE_ID_VIF
                    && pstIspInPortAttr->stBindParam.stChnPort.u32DevId == u32VifDev)
                {
                    *pu32IspDevId = u32IspDevId;
                    *pu32IspChnId = u32IspChnId;
                    return 0;
                }
            }
        }
    }

    return -1;
}

MI_S32 CarDV_IspGetDevChnByCamId(MI_U8 u8CamId, MI_ISP_DEV *pu32IspDevId, MI_ISP_CHANNEL *pu32IspChnId)
{
    MI_ISP_DEV     u32IspDevId = 0;
    MI_ISP_CHANNEL u32IspChnId = 0;

    if (pu32IspDevId == NULL || pu32IspChnId == NULL)
        return -1;

    for (u32IspDevId = 0; u32IspDevId < CARDV_MAX_ISP_DEV_NUM; u32IspDevId++)
    {
        CarDV_IspDevAttr_t *pstIspDevAttr = &gstIspModule.stIspDevAttr[u32IspDevId];
        if (pstIspDevAttr->bUsed == TRUE)
        {
            for (u32IspChnId = 0; u32IspChnId < CARDV_MAX_ISP_CHN_NUM; u32IspChnId++)
            {
                CarDV_IspChannelAttr_t *pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];
                if (pstIspChnAttr->bUsed == TRUE && pstIspChnAttr->u8CamId == u8CamId)
                {
                    *pu32IspDevId = u32IspDevId;
                    *pu32IspChnId = u32IspChnId;
                    return 0;
                }
            }
        }
    }

    return -1;
}

MI_S32 CarDV_IspGetVifDev(MI_VIF_DEV *pu32VifDev, MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId)
{
    CarDV_IspDevAttr_t *    pstIspDevAttr    = &gstIspModule.stIspDevAttr[u32IspDevId];
    CarDV_IspChannelAttr_t *pstIspChnAttr    = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];
    CarDV_InPortAttr_t *    pstIspInPortAttr = &pstIspChnAttr->stIspInPortAttr[0];

    if (pu32VifDev == NULL)
        return -1;

    if (pstIspDevAttr->bUsed == TRUE && pstIspChnAttr->bUsed == TRUE
        && pstIspInPortAttr->stBindParam.stChnPort.eModId == E_MI_MODULE_ID_VIF)
    {
        *pu32VifDev = pstIspInPortAttr->stBindParam.stChnPort.u32DevId;
        return 0;
    }

    return -1;
}

MI_S32 CarDV_IspSetExpo(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId, MI_U32 u32ShutterUs, MI_U32 u32ExpoFrameCnt)
{
    CarDV_IspDevAttr_t *    pstIspDevAttr = &gstIspModule.stIspDevAttr[u32IspDevId];
    CarDV_IspChannelAttr_t *pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];

    pstIspChnAttr->u32ShutterUs    = u32ShutterUs;
    pstIspChnAttr->u32ExpoFrameCnt = u32ExpoFrameCnt;

    return 0;
}