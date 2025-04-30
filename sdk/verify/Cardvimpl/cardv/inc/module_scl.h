/*
 * module_scl.h- Sigmastar
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
#ifndef _MODULE_SCL_H_
#define _MODULE_SCL_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_scl.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define CARDV_MAX_SCL_INPORT_NUM  (1)
#define CARDV_MAX_SCL_CHN_NUM     (16)
#define CARDV_MAX_SCL_HWSCLID_NUM (4)
#define CARDV_MAX_SCL_OUTPORT_NUM (CARDV_MAX_SCL_HWSCLID_NUM)
#define CARDV_MAX_SCL_SOURCE_NUM  (5)
#define CARDV_MAX_SCL_DEV_NUM     (3)

#define CARDV_SCL_DEV_FROM_ISP_REALTIME (0) // SCL Dev0
#define CARDV_SCL_DEV_FROM_RDMA0        (1) // SCL Dev1
#define CARDV_SCL_DEV_FROM_YUV_REALTIME (2) // SCL Dev2

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct CarDV_SclChannelAttr_s
{
    MI_U8              u8ChnId;
    MI_BOOL            bUsed;
    MI_BOOL            bCreate;
    MI_BOOL            bBind;
    CarDV_InPortAttr_t stSclInPortAttr[CARDV_MAX_SCL_INPORT_NUM];
    CarDV_PortAttr_t   stSclOutPortAttr[CARDV_MAX_SCL_OUTPORT_NUM];
} CarDV_SclChannelAttr_t;

typedef struct CarDV_SclDevAttr_s
{
    MI_U8                  u8DevId;
    MI_BOOL                bUsed;
    MI_BOOL                bCreate;
    MI_BOOL                bConfigPrivPool;
    MI_U16                 u16PrivPoolMaxWidth;
    MI_U16                 u16PrivPoolMaxHeight;
    MI_U16                 u16PrivPoolRingLine;
    MI_U32                 u32UseHwSclMask;
    MI_U8                  u8HwSclPortId[CARDV_MAX_SCL_HWSCLID_NUM];
    CarDV_SclChannelAttr_t stSclChnlAttr[CARDV_MAX_SCL_CHN_NUM];
} CarDV_SclDevAttr_t;

typedef struct CarDV_SclModAttr_s
{
    CarDV_SclDevAttr_t stSclDevAttr[CARDV_MAX_SCL_DEV_NUM];
} CarDV_SclModAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_SclModAttr_t gstSclModule;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_SclModuleInit(MI_SCL_DEV u32SclDevId);
MI_S32 CarDV_SclModuleUnInit(MI_SCL_DEV u32SclDevId);
MI_S32 CarDV_SclModuleBind(MI_SCL_DEV u32SclDevId, MI_BOOL bBind);
MI_S32 CarDV_SclChannelBind(MI_SCL_DEV u32SclDevId, MI_SCL_CHANNEL u32SclChnId, MI_BOOL bBind);
MI_S32 CarDV_SclGetDevChnByVifDev(MI_VIF_DEV u32VifDev, MI_SCL_DEV *pu32SclDevId, MI_SCL_CHANNEL *pu32SclChnId);
MI_S32 CarDV_SclGetVifDev(MI_VIF_DEV *pu32VifDev, MI_SCL_DEV u32SclDevId, MI_SCL_CHANNEL u32SclChnId);
MI_S32 CarDV_SclChangeOutputPortSize(MI_SCL_DEV u32SclDevId, MI_SCL_CHANNEL u32SclChnId, MI_SCL_PORT SclOutPortId,
                                     MI_SYS_WindowSize_t *pstPortSize);
MI_S32 CarDV_SclEnableOutputPort(MI_SCL_DEV u32SclDevId, MI_SCL_CHANNEL u32SclChnId, MI_SCL_PORT SclOutPortId,
                                 MI_BOOL bEnable);

#endif //#define _MODULE_SCL_H_