/*
 * module_display.h- Sigmastar
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

#ifndef _MODULE_DISPLAY_H_
#define _MODULE_DISPLAY_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_disp.h"
#if (CARDV_PANEL_ENABLE)
#include "mi_panel.h"
#endif

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define CARDV_MAX_DISP_DEV_NUM         (1)
#define CARDV_MAX_DISP_LAYER_NUM       (1)
#define CARDV_MAX_DISP_LAYER0_PORT_NUM (1)
#define CARDV_MAX_DISP_LAYER1_PORT_NUM (0)

#define CARDV_MAX_DISP_BIND_INFO (16)

#define MAKE_YUYV_VALUE(y, u, v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK               MAKE_YUYV_VALUE(0, 128, 128)
#define YUYV_WHITE               MAKE_YUYV_VALUE(255, 128, 128)
#define YUYV_RED                 MAKE_YUYV_VALUE(76, 84, 255)
#define YUYV_GREEN               MAKE_YUYV_VALUE(149, 43, 21)
#define YUYV_BLUE                MAKE_YUYV_VALUE(29, 225, 107)

//==============================================================================
//
//                              STRUCT & ENUM DEFINES
//
//==============================================================================

typedef enum _DispType_e
{
    Disp_Prm_Cam = 0,
    Disp_Snd_Cam = 1,
    Disp_Trd_Cam = 2,
    Disp_Prm_Snd_Cam,
    Disp_Prm_Trd_Cam,
    Disp_Snd_Trd_Cam,
    Disp_Prm_Snd_Trd_Cam,
    Disp_Type_Num,
} DispType_e;

typedef struct CarDV_DispPortAttr_s
{
    MI_BOOL              bCreate;
    MI_BOOL              bUsed;
    MI_BOOL              bBind;
    MI_BOOL              bInject;
    MI_BOOL              bThreadExit;
    pthread_t            pInjectThread;
    MI_U16               u16SrcWidth;
    MI_U16               u16SrcHeight;
    MI_DISP_VidWinRect_t stDispPortWin;
    MI_DISP_VidWinRect_t stDispZoomInWin;
    CarDV_BindParam_t    stDispInBindParam;
} CarDV_DispPortAttr_t;

typedef struct CarDV_DispLayerAttr_s
{
    MI_BOOL              bCreate;
    MI_BOOL              bUsed;
    MI_U16               u16Width;
    MI_U16               u16Height;
    MI_DISP_LAYER        s32LayerId;
    MI_DISP_VidWinRect_t stDispLayerWin;
    CarDV_DispPortAttr_t stDispPortAttr[CARDV_MAX_DISP_LAYER0_PORT_NUM];
} CarDV_DispLayerAttr_t;

typedef struct CarDV_DispDevAttr_s
{
    MI_BOOL                bCreate;
    MI_BOOL                bUsed;
    MI_DISP_Interface_e    eIntfType;
    MI_DISP_OutputTiming_e eIntfSync;
    CarDV_DispLayerAttr_t  stDispLayerAttr[CARDV_MAX_DISP_LAYER_NUM];
} CarDV_DispDevAttr_t;

typedef struct CarDV_DispWbcAttr_s
{
    MI_BOOL                  bCreate;
    MI_BOOL                  bUsed;
    MI_DISP_WBC_SourceType_e eSourceType;
    MI_U32                   u32SourceId;
    MI_U32                   u32Width;
    MI_U32                   u32Height;
} CarDV_DispWbcAttr_t;

typedef struct CarDV_DispModAttr_s
{
    CarDV_DispDevAttr_t stDispDevAttr[CARDV_MAX_DISP_DEV_NUM];
    CarDV_DispWbcAttr_t stDispWbcAttr;
} CarDV_DispModAttr_t;

typedef struct CarDV_DispBindAttr_s
{
    MI_U32               u32BindMapNum;
    MI_U32               u32BindPbMapNum;
    MI_U32               u32BindMap[CARDV_MAX_DISP_BIND_INFO];
    MI_BOOL              bBind[CARDV_MAX_DISP_BIND_INFO];
    CarDV_Sys_BindInfo_T stBindInfo[CARDV_MAX_DISP_BIND_INFO];
    MI_BOOL              bPlayBackBind[CARDV_MAX_DISP_BIND_INFO];
    CarDV_Sys_BindInfo_T stPlayBackBindInfo[CARDV_MAX_DISP_BIND_INFO];
} CarDV_DispBindAttr_t;

//==============================================================================
//
//                              EXTERN GLOBAL VALUE
//
//==============================================================================

// extern DispType_e eDispMode;
extern CarDV_DispModAttr_t  gstDispModule;
extern CarDV_DispBindAttr_t gstDispBindAttr;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_DISP_Interface_e    CarDV_DispModuleParserIntfType(char *str);
MI_DISP_OutputTiming_e CarDV_DispModuleParserIntfSync(char *str);

MI_S32 CarDV_DispModuleInit(void);
MI_S32 CarDV_DispModuleUnInit(void);
MI_S32 CarDV_DispModuleBind(MI_BOOL bBind);
MI_S32 CarDV_DispModuleInput(MI_BOOL bEnable);

#endif //#define _MODULE_DISPLAY_H_