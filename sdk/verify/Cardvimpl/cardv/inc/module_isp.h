/*
 * module_isp.h- Sigmastar
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
#ifndef _MODULE_ISP_H_
#define _MODULE_ISP_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_isp.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define CARDV_MAX_ISP_INPORT_NUM  (1)
#define CARDV_MAX_ISP_CHN_NUM     (16)
#define CARDV_MAX_ISP_OUTPORT_NUM (3)
#define CARDV_MAX_ISP_DEV_NUM     (1)

#define CARDV_ISP_OUTPORT_SCL_REALTIME (0)
#define CARDV_ISP_OUTPORT_BUFFER       (1)
#define CARDV_ISP_OUTPORT_IR           (2)

#define CARDV_ISP_ZOOM_MULTIPLE_NUM (3) // zoom multiple is num/den, Ex: 2.5X = 5/2
#define CARDV_ISP_ZOOM_MULTIPLE_DEN (1)
#define CARDV_ISP_ZOOM_STEP         (8) // align to 2

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct CarDV_IspChannelAttr_s
{
    MI_U8              u8ChnId;
    MI_U8              u8CamId;
    MI_BOOL            bUsed;
    MI_BOOL            bCreate;
    MI_BOOL            bBind;
    MI_BOOL            bZoomCreate;
    MI_BOOL            bZoomStart;
    MI_U32             u32ZoomEntryNum;
    CarDV_InPortAttr_t stIspInPortAttr[CARDV_MAX_ISP_INPORT_NUM];
    CarDV_PortAttr_t   stIspOutPortAttr[CARDV_MAX_ISP_OUTPORT_NUM];
    MI_ISP_ChnParam_t  stIspChnParam;
    MI_SNR_PADID       eBindMiSnrPadId;
    char               szApiBinFilePath[128];
    MI_BOOL            bThreadExit;
    pthread_t          pLoadApiBinThread;
    // For long exposure capture
    MI_U32 u32ShutterUs;
    MI_U32 u32ExpoFrameCnt;
    MI_U32 u32ExpoRecoverFrameCnt;
} CarDV_IspChannelAttr_t;

typedef struct CarDV_IspDevAttr_s
{
    MI_U8                  u8DevId;
    MI_BOOL                bUsed;
    MI_BOOL                bCreate;
    CarDV_IspChannelAttr_t stIspChnlAttr[CARDV_MAX_ISP_CHN_NUM];
    MI_BOOL                bThreadExit;
    pthread_t              pIspFrameSyncThread;
} CarDV_IspDevAttr_t;

typedef struct CarDV_IspModAttr_s
{
    CarDV_IspDevAttr_t stIspDevAttr[CARDV_MAX_ISP_DEV_NUM];
} CarDV_IspModAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_IspModAttr_t gstIspModule;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_S32 CarDV_IspModuleInit(MI_ISP_DEV u32IspDevId);
MI_S32 CarDV_IspModuleUnInit(MI_ISP_DEV u32IspDevId);
MI_S32 CarDV_IspChannelInit(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId);
MI_S32 CarDV_IspChannelUnInit(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId);
MI_S32 CarDV_IspModuleBind(MI_ISP_DEV u32IspDevId, MI_BOOL bBind);
MI_S32 CarDV_IspChannelBind(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId, MI_BOOL bBind);
MI_S32 CarDV_IspGetDevChnByVifDev(MI_VIF_DEV u32VifDev, MI_ISP_DEV *pu32IspDevId, MI_ISP_CHANNEL *pu32IspChnId);
MI_S32 CarDV_IspGetDevChnByCamId(MI_U8 u8CamId, MI_ISP_DEV *pu32IspDevId, MI_ISP_CHANNEL *pu32IspChnId);
MI_S32 CarDV_IspGetVifDev(MI_VIF_DEV *pu32VifDev, MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId);
MI_S32 CarDV_IspZoomStart(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId, MI_BOOL bZoomIn);
MI_S32 CarDV_IspZoomStop(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId);
MI_S32 CarDV_IspZoomReset(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId);
MI_S32 CarDV_IspSetExpo(MI_ISP_DEV u32IspDevId, MI_ISP_CHANNEL u32IspChnId, MI_U32 u32ShutterUs,
                        MI_U32 u32ExpoFrameCnt);

#endif //#define _MODULE_ISP_H_