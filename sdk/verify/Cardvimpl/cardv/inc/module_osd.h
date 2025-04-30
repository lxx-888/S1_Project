/*
 * module_osd.h- Sigmastar
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
#ifndef _MODULE_OSD_H_
#define _MODULE_OSD_H_

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include "mi_rgn.h"
#include "mid_common.h"
#include "module_font.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define CARDV_OSD_BUFFER_NUM     (3)
#define CARDV_OSD_MAX_ATTACH_NUM (8)

#define STAMP_MODE_DATELOGO (0)
#define STAMP_MODE_DATE     (1)
#define STAMP_MODE_LOGO     (2)
#define STAMP_MODE_OFF      (3)

#define DATE_FORMAT_OFF (0)
#define DATE_FORMAT_YMD (1)
#define DATE_FORMAT_MDY (2)
#define DATE_FORMAT_DMY (3)

#define RGN_OSD_START (0)

#define CARDV_OSD_COLOR_INVERSE_THD          (96)
#define CARDV_RGN_PALETTEL_TABLE_ALPHA_INDEX (0x00)
#define DEFAULT_OSD_REFRESH_INTERVAL         (500)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct _TextWidgetAttr_s
{
    char *               string;
    Point_t *            pPoint;
    CarDV_FontSize_e     size;
    MI_RGN_PixelFormat_e pmt;
    Color_t *            pfColor;
    Color_t *            pbColor;
    MI_U8                u32Color;
    BOOL                 bOutline;
    BOOL                 bMirror;
    BOOL                 bFlip;
} TextWidgetAttr_t;

typedef struct _CarDV_OsdBufferAttr
{
    MI_BOOL              bCreate;
    MI_U8                u8AttachCnt;
    MI_RGN_HANDLE        u32RgnHandle;
    MI_U32               u32Width;
    MI_U32               u32Height;
    MI_RGN_PixelFormat_e eRgnFormat;
    // Follow code for OSD buffer.
    MI_BOOL          bDrawStampLogo;
    CarDV_FontSize_e eFontSize;
    Point_t          stPointTime;
    Point_t          stPointGps;
} CarDV_OsdBufferAttr;

typedef struct _CarDV_OsdAttachAttr
{
    MI_BOOL          bUsed;
    MI_BOOL          bAttach;
    MI_RGN_ChnPort_t stRgnChnPort;
    MI_RGN_HANDLE    u32RgnHandle;
} CarDV_OsdAttachAttr;

typedef struct _CarDV_OsdAttr
{
    CarDV_OsdBufferAttr stBufferAttr[CARDV_OSD_BUFFER_NUM];
    CarDV_OsdAttachAttr stAttachAttr[CARDV_OSD_MAX_ATTACH_NUM];
} CarDV_OsdAttr;

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern CarDV_OsdAttr gstOsdAttr;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int  osd_init();
int  osd_uninit();
void OSD_DrawVideoStamp(MI_RGN_CanvasInfo_t *pstRgnCanvasInfo);
void osd_SetStampMode(int s32Mode);
void osd_SetTimeFormat(int s32Mode);

MI_S32 mid_osd_full_init();
MI_S32 mid_osd_full_uninit();
MI_S32 mid_osd_reset_resolution(MI_VENC_CHN VeChn);
MI_S32 mid_osd_restart_resolution(MI_VENC_CHN VeChn, Size_t *pstSize);
MI_S32 mid_OSD_UpdateTextWidget_Rgn(MI_RGN_CanvasInfo_t *pstRgnCanvasInfo, TextWidgetAttr_t *pstTextWidgetAttr);
MI_S32 mid_OSD_CleanWidget_Rgn(MI_RGN_CanvasInfo_t *pstRgnCanvasInfo, Rect_t *pstRect, MI_U32 index);

#endif //#define _MODULE_OSD_H_