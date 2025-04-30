/*
 * module_storage.h- Sigmastar
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
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
#ifndef _MODULE_STORAGE_H_
#define _MODULE_STORAGE_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "module_config.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

typedef enum _FileSytemType
{
    FS_TYPE_FAT32 = 0,
#if (CARDV_EXFAT_ENABLE)
    FS_TYPE_EXFAT,
#endif
    FS_TYPE_NUM,
} FileSytemType;

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

void               StorageStartDetectThread(void);
void               StorageStopDetectThread(void);
BOOL               StorageIsMounted(void);
int                StorageSetVolume(const char *volume);
int                StorageFormat(FileSytemType eFsType);
int                StorageMount(BOOL bFormat);
int                StorageUnMount(BOOL bFormat);
int                StorageReMount(void);
unsigned long long StorageGetTotalSize(void);
unsigned long long StorageGetOtherTotalSize(void);
unsigned long long StorageGetReservedTotalSize(void);

#endif //#define _MODULE_STORAGE_H_
