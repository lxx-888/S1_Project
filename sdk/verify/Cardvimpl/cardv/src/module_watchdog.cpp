/*
 * module_watchdog.cpp- Sigmastar
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

#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include "module_watchdog.h"
#include "module_config.h"

#if (CARDV_WATCHDOG_ENABLE)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

static int             g_watchdog = 0;
static pthread_t       pthreadWatchdog;
static pthread_mutex_t g_wdtMutex;
static pthread_cond_t  g_wdtCond;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void* WatchDog_Task(void* argu)
{
    int             wdt_fd = -1;
    struct timeval  now;
    struct timespec outtime;

    wdt_fd = open("/dev/watchdog", O_WRONLY);

    if (wdt_fd == -1)
    {
        printf("open WatchDog failed\n");
        return NULL;
    }

    int timeout = 10;
    ioctl(wdt_fd, WDIOC_SETTIMEOUT, &timeout);

    while (g_watchdog)
    {
        ioctl(wdt_fd, WDIOC_KEEPALIVE, 0);

        pthread_mutex_lock(&g_wdtMutex);
        gettimeofday(&now, NULL);
        outtime.tv_sec  = now.tv_sec + 5;
        outtime.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&g_wdtCond, &g_wdtMutex, &outtime);
        pthread_mutex_unlock(&g_wdtMutex);
    }

    int option = WDIOS_DISABLECARD;
    ioctl(wdt_fd, WDIOC_SETOPTIONS, &option);

    printf("release WatchDog\n");
    close(wdt_fd);
    wdt_fd = -1;

    return NULL;
}

void watchDogInit()
{
    g_watchdog = 1;

    pthread_cond_init(&g_wdtCond, NULL);
    pthread_mutex_init(&g_wdtMutex, NULL);

    pthread_create(&pthreadWatchdog, NULL, WatchDog_Task, NULL);
    pthread_setname_np(pthreadWatchdog, "WatchDog_Task");
}

void watchDogUninit()
{
    if (g_watchdog > 0)
    {
        g_watchdog = 0;
        pthread_mutex_lock(&g_wdtMutex);
        pthread_cond_signal(&g_wdtCond);
        pthread_mutex_unlock(&g_wdtMutex);

        pthread_join(pthreadWatchdog, NULL);
        pthread_cond_destroy(&g_wdtCond);
        pthread_mutex_destroy(&g_wdtMutex);
    }
}
#endif