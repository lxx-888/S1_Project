/*************************************************
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 * All rights reserved.
 *
 **************************************************
 * File name:  module_system.c
 * Author:     andely.zhou@sigmastar.com.cn
 * Version:    Initial Draft
 * Date:       2018/6/13
 * Description: CarDV system module source file
 *
 *
 *
 * History:
 *
 *    1. Date  :        2018/6/13
 *       Author:        andely.zhou@sigmastar.com.cn
 *       Modification:  Created file
 *
 **************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include <execinfo.h>

#include "module_common.h"
#include "module_video.h"
#include "module_gps.h"
#include "module_menusetting.h"
#include "module_watchdog.h"
#include "mid_common.h"
#include "mid_demux.h"
#include "mi_iqserver.h"
#include "mi_sensor.h"
#include "live555.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define COREDUMP_TO_SDCARD (1)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

live555 *g_live555Server = NULL;

extern MI_AudioEncoder *g_pAudioEncoderArray[];
extern MI_VideoEncoder *g_videoEncoderArray[];

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void cardv_send_to_fifo(const void *cmd, size_t size)
{
    int pipe_fd_w = 0;
    pipe_fd_w     = open(FIFO_NAME_HIGH_PRIO, O_WRONLY);
    if (pipe_fd_w >= 0)
    {
        write(pipe_fd_w, cmd, size);
        close(pipe_fd_w);
    }
}

void cardv_update_status(const char *status_name, const char *status, size_t size)
{
    int pipe_fd_wr = 0;
    pipe_fd_wr     = open(status_name, O_RDWR | O_CREAT, 0666);
    if (pipe_fd_wr >= 0)
    {
        write(pipe_fd_wr, status, size);
        close(pipe_fd_wr);
    }
}

int cardv_get_status(const char *status_name)
{
    int  pipe_fd_rd = 0;
    char status[2]  = {0};
    pipe_fd_rd      = open(status_name, O_RDONLY);
    if (pipe_fd_rd >= 0)
    {
        read(pipe_fd_rd, status, sizeof(status));
        close(pipe_fd_rd);
    }
    return atoi(status);
}

void cardv_record_boot_time(int line)
{
    int  fd_w   = 0;
    char buf[8] = {0};
    sprintf(buf, "%d", line);
    fd_w = open("/sys/devices/virtual/mstar/msys/booting_time", O_WRONLY);
    if (fd_w >= 0)
    {
        write(fd_w, buf, 8);
        close(fd_w);
    }
}

#if (COREDUMP_TO_SDCARD)
void cardv_set_coredump_path(void)
{
    int  fd_w  = 0;
    char buf[] = "/mnt/mmc/core-%e-%p-%t";
    fd_w       = open("/proc/sys/kernel/core_pattern", O_WRONLY);
    if (fd_w >= 0)
    {
        write(fd_w, buf, sizeof(buf));
        close(fd_w);
    }
}
#endif

MI_S32 system_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    switch (id)
    {
    case CMD_SYSTEM_INIT:
        CARDVCHECKRESULT(CarDV_Sys_Init());
        break;

    case CMD_SYSTEM_UNINIT:
        CARDVCHECKRESULT(CarDV_Sys_Exit());
        break;

#if (CARDV_IQSERVER_ENABLE)
    case CMD_SYSTEM_IQSERVER_OPEN:
        CARDVCHECKRESULT(MI_IQSERVER_SetDataPath((char *)"/config/iqfile"));
        CARDVCHECKRESULT(MI_IQSERVER_Open());
        break;

    case CMD_SYSTEM_IQSERVER_CLOSE:
        CARDVCHECKRESULT(MI_IQSERVER_Close());
        break;
#endif

#if (CARDV_LIVE555_ENABLE)
    case CMD_SYSTEM_LIVE555_OPEN: {
        char streamName[64] = {
            0,
        };

        if (g_live555Server == NULL)
        {
            g_live555Server = live555::createNew();
            if (g_live555Server)
            {
                // sample: "video0" "video1"
                for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
                {
                    if (g_videoEncoderArray[i] && g_videoEncoderArray[i]->getVideoType() == VIDEO_STREAM_RTSP)
                    {
#if 0 // For debug
                        sprintf(streamName, "video%d", g_videoEncoderArray[i]->getVencChn());
#else
                        char str[4] = {
                            0,
                        };
                        cardv_menu_getResult(GET_CONFIG_SETTING " Camera.Preview.RTSP.av", str, sizeof(str));
                        sprintf(streamName, "liveRTSP/av%d", atoi(str) + g_videoEncoderArray[i]->getCamId());
#endif
                        if (g_videoEncoderArray[i]->getCamId() == 0)
                        {
                            g_live555Server->createServerMediaSession(g_videoEncoderArray[i], g_pAudioEncoderArray[0],
                                                                      streamName, g_videoEncoderArray[i]->getCamId());
                        }
                        else
                        {
                            g_live555Server->createServerMediaSession(g_videoEncoderArray[i], NULL, streamName,
                                                                      g_videoEncoderArray[i]->getCamId());
                        }
                    }
                }

                g_live555Server->startLive555Server();
            }
        }
    }
    break;

    case CMD_SYSTEM_LIVE555_CLOSE_SESSION: {
        if (g_live555Server)
        {
            // Sample : close all camera rtsp sessions.
            for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
            {
                if (g_videoEncoderArray[i] && g_videoEncoderArray[i]->getVideoType() == VIDEO_STREAM_RTSP)
                {
                    g_live555Server->closeServerMediaSession(g_videoEncoderArray[i]->getCamId());
                }
            }
        }
    }
    break;

    case CMD_SYSTEM_LIVE555_CLOSE: {
        if (g_live555Server)
        {
            g_live555Server->stopLive555Server();
            delete g_live555Server;
            g_live555Server = NULL;
        }
    }
    break;
#endif
    case CMD_SYSTEM_CORE_BACKTRACE: {
        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));
        sigemptyset(&sa.sa_mask);
        sa.sa_sigaction = segfault_sigaction;
        sa.sa_flags     = SA_SIGINFO;

        sigaction(SIGSEGV, &sa, NULL);
        sigaction(SIGILL, &sa, NULL);
        sigaction(SIGABRT, &sa, NULL);
        sigaction(SIGBUS, &sa, NULL);

#if (COREDUMP_TO_SDCARD)
        cardv_set_coredump_path();
#endif
    }
    break;

    case CMD_SYSTEM_VENDOR_GPS: {
        MI_S32 GpsEnable = *param;
        if (GpsEnable)
            GpsStartThread();
        else
            GpsStopThread();
    }
    break;

    case CMD_SYSTEM_VENDOR_EDOG_CTRL: {
        MI_S32 EdogParam[2] = {0};
        memcpy(EdogParam, param, sizeof(EdogParam));
        EDOGCtrl_Operation(EdogParam, sizeof(EdogParam));
    }
    break;

    case CMD_SYSTEM_DEMUX_THUMB: {
        char filename[64];
        memset(filename, 0x00, sizeof(filename));
        memcpy(filename, param, paramLen);
        DemuxThumbnail(filename);
    }
    break;

    case CMD_SYSTEM_DEMUX_TOTALTIME: {
        char filename[64];
        memset(filename, 0x00, sizeof(filename));
        memcpy(filename, param, paramLen);
        DemuxDuration(filename);
    }
    break;

#if (CARDV_WATCHDOG_ENABLE)
    case CMD_SYSTEM_WATCHDOG_OPEN:
        watchDogInit();
        break;

    case CMD_SYSTEM_WATCHDOG_CLOSE:
        watchDogUninit();
        break;
#endif

    default:
        break;
    }

    return 0;
}