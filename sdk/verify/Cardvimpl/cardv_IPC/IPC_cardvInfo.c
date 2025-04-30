/*
 * IPC_cardvInfo.c - Sigmastar
 *
 * Copyright (C) 2019 Sigmastar Technology Corp.
 *
 * Author: jeff.li <jeff.li@sigmastar.com.cn>
 * Desc: for Inter-Process Communication
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "IPC_cardvInfo.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

int gCarInfoFd = 0;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int IPC_CarInfo_Open(void)
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    gCarInfoFd = open("/tmp/car_info", O_RDWR | O_CREAT, 0666);
    if (gCarInfoFd != 0)
        return 0;
    else
        return -1;
#else
    return 0;
#endif
}

int IPC_CarInfo_Close(void)
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    if (gCarInfoFd != 0)
    {
        close(gCarInfoFd);
        return 0;
    }
    return -1;
#else
    return 0;
#endif
}

int IPC_CarInfo_Write(struct CarDV_Info *pCarInfo)
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    ssize_t size = sizeof(struct CarDV_Info);
    if (gCarInfoFd != 0)
    {
        if (pwrite(gCarInfoFd, pCarInfo, size, 0) == size)
            return 0;
    }
    return -1;
#else
    return 0;
#endif
}

int IPC_CarInfo_Read(struct CarDV_Info *pCarInfo)
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    ssize_t size = sizeof(struct CarDV_Info);
    if (gCarInfoFd != 0)
    {
        if (pread(gCarInfoFd, pCarInfo, size, 0) == size)
            return 0;
    }
    return -1;
#else
    return 0;
#endif
}

int IPC_CarInfo_Write_PartInfo(void *pCarPartInfo, ssize_t size, ssize_t offset)
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    if (gCarInfoFd != 0)
    {
        if (pwrite(gCarInfoFd, pCarPartInfo, size, offset) == size)
            return 0;
    }
    return -1;
#else
    return 0;
#endif
}

int IPC_CarInfo_Read_PartInfo(void *pCarPartInfo, ssize_t size, ssize_t offset)
{
#if (defined CHIP_I6E || defined CHIP_M6 || defined CHIP_I6C || defined CHIP_IFORD)
    if (gCarInfoFd != 0)
    {
        if (pread(gCarInfoFd, pCarPartInfo, size, offset) == size)
            return 0;
    }
    return -1;
#else
    return 0;
#endif
}
