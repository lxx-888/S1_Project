/*
 * module_lightdetect.cpp- Sigmastar
 *
 * Copyright (C) 2021 Sigmastar Technology Corp.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <pthread.h>

#include "module_common.h"
#include "module_config.h"
#include "module_lightdetect.h"
#include "module_scl.h"

#if (CARDV_LD_ENABLE)

#include "LdSdk.h"
#include "struct_common.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

static BOOL      g_LightDetectThreadExit = TRUE;
static pthread_t g_LightDetectThread     = 0;
static BOOL      g_LightDetectUsed       = FALSE;
static int       g_LDDbgLog              = 0;
MI_SYS_ChnPort_t gstLDSrcModule          = {E_MI_MODULE_ID_MAX, 0, 0, 0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void LightDetect_Process(long long handle, int w, int h, unsigned char *img)
{
    Detectbox *output;
    int        count = 0;

    trafficlight::TrafficLightDetect(handle, img, w, h, 4, &output, &count);
    for (int j = 0; j < count; ++j)
    {
        Detectbox box;
        box.score = output[j].score;
        if (!output[j].current)
        {
            continue;
        }
#if 0
        box.x1 = int((output[j].x1) * crop_w / 320 + crop_x);
        box.y1 = int((output[j].y1) * crop_h / 320 + crop_y);
        box.x2 = int((output[j].x2) * crop_w / 320 + crop_x);
        box.y2 = int((output[j].y2) * crop_h / 320 + crop_y);
#else
        box.x1 = int(output[j].x1);
        box.y1 = int(output[j].y1);
        box.x2 = int(output[j].x2);
        box.y2 = int(output[j].y2);
#endif
        box.classid = output[j].classid;
        box.flag    = output[j].flag;

        LD_DBG_MSG((g_LDDbgLog & 1), "[%f,%f],[%f,%f], classid = %x, flag = %x, score = %f\n", box.x1, box.y1, box.x2,
                   box.y2, box.classid, box.flag, box.score);
    }
}

void *LightDetect_Task(void *args)
{
    MI_S32            s32Ret = 0;
    MI_SYS_ChnPort_t  stChnPort;
    MI_SYS_BufInfo_t  stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_S32            s32Fd = -1;
    MI_U32            u32LightDetectWidth, u32LightDetectHeight;
    fd_set            read_fds;
    struct timeval    TimeoutVal;
    long long         handle;

    CARDV_THREAD();

    s32Ret = trafficlight::TrafficLightInit(&handle);
    if (s32Ret != 0)
    {
        LD_DBG_ERR(1, "LD init err\n");
        pthread_exit(NULL);
    }

    memcpy(&stChnPort, &gstLDSrcModule, sizeof(MI_SYS_ChnPort_t));
    if (E_MI_MODULE_ID_SCL == stChnPort.eModId)
    {
        MI_SCL_OutPortParam_t stSclOutputParam;
        memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
        MI_SCL_GetOutputPortParam((MI_SCL_DEV)stChnPort.u32DevId, stChnPort.u32ChnId, stChnPort.u32PortId,
                                  &stSclOutputParam);
        u32LightDetectWidth  = stSclOutputParam.stSCLOutputSize.u16Width;
        u32LightDetectHeight = stSclOutputParam.stSCLOutputSize.u16Height;
    }
    else
    {
        LD_DBG_ERR(1, "NOT support LD input module [%d]\n", stChnPort.eModId);
        trafficlight::TrafficLightCleanUp(handle);
        pthread_exit(NULL);
    }

    s32Ret = MI_SYS_GetFd(&stChnPort, &s32Fd);
    if (MI_SUCCESS != s32Ret)
    {
        LD_DBG_ERR(1, "MI_SYS_GetFd err [%x]\n", s32Ret);
    }

    while (g_LightDetectThreadExit == FALSE)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32Fd, &read_fds);

        TimeoutVal.tv_sec  = 0;
        TimeoutVal.tv_usec = 1000 * 10;

        s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            LD_DBG_ERR(1, "select failed!\n");
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
                    LightDetect_Process(handle, u32LightDetectWidth, u32LightDetectHeight,
                                        (MI_U8 *)stBufInfo.stFrameData.pVirAddr[0]);
                    MI_SYS_ChnOutputPortPutBuf(hHandle);
                }
            }
        }
    }

    trafficlight::TrafficLightCleanUp(handle);
    MI_SYS_CloseFd(s32Fd);
    pthread_exit(NULL);
}

MI_S32 lightdetect_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    if (gstLDSrcModule.eModId == E_MI_MODULE_ID_MAX)
        return -1;

    switch (id)
    {
    case CMD_LD_INIT:
        if (g_LightDetectUsed == FALSE)
        {
            g_LightDetectThreadExit = FALSE;
            int ret                 = pthread_create(&g_LightDetectThread, NULL, LightDetect_Task, NULL);
            if (0 == ret)
                pthread_setname_np(g_LightDetectThread, "LD_task");
            else
                LD_DBG_ERR(1, "LD Thread Failed\n");
            g_LightDetectUsed = TRUE;
        }
        break;

    case CMD_LD_DEINIT:
        if (g_LightDetectUsed == TRUE)
        {
            g_LightDetectThreadExit = TRUE;
            pthread_join(g_LightDetectThread, NULL);
            g_LightDetectUsed = FALSE;
        }
        break;
    case CMD_LD_SET_PARAM:
        if (g_LightDetectUsed == FALSE)
        {
            if ((NULL != param) && (0 < paramLen))
            {
                MI_U32 ldparam[10];

                memcpy(ldparam, param, paramLen);
                gstLDSrcModule.eModId    = (MI_ModuleId_e)ldparam[0];
                gstLDSrcModule.u32DevId  = ldparam[1];
                gstLDSrcModule.u32ChnId  = ldparam[2];
                gstLDSrcModule.u32PortId = ldparam[3];
                g_LDDbgLog               = ldparam[4];
            }
            LD_DBG_MSG((g_LDDbgLog & 2), "gstLDSrcModule[%d %d %d %d]\n", gstLDSrcModule.eModId,
                       gstLDSrcModule.u32DevId, gstLDSrcModule.u32ChnId, gstLDSrcModule.u32PortId);
        }
        break;
    default:
        break;
    }

    return 0;
}
#endif