/*
 * module_ipc.h- Sigmastar
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
#ifndef _MODULE_IPC_H_
#define _MODULE_IPC_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mid_common.h"

//==============================================================================
//
//                              TYPE DEFINES
//
//==============================================================================

typedef void (*SvrConnect)(MI_S32, void *, MI_U32);
typedef void (*SvrDisConnect)(void *, MI_U32);

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct
{
    SvrConnect    pSvrConnect;
    SvrDisConnect pSvrDisConnect;
    void *        pArg;
    MI_U32        u32ArgLen;
} CarDV_IpcParam_t;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int  IpcClientConnect(CarDV_IpcParam_t *pstParam);
int  IpcClientDisConnect(int fd);
void IpcServerStart(void);
void IpcServerStop(void);

#endif // _MODULE_IPC_H_