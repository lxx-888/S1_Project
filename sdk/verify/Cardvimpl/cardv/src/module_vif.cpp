/*
 * module_vif.cpp - Sigmastar
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
#include "mid_vif.h"
#include "module_vif.h"
#include "module_system.h"
#include "module_common.h"
#include "module_sensor.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

CarDV_VifModAttr_t gstVifModule = {0};

static pthread_t gAnaDecDetectThread = 0;
static BOOL      gAnaDecDetectExit;

extern BOOL g_camExisted[];

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

static void *_AnaDecDetectTask(void *argv)
{
    MI_BOOL bSrcChange = FALSE;
#if (CARDV_AUTO_DETECT_ANADEC)
    MI_BOOL bNeedSetAnadecAttr[CARDV_MAX_SENSOR_NUM] = {0};
#endif
    MI_VIF_GROUP          u32VifGroup      = 0;
    MI_S8                 vifDevIdPerGroup = 0;
    CarDV_VifGroupAttr_t *pstVifGroupAttr  = NULL;

    for (u32VifGroup = 0; u32VifGroup < CARDV_MAX_VIF_GROUP_NUM; u32VifGroup++)
    {
        pstVifGroupAttr = &gstVifModule.stVifGroupAttr[u32VifGroup];
        if (pstVifGroupAttr->bUsed && pstVifGroupAttr->bCheckSignal)
            break;
    }

    if (u32VifGroup == CARDV_MAX_VIF_GROUP_NUM)
    {
        printf("NOT need AHD detect\n");
        pthread_exit(NULL);
    }

    while (gAnaDecDetectExit == FALSE)
    {
        for (u32VifGroup = 0; u32VifGroup < CARDV_MAX_VIF_GROUP_NUM; u32VifGroup++)
        {
            pstVifGroupAttr = &gstVifModule.stVifGroupAttr[u32VifGroup];
            if (pstVifGroupAttr->bUsed && pstVifGroupAttr->bCreate && pstVifGroupAttr->bCheckSignal)
            {
                for (vifDevIdPerGroup = 0; vifDevIdPerGroup < CARDV_MAX_VIF_DEV_PERGROUP; vifDevIdPerGroup++)
                {
                    CarDV_VifDevAttr_t *pstVifDevAttr = &pstVifGroupAttr->stVifDevAttr[vifDevIdPerGroup];
                    if (pstVifDevAttr->bUsed == TRUE && pstVifDevAttr->bCreate == TRUE)
                    {
                        MI_SNR_Anadec_SrcAttr_t stSrcAttr;
                        MI_SNR_PADID            u32SnrPad = (MI_SNR_PADID)pstVifGroupAttr->stGroupExtAttr.eSensorPadID;
                        MI_U32 u32PlaneId = pstVifGroupAttr->stGroupExtAttr.u32PlaneID[vifDevIdPerGroup];
                        // Do debounce in sensor driver.
                        if (MI_SNR_GetAnadecSrcAttr(u32SnrPad, u32PlaneId, &stSrcAttr) != MI_SUCCESS)
                            continue;

                        // printf("Plane [%d] Src [%d]\n", u32PlaneId, stSrcAttr.eStatus);
                        if (pstVifDevAttr->stAnadecSrcAttr.eStatus != E_MI_SNR_ANADEC_STATUS_NUM
                            && pstVifDevAttr->stAnadecSrcAttr.eStatus != stSrcAttr.eStatus)
                        {
                            bSrcChange = TRUE;
                            printf("Cam [%d] Src [%d] to [%d]\n", pstVifDevAttr->u8CamId,
                                   pstVifDevAttr->stAnadecSrcAttr.eStatus, stSrcAttr.eStatus);
                            if (stSrcAttr.eStatus == E_MI_SNR_ANADEC_STATUS_NO_READY
                                || stSrcAttr.eStatus == E_MI_SNR_ANADEC_STATUS_DISCNT)
                            {
                                g_camExisted[pstVifDevAttr->u8CamId] = FALSE;
                            }
                            else
                            {
                                g_camExisted[pstVifDevAttr->u8CamId] = TRUE;
                            }
                        }

#if (CARDV_AUTO_DETECT_ANADEC)
                        if (g_camExisted[pstVifDevAttr->u8CamId] == TRUE
                            && (stSrcAttr.stRes.u16Width != pstVifDevAttr->stAnadecSrcAttr.stRes.u16Width
                                || stSrcAttr.stRes.u16Height != pstVifDevAttr->stAnadecSrcAttr.stRes.u16Height
                                || stSrcAttr.u32Fps != pstVifDevAttr->stAnadecSrcAttr.u32Fps
                                || stSrcAttr.eTransferMode != pstVifDevAttr->stAnadecSrcAttr.eTransferMode))
                        {
                            printf("set pad [%d] plane [%d] [%dx%d] %dfps mode [%d]\n", u32SnrPad, u32PlaneId,
                                   stSrcAttr.stRes.u16Width, stSrcAttr.stRes.u16Height, stSrcAttr.u32Fps,
                                   stSrcAttr.eTransferMode);
                            if (MI_SNR_SetAnadecSrcAttr(u32SnrPad, u32PlaneId, &stSrcAttr) == MI_SUCCESS)
                            {
                                bNeedSetAnadecAttr[u32SnrPad] = TRUE;
                                memcpy(&pstVifDevAttr->stAnadecSrcAttr, &stSrcAttr, sizeof(MI_SNR_Anadec_SrcAttr_t));
                            }
                        }
#endif

                        pstVifDevAttr->stAnadecSrcAttr.eStatus = stSrcAttr.eStatus;
                    }
                }
            }
        }

#if (CARDV_AUTO_DETECT_ANADEC)
        for (MI_SNR_PADID u32SnrPad = 0; u32SnrPad < CARDV_MAX_SENSOR_NUM; u32SnrPad++)
        {
            if (bNeedSetAnadecAttr[u32SnrPad])
            {
                CarDV_VideoPipeSwitchSensorResIdx(u32SnrPad, 0);
                bNeedSetAnadecAttr[u32SnrPad] = FALSE;
            }
        }
#endif

        if (bSrcChange)
        {
            bSrcChange = FALSE;
            cardv_send_to_fifo(CARDV_CMD_SRC_CHANGE, sizeof(CARDV_CMD_SRC_CHANGE));
        }

        usleep(300000);
    }

    printf("AHD detect exit\n");
    pthread_exit(NULL);
}

MI_S32 CarDV_StartAnaDecDetectThread(void)
{
    gAnaDecDetectExit = FALSE;
    int ret           = 0;

    printf("start AHD detect thread\n");

    if (0 != gAnaDecDetectThread)
    {
        CARDV_ERR("alread start\n");
        return 0;
    }

    ret = pthread_create(&gAnaDecDetectThread, NULL, _AnaDecDetectTask, NULL);
    if (0 == ret)
        pthread_setname_np(gAnaDecDetectThread, "AHD_detect");
    else
        CARDV_ERR("pthread_create failed\n");

    return ret;
}

MI_S32 CarDV_StopAnaDecDetectThread(void)
{
    if (gAnaDecDetectThread)
    {
        gAnaDecDetectExit = TRUE;
        pthread_join(gAnaDecDetectThread, NULL);
        gAnaDecDetectThread = 0;
    }
    return 0;
}

MI_S32 CarDv_VifSetAnadecSrcAttr(MI_VIF_DEV vifDev, MI_SNR_Anadec_SrcAttr_t *pstAnadecSrcAttr)
{
    MI_VIF_GROUP          GroupId          = vifDev / CARDV_MAX_VIF_DEV_PERGROUP;
    MI_S8                 vifDevIdPerGroup = vifDev % CARDV_MAX_VIF_DEV_PERGROUP;
    CarDV_VifGroupAttr_t *pstVifGroupAttr  = &gstVifModule.stVifGroupAttr[GroupId];

    if (pstVifGroupAttr->bUsed)
    {
        MI_SNR_PADID        u32SnrPad     = (MI_SNR_PADID)pstVifGroupAttr->stGroupExtAttr.eSensorPadID;
        MI_U32              u32PlaneId    = pstVifGroupAttr->stGroupExtAttr.u32PlaneID[vifDevIdPerGroup];
        CarDV_VifDevAttr_t *pstVifDevAttr = &pstVifGroupAttr->stVifDevAttr[vifDevIdPerGroup];

        if (pstAnadecSrcAttr)
        {
#if (CARDV_AUTO_DETECT_ANADEC == 0)
            memcpy(&pstVifDevAttr->stAnadecSrcAttr, pstAnadecSrcAttr, sizeof(MI_SNR_Anadec_SrcAttr_t));
#endif
        }
        else
        {
            pstAnadecSrcAttr = &pstVifDevAttr->stAnadecSrcAttr;
        }

        printf("set pad [%d] plane [%d] [%dx%d] %dfps mode [%d]\n", u32SnrPad, u32PlaneId,
               pstAnadecSrcAttr->stRes.u16Width, pstAnadecSrcAttr->stRes.u16Height, pstAnadecSrcAttr->u32Fps,
               pstAnadecSrcAttr->eTransferMode);

        CARDVCHECKRESULT_OS(MI_SNR_SetAnadecSrcAttr(u32SnrPad, u32PlaneId, pstAnadecSrcAttr));
    }

    return 0;
}

MI_S32 CarDV_VifDevInit(MI_VIF_DEV vifDev)
{
    MI_VIF_GROUP          GroupId          = vifDev / CARDV_MAX_VIF_DEV_PERGROUP;
    MI_S8                 vifDevIdPerGroup = vifDev % CARDV_MAX_VIF_DEV_PERGROUP;
    CarDV_VifGroupAttr_t *pstVifGroupAttr  = &gstVifModule.stVifGroupAttr[GroupId];
    CarDV_VifDevAttr_t *  pstVifDevAttr    = &pstVifGroupAttr->stVifDevAttr[vifDevIdPerGroup];
    MI_VIF_PORT           vifPort          = 0;
    MI_VIF_DevAttr_t      stVifDevAttr;
    MI_SNR_PlaneInfo_t    stSnrPlane0Info;
    MI_SNR_PADID          u32SnrPad  = (MI_SNR_PADID)pstVifGroupAttr->stGroupExtAttr.eSensorPadID;
    MI_U32                u32PlaneId = pstVifGroupAttr->stGroupExtAttr.u32PlaneID[vifDevIdPerGroup];
    MI_SYS_ChnPort_t      stChnPort;

    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t));
    CARDVCHECKRESULT(MI_SNR_GetPlaneInfo(u32SnrPad, u32PlaneId, &stSnrPlane0Info));

    memset(&stVifDevAttr, 0x0, sizeof(MI_VIF_DevAttr_t));
    stVifDevAttr.stInputRect.u16X      = stSnrPlane0Info.stCapRect.u16X;
    stVifDevAttr.stInputRect.u16Y      = stSnrPlane0Info.stCapRect.u16Y;
    stVifDevAttr.stInputRect.u16Width  = stSnrPlane0Info.stCapRect.u16Width;
    stVifDevAttr.stInputRect.u16Height = stSnrPlane0Info.stCapRect.u16Height;
    if (stSnrPlane0Info.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
        stVifDevAttr.eInputPixel = stSnrPlane0Info.ePixel;
    else
        stVifDevAttr.eInputPixel =
            (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);
    stVifDevAttr.eField       = pstVifDevAttr->eField;
    stVifDevAttr.bEnH2T1PMode = pstVifDevAttr->bEnH2T1PMode;
    // printf("setdevattr (%d,%d,%d,%d)\n",
    //     stVifDevAttr.stInputRect.u16X, stVifDevAttr.stInputRect.u16Y, stVifDevAttr.stInputRect.u16Width,
    //     stVifDevAttr.stInputRect.u16Height);

    CARDVCHECKRESULT_OS(MI_VIF_SetDevAttr(vifDev, &stVifDevAttr));
    CARDVCHECKRESULT_OS(MI_VIF_EnableDev(vifDev));
    pstVifDevAttr->bCreate = TRUE;

    if (pstVifGroupAttr->bCheckSignal == FALSE
        && (stVifDevAttr.eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV
            || stVifDevAttr.eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_UYVY
            || stVifDevAttr.eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_YVYU
            || stVifDevAttr.eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY))
    {
        pstVifGroupAttr->bCheckSignal = TRUE;
        MI_SNR_Anadec_SrcAttr_t stSrcAttr;
        CARDVCHECKRESULT(MI_SNR_GetAnadecSrcAttr(u32SnrPad, u32PlaneId, &stSrcAttr));
        pstVifDevAttr->stAnadecSrcAttr.eStatus = stSrcAttr.eStatus;
        if (stSrcAttr.eStatus == E_MI_SNR_ANADEC_STATUS_NO_READY || stSrcAttr.eStatus == E_MI_SNR_ANADEC_STATUS_DISCNT)
        {
            g_camExisted[pstVifDevAttr->u8CamId] = FALSE;
            printf("Cam [%d] NOT Existed\n", pstVifDevAttr->u8CamId);
        }
    }

    for (vifPort = 0; vifPort < CARDV_MAX_VIF_OUTPORT_NUM; vifPort++)
    {
        CarDV_VifPortAttr_t *pstVifPortAttr = &pstVifDevAttr->stVifOutPortAttr[vifPort];
        if (pstVifPortAttr->bUsed == TRUE && pstVifPortAttr->bCreate == FALSE)
        {
            MI_VIF_OutputPortAttr_t stVifPortInfo;
            memset(&stVifPortInfo, 0, sizeof(MI_VIF_OutputPortAttr_t));

            if (pstVifPortAttr->stCapRect.u16Width == 0 || pstVifPortAttr->stCapRect.u16Height == 0)
            {
                stVifPortInfo.stCapRect.u16X      = 0; // stSnrPlane0Info.stCapRect.u16X;
                stVifPortInfo.stCapRect.u16Y      = 0; // stSnrPlane0Info.stCapRect.u16Y;
                stVifPortInfo.stCapRect.u16Width  = stSnrPlane0Info.stCapRect.u16Width;
                stVifPortInfo.stCapRect.u16Height = stSnrPlane0Info.stCapRect.u16Height;
                if (pstVifDevAttr->bEnH2T1PMode)
                    stVifPortInfo.stCapRect.u16Width /= 2;
            }
            else
            {
                stVifPortInfo.stCapRect.u16X      = 0; // pstVifPortAttr->stCapRect.u16X;
                stVifPortInfo.stCapRect.u16Y      = 0; // pstVifPortAttr->stCapRect.u16Y;
                stVifPortInfo.stCapRect.u16Width  = pstVifPortAttr->stCapRect.u16Width;
                stVifPortInfo.stCapRect.u16Height = pstVifPortAttr->stCapRect.u16Height;
            }

            if (pstVifPortAttr->stDestSize.u16Width == 0 || pstVifPortAttr->stDestSize.u16Height == 0)
            {
                stVifPortInfo.stDestSize.u16Width  = stSnrPlane0Info.stCapRect.u16Width;
                stVifPortInfo.stDestSize.u16Height = stSnrPlane0Info.stCapRect.u16Height;
                if (pstVifDevAttr->bEnH2T1PMode)
                    stVifPortInfo.stDestSize.u16Width /= 2;
            }
            else
            {
                stVifPortInfo.stDestSize.u16Width  = pstVifPortAttr->stDestSize.u16Width;
                stVifPortInfo.stDestSize.u16Height = pstVifPortAttr->stDestSize.u16Height;
            }

            stVifPortInfo.ePixFormat = stSnrPlane0Info.ePixel;

            stVifPortInfo.eFrameRate    = E_MI_VIF_FRAMERATE_FULL;
            stVifPortInfo.eCompressMode = pstVifPortAttr->eCompressMode;

            CARDVCHECKRESULT_OS(MI_VIF_SetOutputPortAttr(vifDev, vifPort, &stVifPortInfo));
            CARDVCHECKRESULT_OS(MI_VIF_EnableOutputPort(vifDev, vifPort));
            pstVifPortAttr->bCreate = TRUE;

            if (vifPort == 0)
            {
                stChnPort.eModId    = E_MI_MODULE_ID_VIF;
                stChnPort.u32DevId  = vifDev;
                stChnPort.u32ChnId  = 0;
                stChnPort.u32PortId = vifPort;
                CARDVCHECKRESULT(CarDV_GetRawEnable(pstVifDevAttr->u8CamId, &stChnPort));
            }
        }
    }

    return 0;
}

MI_S32 CarDV_VifDevUnInit(MI_VIF_DEV vifDev)
{
    MI_VIF_GROUP          GroupId          = vifDev / CARDV_MAX_VIF_DEV_PERGROUP;
    MI_S8                 vifDevIdPerGroup = vifDev % CARDV_MAX_VIF_DEV_PERGROUP;
    CarDV_VifGroupAttr_t *pstVifGroupAttr  = &gstVifModule.stVifGroupAttr[GroupId];
    CarDV_VifDevAttr_t *  pstVifDevAttr    = &pstVifGroupAttr->stVifDevAttr[vifDevIdPerGroup];
    MI_VIF_PORT           vifPort          = 0;

    for (vifPort = 0; vifPort < CARDV_MAX_VIF_OUTPORT_NUM; vifPort++)
    {
        CarDV_VifPortAttr_t *pstVifOutputPortAttr = &pstVifDevAttr->stVifOutPortAttr[vifPort];
        if (pstVifOutputPortAttr->bUsed == TRUE && pstVifOutputPortAttr->bCreate == TRUE)
        {
            if (vifPort == 0)
            {
                CARDVCHECKRESULT(CarDV_GetRawDisable(pstVifDevAttr->u8CamId));
            }

            CARDVCHECKRESULT(MI_VIF_DisableOutputPort(vifDev, vifPort));
            pstVifOutputPortAttr->bCreate = FALSE;
        }
    }

    CARDVCHECKRESULT(MI_VIF_DisableDev(vifDev));
    pstVifDevAttr->bCreate = FALSE;
    return 0;
}

MI_S32 CarDV_VifModuleInit(MI_VIF_GROUP GroupId)
{
    MI_VIF_DEV            vifDev           = 0;
    MI_S8                 vifDevIdPerGroup = 0;
    CarDV_VifGroupAttr_t *pstVifGroupAttr  = &gstVifModule.stVifGroupAttr[GroupId];
    MI_SNR_PADID          u32SnrPad        = (MI_SNR_PADID)pstVifGroupAttr->stGroupExtAttr.eSensorPadID;

    MI_SNR_PADInfo_t stPad0Info;
    memset(&stPad0Info, 0x0, sizeof(MI_SNR_PADInfo_t));

    MI_VIF_GroupAttr_t stGroupAttr;
    memset(&stGroupAttr, 0x0, sizeof(MI_VIF_GroupAttr_t));

    if (pstVifGroupAttr->bUsed == TRUE && pstVifGroupAttr->bCreate == FALSE)
    {
        CARDVCHECKRESULT(MI_SNR_GetPadInfo(u32SnrPad, &stPad0Info));
        stGroupAttr.eIntfMode = (MI_VIF_IntfMode_e)stPad0Info.eIntfMode;
        stGroupAttr.eWorkMode = pstVifGroupAttr->eWorkMode;
        stGroupAttr.eHDRType  = pstVifGroupAttr->eHDRType;
        if (stGroupAttr.eIntfMode == E_MI_VIF_MODE_BT656)
        {
            stGroupAttr.eClkEdge = (MI_VIF_ClkEdge_e)stPad0Info.unIntfAttr.stBt656Attr.eClkEdge;
        }
        else
        {
            stGroupAttr.eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;
        }

        CARDVCHECKRESULT_OS(MI_VIF_CreateDevGroup(GroupId, &stGroupAttr));

        pstVifGroupAttr->bCreate = TRUE;
    }

    for (vifDevIdPerGroup = 0; vifDevIdPerGroup < CARDV_MAX_VIF_DEV_PERGROUP; vifDevIdPerGroup++)
    {
        CarDV_VifDevAttr_t *pstVifDevAttr = &pstVifGroupAttr->stVifDevAttr[vifDevIdPerGroup];
        if (pstVifDevAttr->bUsed == TRUE && pstVifDevAttr->bCreate == FALSE)
        {
            vifDev = GroupId * CARDV_MAX_VIF_DEV_PERGROUP + vifDevIdPerGroup;
            CARDVCHECKRESULT(CarDV_VifDevInit(vifDev));
            pstVifDevAttr->bCreate = TRUE;
        }
    }

    return 0;
}

MI_S32 CarDV_VifModuleUnInit(MI_VIF_GROUP GroupId)
{
    MI_VIF_DEV            vifDev           = 0;
    MI_S8                 vifDevIdPerGroup = 0;
    CarDV_VifGroupAttr_t *pstVifGroupAttr  = &gstVifModule.stVifGroupAttr[GroupId];
    for (vifDevIdPerGroup = CARDV_MAX_VIF_DEV_PERGROUP - 1; vifDevIdPerGroup >= 0; vifDevIdPerGroup--)
    {
        CarDV_VifDevAttr_t *pstVifDevAttr = &pstVifGroupAttr->stVifDevAttr[vifDevIdPerGroup];
        if (pstVifDevAttr->bUsed == TRUE && pstVifDevAttr->bCreate == TRUE)
        {
            vifDev = GroupId * CARDV_MAX_VIF_DEV_PERGROUP + vifDevIdPerGroup;
            CARDVCHECKRESULT(CarDV_VifDevUnInit(vifDev));
            pstVifDevAttr->bCreate = FALSE;
        }
    }

    if (pstVifGroupAttr->bUsed == TRUE && pstVifGroupAttr->bCreate == TRUE)
    {
        CARDVCHECKRESULT(MI_VIF_DestroyDevGroup(GroupId));
        pstVifGroupAttr->bCreate = FALSE;
    }

    return 0;
}
