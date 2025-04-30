/*************************************************
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 * All rights reserved.
 *
 **************************************************
 * File name:  module_gsensor.c
 * Author:     jeff.li@sigmastar.com.cn
 * Version:    Initial Draft
 * Date:       2019/4/8
 * Description: gsensor module source file
 *
 *
 *
 * History:
 *
 *    1. Date  :        2019/4/8
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
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <linux/input.h>

#include "module_common.h"
#include "module_gsensor.h"
#include "module_system.h"
#include "module_mux.h"
#include "mid_mux.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define SENSITIVITY_OFF (100) // percent
#define SENSITIVITY_LV0 (15)  // percent
#define SENSITIVITY_LV1 (13)  // percent
#define SENSITIVITY_LV2 (11)  // percent
#define SENSITIVITY_LV3 (9)   // percent
#define SENSITIVITY_LV4 (7)   // percent
#define SENSITIVITY_LV5 (5)   // percent

#define GSNR_CHUNK_MAX_CNT (512)

#define DEFAULT_RANGE 2000
#define SENSOR_SC7A20 1
#ifdef SENSOR_SC7A20
#define RANGE_DIV 10 // SC7A20 range is too big to be active.
#else
#define RANGE_DIV 1
#endif
//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

pthread_t   g_gsensorThread = 0;
BOOL        g_gsensorThread_exit;
const char *gsensor_input_event      = "/dev/input/event0";
const char *gsensor_int_enable       = "/sys/devices/virtual/input/input0/int2_enable";
const char *gsensor_int_start_stauts = "/sys/devices/virtual/input/input0/int2_start_status";
int         gsensor_fd               = -1;
int         gsensor_int_fd           = -1;
int         x_range = 0, y_range = 0, z_range = 0;
int         g_xrange = 0, g_yrange = 0, g_zrange = 0;

static int g_parkingSensitivity = 0;
static int g_EmergySensitivity  = SENSITIVITY_LV1;

GSNRINFOCHUCK          gstGsnrInfoChuck    = {AXIS_INIT_VALUE, AXIS_INIT_VALUE, AXIS_INIT_VALUE};
MI_U8 *                gpu8GsnrCompBuf     = NULL;
MI_U32                 gu32GsnrChunkWrIdx  = 0;
MI_BOOL                gbGsnrChunkStartRec = FALSE;
static pthread_mutex_t _gstGsnrMutex       = PTHREAD_MUTEX_INITIALIZER;

extern MI_S32    g_s32SocId;
extern MI_Muxer *g_muxerArray[];

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void GsensorSetPowerOnByInt(void)
{
    if (g_parkingSensitivity)
    {
        gsensor_int_fd = open(gsensor_int_enable, O_RDWR | O_NONBLOCK);
        if (gsensor_int_fd < 0)
        {
            printf("open gsensor %s fail\n", gsensor_int_enable);
        }
        else
        {
            // TODO : Set Sensitivity Level
            printf("set [%s]\n", gsensor_int_enable);
            write(gsensor_int_fd, "1", 2);
            close(gsensor_int_fd);
        }
    }
}

BOOL GsensorIsPowerOnByInt(void)
{
    char szStr[4] = {0};
    BOOL bPowerOnByInt;
    gsensor_int_fd = open(gsensor_int_start_stauts, O_RDWR | O_NONBLOCK);
    if (gsensor_int_fd < 0)
    {
        printf("open gsensor %s fail\n", gsensor_int_start_stauts);
        return FALSE;
    }
    else
    {
        read(gsensor_int_fd, szStr, 4);
        bPowerOnByInt = atoi(szStr);
        printf("Power On By G-sensor [%d]\n", bPowerOnByInt);
        close(gsensor_int_fd);
        return bPowerOnByInt;
    }
}

void GsensorHandler(input_event *pinput_ev)
{
    int x_diff = 0, y_diff = 0, z_diff = 0;

    if (g_EmergySensitivity == SENSITIVITY_OFF)
        return;

    if (pinput_ev->code == ABS_X)
    {
        if (gstGsnrInfoChuck.x != AXIS_INIT_VALUE)
        {
            x_diff = pinput_ev->value - gstGsnrInfoChuck.x;
            if (x_diff > x_range)
            {
                cardv_send_to_fifo(CARDV_CMD_EMERG_REC, sizeof(CARDV_CMD_EMERG_REC));
                CARDV_INFO("[x] Emerg Diff = %d (%d %d)\n", x_diff, pinput_ev->value, gstGsnrInfoChuck.x);
            }
        }
        gstGsnrInfoChuck.x = pinput_ev->value;
    }
    else if (pinput_ev->code == ABS_Y)
    {
        if (gstGsnrInfoChuck.y != AXIS_INIT_VALUE)
        {
            y_diff = pinput_ev->value - gstGsnrInfoChuck.y;
            if (y_diff > y_range)
            {
                cardv_send_to_fifo(CARDV_CMD_EMERG_REC, sizeof(CARDV_CMD_EMERG_REC));
                CARDV_INFO("[y] Emerg Diff = %d (%d %d)\n", y_diff, pinput_ev->value, gstGsnrInfoChuck.y);
            }
        }
        gstGsnrInfoChuck.y = pinput_ev->value;
    }
    else if (pinput_ev->code == ABS_Z)
    {
        if (gstGsnrInfoChuck.z != AXIS_INIT_VALUE)
        {
            z_diff = pinput_ev->value - gstGsnrInfoChuck.z;
            if (z_diff > z_range)
            {
                cardv_send_to_fifo(CARDV_CMD_EMERG_REC, sizeof(CARDV_CMD_EMERG_REC));
                CARDV_INFO("[z] Emerg Diff = %d (%d %d)\n", z_diff, pinput_ev->value, gstGsnrInfoChuck.z);
            }
        }
        gstGsnrInfoChuck.z = pinput_ev->value;
    }

    // printf("input type = %d, code = %d, val = %d\n", pinput_ev->type, pinput_ev->code, pinput_ev->value);
    CARDV_INFO("[%-6d,%-6d,%-6d]\n", x_diff, y_diff, z_diff); // output data to make curve graph by excel`s tool
}

void GsensorStartRec(MI_BOOL bStart)
{
    pthread_mutex_lock(&_gstGsnrMutex);
    gbGsnrChunkStartRec = bStart;
    gu32GsnrChunkWrIdx  = 0;
    pthread_mutex_unlock(&_gstGsnrMutex);
}

MI_U32 GsensorMove2CompBuf(MI_U8 *pu8SrcBuf)
{
    MI_U32 u32DstAddr = 0;

    if (gu32GsnrChunkWrIdx >= GSNR_CHUNK_MAX_CNT)
        return 0;

    for (int i = 0; i < MAX_MUXER_NUMBER; i++)
    {
        if (g_muxerArray[i] == NULL)
            continue;
        if (g_muxerArray[i]->IsMuxerEnable() && g_muxerArray[i]->IsGsnrFrameQFull())
            return 0;
    }

    u32DstAddr = (MI_U32)(gpu8GsnrCompBuf + gu32GsnrChunkWrIdx * sizeof(GSNRINFOCHUCK));
    memcpy((MI_U8 *)u32DstAddr, pu8SrcBuf, sizeof(GSNRINFOCHUCK));
    gu32GsnrChunkWrIdx = (gu32GsnrChunkWrIdx + 1) % GSNR_CHUNK_MAX_CNT;
    return u32DstAddr;
}

void *GsensorTask(void *argv)
{
    struct pollfd        poll_fd;
    struct input_event   input_ev;
    struct input_absinfo input_absinfo;
    int                  ret;
    MI_U64               u64LastPts = 0;

    CARDV_THREAD();

    gpu8GsnrCompBuf = (MI_U8 *)MALLOC(sizeof(GSNRINFOCHUCK) * GSNR_CHUNK_MAX_CNT);
    MuxerSetGsnrChunkSize(sizeof(GSNRINFOCHUCK));

    while (g_gsensorThread_exit == FALSE)
    {
        ret = access(gsensor_input_event, F_OK);
        if (ret == 0)
            break;

        usleep(100000);
    }

    gsensor_fd = open(gsensor_input_event, O_RDWR | O_NONBLOCK);
    if (gsensor_fd < 0)
    {
        printf("open gsensor %s fail\n", gsensor_input_event);
        pthread_exit(NULL);
    }

    ioctl(gsensor_fd, EVIOCGABS(ABS_X), &input_absinfo);
    g_xrange = (input_absinfo.maximum - input_absinfo.minimum) / RANGE_DIV;

    ioctl(gsensor_fd, EVIOCGABS(ABS_Y), &input_absinfo);
    g_yrange = (input_absinfo.maximum - input_absinfo.minimum) / RANGE_DIV;

    ioctl(gsensor_fd, EVIOCGABS(ABS_Z), &input_absinfo);
    g_zrange = (input_absinfo.maximum - input_absinfo.minimum) / RANGE_DIV;

    if (!g_xrange || !g_yrange || !g_zrange)
    {
        g_xrange = g_yrange = g_zrange = DEFAULT_RANGE;
    }
    x_range = (g_xrange * g_EmergySensitivity) / 100;
    y_range = (g_yrange * g_EmergySensitivity) / 100;
    z_range = (g_zrange * g_EmergySensitivity) / 100;
    printf("xyz range[%d %d %d]->[%d %d %d].\n", g_xrange, g_yrange, g_zrange, x_range, y_range, z_range);

    poll_fd.fd     = gsensor_fd;
    poll_fd.events = POLLIN;

    while (g_gsensorThread_exit == FALSE)
    {
        int retval = poll(&poll_fd, 1, 1000);
        if (retval == -1)
        {
            CARDV_ERR("gsensor poll error.\n");
            continue;
        }
        else if (retval)
        {
            ssize_t size = sizeof(struct input_event);
            if (read(gsensor_fd, &input_ev, size) == size)
            {
                if (input_ev.type == EV_ABS)
                {
                    GsensorHandler(&input_ev);
                }
            }
        }
        else
        {
            // printf("No data within one second.\n");
        }

        pthread_mutex_lock(&_gstGsnrMutex);
        if (gbGsnrChunkStartRec)
        {
            MI_U64 u64Pts;
            MI_SYS_GetCurPts(g_s32SocId, &u64Pts);
            if (u64Pts - u64LastPts >= 30000)
            { // 30ms
                MI_U32 u32FrameBuf = GsensorMove2CompBuf((MI_U8 *)&gstGsnrInfoChuck);
                if (u32FrameBuf)
                {
                    for (int i = 0; i < MAX_MUXER_NUMBER; i++)
                    {
                        if (g_muxerArray[i] == NULL)
                            continue;
                        if (g_muxerArray[i]->IsMuxerEnable())
                            g_muxerArray[i]->PushInfoToGsnrFrameQ(u32FrameBuf, u64Pts);
                    }
                    u64LastPts = u64Pts;
                }
            }
        }
        pthread_mutex_unlock(&_gstGsnrMutex);
    }

    FREEIF(gpu8GsnrCompBuf);
    close(gsensor_fd);
    pthread_exit(NULL);
}

void GsensorStartThread(void)
{
    int ret              = 0;
    g_gsensorThread_exit = FALSE;

    if (0 != g_gsensorThread)
    {
        CARDV_ERR("%s alread start\n", __func__);
        return;
    }

    ret = pthread_create(&g_gsensorThread, NULL, GsensorTask, NULL);
    if (0 == ret)
    {
        pthread_setname_np(g_gsensorThread, "gsensor_task");
    }
    else
    {
        CARDV_ERR("%s pthread_create failed\n", __func__);
    }
}

void GsensorStopThread(void)
{
    g_gsensorThread_exit = TRUE;
    pthread_join(g_gsensorThread, NULL);
    return;
}

void GsensorSetSensitivity(MI_S8 level)
{
    switch (level)
    {
    case 0:
        g_EmergySensitivity = SENSITIVITY_OFF;
        break;
    case 1:
        g_EmergySensitivity = SENSITIVITY_LV0;
        break;
    case 2:
        g_EmergySensitivity = SENSITIVITY_LV1;
        break;
    case 3:
        g_EmergySensitivity = SENSITIVITY_LV2;
        break;
    case 4:
        g_EmergySensitivity = SENSITIVITY_LV3;
        break;
    case 5:
        g_EmergySensitivity = SENSITIVITY_LV4;
        break;
    case 6:
        g_EmergySensitivity = SENSITIVITY_LV5;
        break;
    default:
        CARDV_ERR("parkingSensitivity is out off range!!\n");
        break;
    }

    x_range = (g_xrange * g_EmergySensitivity) / 100;
    y_range = (g_yrange * g_EmergySensitivity) / 100;
    z_range = (g_zrange * g_EmergySensitivity) / 100;

    printf("xyz range[%d %d %d]->[%d %d %d].\n", g_xrange, g_yrange, g_zrange, x_range, y_range, z_range);
}

void GsensorParkingMonitor(int param)
{
    g_parkingSensitivity = param;
}