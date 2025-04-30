/*
 * module_font.h- Sigmastar
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
#ifndef _MODULE_FONT_H_
#define _MODULE_FONT_H_

#include "mid_common.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define MI_RGN_COLOR_KEY_VALUE 0x2323

#define HZ_8P_BIN_SIZE  65424
#define HZ_12P_BIN_SIZE 196272
#define HZ_16P_BIN_SIZE 261696

#define HZ_24_BIN_SIZE    636192
#define HZ_32_BIN_SIZE    1131008
#define ASCII_24_BIN_SIZE 18432
#define ASCII_32_BIN_SIZE 32768

#define CARDV_OSD_FONT_24_BIN "ascii_24x24.mz" // "hanzi_24x24.mz"
#define CARDV_OSD_FONT_32_BIN "ascii_32x32.mz" // "hanzi_32x32.mz"

//==============================================================================
//
//                              ENUM & STRUCT DEFINES
//
//==============================================================================

typedef enum
{
    FONT_SIZE_8  = 8,
    FONT_SIZE_12 = 12,
    FONT_SIZE_16 = 16,
    FONT_SIZE_24 = 24,
    FONT_SIZE_32 = 32,
    FONT_SIZE_48 = 48,
    FONT_SIZE_64 = 64,
    FONT_SIZE_72 = 72,
    FONT_SIZE_96 = 96
} CarDV_FontSize_e;

typedef enum
{
    HZ_DOT_8,
    HZ_DOT_12,
    HZ_DOT_16,
    HZ_DOT_24,
    HZ_DOT_32,
    HZ_DOT_NUM
} CarDV_FontDot_e;

typedef struct CarDV_Font_s
{
    MI_U32 nFontSize;
    MI_U8* pData;
    MI_U8  u8FontType;
} CarDV_Font_t;

typedef struct CarDV_FontData_s
{
    MI_RGN_PixelFormat_e pmt;
    MI_U16               stride;
    MI_U16               height;
    MI_U8*               buffer;
    MI_U16               offset;    // next font offset, unit : pixel of font size
    MI_U16               fontwidth; // next font offset, unit : pixel of OSD buffer
} CarDV_FontData_t;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int    cardv_Font_Exit();
MI_U32 cardv_Font_Strlen(const MI_U8* phzstr);
MI_U32 cardv_Font_GetSizeByType(CarDV_FontSize_e eFontSize);
MI_S32 cardv_Font_DrawText(CarDV_FontData_t* pFontBuffer, const MI_U8* pStr, MI_U32 idx_start,
                           CarDV_FontSize_e eFontSize, Color_t* pfColor, Color_t* pbColor, MI_U16 u16ColorIdx,
                           BOOL bOutline);

#endif //#define _MODULE_FONT_H_
