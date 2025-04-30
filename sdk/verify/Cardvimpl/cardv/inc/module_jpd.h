/*
 * module_jpd.h- Sigmastar
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
#ifndef _MODULE_JPD_H_
#define _MODULE_JPD_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_jpd.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define CARDV_MAX_JPD_CHN_NUM (16)
#define CARDV_JPD_DEV_ID      (0)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct CarDV_JpdChnAttr_s
{
    MI_JPD_CHN u32ChnId;
    MI_BOOL    bUsed;
    MI_BOOL    bCreate;
    MI_U32     u32MaxW;
    MI_U32     u32MaxH;
    MI_U32     u32StreamBufSize;
    MI_U32     u32Fps;
    MI_BOOL    bThreadExit;
    pthread_t  pPutDataThread;
    char       szFilePath[64];
} CarDV_JpdChnAttr_t;

typedef struct CarDV_JpdModAttr_s
{
    CarDV_JpdChnAttr_t stJpdChnAttr[CARDV_MAX_JPD_CHN_NUM];
} CarDV_JpdModAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_JpdModAttr_t gstJpdModule;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_JpdModuleInit(MI_JPD_CHN u32JpdChn);
MI_S32 CarDV_JpdModuleUnInit(MI_JPD_CHN u32JpdChn);

#endif //#define _MODULE_JPD_H_