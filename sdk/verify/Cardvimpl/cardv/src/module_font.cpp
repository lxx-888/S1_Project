/*
 * module_font.cpp- Sigmastar
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

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "module_common.h"
#include "module_font.h"
#include "miniz_tinfl.h"

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define FONT_TYPE_ASCII  (0)
#define FONT_TYPE_GB2312 (1)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

static CarDV_Font_t*   gFontInfo[HZ_DOT_NUM]    = {NULL};
static pthread_mutex_t g_font_mutex[HZ_DOT_NUM] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
                                                   PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
                                                   PTHREAD_MUTEX_INITIALIZER};

//==============================================================================
//
//                              LOCAL FUNCTIONS
//
//==============================================================================

static void cardv_Font_Open(CarDV_FontSize_e eFontSize)
{
    MI_U8 *data_file, *data_bin;
    MI_U32 bin_size = 0;
    FILE*  fd_zk;
    char   hz_path[64] = {
        0,
    };
    struct stat     hz_stat;
    MI_U32          font_size  = 0;
    MI_U8           u8FontType = FONT_TYPE_ASCII;
    CarDV_Font_t*   pFont;
    CarDV_FontDot_e hz_dot;

    if (FONT_SIZE_8 == eFontSize)
        hz_dot = HZ_DOT_8;
    else if (FONT_SIZE_12 == eFontSize)
        hz_dot = HZ_DOT_12;
    else if (FONT_SIZE_16 == eFontSize)
        hz_dot = HZ_DOT_16;
    else if ((FONT_SIZE_24 == eFontSize) || (FONT_SIZE_48 == eFontSize) || (FONT_SIZE_72 == eFontSize))
        hz_dot = HZ_DOT_24;
    else if ((FONT_SIZE_32 == eFontSize) || (FONT_SIZE_64 == eFontSize) || (FONT_SIZE_96 == eFontSize))
        hz_dot = HZ_DOT_32;
    else
    {
        CARDV_ERR("font %d exit!\n", eFontSize);
        return;
    }

    pthread_mutex_lock(&g_font_mutex[hz_dot]);

    if (gFontInfo[hz_dot])
    {
        pthread_mutex_unlock(&g_font_mutex[hz_dot]);
        return;
    }

    if (FONT_SIZE_8 == eFontSize)
    {
        sprintf(hz_path, "%s/font/HZ8.mz", CARDV_FONT_PATH);
        bin_size   = HZ_8P_BIN_SIZE;
        u8FontType = FONT_TYPE_GB2312;
        font_size  = 8;
    }
    else if (FONT_SIZE_12 == eFontSize)
    {
        sprintf(hz_path, "%s/font/HZ12.mz", CARDV_FONT_PATH);
        bin_size   = HZ_12P_BIN_SIZE;
        u8FontType = FONT_TYPE_GB2312;
        font_size  = 12;
    }
    else if (FONT_SIZE_16 == eFontSize)
    {
        sprintf(hz_path, "%s/font/HZ16.mz", CARDV_FONT_PATH);
        bin_size   = HZ_16P_BIN_SIZE;
        u8FontType = FONT_TYPE_GB2312;
        font_size  = 16;
    }
    else if ((FONT_SIZE_24 == eFontSize) || (FONT_SIZE_48 == eFontSize) || (FONT_SIZE_72 == eFontSize))
    {
        sprintf(hz_path, "%s/font/%s", CARDV_FONT_PATH, CARDV_OSD_FONT_24_BIN);
        if (strstr(CARDV_OSD_FONT_24_BIN, "hanzi") != NULL)
        {
            bin_size   = HZ_24_BIN_SIZE;
            u8FontType = FONT_TYPE_GB2312;
        }
        else
        {
            bin_size   = ASCII_24_BIN_SIZE;
            u8FontType = FONT_TYPE_ASCII;
        }
        font_size = 24;
    }
    else if ((FONT_SIZE_32 == eFontSize) || (FONT_SIZE_64 == eFontSize) || (FONT_SIZE_96 == eFontSize))
    {
        sprintf(hz_path, "%s/font/%s", CARDV_FONT_PATH, CARDV_OSD_FONT_32_BIN);
        if (strstr(CARDV_OSD_FONT_32_BIN, "hanzi") != NULL)
        {
            bin_size   = HZ_32_BIN_SIZE;
            u8FontType = FONT_TYPE_GB2312;
        }
        else
        {
            bin_size   = ASCII_32_BIN_SIZE;
            u8FontType = FONT_TYPE_ASCII;
        }
        font_size = 32;
    }

    if (stat(hz_path, &hz_stat) < 0)
    {
        pthread_mutex_unlock(&g_font_mutex[hz_dot]);
        CARDV_ERR("%s %s file not exit !\n", __FUNCTION__, hz_path);
        return;
    }

    data_file = (MI_U8*)MALLOC(hz_stat.st_size);
    fd_zk     = fopen(hz_path, "rb");

    fread(data_file, hz_stat.st_size, 1, fd_zk);
    fclose(fd_zk);

    data_bin = (MI_U8*)MALLOC(bin_size);

    if (bin_size
        != tinfl_decompress_mem_to_mem(data_bin, bin_size, data_file, hz_stat.st_size, TINFL_FLAG_PARSE_ZLIB_HEADER))
    {
        FREEIF(data_bin);
        FREEIF(data_file);
        pthread_mutex_unlock(&g_font_mutex[hz_dot]);
        CARDV_ERR("decompress err!\n");
        return;
    }

    FREEIF(data_file);
    pFont             = (CarDV_Font_t*)MALLOC(sizeof(CarDV_Font_t));
    pFont->nFontSize  = font_size;
    pFont->pData      = data_bin;
    pFont->u8FontType = u8FontType;
    gFontInfo[hz_dot] = pFont;

    pthread_mutex_unlock(&g_font_mutex[hz_dot]);
}

static void cardv_Font_Close(CarDV_Font_t* pFont)
{
    FREEIF(pFont->pData);
    FREEIF(pFont);
}

static CarDV_Font_t* cardv_Font_GetHandle(CarDV_FontSize_e eFontSize, MI_U32* pMultiple)
{
    cardv_Font_Open(eFontSize);

    switch (eFontSize)
    {
    case FONT_SIZE_8:
        *pMultiple = 1; // Original output, must set "1"
        return gFontInfo[HZ_DOT_8];

    case FONT_SIZE_12:
        *pMultiple = 1; // Original output, must set "1"
        return gFontInfo[HZ_DOT_12];

    case FONT_SIZE_16:
        *pMultiple = 1; // Original output, must set "1"
        return gFontInfo[HZ_DOT_16];

    case FONT_SIZE_24:
        *pMultiple = 1;
        return gFontInfo[HZ_DOT_24];

    case FONT_SIZE_32:
        *pMultiple = 1;
        return gFontInfo[HZ_DOT_32];

    case FONT_SIZE_48:
        *pMultiple = 2;
        return gFontInfo[HZ_DOT_24];

    case FONT_SIZE_64:
        *pMultiple = 2;
        return gFontInfo[HZ_DOT_32];

    case FONT_SIZE_72:
        *pMultiple = 3;
        return gFontInfo[HZ_DOT_24];

    case FONT_SIZE_96:
        *pMultiple = 3;
        return gFontInfo[HZ_DOT_32];

    default:
        return NULL;
    }
}

static MI_U32 cardv_Font_GetCharQW(const MI_U8* str, MI_U32* p_qu, MI_U32* p_wei)
{
    int          qu, wei;
    int          offset  = 0;
    const MI_U8* pString = (const MI_U8*)str;

    if (*pString >= 128)
    {
        qu = *pString - 160;
        pString++;

        if (*pString >= 128)
        {
            wei = *pString - 160;

            offset = 2;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        qu     = 3;
        wei    = *pString - 32;
        offset = 1;
    }

    *p_qu  = qu;
    *p_wei = wei;

    return offset;
}

static void cardv_Font_BmpDilate(MI_U16* pbuffer, MI_U32 width, MI_U32 height, MI_U32 stride, MI_U16 rgbColor)
{
    MI_U32  i, j;
    MI_U16* pRGBbuffer;
    MI_U16  strokeColor = ((65535 - rgbColor) | 0xF);

    pRGBbuffer = (MI_U16*)pbuffer + stride;

    for (i = 1; i < height - 1; i++)
    {
        for (j = 1; j < width - 1; j++)
        {
            if ((pRGBbuffer[j] & 0xF) > 1)
            {
                // printf("%d %d           ",i,j);
                if ((pRGBbuffer[j - stride] & 0xF) == 0)
                {
                    pRGBbuffer[j - stride] = 1;
                    // printf("%d %d   ",i-1,j);
                }

                if ((pRGBbuffer[j - 1] & 0xF) == 0)
                {
                    pRGBbuffer[j - 1] = 1;
                    // printf("%d %d   ",i,j-1);
                }

                if ((pRGBbuffer[j + stride] & 0xF) == 0)
                {
                    pRGBbuffer[j + stride] = 1;
                    // printf("%d %d   ",i+1,j);
                }

                if ((pRGBbuffer[j + 1] & 0xF) == 0)
                {
                    pRGBbuffer[j + 1] = 1;
                    // printf("%d %d",i,j+1);
                }

                // printf("\n");
            }
            else
            {
                // printf(" ");
            }
        }

        pRGBbuffer += stride;
        // printf("\n");
    }

    pRGBbuffer = (MI_U16*)pbuffer + stride;

    for (i = 1; i < height - 1; i++)
    {
        for (j = 1; j < width - 1; j++)
        {
            if (pRGBbuffer[j] == 1)
            {
                pRGBbuffer[j] = strokeColor;
                // printf("*");
            }
            else if (pRGBbuffer[j] > 1)
            {
                // printf("#");
            }
            else
            {
                // printf("-");
            }
        }

        pRGBbuffer += stride;
        // printf("\n");
    }
}

static MI_S32 cardv_Font_DrawRGB1555(CarDV_Font_t* pFont, MI_U32 qu, MI_U32 wei, MI_U32 nMultiple,
                                     CarDV_FontData_t* pFontBuffer, Color_t* pfColor, Color_t* pbColor, BOOL bOutline)
{
    MI_U32  nCharWidth  = (pFont->nFontSize + 7) / 8;
    MI_U32  nFontWidth  = 0;
    MI_U8*  pFontBitmap = pFont->pData + ((qu - 1) * 94 + wei - 1) * nCharWidth * pFont->nFontSize;
    MI_U32  i, j, ii, jj;
    MI_U8*  pBuffer = (MI_U8*)pFontBuffer->buffer + pFontBuffer->offset * 2;
    MI_U32  bDot;
    MI_U16* _pBuffer = (MI_U16*)pBuffer;
    MI_U16* __pBuffer;
    MI_U16* ___pBuffer;
    MI_U16  r, g, b, a;
    MI_U16  fcolor1555 = 0;
    MI_U16  bcolor1555 = 0;

    r          = pfColor->r;
    g          = pfColor->g;
    b          = pfColor->b;
    a          = pfColor->a;
    fcolor1555 = ((a & 0x80) << 8) | ((r & 0xF8) << 7) | ((g & 0xF8) >> 2) | ((b & 0xF8) >> 3);

    r = pbColor->r;
    g = pbColor->g;
    b = pbColor->b;
    a = pbColor->a;
    if ((a & 0x80))
    {
        bcolor1555 = 0x2323;
    }
    else
    {
        bcolor1555 = ((a & 0x80) << 8) | ((r & 0xF8) << 7) | ((g & 0xF8) >> 2) | ((b & 0xF8) >> 3);
    }

    // printf("=======> _pBuffer = %p fcolor1555=%x  bcolor1555=%x \n", _pBuffer, fcolor1555, bcolor1555);

    for (j = 0; j < pFont->nFontSize; j++)
    {
        __pBuffer = _pBuffer;

        for (i = 0; i < pFont->nFontSize; i++)
        {
            bDot = *(pFontBitmap + (i >> 3)) & (0x80 >> (i & 0x07));
            if (bDot)
            {
                if (i + 1 > nFontWidth)
                    nFontWidth = i + 1;
            }

            ___pBuffer = __pBuffer;

            for (jj = 0; jj < nMultiple; jj++)
            {
                for (ii = 0; ii < nMultiple; ii++)
                {
                    ___pBuffer[ii] = bDot ? fcolor1555 : bcolor1555;
                }

                ___pBuffer += pFontBuffer->stride;
            }

            __pBuffer += nMultiple;
        }

        _pBuffer += nMultiple * pFontBuffer->stride;
        pFontBitmap += nCharWidth;
    }

    // Update next font buffer offset
    pFontBuffer->offset += (nFontWidth ? nFontWidth + 2 : pFont->nFontSize);
    pFontBuffer->fontwidth = pFontBuffer->offset * nMultiple;

    if (bOutline)
    {
        cardv_Font_BmpDilate((MI_U16*)pBuffer, nMultiple * pFont->nFontSize, nMultiple * pFont->nFontSize,
                             pFontBuffer->stride, fcolor1555);
    }

    return MI_SUCCESS;
}

static MI_S32 cardv_Font_DrawRGB1555_ASCII(CarDV_Font_t* pFont, MI_U8 nChar, MI_U32 nMultiple,
                                           CarDV_FontData_t* pFontBuffer, Color_t* pfColor, Color_t* pbColor,
                                           BOOL bOutline)
{
    MI_U32  nCharWidth  = (pFont->nFontSize + 7) / 8;
    MI_U32  nFontWidth  = 0;
    MI_U8*  pFontBitmap = pFont->pData + nChar * nCharWidth * pFont->nFontSize;
    MI_U32  i, j, ii, jj;
    MI_U8*  pBuffer = (MI_U8*)pFontBuffer->buffer + pFontBuffer->offset * 2;
    MI_U32  bDot;
    MI_U16* _pBuffer = (MI_U16*)pBuffer;
    MI_U16* __pBuffer;
    MI_U16* ___pBuffer;
    MI_U16  r, g, b, a;
    MI_U16  fcolor1555 = 0;
    MI_U16  bcolor1555 = 0;

    r          = pfColor->r;
    g          = pfColor->g;
    b          = pfColor->b;
    a          = pfColor->a;
    fcolor1555 = ((a & 0x80) << 8) | ((r & 0xF8) << 7) | ((g & 0xF8) << 2) | ((b & 0xF8) >> 3);

    r = pbColor->r;
    g = pbColor->g;
    b = pbColor->b;
    a = pbColor->a;

    bcolor1555 = ((a & 0x80) << 8) | ((r & 0xF8) << 7) | ((g & 0xF8) << 2) | ((b & 0xF8) >> 3);

    // printf("=======> _pBuffer = %p fcolor1555=%x  bcolor1555=%x \n", _pBuffer, fcolor1555, bcolor1555);

    for (j = 0; j < pFont->nFontSize; j++) // 32 bit column
    {
        __pBuffer = _pBuffer;

        for (i = 0; i < pFont->nFontSize; i++) // 32 bit row
        {
            bDot = *(pFontBitmap + (i >> 3)) & (0x80 >> (i & 0x07)); // i>>3 (i/8)find byte ,i && 0x07 (i%8)low 3rd bit
            if (bDot)
            {
                if (i + 1 > nFontWidth)
                    nFontWidth = i + 1;
            }

            ___pBuffer = __pBuffer;

            for (jj = 0; jj < nMultiple; jj++)
            {
                for (ii = 0; ii < nMultiple; ii++)
                {
                    ___pBuffer[ii] = bDot ? fcolor1555 : bcolor1555;
                }

                ___pBuffer += pFontBuffer->stride / 2; // no meaning!
            }
            __pBuffer += nMultiple; // next point
        }

        _pBuffer += nMultiple * pFontBuffer->stride / 2; // pFontBuffer->stride：128
        pFontBitmap += nCharWidth;                       // next
    }

    // Update next font buffer offset
    pFontBuffer->offset += (nFontWidth ? nFontWidth + 2 : pFont->nFontSize);
    pFontBuffer->fontwidth = pFontBuffer->offset * nMultiple;

    if (bOutline)
    {
        cardv_Font_BmpDilate((MI_U16*)pBuffer, nMultiple * pFont->nFontSize, nMultiple * pFont->nFontSize,
                             pFontBuffer->stride, fcolor1555);
    }

    return MI_SUCCESS;
}

static MI_S32 cardv_Font_DrawPaletteTable_I4(CarDV_Font_t* pFont, MI_U32 qu, MI_U32 wei, MI_U32 nMultiple,
                                             CarDV_FontData_t* pFontBuffer, MI_U16 u16ColorIdx, BOOL bOutline)
{
    MI_U32 nCharWidth  = (pFont->nFontSize + 7) / 8;
    MI_U32 nFontWidth  = 0;
    MI_U8* pFontBitmap = pFont->pData + ((qu - 1) * 94 + wei - 1) * nCharWidth * pFont->nFontSize;
    MI_U32 i = 0, j = 0, ii = 0, jj = 0;
    MI_U8* pBuffer = (MI_U8*)pFontBuffer->buffer + (pFontBuffer->offset * nMultiple) / 2;
    MI_U32 bDot;
    MI_U8* _pBuffer = (MI_U8*)pBuffer;
    MI_U8* __pBuffer;
    MI_U8* ___pBuffer;
    MI_U16 fcolorI4 = 0;
    MI_U16 bcolorI4 = 0;

    fcolorI4 = u16ColorIdx & 0xF;
    bcolorI4 = 0; // index = 0, defalut transparency

    for (j = 0; j < pFont->nFontSize; j++)
    {
        __pBuffer = _pBuffer;

        for (i = 0; i < pFont->nFontSize; i++)
        {
            bDot = *(pFontBitmap + (i >> 3)) & (0x80 >> (i & 0x07));
            if (bDot)
            {
                if (i + 1 > nFontWidth)
                    nFontWidth = i + 1;
            }

            if (1 == nMultiple)
            {
                ___pBuffer = __pBuffer;

                if (bDot)
                {
                    // If it is an even position, it is stored in a lower 4bit position of one byte.
                    // elif it is an odd position, it is stored in the high 4bit position of one byte.
                    if (0 == (i % 2))
                    {
                        ___pBuffer[i / 2] = ((bcolorI4 & 0x0F) << 4) | (fcolorI4 & 0x0F);
                    }
                    else if ((0 < i) && (0 != (i % 2)))
                    {
                        ___pBuffer[(i - 1) / 2] |= (fcolorI4 & 0x0F) << 4;
                    }
                }
            }
            else
            {
                ___pBuffer = __pBuffer;
                // Set the starting coordinate value of the color index value of the current point
                ___pBuffer += (nMultiple / 2) * i;

                for (jj = 0; jj < nMultiple; jj++) // Each column index value of a point needs to be processed
                                                   // separately
                {
                    ___pBuffer = __pBuffer;
                    ___pBuffer += (nMultiple / 2) * i;
                    ___pBuffer += pFontBuffer->stride * jj; // Start processing the columns of the enlarged font

                    for (ii = 0; ii < nMultiple / 2; ii++) // nMultiple = 2, 4, 6, 8 ...
                    {
                        if (bDot)
                        {
                            // One byte describing the color index value of a repeated point
                            ___pBuffer[ii] = ((fcolorI4 & 0x0F) << 4) | (fcolorI4 & 0x0F);
                        }
                        else
                        {
                            // One byte describing the color index value of a repeated point
                            ___pBuffer[ii] = ((bcolorI4 & 0x0F) << 4) | (bcolorI4 & 0x0F);
                        }
                    }
                }
            }
        }

        // Start processing the next line of the font
        _pBuffer += pFontBuffer->stride * nMultiple;
        pFontBitmap += nCharWidth;
    }

    // Update next font buffer offset
    pFontBuffer->offset += (nFontWidth ? nFontWidth + 2 : pFont->nFontSize);
    pFontBuffer->fontwidth = pFontBuffer->offset * nMultiple;

    if (bOutline)
    {
        // cardv_Font_BmpDilate((MI_U16*)pBuffer, nMultiple * pFont->nFontSize, nMultiple * pFont->nFontSize,
        // pFontBuffer->stride, fcolorI4);
    }

    return MI_SUCCESS;
}

static MI_S32 cardv_Font_DrawPaletteTable_I4_ASCII(CarDV_Font_t* pFont, MI_U8 nChar, MI_U32 nMultiple,
                                                   CarDV_FontData_t* pFontBuffer, MI_U16 u16ColorIdx, BOOL bOutline)
{
    MI_U32 nCharWidth  = (pFont->nFontSize + 7) / 8;
    MI_U32 nFontWidth  = 0;
    MI_U8* pFontBitmap = pFont->pData + nChar * nCharWidth * pFont->nFontSize;
    MI_U32 i = 0, j = 0, ii = 0, jj = 0;
    MI_U8* pBuffer = (MI_U8*)pFontBuffer->buffer + (pFontBuffer->offset * nMultiple) / 2;
    MI_U32 bDot;
    MI_U8* _pBuffer = (MI_U8*)pBuffer;
    MI_U8* __pBuffer;
    MI_U8* ___pBuffer;
    MI_U16 fcolorI4 = 0;
    MI_U16 bcolorI4 = 0;

    fcolorI4 = u16ColorIdx & 0xF;
    bcolorI4 = 0; // index = 0, defalut transparency

    for (j = 0; j < pFont->nFontSize; j++)
    {
        __pBuffer = _pBuffer;

        for (i = 0; i < pFont->nFontSize; i++)
        {
            bDot = *(pFontBitmap + (i >> 3)) & (0x80 >> (i & 0x07));
            if (bDot)
            {
                if (i + 1 > nFontWidth)
                    nFontWidth = i + 1;
            }

            if (1 == nMultiple)
            {
                ___pBuffer = __pBuffer;

                if (bDot)
                {
                    // If it is an even position, it is stored in a lower 4bit position of one byte.
                    // elif it is an odd position, it is stored in the high 4bit position of one byte.
                    if (0 == (i % 2))
                    {
                        ___pBuffer[i / 2] = ((bcolorI4 & 0x0F) << 4) | (fcolorI4 & 0x0F);
                    }
                    else if ((0 < i) && (0 != (i % 2)))
                    {
                        ___pBuffer[(i - 1) / 2] |= (fcolorI4 & 0x0F) << 4;
                    }
                }
            }
            else
            {
                ___pBuffer = __pBuffer;
                // Set the starting coordinate value of the color index value of the current point
                ___pBuffer += (nMultiple / 2) * i;

                for (jj = 0; jj < nMultiple; jj++) // Each column index value of a point needs to be processed
                                                   // separately
                {
                    ___pBuffer = __pBuffer;
                    ___pBuffer += (nMultiple / 2) * i;
                    ___pBuffer += pFontBuffer->stride * jj; // Start processing the columns of the enlarged font

                    for (ii = 0; ii < nMultiple / 2; ii++) // nMultiple = 2, 4, 6, 8 ...
                    {
                        if (bDot)
                        {
                            // One byte describing the color index value of a repeated point
                            ___pBuffer[ii] = ((fcolorI4 & 0x0F) << 4) | (fcolorI4 & 0x0F);
                        }
                        else
                        {
                            // One byte describing the color index value of a repeated point
                            ___pBuffer[ii] = ((bcolorI4 & 0x0F) << 4) | (bcolorI4 & 0x0F);
                        }
                    }
                }
            }
        }

        // Start processing the next line of the font
        _pBuffer += pFontBuffer->stride * nMultiple;
        pFontBitmap += nCharWidth;
    }

    // Update next font buffer offset
    pFontBuffer->offset += (nFontWidth ? nFontWidth + 2 : pFont->nFontSize);
    pFontBuffer->fontwidth = pFontBuffer->offset * nMultiple;

    if (bOutline)
    {
        // cardv_Font_BmpDilate((MI_U16*)pBuffer, nMultiple * pFont->nFontSize, nMultiple * pFont->nFontSize,
        // pFontBuffer->stride, fcolorI4);
    }
    return MI_SUCCESS;
}

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int cardv_Font_Exit()
{
    for (U32 i = 0; i < HZ_DOT_NUM; i++)
    {
        if (gFontInfo[i])
        {
            cardv_Font_Close(gFontInfo[i]);
            gFontInfo[i] = NULL;
        }
    }
    return MI_SUCCESS;
}

MI_U32 cardv_Font_Strlen(const MI_U8* phzstr)
{
    MI_U8  count = 0;
    MI_U8* str;
    MI_U32 len;

    len = strlen((char*)phzstr);
    str = (MI_U8*)phzstr;

    while (len > 0)
    {
        if ((*str) < 0x80)
        {
            count++;
            str += 1;
            len -= 1;
        }
        else
        {
            count++;
            str += 2;
            len -= 2;
        }
    }

    return count;
}

MI_U32 cardv_Font_GetSizeByType(CarDV_FontSize_e eFontSize)
{
    switch (eFontSize)
    {
    case FONT_SIZE_8:
        return 8;

    case FONT_SIZE_12:
        return 12;

    case FONT_SIZE_16:
        return 16;

    case FONT_SIZE_24:
        return 24;

    case FONT_SIZE_32:
        return 32;

    case FONT_SIZE_48:
        return 48;

    case FONT_SIZE_64:
        return 64;

    case FONT_SIZE_72:
        return 72;

    case FONT_SIZE_96:
        return 96;

    default:
        printf("%s:%d can not get the font size of: %d", __func__, __LINE__, eFontSize);
        return 0;
    }
}

MI_S32 cardv_Font_DrawText(CarDV_FontData_t* pFontBuffer, const MI_U8* pStr, MI_U32 idx_start,
                           CarDV_FontSize_e eFontSize, Color_t* pfColor, Color_t* pbColor, MI_U16 u16ColorIdx,
                           BOOL bOutline)
{
    MI_U32        hzLen = cardv_Font_Strlen(pStr);
    MI_U32        idx;
    CarDV_Font_t* pFont;
    MI_U32        nMultiple = 0;
    MI_U32        qu = 0, wei = 0, offset = 0;

    if (NULL == (pFont = cardv_Font_GetHandle(eFontSize, &nMultiple)))
    {
        printf("%s:%d cannot get the Font handle and return failed!\n", __func__, __LINE__);
        return E_MI_ERR_FAILED;
    }

    for (idx = 0; idx < hzLen; idx++)
    {
        if (pFont->u8FontType == FONT_TYPE_GB2312)
            offset = cardv_Font_GetCharQW(pStr, &qu, &wei);
        else
            offset = 1;

        switch (pFontBuffer->pmt)
        {
        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:
            if (pFont->u8FontType == FONT_TYPE_GB2312)
                cardv_Font_DrawRGB1555(pFont, qu, wei, nMultiple, pFontBuffer, pfColor, pbColor, bOutline);
            else
                cardv_Font_DrawRGB1555_ASCII(pFont, (MI_U8)*pStr, nMultiple, pFontBuffer, pfColor, pbColor, bOutline);
            break;

        case E_MI_RGN_PIXEL_FORMAT_I4:
            if (pFont->u8FontType == FONT_TYPE_GB2312)
                cardv_Font_DrawPaletteTable_I4(pFont, qu, wei, nMultiple, pFontBuffer, u16ColorIdx, bOutline);
            else
                cardv_Font_DrawPaletteTable_I4_ASCII(pFont, (MI_U8)*pStr, nMultiple, pFontBuffer, u16ColorIdx,
                                                     bOutline);
            break;

        default:
            CARDV_ERR("only support argb1555 or I4\n");
            break;
        }
        pStr += offset;
    }

    return MI_SUCCESS;
}
