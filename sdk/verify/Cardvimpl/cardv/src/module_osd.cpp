/* module_osd.cpp- Sigmastar
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

#include <time.h>

#include "module_common.h"
#include "module_gps.h"
#include "module_font.h"
#include "module_osd.h"
#include "module_scl.h"
#include "module_video.h"
#include "mid_common.h"
#include "mid_osd.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================
#define STAMP_LEFT_UP 100
#define RGN_MAX_LIMIT (4096)

CarDV_OsdAttr gstOsdAttr = {0};

static BOOL      g_exit;
static pthread_t pthreadOSD;
static int       g_osdRefreshIntervalMS = DEFAULT_OSD_REFRESH_INTERVAL;
static int       g_osd_init_done        = 0;

static pthread_mutex_t mutexCarDVOsdTxtRgnBuf;
static MI_U8 *         g_pbuffer = NULL;

int DatelogoStampMode = STAMP_MODE_DATELOGO; // 0-DATELOGO, 1-DATE, 2-LOGO, 3-OFF
int DateFormat        = DATE_FORMAT_YMD;

MI_RGN_PaletteTable_t g_stPaletteTable = {{
// index0 ~ index15
#if 0
    {0, 0, 0, 0},
    {255, 255, 0, 0},
    {255, 0, 255, 0},
    {255, 0, 0, 255},
    {255, 255, 255, 0},
    {255, 0, 112, 255},
    {255, 0, 255, 255},
    {255, 255, 255, 255},
    {255, 128, 0, 0},
    {255, 128, 128, 0},
    {255, 128, 0, 128},
    {255, 0, 128, 0},
    {255, 0, 0, 128},
    {255, 0, 128, 128},
    {255, 128, 128, 128},
    {255, 64, 64, 64}
#else
    {0, 0, 0, 0},
    {255, 80, 0, 0},
    {255, 0, 80, 0},
    {255, 80, 80, 0},
    {255, 0, 0, 80},
    {255, 80, 0, 80},
    {255, 0, 80, 80},
    {255, 80, 80, 80},
    {255, 192, 192, 192},
    {255, 255, 0, 0},
    {255, 0, 255, 0},
    {255, 255, 255, 0},
    {255, 0, 0, 255},
    {255, 255, 0, 255},
    {255, 0, 162, 232},
    {255, 255, 255, 255}
#endif
}};

Color_t blackColor  = {255, 0, 0, 0};
Color_t redColor    = {255, 255, 0, 0};
Color_t silverColor = {255, 192, 192, 192};
Color_t whiteColor  = {255, 255, 255, 255};

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern MI_S32           g_s32SocId;
extern MI_VideoEncoder *g_videoEncoderArray[];

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

MI_RGN_HANDLE OSD_GetHandleByVideoSize(U32 u32VideoWidth, U32 u32VideoHeight)
{
    CarDV_OsdBufferAttr *pstBufferAttr;
    MI_U32               u32Osd;

    if (u32VideoWidth > 2560)
        u32Osd = (2 < CARDV_OSD_BUFFER_NUM ? 2 : CARDV_OSD_BUFFER_NUM - 1);
    else if (u32VideoWidth > 1920)
        u32Osd = (1 < CARDV_OSD_BUFFER_NUM ? 1 : CARDV_OSD_BUFFER_NUM - 1);
    else
        u32Osd = 0;

    pstBufferAttr = &gstOsdAttr.stBufferAttr[u32Osd];
    return pstBufferAttr->u32RgnHandle;
}

void OSD_CalcVideoStampPosition(U32 u32VideoWidth, U32 u32VideoHeight, U32 u32OsdWidth, U32 u32OsdHeight, U32 *pu32X,
                                U32 *pu32Y)
{
    if (u32VideoWidth > u32OsdWidth)
    {
        *pu32X = (u32VideoWidth - u32OsdWidth) / 2;
    }
    else
    {
        *pu32X = 0;
    }

    if (u32VideoHeight > u32OsdHeight + 10)
    {
        *pu32Y = u32VideoHeight - u32OsdHeight - 10;
    }
    else
    {
        *pu32Y = 0;
    }

    if (u32VideoWidth > RGN_MAX_LIMIT || u32VideoHeight > RGN_MAX_LIMIT)
    {
        *pu32X = *pu32Y = STAMP_LEFT_UP;
    }
}

void OSD_DrawVideoStamp(CarDV_OsdBufferAttr *pstBufferAttr, MI_RGN_CanvasInfo_t *pstRgnCanvasInfo)
{
    char             u8StringTime[64];
    char             u8StringGps[64];
    time_t           t;
    struct tm *      local;
    GPSINFOCHUCK *   pstGpsChuck = NULL;
    TextWidgetAttr_t stTextWidgetAttr;
    DrawPoint_t      stDrawPt;
    Rect_t           stRect;

    if (DatelogoStampMode == STAMP_MODE_DATELOGO || DatelogoStampMode == STAMP_MODE_DATE)
    {
        t     = time(NULL);
        local = localtime(&t);
        if (DateFormat == DATE_FORMAT_YMD)
        {
            sprintf(u8StringTime, "%d-%02d-%02d %02d:%02d:%02d", local->tm_year + 1900, local->tm_mon + 1,
                    local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
        }
        else if (DateFormat == DATE_FORMAT_MDY)
        {
            sprintf(u8StringTime, "%02d-%02d-%d %02d:%02d:%02d", local->tm_mon + 1, local->tm_mday,
                    local->tm_year + 1900, local->tm_hour, local->tm_min, local->tm_sec);
        }
        else if (DateFormat == DATE_FORMAT_DMY)
        {
            sprintf(u8StringTime, "%02d-%02d-%d %02d:%02d:%02d", local->tm_mday, local->tm_mon + 1,
                    local->tm_year + 1900, local->tm_hour, local->tm_min, local->tm_sec);
        }
        else if (DateFormat == DATE_FORMAT_OFF)
        {
            u8StringTime[0] = '\0';
        }
    }

    pstGpsChuck = (GPSINFOCHUCK *)GpsGetChunk(NULL);
    if (pstGpsChuck && pstGpsChuck->ubFlag == TRUE)
        sprintf(u8StringGps, "%010.6lf%c %010.6lf%c %dkm/h",
                (pstGpsChuck->dwLat > 0 ? pstGpsChuck->dwLat / 100 : -pstGpsChuck->dwLat / 100),
                (pstGpsChuck->dwLat > 0 ? 'E' : 'W'),
                (pstGpsChuck->dwLon > 0 ? pstGpsChuck->dwLon / 100 : -pstGpsChuck->dwLon / 100),
                (pstGpsChuck->dwLon > 0 ? 'N' : 'S'), pstGpsChuck->usSpeed);

    if (DatelogoStampMode == STAMP_MODE_DATELOGO || DatelogoStampMode == STAMP_MODE_LOGO)
    {
        if (pstBufferAttr->bDrawStampLogo == FALSE)
        {
            stDrawPt.u16X = 0;
            stDrawPt.u16Y = 0;
            DrawBmpToI4((void *)pstRgnCanvasInfo->virtAddr, pstRgnCanvasInfo->u32Stride, OSD_PATH, stDrawPt);
            pstBufferAttr->bDrawStampLogo = TRUE;
        }
        stRect.x      = pstBufferAttr->stPointTime.x;
        stRect.y      = pstBufferAttr->stPointTime.y;
        stRect.width  = pstRgnCanvasInfo->stSize.u32Width - pstBufferAttr->stPointTime.x;
        stRect.height = pstRgnCanvasInfo->stSize.u32Height - pstBufferAttr->stPointTime.y;
        mid_OSD_CleanWidget_Rgn(pstRgnCanvasInfo, &stRect, CARDV_RGN_PALETTEL_TABLE_ALPHA_INDEX);
    }
    else
    {
        mid_OSD_CleanWidget_Rgn(pstRgnCanvasInfo, NULL, CARDV_RGN_PALETTEL_TABLE_ALPHA_INDEX);
        pstBufferAttr->bDrawStampLogo = FALSE;
    }

    memset(&stTextWidgetAttr, 0x00, sizeof(TextWidgetAttr_t));
    stTextWidgetAttr.string   = u8StringTime;
    stTextWidgetAttr.pPoint   = &pstBufferAttr->stPointTime;
    stTextWidgetAttr.size     = pstBufferAttr->eFontSize;
    stTextWidgetAttr.pmt      = pstBufferAttr->eRgnFormat;
    stTextWidgetAttr.u32Color = 15;          // For I4 format
    stTextWidgetAttr.pfColor  = &whiteColor; // For RGB format
    stTextWidgetAttr.pbColor  = &blackColor; // For RGB format
    stTextWidgetAttr.bOutline = FALSE;
    stTextWidgetAttr.bMirror  = FALSE;
    stTextWidgetAttr.bFlip    = FALSE;
    if (DatelogoStampMode == STAMP_MODE_DATELOGO || DatelogoStampMode == STAMP_MODE_DATE)
    {
        mid_OSD_UpdateTextWidget_Rgn(pstRgnCanvasInfo, &stTextWidgetAttr);
    }

    if (pstGpsChuck && pstGpsChuck->ubFlag == TRUE)
    {
        stTextWidgetAttr.string = u8StringGps;
        stTextWidgetAttr.pPoint = &pstBufferAttr->stPointGps;
        stTextWidgetAttr.size   = pstBufferAttr->eFontSize;
        mid_OSD_UpdateTextWidget_Rgn(pstRgnCanvasInfo, &stTextWidgetAttr);
    }
}

void *OSD_Task(void *argu)
{
    int                  i;
    MI_S32               s32Ret = E_MI_ERR_FAILED;
    MI_RGN_CanvasInfo_t  stRgnCanvasInfo;
    CarDV_OsdBufferAttr *pstBufferAttr;

    CARDV_THREAD();

    while (g_exit == FALSE)
    {
        for (i = 0; i < CARDV_OSD_BUFFER_NUM; i++)
        {
            pstBufferAttr = &gstOsdAttr.stBufferAttr[i];
            if (pstBufferAttr->bCreate == FALSE || pstBufferAttr->u8AttachCnt == 0)
            {
                continue;
            }

            s32Ret = MI_RGN_GetCanvasInfo(g_s32SocId, pstBufferAttr->u32RgnHandle, &stRgnCanvasInfo);
            if (MI_RGN_OK != s32Ret)
            {
                CARDV_ERR("MI_RGN_GetCanvasInfo error, %X\n", s32Ret);
                pthread_exit(NULL);
            }

            OSD_DrawVideoStamp(pstBufferAttr, &stRgnCanvasInfo);

            s32Ret = MI_RGN_UpdateCanvas(g_s32SocId, pstBufferAttr->u32RgnHandle);
            if (MI_RGN_OK != s32Ret)
            {
                CARDV_ERR("MI_RGN_UpdateCanvas error, %X\n", s32Ret);
            }
        }
        usleep(1000 * g_osdRefreshIntervalMS);
    }

    pthread_exit(NULL);
}

MI_S32 mid_OSD_UpdateTextWidget_Rgn(MI_RGN_CanvasInfo_t *pstRgnCanvasInfo, TextWidgetAttr_t *pstTextWidgetAttr)
{
    static MI_U32    u32BuffSize = 0;
    MI_S32           s32Ret      = E_MI_ERR_FAILED;
    MI_U32           hzLen       = 0;
    MI_U32           nFontSize   = 0;
    CarDV_FontData_t fontData;

    if (NULL == pstRgnCanvasInfo || NULL == pstTextWidgetAttr)
    {
        CARDV_ERR("The RGN pointer is NULL!\n");
        return s32Ret;
    }

    pthread_mutex_lock(&mutexCarDVOsdTxtRgnBuf);

    hzLen = cardv_Font_Strlen((MI_U8 *)pstTextWidgetAttr->string);
    // nFontSize = font width *nMultiple, 8, 12, 16, 24, 32, 48...
    nFontSize = cardv_Font_GetSizeByType(pstTextWidgetAttr->size);

    if (hzLen == 0)
    {
        pthread_mutex_unlock(&mutexCarDVOsdTxtRgnBuf);
        return MI_SUCCESS;
    }

    memset(&fontData, 0x00, sizeof(CarDV_FontData_t));
    fontData.pmt = pstTextWidgetAttr->pmt;
    // Height= font height
    fontData.height = nFontSize;
    if (E_MI_RGN_PIXEL_FORMAT_ARGB1555 == fontData.pmt)
    {
        // width = font width  * number of string * 2
        fontData.stride = nFontSize * hzLen * 2;
    }
    else if (E_MI_RGN_PIXEL_FORMAT_I4 == fontData.pmt)
    {
        // width = font width  * number of string / 2
        fontData.stride = nFontSize * hzLen / 2;
    }

    if (fontData.height != 0)
    {
        MI_U32 u32BuffSize_new = fontData.stride * fontData.height;

        if (NULL == g_pbuffer)
        {
            if (NULL == (g_pbuffer = (MI_U8 *)malloc(u32BuffSize_new)))
            {
                pthread_mutex_unlock(&mutexCarDVOsdTxtRgnBuf);
                return s32Ret;
            }
            u32BuffSize = u32BuffSize_new;
        }
        else if ((NULL != g_pbuffer) && (u32BuffSize < u32BuffSize_new))
        {
            if (NULL == (g_pbuffer = (MI_U8 *)realloc(g_pbuffer, u32BuffSize_new)))
            {
                pthread_mutex_unlock(&mutexCarDVOsdTxtRgnBuf);
                return s32Ret;
            }
            u32BuffSize = u32BuffSize_new;
        }

        fontData.buffer = g_pbuffer;
        memset(fontData.buffer, 0x00, u32BuffSize);
    }

    s32Ret = cardv_Font_DrawText(&fontData, (MI_U8 *)pstTextWidgetAttr->string, pstTextWidgetAttr->pPoint->x,
                                 pstTextWidgetAttr->size, pstTextWidgetAttr->pfColor, pstTextWidgetAttr->pbColor,
                                 pstTextWidgetAttr->u32Color, pstTextWidgetAttr->bOutline);
    if (MI_SUCCESS != s32Ret)
    {
        CARDV_ERR("cardv_Font_DrawText error, %X\n", s32Ret);
        pthread_mutex_unlock(&mutexCarDVOsdTxtRgnBuf);
        return s32Ret;
    }

    switch (fontData.pmt)
    {
    case E_MI_RGN_PIXEL_FORMAT_ARGB1555: {
        MI_U16 *dstBuf, *dstBuf1;
        MI_U16 *srcBuf, *srcBuf1;
        MI_S32  dd = 0;
        Point_t stPoint;

        stPoint.x =
            (pstTextWidgetAttr->pPoint->x % 2) ? (pstTextWidgetAttr->pPoint->x - 1) : pstTextWidgetAttr->pPoint->x;
        stPoint.y =
            (pstTextWidgetAttr->pPoint->y % 2) ? (pstTextWidgetAttr->pPoint->y - 1) : pstTextWidgetAttr->pPoint->y;

        srcBuf = (MI_U16 *)fontData.buffer;
        dstBuf = (MI_U16 *)((pstRgnCanvasInfo->virtAddr) + pstRgnCanvasInfo->u32Stride * stPoint.y) + stPoint.x;

        for (dd = 0; dd < fontData.height; dd++)
        {
            srcBuf1 = srcBuf;
            dstBuf1 = dstBuf;

            for (int i = 0; i < fontData.fontwidth; i++)
            {
                *dstBuf1 = *srcBuf1;

                srcBuf1++;
                dstBuf1++;
            }

            srcBuf += fontData.stride;
            dstBuf += pstRgnCanvasInfo->u32Stride / 2;
        }
    }
    break;

    case E_MI_RGN_PIXEL_FORMAT_I4: {
        MI_U8 * dstBuf, *dstBuf1;
        MI_U8 * srcBuf, *srcBuf1;
        Point_t stPoint;

        stPoint.x =
            (pstTextWidgetAttr->pPoint->x % 2) ? (pstTextWidgetAttr->pPoint->x - 1) : pstTextWidgetAttr->pPoint->x;
        stPoint.y =
            (pstTextWidgetAttr->pPoint->y % 2) ? (pstTextWidgetAttr->pPoint->y - 1) : pstTextWidgetAttr->pPoint->y;

        if (pstTextWidgetAttr->bMirror)
            stPoint.x = pstRgnCanvasInfo->u32Stride * 2 - stPoint.x;

        srcBuf = (MI_U8 *)fontData.buffer;
        dstBuf = (MI_U8 *)((pstRgnCanvasInfo->virtAddr) + pstRgnCanvasInfo->u32Stride * stPoint.y + stPoint.x / 2);

        for (MI_S32 dd = 0; dd < fontData.height; dd++)
        {
            if (pstTextWidgetAttr->bFlip)
                srcBuf1 = srcBuf + fontData.stride * (fontData.height - dd - 1);
            else
                srcBuf1 = srcBuf + fontData.stride * dd;
            dstBuf1 = dstBuf + pstRgnCanvasInfo->u32Stride * dd;

            if (pstTextWidgetAttr->bMirror)
            {
                dstBuf1 += pstRgnCanvasInfo->u32Stride - 1;
                for (int i = 0; i < fontData.fontwidth / 2; i++)
                {
                    *dstBuf1 = (*srcBuf1 << 4) | (*srcBuf1 >> 4);
                    srcBuf1++;
                    dstBuf1--;
                }
            }
            else
                memcpy(dstBuf1, srcBuf1, fontData.fontwidth / 2);
        }
    }
    break;

    default:
        CARDV_ERR("only support %s now\n", (0 == fontData.pmt) ? "ARGB1555" : "I4");
    }

    pthread_mutex_unlock(&mutexCarDVOsdTxtRgnBuf);

    s32Ret = MI_SUCCESS;
    return s32Ret;
}

MI_S32 mid_OSD_CleanWidget_Rgn(MI_RGN_CanvasInfo_t *pstRgnCanvasInfo, Rect_t *pstRect, MI_U32 index)
{
    MI_S32 s32Ret = E_MI_ERR_FAILED;
    Rect_t stRect;

    if (NULL == pstRgnCanvasInfo)
        return s32Ret;

    pthread_mutex_lock(&mutexCarDVOsdTxtRgnBuf);

    if ((NULL == pstRect) || ((0 == pstRect->width) || (0 == pstRect->height)))
    {
        stRect.x      = 0;
        stRect.y      = 0;
        stRect.width  = pstRgnCanvasInfo->stSize.u32Width;
        stRect.height = pstRgnCanvasInfo->stSize.u32Height;
    }
    else
    {
        stRect.x      = pstRect->x;
        stRect.y      = pstRect->y;
        stRect.width  = pstRect->width;
        stRect.height = pstRect->height;
    }

    switch (pstRgnCanvasInfo->ePixelFmt)
    {
    case E_MI_RGN_PIXEL_FORMAT_I4: {
        MI_U32 dd = 0, i = 0;
        MI_U8 *pu8BaseAddr = NULL;

        if (((MI_U32)(stRect.x + stRect.width) <= pstRgnCanvasInfo->stSize.u32Width)
            && ((MI_U32)(stRect.y + stRect.height) <= pstRgnCanvasInfo->stSize.u32Height))
        {
            for (dd = 0; dd < (MI_U32)stRect.height; dd++)
            {
                pu8BaseAddr = (MI_U8 *)(pstRgnCanvasInfo->virtAddr + pstRgnCanvasInfo->u32Stride * (dd + stRect.y)
                                        + stRect.x / 2);

                for (i = 0; i < (MI_U32)stRect.width / 2; i++)
                {
                    *(pu8BaseAddr + i) = ((index & 0x0F) << 4) | (index & 0x0F);
                }
            }
        }
    }
    break;

    case E_MI_RGN_PIXEL_FORMAT_ARGB1555: {
        MI_U32  dd = 0, i = 0;
        MI_U16 *pu16BaseAddr = NULL;

        if (((MI_U32)(stRect.x + stRect.width) <= pstRgnCanvasInfo->stSize.u32Width)
            && ((MI_U32)(stRect.y + stRect.height) <= pstRgnCanvasInfo->stSize.u32Height))
        {
            for (dd = 0; dd < (MI_U32)stRect.height; dd++)
            {
                pu16BaseAddr = (MI_U16 *)(pstRgnCanvasInfo->virtAddr + pstRgnCanvasInfo->u32Stride * (dd + stRect.y)
                                          + stRect.x * 2);

                for (i = 0; i < (MI_U32)stRect.width; i++)
                {
                    *(pu16BaseAddr + i) = index & 0xFFFF;
                }
            }
        }
    }
    break;

    default:
        CARDV_ERR("only support %s now\n", (0 == pstRgnCanvasInfo->ePixelFmt) ? "ARGB1555" : "I4");
        pthread_mutex_unlock(&mutexCarDVOsdTxtRgnBuf);
        return s32Ret;
    }

    s32Ret = MI_RGN_OK;
    pthread_mutex_unlock(&mutexCarDVOsdTxtRgnBuf);

    return s32Ret;
}

S32 cardv_divideSize(S32 value)
{
    if (0 == value % 32)
        return value / 32;
    else if (0 == value % 30)
        return value / 30;
    else if (0 == value % 28)
        return value / 28;
    else if (0 == value % 27)
        return value / 27;
    else if (0 == value % 25)
        return value / 25;
    else if (0 == value % 24)
        return value / 24;
    else if (0 == value % 22)
        return value / 22;
    else if (0 == value % 21)
        return value / 21;
    else if (0 == value % 20)
        return value / 20;
    else if (0 == value % 18)
        return value / 18;
    else if (0 == value % 16)
        return value / 16;
    else if (0 == value % 15)
        return value / 15;
    else if (0 == value % 14)
        return value / 14;
    else if (0 == value % 12)
        return value / 12;
    else if (0 == value % 10)
        return value / 10;
    else if (0 == value % 9)
        return value / 9;
    else if (0 == value % 8)
        return value / 8;
    else if (0 == value % 7)
        return value / 7;
    else if (0 == value % 6)
        return value / 6;
    else if (0 == value % 5)
        return value / 5;
    else if (0 == value % 4)
        return value / 4;
    else if (0 == value % 3)
        return value / 3;
    else if (0 == value % 2)
        return value / 2;
    else
    {
        printf("%s:%d the input value=%d is not support!\n", __func__, __LINE__, value);
        return value;
    }
}

#ifdef DUAL_OS
MI_S32 cardv_setRtosOsdOff(void)
{
    int fd = open("/proc/dualos/rtos", O_WRONLY);
    if (fd >= 0)
    {
        char cmd[64] = "cli l2rdata --timeosdoff";
        write(fd, cmd, 64);
        close(fd);
        return 0;
    }
    return -1;
}
#endif

MI_S32 cardv_setRgnRealtimeFlip(MI_RGN_ChnPort_t *pstRgnChnPort, MI_BOOL bRealtimeFlip)
{
#ifdef DUAL_OS
    // Dual OS not support setting to proc.
    return 0;
#else
    if (pstRgnChnPort == NULL)
        return -1;

    int fd = open("/proc/mi_modules/mi_rgn/mi_rgn0", O_WRONLY);
    if (fd >= 0)
    {
        char    cmd[64]    = {0};
#if defined(CHIP_IFADO) || defined(CHIP_IFORD)
        MI_BOOL bInputPort = FALSE;
        sprintf(cmd, "setRealtimeFlip %d %d %d %d %d %d", pstRgnChnPort->eModId, pstRgnChnPort->s32DevId,
                pstRgnChnPort->s32ChnId, pstRgnChnPort->s32PortId, bInputPort, bRealtimeFlip);
#else
        sprintf(cmd, "setRealtimeFlip %d %d %d %d %d", pstRgnChnPort->eModId, pstRgnChnPort->s32DevId,
                pstRgnChnPort->s32ChnId, pstRgnChnPort->s32PortId, bRealtimeFlip);
#endif
        write(fd, cmd, 64);
        close(fd);
        return 0;
    }
    return -1;
#endif
}

MI_S32 mid_osd_full_init()
{
    MI_U32                u32VideoWidth, u32VideoHeight;
    MI_RGN_Attr_t         stRgnAttr;
    MI_RGN_ChnPortParam_t stChnPortParam;
    CarDV_OsdBufferAttr * pstBufferAttr;
    CarDV_OsdAttachAttr * pstAttachAttr;

    pthread_mutex_init(&mutexCarDVOsdTxtRgnBuf, NULL);

    for (MI_S32 OsdID = 0; OsdID < CARDV_OSD_BUFFER_NUM; OsdID++)
    {
        pstBufferAttr = &gstOsdAttr.stBufferAttr[OsdID];
        if (pstBufferAttr->bCreate == FALSE)
        {
            pstBufferAttr->u32RgnHandle = RGN_OSD_START + OsdID;
            memset(&stRgnAttr, 0x00, sizeof(MI_RGN_Attr_t));
            stRgnAttr.eType                           = E_MI_RGN_TYPE_OSD;
            stRgnAttr.stOsdInitParam.ePixelFmt        = pstBufferAttr->eRgnFormat;
            stRgnAttr.stOsdInitParam.stSize.u32Width  = pstBufferAttr->u32Width;
            stRgnAttr.stOsdInitParam.stSize.u32Height = pstBufferAttr->u32Height;
            CARDVCHECKRESULT_OS(MI_RGN_Create(g_s32SocId, pstBufferAttr->u32RgnHandle, &stRgnAttr));
            pstBufferAttr->bCreate = TRUE;
        }
    }

    for (MI_S32 AttachId = 0; AttachId < CARDV_OSD_MAX_ATTACH_NUM; AttachId++)
    {
        pstAttachAttr = &gstOsdAttr.stAttachAttr[AttachId];
        if (pstAttachAttr->bUsed == TRUE)
        {
            if (pstAttachAttr->stRgnChnPort.eModId == E_MI_MODULE_ID_SCL)
            {
                MI_U32                u32BindDevId = pstAttachAttr->stRgnChnPort.s32DevId;
                MI_U32                u32BindChn   = pstAttachAttr->stRgnChnPort.s32ChnId;
                MI_U32                u32BindPort  = pstAttachAttr->stRgnChnPort.s32PortId;
                MI_SCL_OutPortParam_t stSclOutputParam;

                memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
                CARDVCHECKRESULT(
                    MI_SCL_GetOutputPortParam((MI_SCL_DEV)u32BindDevId, u32BindChn, u32BindPort, &stSclOutputParam));
                u32VideoWidth  = stSclOutputParam.stSCLOutputSize.u16Width;
                u32VideoHeight = stSclOutputParam.stSCLOutputSize.u16Height;
            }
            else if (pstAttachAttr->stRgnChnPort.eModId == E_MI_MODULE_ID_VENC)
            {
                int i;
                for (i = 0; i < MAX_VIDEO_NUMBER; i++)
                {
                    if (g_videoEncoderArray[i]
                        && g_videoEncoderArray[i]->getVencDev() == pstAttachAttr->stRgnChnPort.s32DevId
                        && g_videoEncoderArray[i]->getVencChn() == pstAttachAttr->stRgnChnPort.s32ChnId)
                    {
                        u32VideoWidth  = g_videoEncoderArray[i]->getWidth();
                        u32VideoHeight = g_videoEncoderArray[i]->getHeight();
                        break;
                    }
                }

                if (i == MAX_VIDEO_NUMBER)
                {
                    printf("RGN Venc Dev [%d] Chn [%d] Not Created\n", pstAttachAttr->stRgnChnPort.s32DevId,
                           pstAttachAttr->stRgnChnPort.s32ChnId);
                    continue;
                }
            }
            else
            {
                printf("RGN Not support attach module [%d]\n", pstAttachAttr->stRgnChnPort.eModId);
                continue;
            }

            pstAttachAttr->u32RgnHandle = OSD_GetHandleByVideoSize(u32VideoWidth, u32VideoHeight);
            if (pstAttachAttr->u32RgnHandle >= 0)
            {
                pstBufferAttr = &gstOsdAttr.stBufferAttr[pstAttachAttr->u32RgnHandle];
                memset(&stChnPortParam, 0x00, sizeof(MI_RGN_ChnPortParam_t));
                stChnPortParam.bShow = TRUE;
#if defined(CHIP_IFADO)
                OSD_CalcVideoStampPosition(u32VideoWidth, u32VideoHeight, pstBufferAttr->u32Width,
                                           pstBufferAttr->u32Height,
                                           (unsigned int *)&stChnPortParam.stOsdChnPort.stPoint.s32X,
                                           (unsigned int *)&stChnPortParam.stOsdChnPort.stPoint.s32Y);
#elif defined(CHIP_IFORD)
                OSD_CalcVideoStampPosition(u32VideoWidth, u32VideoHeight, pstBufferAttr->u32Width,
                                           pstBufferAttr->u32Height,
                                           (unsigned int *)&stChnPortParam.stOsdChnPort.stPoint.u32X,
                                           (unsigned int *)&stChnPortParam.stOsdChnPort.stPoint.u32Y);
#else
                OSD_CalcVideoStampPosition(u32VideoWidth, u32VideoHeight, pstBufferAttr->u32Width,
                                           pstBufferAttr->u32Height, &stChnPortParam.stPoint.u32X,
                                           &stChnPortParam.stPoint.u32Y);
                stChnPortParam.unPara.stOsdChnPort.u32Layer = pstAttachAttr->u32RgnHandle;
#endif
                CARDVCHECKRESULT_OS(MI_RGN_AttachToChn(g_s32SocId, pstAttachAttr->u32RgnHandle,
                                                       &pstAttachAttr->stRgnChnPort, &stChnPortParam));
                pstAttachAttr->bAttach = TRUE;
                pstBufferAttr->u8AttachCnt++;
                CARDVCHECKRESULT_OS(cardv_setRgnRealtimeFlip(&pstAttachAttr->stRgnChnPort, TRUE));
            }
        }
    }

    return 0;
}

MI_S32 mid_osd_full_uninit()
{
    CarDV_OsdBufferAttr *pstBufferAttr;
    CarDV_OsdAttachAttr *pstAttachAttr;

    for (MI_S32 AttachId = 0; AttachId < CARDV_OSD_MAX_ATTACH_NUM; AttachId++)
    {
        pstAttachAttr = &gstOsdAttr.stAttachAttr[AttachId];
        if (pstAttachAttr->bUsed == TRUE && pstAttachAttr->bAttach == TRUE)
        {
            pstBufferAttr = &gstOsdAttr.stBufferAttr[pstAttachAttr->u32RgnHandle];
            CARDVCHECKRESULT(cardv_setRgnRealtimeFlip(&pstAttachAttr->stRgnChnPort, FALSE));
            CARDVCHECKRESULT(
                MI_RGN_DetachFromChn(g_s32SocId, pstAttachAttr->u32RgnHandle, &pstAttachAttr->stRgnChnPort));
            pstAttachAttr->bAttach = FALSE;
            pstBufferAttr->u8AttachCnt--;
        }
    }

    for (MI_S32 OsdID = 0; OsdID < CARDV_OSD_BUFFER_NUM; OsdID++)
    {
        pstBufferAttr = &gstOsdAttr.stBufferAttr[OsdID];
        if (pstBufferAttr->bCreate == TRUE)
        {
            CARDVCHECKRESULT(MI_RGN_Destroy(g_s32SocId, pstBufferAttr->u32RgnHandle));
            pstBufferAttr->bCreate        = FALSE;
            pstBufferAttr->bDrawStampLogo = FALSE;
        }
    }

    pthread_mutex_destroy(&mutexCarDVOsdTxtRgnBuf);
    FREEIF(g_pbuffer);

    return 0;
}

MI_S32 mid_osd_reset_resolution(MI_VENC_CHN VeChn)
{
    MI_RGN_ChnPort_t     stRgnChnPort;
    MI_RGN_ChnPort_t     stRgnChnPort1;
    CarDV_BindParam_t    stBindParam;
    CarDV_OsdAttachAttr *pstAttachAttr;

    if (VeChn >= MAX_VIDEO_NUMBER || NULL == g_videoEncoderArray[VeChn]
        || VeChn != g_videoEncoderArray[VeChn]->getVencChn())
    {
        CARDV_ERR("venc channel [%d] error\n", VeChn);
        return -1;
    }

    stBindParam            = g_videoEncoderArray[VeChn]->getInBindParam();
    stRgnChnPort.eModId    = stBindParam.stChnPort.eModId;
    stRgnChnPort.s32DevId  = stBindParam.stChnPort.u32DevId;
    stRgnChnPort.s32ChnId  = stBindParam.stChnPort.u32ChnId;
    stRgnChnPort.s32PortId = stBindParam.stChnPort.u32PortId;

    stRgnChnPort1.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort1.s32DevId  = g_videoEncoderArray[VeChn]->getVencDev();
    stRgnChnPort1.s32ChnId  = g_videoEncoderArray[VeChn]->getVencChn();
    stRgnChnPort1.s32PortId = 0;

    for (MI_S32 AttachId = 0; AttachId < CARDV_OSD_MAX_ATTACH_NUM; AttachId++)
    {
        pstAttachAttr = &gstOsdAttr.stAttachAttr[AttachId];
        if (pstAttachAttr->bUsed == FALSE || pstAttachAttr->bAttach == FALSE)
            continue;

        if ((pstAttachAttr->stRgnChnPort.eModId == E_MI_MODULE_ID_VENC
             && memcmp(&stRgnChnPort1, &pstAttachAttr->stRgnChnPort, sizeof(MI_RGN_ChnPort_t)) == 0)
            || (pstAttachAttr->stRgnChnPort.eModId != E_MI_MODULE_ID_VENC
                && memcmp(&stRgnChnPort, &pstAttachAttr->stRgnChnPort, sizeof(MI_RGN_ChnPort_t)) == 0))
        {
            CarDV_OsdBufferAttr *pstBufferAttr = &gstOsdAttr.stBufferAttr[pstAttachAttr->u32RgnHandle];
            CARDVCHECKRESULT(cardv_setRgnRealtimeFlip(&pstAttachAttr->stRgnChnPort, FALSE));
            CARDVCHECKRESULT(
                MI_RGN_DetachFromChn(g_s32SocId, pstAttachAttr->u32RgnHandle, &pstAttachAttr->stRgnChnPort));
            pstAttachAttr->bAttach = FALSE;
            pstBufferAttr->u8AttachCnt--;
        }
    }

    return 0;
}

MI_S32 mid_osd_restart_resolution(MI_VENC_CHN VeChn, Size_t *pstSize)
{
    MI_RGN_ChnPortParam_t stChnPortParam;
    MI_RGN_ChnPort_t      stRgnChnPort;
    MI_RGN_ChnPort_t      stRgnChnPort1;
    CarDV_BindParam_t     stBindParam;
    CarDV_OsdAttachAttr * pstAttachAttr;

    memset(&stChnPortParam, 0, sizeof(MI_RGN_ChnPortParam_t));
    memset(&stRgnChnPort, 0, sizeof(MI_RGN_ChnPort_t));
    memset(&stRgnChnPort1, 0, sizeof(MI_RGN_ChnPort_t));
    memset(&stBindParam, 0, sizeof(CarDV_BindParam_t));

    if (VeChn >= MAX_VIDEO_NUMBER || NULL == g_videoEncoderArray[VeChn]
        || VeChn != g_videoEncoderArray[VeChn]->getVencChn())
    {
        CARDV_ERR("venc channel [%d] error\n", VeChn);
        return -1;
    }

    stBindParam            = g_videoEncoderArray[VeChn]->getInBindParam();
    stRgnChnPort.eModId    = stBindParam.stChnPort.eModId;
    stRgnChnPort.s32DevId  = stBindParam.stChnPort.u32DevId;
    stRgnChnPort.s32ChnId  = stBindParam.stChnPort.u32ChnId;
    stRgnChnPort.s32PortId = stBindParam.stChnPort.u32PortId;

    stRgnChnPort1.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort1.s32DevId  = g_videoEncoderArray[VeChn]->getVencDev();
    stRgnChnPort1.s32ChnId  = g_videoEncoderArray[VeChn]->getVencChn();
    stRgnChnPort1.s32PortId = 0;

    for (MI_S32 AttachId = 0; AttachId < CARDV_OSD_MAX_ATTACH_NUM; AttachId++)
    {
        pstAttachAttr = &gstOsdAttr.stAttachAttr[AttachId];

        if (pstAttachAttr->bUsed == FALSE || pstAttachAttr->bAttach == TRUE)
            continue;

        if ((pstAttachAttr->stRgnChnPort.eModId == E_MI_MODULE_ID_VENC
             && memcmp(&stRgnChnPort1, &pstAttachAttr->stRgnChnPort, sizeof(MI_RGN_ChnPort_t)) == 0)
            || (pstAttachAttr->stRgnChnPort.eModId != E_MI_MODULE_ID_VENC
                && memcmp(&stRgnChnPort, &pstAttachAttr->stRgnChnPort, sizeof(MI_RGN_ChnPort_t)) == 0))
        {
            MI_U32 u32VideoWidth, u32VideoHeight;
            if (pstAttachAttr->stRgnChnPort.eModId == E_MI_MODULE_ID_SCL)
            {
                MI_U32                u32BindDevId = pstAttachAttr->stRgnChnPort.s32DevId;
                MI_U32                u32BindChn   = pstAttachAttr->stRgnChnPort.s32ChnId;
                MI_U32                u32BindPort  = pstAttachAttr->stRgnChnPort.s32PortId;
                MI_SCL_OutPortParam_t stSclOutputParam;

                memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
                CARDVCHECKRESULT(
                    MI_SCL_GetOutputPortParam((MI_SCL_DEV)u32BindDevId, u32BindChn, u32BindPort, &stSclOutputParam));
                u32VideoWidth  = stSclOutputParam.stSCLOutputSize.u16Width;
                u32VideoHeight = stSclOutputParam.stSCLOutputSize.u16Height;
            }
            else if (pstAttachAttr->stRgnChnPort.eModId == E_MI_MODULE_ID_VENC)
            {
                u32VideoWidth  = g_videoEncoderArray[VeChn]->getWidth();
                u32VideoHeight = g_videoEncoderArray[VeChn]->getHeight();
            }
            else
            {
                printf("RGN Not support attach module [%d]\n", pstAttachAttr->stRgnChnPort.eModId);
                continue;
            }

            pstAttachAttr->u32RgnHandle = OSD_GetHandleByVideoSize(u32VideoWidth, u32VideoHeight);
            if (pstAttachAttr->u32RgnHandle >= 0)
            {
                CarDV_OsdBufferAttr *pstBufferAttr = &gstOsdAttr.stBufferAttr[pstAttachAttr->u32RgnHandle];
                memset(&stChnPortParam, 0x00, sizeof(MI_RGN_ChnPortParam_t));
                stChnPortParam.bShow = TRUE;
#if defined(CHIP_IFADO)
                OSD_CalcVideoStampPosition(u32VideoWidth, u32VideoHeight, pstBufferAttr->u32Width,
                                           pstBufferAttr->u32Height,
                                           (unsigned int *)&stChnPortParam.stOsdChnPort.stPoint.s32X,
                                           (unsigned int *)&stChnPortParam.stOsdChnPort.stPoint.s32Y);
#elif defined(CHIP_IFORD)
                OSD_CalcVideoStampPosition(u32VideoWidth, u32VideoHeight, pstBufferAttr->u32Width,
                                           pstBufferAttr->u32Height,
                                           (unsigned int *)&stChnPortParam.stOsdChnPort.stPoint.u32X,
                                           (unsigned int *)&stChnPortParam.stOsdChnPort.stPoint.u32Y);
#else
                OSD_CalcVideoStampPosition(u32VideoWidth, u32VideoHeight, pstBufferAttr->u32Width,
                                           pstBufferAttr->u32Height, &stChnPortParam.stPoint.u32X,
                                           &stChnPortParam.stPoint.u32Y);

                stChnPortParam.unPara.stOsdChnPort.u32Layer = pstAttachAttr->u32RgnHandle;
#endif

                CARDVCHECKRESULT(MI_RGN_AttachToChn(g_s32SocId, pstAttachAttr->u32RgnHandle,
                                                    &pstAttachAttr->stRgnChnPort, &stChnPortParam));
                pstAttachAttr->bAttach = TRUE;
                pstBufferAttr->u8AttachCnt++;
                CARDVCHECKRESULT(cardv_setRgnRealtimeFlip(&pstAttachAttr->stRgnChnPort, TRUE));
            }
        }
    }

    return 0;
}

int osd_init()
{
#ifndef DUAL_OS

    mid_osd_full_init();

    g_exit = FALSE;
    pthread_create(&pthreadOSD, NULL, OSD_Task, (void *)NULL);
    pthread_setname_np(pthreadOSD, "OSD_Task");
    g_osd_init_done = 1;
#endif
    return 0;
}

int osd_uninit()
{
    if (1 != g_osd_init_done)
    {
        return 0;
    }

    g_exit = TRUE;
    pthread_join(pthreadOSD, NULL);
    g_osd_init_done = 0;

    mid_osd_full_uninit();
    cardv_Font_Exit();
    return 0;
}

int osd_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    switch (id)
    {
    case CMD_OSD_OPEN:
        osd_init();
        break;

    case CMD_OSD_CLOSE:
        osd_uninit();
        break;

    case CMD_OSD_RESET_RESOLUTION:
        if ((NULL != param) && (0 < paramLen))
        {
            MI_U32 Osdparam[3];
            MI_U32 veChannel = 0;

            memcpy(Osdparam, param, paramLen);
            veChannel = Osdparam[0];
            mid_osd_reset_resolution(veChannel);
        }
        break;

    case CMD_OSD_RESTART_RESOLUTION:
        if ((NULL != param) && (0 < paramLen))
        {
            Size_t size;
            MI_U32 Osdparam[3];
            MI_U32 veChannel = 0;

            memcpy(Osdparam, param, paramLen);
            veChannel   = Osdparam[0];
            size.width  = Osdparam[1];
            size.height = Osdparam[2];
            mid_osd_restart_resolution(veChannel, &size);
        }
        break;

    case CMD_OSD_SET_STAMP:
        DatelogoStampMode = *param; // 0-DATELOGO, 1-DATE, 2-LOGO, 3-OFF
        break;

    case CMD_OSD_SET_TIMEFORMAT:
        DateFormat = *param; // 0-OFF, 1-YMD, 2-MDY, 3-DMY
        break;
    default:
        break;
    }

    return 0;
}

void osd_SetStampMode(int s32Mode)
{
    if (STAMP_MODE_DATELOGO == s32Mode || STAMP_MODE_DATE == s32Mode || STAMP_MODE_LOGO == s32Mode
        || STAMP_MODE_OFF == s32Mode)
        DatelogoStampMode = s32Mode;
    else
        DatelogoStampMode = STAMP_MODE_DATE;
}

void osd_SetTimeFormat(int s32Mode)
{
    if (DATE_FORMAT_OFF == s32Mode || DATE_FORMAT_YMD == s32Mode || DATE_FORMAT_MDY == s32Mode
        || DATE_FORMAT_DMY == s32Mode)
        DateFormat = s32Mode;
    else
        DateFormat = DATE_FORMAT_YMD;
}