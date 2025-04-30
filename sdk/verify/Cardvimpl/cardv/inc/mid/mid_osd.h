/*
 * mid_osd.h- Sigmastar
 *
 * Copyright (C) 2021 Sigmastar Technology Corp.
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

#ifndef _MID_OSD_H_
#define _MID_OSD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    MI_U32 u16X;
    MI_U32 u16Y;
} DrawPoint_t;

typedef struct
{
    MI_RGN_PixelFormat_e ePixelFmt;
    MI_U32               u32Color;
} DrawRgnColor_t;

void DrawPoint(void *pBaseAddr, MI_U32 u32Stride, DrawPoint_t stPt, DrawRgnColor_t stColor);
void DrawLine(void *pBaseAddr, MI_U32 u32Stride, DrawPoint_t stStartPt, DrawPoint_t stEndPt, MI_U8 u8BorderWidth,
              DrawRgnColor_t stColor);
void DrawRect(void *pBaseAddr, MI_U32 u32Stride, DrawPoint_t stLeftTopPt, DrawPoint_t stRightBottomPt,
              MI_U8 u8BorderWidth, DrawRgnColor_t stColor);
void DrawRect2(void *pBaseAddr, MI_U32 u32Stride, DrawPoint_t *pstLeftTopPt, DrawPoint_t *pstRightBottomPt,
               MI_U8 u8BorderWidth, DrawRgnColor_t stColor);
void DrawBmpToI4(void *pBaseAddr, MI_U32 u32Stride, const char *pBmpFileName, DrawPoint_t stDrawPt);

void FillRect(void *pBaseAddr, MI_U32 u32Stride, DrawPoint_t stLeftTopPt, DrawPoint_t stRightBottomPt,
              DrawRgnColor_t stColor);
void DrawCircular(void *pBaseAddr, MI_U32 u32Stride, DrawPoint_t stCenterPt, MI_U32 u32Radius, MI_U8 u8BorderWidth,
                  DrawRgnColor_t stColor);
void FillCircular(void *pBaseAddr, MI_U32 u32Stride, DrawPoint_t stCenterPt, MI_U32 u32Radius, DrawRgnColor_t stColor);

#ifdef __cplusplus
}
#endif

#endif //_MID_OSD_H_
