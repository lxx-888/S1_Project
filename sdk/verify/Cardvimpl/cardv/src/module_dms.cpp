/*************************************************
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 * All rights reserved.
 *
 **************************************************
 * File name:  module_dms.c
 * Author:     jeff.li@sigmastar.com.cn
 * Version:    Initial Draft
 * Date:       2019/7/2
 * Description: DMS module source file
 *
 *
 *
 * History:
 *
 *    1. Date  :        2019/7/2
 *       Author:        jeff.li@sigmastar.com.cn
 *       Modification:  Created file
 *
 **************************************************/

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <pthread.h>

#include "module_common.h"
#include "mid_common.h"
#if (CARDV_DISPLAY_ENABLE)
#include "module_display.h"
#endif

#if CARDV_DMS_ENABLE

/*  imo dms add  */
#include "IPC_cardvInfo.h"
#include "dms/dms.h"
#include "imo-core/imo_err.h"
#include "imo-core/imo_sdk.h"
#include "opencv2/opencv.hpp"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define DMS_DBG_MSG(enable, format, args...) \
    do                                       \
    {                                        \
        if (enable)                          \
            printf(format, ##args);          \
    } while (0)
#define DMS_DBG_ERR(enable, format, args...) \
    do                                       \
    {                                        \
        if (1)                               \
            printf(format, ##args);          \
    } while (0)

#define DMS_PIXEL (E_MI_SYS_PIXEL_FRAME_ABGR8888)

#define DMS_IN_WIDTH  (1280)
#define DMS_IN_HEIGHT (720)

#define DMS_MID_WIDTH  (640)
#define DMS_MID_HEIGHT (360)

#define DMS_OUT_WIDTH  (256)
#define DMS_OUT_HEIGHT (160)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct CarDV_Sys_Rect_s
{
    MI_U32 u32X;
    MI_U32 u32Y;
    MI_U16 u16PicW;
    MI_U16 u16PicH;
} CarDV_Rect_T;

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

BOOL      g_bDmsEnable    = FALSE;
BOOL      g_DmsThreadExit = TRUE;
pthread_t g_DmsThread     = 0;

static imo_dms_handle g_dms_handle;

//==============================================================================
//
//                              EXTERN VARIABLES
//
//==============================================================================

extern CarDV_VencAttr_t g_dispParam[];
extern CarDV_Info       carInfo;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

static MI_S32 getDisplaySourceChnPort(CarDV_VencAttr_t *pDispParam, MI_SYS_ChnPort_t *pstChnPort)
{
    pstChnPort->eModId    = pDispParam->stChnPort[pDispParam->u32ChnPortNum - 1].eModId;
    pstChnPort->u32DevId  = pDispParam->stChnPort[pDispParam->u32ChnPortNum - 1].u32DevId;
    pstChnPort->u32ChnId  = pDispParam->stChnPort[pDispParam->u32ChnPortNum - 1].u32ChnId;
    pstChnPort->u32PortId = pDispParam->stChnPort[pDispParam->u32ChnPortNum - 1].u32PortId;
    return 0;
}

MI_S32 Dms_PipeInit()
{
    MI_DIVP_ChnAttr_t        stDivpChnattr;
    MI_DIVP_OutputPortAttr_t stDivpOutputAttr;
    MI_SYS_ChnPort_t         stDivpChnPort;
    CarDV_Sys_BindInfo_T     stBindInfo;
    MI_U32                   ret = 0;

    memset(&stDivpOutputAttr, 0x0, sizeof(MI_DIVP_OutputPortAttr_t));
    memset(&stDivpChnattr, 0x0, sizeof(MI_DIVP_ChnAttr_t));

    stDivpChnattr.bHorMirror           = FALSE;
    stDivpChnattr.bVerMirror           = FALSE;
    stDivpChnattr.eDiType              = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnattr.eRotateType          = E_MI_SYS_ROTATE_NONE;
    stDivpChnattr.eTnrLevel            = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnattr.stCropRect.u16X      = 0;
    stDivpChnattr.stCropRect.u16Y      = 0;
    stDivpChnattr.stCropRect.u16Width  = DMS_IN_WIDTH;
    stDivpChnattr.stCropRect.u16Height = DMS_IN_HEIGHT;
    stDivpChnattr.u32MaxWidth          = DMS_IN_WIDTH;
    stDivpChnattr.u32MaxHeight         = DMS_IN_HEIGHT;
    MI_DIVP_CreateChn(DIVP_CHANNEL_DMS, &stDivpChnattr);

    stDivpOutputAttr.u32Width     = DMS_OUT_WIDTH;
    stDivpOutputAttr.u32Height    = DMS_OUT_HEIGHT;
    stDivpOutputAttr.eCompMode    = E_MI_SYS_COMPRESS_MODE_NONE;
    stDivpOutputAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_ABGR8888;
    MI_DIVP_SetOutputPortAttr(DIVP_CHANNEL_DMS, &stDivpOutputAttr);
    MI_DIVP_StartChn(DIVP_CHANNEL_DMS);

    // From disp source ChnPort
    memset(&stBindInfo, 0x0, sizeof(CarDV_Sys_BindInfo_T));
    getDisplaySourceChnPort(&g_dispParam[0], &stBindInfo.stSrcChnPort);
    stBindInfo.stDstChnPort.eModId    = E_MI_MODULE_ID_DIVP;
    stBindInfo.stDstChnPort.u32DevId  = 0;
    stBindInfo.stDstChnPort.u32ChnId  = DIVP_CHANNEL_DMS;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate          = 30;
    stBindInfo.u32DstFrmrate          = 30;
    stBindInfo.eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    CARDVCHECKRESULT(CarDV_Sys_Bind(&stBindInfo));

    // Set Divp ChnPort Depth
    memset(&stDivpChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stDivpChnPort.eModId    = E_MI_MODULE_ID_DIVP;
    stDivpChnPort.u32ChnId  = DIVP_CHANNEL_DMS;
    stDivpChnPort.u32DevId  = 0;
    stDivpChnPort.u32PortId = 0;
    ret                     = MI_SYS_SetChnOutputPortDepth(&stDivpChnPort, 4, 4);
    if (ret != 0)
    {
        CARDV_ERR("MI_SYS_SetChnOutputPortDepth err [%d]\n", ret);
    }

    return 0;
}

MI_S32 Dms_PipeDeInit()
{
    CarDV_Sys_BindInfo_T stBindInfo;

    MI_U32 ret = 0;
    memset(&stBindInfo, 0x0, sizeof(CarDV_Sys_BindInfo_T));
    getDisplaySourceChnPort(&g_dispParam[0], &stBindInfo.stSrcChnPort);
    stBindInfo.stDstChnPort.eModId    = E_MI_MODULE_ID_DIVP;
    stBindInfo.stDstChnPort.u32DevId  = 0;
    stBindInfo.stDstChnPort.u32ChnId  = DIVP_CHANNEL_DMS;
    stBindInfo.stDstChnPort.u32PortId = 0;
    CARDVCHECKRESULT(CarDV_Sys_UnBind(&stBindInfo));

    MI_DIVP_StopChn(DIVP_CHANNEL_DMS);
    MI_DIVP_DestroyChn(DIVP_CHANNEL_DMS);

    return ret;
}

void *Dms_Task(void *args)
{
    // get image data
    MI_S32            s32Ret = 0;
    MI_SYS_ChnPort_t  stChnPort;
    MI_SYS_BufInfo_t  stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    unsigned char *   source_image = NULL;

    MI_S32         s32Fd = -1;
    fd_set         read_fds;
    struct timeval TimeoutVal;

    CARDV_THREAD();

    carInfo.stDmsInfo.faceRect.detHeight = DMS_OUT_HEIGHT;
    carInfo.stDmsInfo.faceRect.detWidth  = DMS_OUT_WIDTH;

    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId    = E_MI_MODULE_ID_DIVP;
    stChnPort.u32DevId  = 0;
    stChnPort.u32ChnId  = DIVP_CHANNEL_DMS;
    stChnPort.u32PortId = 0;
    s32Ret              = MI_SYS_GetFd(&stChnPort, &s32Fd);
    if (MI_SUCCESS != s32Ret)
    {
        DMS_DBG_ERR(1, "MI_SYS_GetFd err [%x]\n", s32Ret);
    }

    while (g_DmsThreadExit == FALSE)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32Fd, &read_fds);

        TimeoutVal.tv_sec  = 0;
        TimeoutVal.tv_usec = 1000 * 10;

        s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            DMS_DBG_ERR(1, "select failed!\n");
            continue;
        }
        else if (s32Ret == 0)
        {
            // Time Out
            continue;
        }
        else
        {
            if (FD_ISSET(s32Fd, &read_fds))
            {
                s32Ret = MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &hHandle);
                if (MI_SUCCESS == s32Ret)
                {
                    void *pViraddr = NULL;
                    if (stBufInfo.eBufType == E_MI_SYS_BUFDATA_FRAME)
                    {
                        pViraddr     = stBufInfo.stFrameData.pVirAddr[0];
                        source_image = static_cast<unsigned char *>(pViraddr);
                        // double start = cv::getTickCount();

                        cv::Mat srcMat(DMS_OUT_HEIGHT, DMS_OUT_WIDTH, CV_8UC4, (void *)source_image);
                        cv::Mat grayMat;

                        cv::cvtColor(srcMat, grayMat, cv::COLOR_BGRA2GRAY);

                        imo_dms_io dms_io;
                        dms_io.image.data   = (const char *)grayMat.data;
                        dms_io.image.height = grayMat.rows;
                        dms_io.image.width  = grayMat.cols;
                        dms_io.image.format = IMO_IMAGE_GRAY8;
                        s32Ret              = imo_dms_exec(&g_dms_handle, &dms_io);

                        memcpy(&carInfo.stDmsInfo.preFaceRect, &carInfo.stDmsInfo.faceRect, sizeof(dms_face_rect));

                        // printf("face rect pos(x = %d, y = %d), width = %d, height = %d \n" , faceRect.x, faceRect.y,
                        // faceRect.width, faceRect.height);
                        carInfo.stDmsInfo.faceRect.x      = dms_io.face_rect.x;
                        carInfo.stDmsInfo.faceRect.y      = dms_io.face_rect.y;
                        carInfo.stDmsInfo.faceRect.width  = dms_io.face_rect.width;
                        carInfo.stDmsInfo.faceRect.height = dms_io.face_rect.height;

                        if (dms_io.detect_face_count > 0)
                        {
                            carInfo.stDmsInfo.bLandmarkValid = true;
                            for (int lmIndex = 0; lmIndex < IMO_FACE_LANDMARK_POINTS; lmIndex++)
                            {
                                carInfo.stDmsInfo.landmarkPoints[lmIndex].x = dms_io.points[2 * lmIndex];
                                carInfo.stDmsInfo.landmarkPoints[lmIndex].y = dms_io.points[2 * lmIndex + 1];
                            }
                        }
                        else
                        {
                            carInfo.stDmsInfo.bLandmarkValid = false;
                        }

                        // double end = cv::getTickCount();
                        // printf("=========   Dms All   %s(%d) Cost Time:  %f ms \n",  __FUNCTION__, __LINE__, (end -
                        // start) / cv::getTickFrequency() * 1000);

                        std::string showStr = "$tired = " + std::to_string(dms_io.is_tired)
                                              + "   $yawn = " + std::to_string(dms_io.is_yawn)
                                              + "   $smoke = " + std::to_string(dms_io.is_smoke)
                                              + "   $phone = " + std::to_string(dms_io.is_phone)
                                              + "   $distract = " + std::to_string(dms_io.is_distracted);
                        std::cout << showStr << std::endl;
                        carInfo.stDmsInfo.status.isTired      = dms_io.is_tired;
                        carInfo.stDmsInfo.status.isYawn       = dms_io.is_yawn;
                        carInfo.stDmsInfo.status.isSmoke      = dms_io.is_smoke;
                        carInfo.stDmsInfo.status.isPhone      = dms_io.is_phone;
                        carInfo.stDmsInfo.status.isDistracted = dms_io.is_distracted;

                        IPC_CarInfo_Write_AdasInfo(&carInfo.stDmsInfo);
                    }
                    else if (stBufInfo.eBufType == E_MI_SYS_BUFDATA_META)
                    {
                        printf("incorrect type \n");
                    }
                    MI_SYS_ChnOutputPortPutBuf(hHandle);
                }
            }
        }
    }

    MI_SYS_CloseFd(s32Fd);
    pthread_exit(NULL);
}

MI_S32 dms_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    switch (id)
    {
    case CMD_DMS_INIT: {
        int ret = 0;
        // Dms全局初始化接口，第一个参数为ipu_firmware.bin和SigmastarDms.bin文件路径, 第二个参数为授权key
        ret = imo_sdk_init("/customer/imo_dms_model", "96A8049D656ECFB8");
        if (IMO_API_RET_SUCCESS != ret)
        {
            std::cout << "  imo_sdk_init failed ! imo_ret = " << ret << std::endl;
        }
        // Dms获取版本信息
        imo_version_io version_io;
        imo_dms_get_version(&version_io);
        std::cout << version_io.version << std::endl;

        imo_dms_config dms_config;
        dms_config.thresh = 0.45;
        // Dms初始化
        ret = imo_dms_create(&g_dms_handle, &dms_config);
        if (IMO_API_RET_SUCCESS != ret)
        {
            std::cout << "  imo_dms_create failed ! imo_ret = " << ret << std::endl;
        }
        if (g_bDmsEnable == TRUE)
            break;

        ret = Dms_PipeInit();
        if (0 != ret)
        {
            break;
        }

        g_bDmsEnable = TRUE;

        g_DmsThreadExit = FALSE;
        ret             = pthread_create(&g_DmsThread, NULL, Dms_Task, NULL);
        if (0 == ret)
        {
            pthread_setname_np(g_DmsThread, "Dms_task");
        }
        else
        {
            CARDV_ERR("%s pthread_create failed\n", __func__);
        }
    }
    break;

    case CMD_DMS_DEINIT: {
        if (g_bDmsEnable == FALSE)
            break;

        g_DmsThreadExit = TRUE;
        pthread_join(g_DmsThread, NULL);
        // dms销毁，释放资源
        imo_dms_destroy(&g_dms_handle);
        Dms_PipeDeInit();
        g_bDmsEnable = FALSE;
    }
    break;

    default:
        break;
    }

    return 0;
}
#endif