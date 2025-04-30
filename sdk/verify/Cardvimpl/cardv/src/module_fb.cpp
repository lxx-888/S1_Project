/*
 * st_fb.c- Sigmastar
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 *
 * Author: XXXX <XXXX@sigmastar.com.cn>
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
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#include "mi_sys.h"
#include "module_fb.h"
#include "mid_common.h"

#if (CARDV_FB_ENABLE)

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

// Start of frame buffer mem
static MI_U8 *frameBuffer  = NULL;
static MI_S32 g_fbFd       = 0;
static MI_S32 g_screensize = 0;

#define CHECK_FB_INIT                 \
    do                                \
    {                                 \
        if (frameBuffer == NULL)      \
        {                             \
            printf("fb init fail\n"); \
            return;                   \
        }                             \
    } while (0)

/**
 *dump fix info of Framebuffer
 */
#define SWITCH_NEXT_Y_OFFSET(yres, yoffset, yres_virtual) \
    (yoffset = ((yoffset + yres < yres_virtual) ? yoffset + yres : 0))
#define SWITCH_NEXT_Y_AND_CONVERT_TO_BUF_ID(yres, yoffset, yres_virtual) \
    (SWITCH_NEXT_Y_OFFSET(yres, yoffset, yres_virtual) / yres)
#define CUR_FB_BUF_ID(yres, yoffset) (yoffset / yres)

#define FB_MAX_BUF 4
typedef struct ST_FB_DirtyRectangle_s
{
    MI_SYS_WindowRect_t astDirtyRect[FB_MAX_BUF];
} ST_FB_DirtyRectangle_t;

static ST_FB_DirtyRectangle_t stDirtyRect;
static pthread_mutex_t        _gstFbMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 *get Color format fo framebuffer
 */
MI_FB_ColorFmt_e getFBColorFmt(struct fb_var_screeninfo *var)
{
    MI_FB_ColorFmt_e fmt = E_MI_FB_COLOR_FMT_INVALID;

    switch (var->bits_per_pixel)
    {
    case 16: {
        if (var->transp.length == 0) // RGB565
        {
            fmt = E_MI_FB_COLOR_FMT_RGB565;
        }
        else if (var->transp.length == 1) // ARGB 1555
        {
            fmt = E_MI_FB_COLOR_FMT_ARGB1555;
        }
        else if (var->transp.length == 4) // ARGB4444
        {
            fmt = E_MI_FB_COLOR_FMT_ARGB4444;
        }
    }
    break;
    // ARGB8888
    case 32: {
        fmt = E_MI_FB_COLOR_FMT_ARGB8888;
    }
    break;
    default:
        fmt = E_MI_FB_COLOR_FMT_INVALID;
        break;
    }
    return fmt;
}

static MI_U32 getBytePerPixel()
{
    MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);

    switch (fmt)
    {
    case E_MI_FB_COLOR_FMT_ARGB8888:
        return 4;
    case E_MI_FB_COLOR_FMT_RGB565:
        return 2;
    case E_MI_FB_COLOR_FMT_ARGB4444:
        return 2;
    case E_MI_FB_COLOR_FMT_ARGB1555:
        return 2;
    default:
        printf("Warning: drawRect() not implemented for color Fmt %i\n", fmt);
    }

    return 0;
}
MI_S32 ST_FB_SyncDirtyDown()
{
    char * pCurStartAddr   = NULL;
    char * pLoopStartAddr  = NULL;
    MI_U16 stride          = 0;
    MI_U16 u16BytePerPixel = 0;
    MI_U16 u16LoopBufId    = 0;
    MI_U16 u16CurBufId     = 0;
    MI_U16 u16YOffsetTmp   = 0;
    MI_U16 i               = 0;

    pthread_mutex_lock(&_gstFbMutex);
    u16BytePerPixel = getBytePerPixel();
    stride          = finfo.line_length;
    u16YOffsetTmp   = vinfo.yoffset;
    u16LoopBufId    = SWITCH_NEXT_Y_AND_CONVERT_TO_BUF_ID(vinfo.yres, u16YOffsetTmp, vinfo.yres_virtual);
    u16CurBufId     = CUR_FB_BUF_ID(vinfo.yres, vinfo.yoffset);
    while (u16LoopBufId != u16CurBufId)
    {
        if (u16LoopBufId >= FB_MAX_BUF || u16CurBufId >= FB_MAX_BUF)
        {
            printf("FB MAX buf error %d %d\n", u16LoopBufId, u16CurBufId);

            assert(0);
        }
        pCurStartAddr = (char *)(frameBuffer)
                        + (u16CurBufId * vinfo.yres + stDirtyRect.astDirtyRect[u16LoopBufId].u16Y) * stride
                        + stDirtyRect.astDirtyRect[u16LoopBufId].u16X * u16BytePerPixel;
        pLoopStartAddr = (char *)(frameBuffer)
                         + (u16LoopBufId * vinfo.yres + stDirtyRect.astDirtyRect[u16LoopBufId].u16Y) * stride
                         + stDirtyRect.astDirtyRect[u16LoopBufId].u16X * u16BytePerPixel;
        /*printf("Blit from id %d x %d y %d w %d h %d\n", u16LoopBufId, stDirtyRect.astDirtyRect[u16LoopBufId].u16X,
           stDirtyRect.astDirtyRect[u16LoopBufId].u16Y,
                                                  stDirtyRect.astDirtyRect[u16LoopBufId].u16Width,
           stDirtyRect.astDirtyRect[u16LoopBufId].u16Height);*/
        for (i = 0; i < stDirtyRect.astDirtyRect[u16LoopBufId].u16Height; i++)
        {
            memcpy(pCurStartAddr, pLoopStartAddr, u16BytePerPixel * stDirtyRect.astDirtyRect[u16LoopBufId].u16Width);
            pCurStartAddr += stride;
            pLoopStartAddr += stride;
        }
        u16LoopBufId = SWITCH_NEXT_Y_AND_CONVERT_TO_BUF_ID(vinfo.yres, u16YOffsetTmp, vinfo.yres_virtual);
    }

    return MI_SUCCESS;
}
MI_S32 ST_FB_SyncDirtyUp(const MI_SYS_WindowRect_t *pRect)
{
    MI_U16 u16CurBufId   = 0;
    MI_U16 u16LoopBufId  = 0;
    MI_U16 u16YOffsetTmp = 0;

    u16CurBufId   = CUR_FB_BUF_ID(vinfo.yres, vinfo.yoffset);
    u16YOffsetTmp = vinfo.yoffset;
    u16LoopBufId  = SWITCH_NEXT_Y_AND_CONVERT_TO_BUF_ID(vinfo.yres, u16YOffsetTmp, vinfo.yres_virtual);
    if (u16LoopBufId >= FB_MAX_BUF || u16CurBufId >= FB_MAX_BUF)
    {
        printf("FB MAX buf error %d %d\n", u16LoopBufId, u16CurBufId);

        assert(0);
    }
    stDirtyRect.astDirtyRect[u16CurBufId] = *pRect;
    if (u16LoopBufId != u16CurBufId)
    {
        if (ioctl(g_fbFd, FBIOPAN_DISPLAY, &vinfo) == -1)
        {
            perror("Error: failed to FBIOPAN_DISPLAY");
            assert(0);
        }
    }
    SWITCH_NEXT_Y_OFFSET(vinfo.yres, vinfo.yoffset, vinfo.yres_virtual);
    pthread_mutex_unlock(&_gstFbMutex);

    return MI_SUCCESS;
}
MI_S32 ST_FB_CombineRect(MI_SYS_WindowRect_t *pRectDst, MI_SYS_WindowRect_t *pRectA, MI_SYS_WindowRect_t *pRectB)
{
    pRectDst->u16X = MIN(pRectA->u16X, pRectB->u16X);
    pRectDst->u16Y = MIN(pRectA->u16Y, pRectB->u16Y);
    pRectDst->u16Width =
        MAX(pRectA->u16Width + pRectA->u16X - pRectDst->u16X, pRectB->u16Width + pRectB->u16X - pRectDst->u16X);
    pRectDst->u16Height =
        MAX(pRectA->u16Height + pRectA->u16Y - pRectDst->u16Y, pRectB->u16Height + pRectB->u16Y - pRectDst->u16Y);

    return MI_SUCCESS;
}
/**
 * draw Rectangle. the colormat of Framebuffer is ARGB8888
 */
void drawRect_rgb32(int x0, int y0, int width, int height, int color)
{
    const int bytesPerPixel = 4;
    int       stride        = 0;
    int *     dest          = NULL;
    int       x = 0, y = 0;

    CHECK_FB_INIT;

    stride = finfo.line_length;
    dest   = (int *)((char *)(frameBuffer) + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset) * bytesPerPixel);

    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            dest[x] = color;
        }
        dest = (int *)((char *)dest + stride);
    }
}

/**
 * draw Rectangle. the colormat of Framebuffer is RGB565
 */
void drawRect_rgb16(int x0, int y0, int width, int height, int color)
{
    const int   bytesPerPixel = 2;
    int         stride        = 0;
    const int   red           = (color & 0xff0000) >> (16 + 3);
    const int   green         = (color & 0xff00) >> (8 + 2);
    const int   blue          = (color & 0xff) >> 3;
    const short color16       = blue | (green << 5) | (red << (5 + 6));
    short *     dest          = NULL;
    int         x = 0, y = 0;

    CHECK_FB_INIT;

    stride = finfo.line_length;
    dest   = (short *)((char *)(frameBuffer) + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset) * bytesPerPixel);

    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            dest[x] = color16;
        }
        dest = (short *)((char *)dest + stride);
    }
}

/**
 *draw point. the color format of Framebuffer is ARGB1555
 */
void drawPoint_rgb15(int x, int y, int color)
{
    const int   bytesPerPixel = 2;
    int         stride        = 0;
    const int   red           = (color & 0xff0000) >> (16 + 3);
    const int   green         = (color & 0xff00) >> (8 + 3);
    const int   blue          = (color & 0xff) >> 3;
    const short color15       = blue | (green << 5) | (red << (5 + 5)) | 0x8000;
    short *     dest          = NULL;

    CHECK_FB_INIT;

    stride = finfo.line_length;
    dest   = (short *)((char *)(frameBuffer) + (y + vinfo.yoffset) * stride + (x + vinfo.xoffset) * bytesPerPixel);

    dest[0] = color15;
    // printf("color:%X  and color15:%X ,color15:%x ,color15:%d \n",color,color15,color15,color15);
}

void drawLine_rgb15(int x0, int y0, int x1, int y1, int color)
{
    int dx  = x1 - x0;
    int dy  = y1 - y0;
    int ux  = dx > 0 ? 1 : -1;
    int uy  = dy > 0 ? 1 : -1;
    int dx2 = dx << 1;
    int dy2 = dy << 1;
    int e, x, y;

    if (abs(dx) > abs(dy))
    {
        e = -dx;
        x = x0;
        y = y0;
        for (x = x0; x < x1; x += ux)
        {
            drawPoint_rgb15(x, y, color);
            e = e + dy2;
            if (e > 0)
            {
                y += uy;
                e = e - dx2;
            }
        }
    }
    else
    {
        e = -dy;
        x = x0;
        y = y0;
        for (y = y0; y < y1; y += uy)
        {
            // printf ("%d,%d, y1:%d\n",x, y, y1);
            drawPoint_rgb15(x, y, color);
            e = e + dx2;
            if (e > 0)
            {
                x += ux;
                e = e - dy2;
            }
        }
    }
}

#if 0
void drawCircle_rgb15(int xc, int yc, int r, int color)
{
    int x, y, d;

    x = 0;
    y = r;

    d = 3 - 2 * r;

    printf("ddddddddddddddddddddd, %d,%d - %d,%d\n", xc, yc, x, y);
    drawLine_rgb15(xc, yc, x, y, color);

    while (x < y)
    {
        if (d < 0)
        {
            d = d + 4 * x + 6;
        }
        else
        {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;

        drawLine_rgb15(xc, yc, x, y, color);
    }
}
#endif

void drawCircle_rgb15(int cx, int cy, int r, int color)
{
    int r2 = r * r + r;
    int x = 0, x2 = 0, dx2 = 1;
    int y = r, y2 = y * y, dy2 = 2 * y - 1;
    int sum = r2;

    while (x <= y)
    {
        drawLine_rgb15(cx - y, cy + x, cx + y, cy + x, color);
        if (x)
            drawLine_rgb15(cx - y, cy - x, cx + y, cy - x, color);

        sum -= dx2;
        if (sum <= y2)
        {
            if (x != y)
            {
                drawLine_rgb15(cx - x, cy - y, cx + x, cy - y, color);
                if (y)
                    drawLine_rgb15(cx - x, cy + y, cx + x, cy + y, color);
            } /* if */
            y--;
            y2 -= dy2;
            dy2 -= 2;
        } /* if */
        x++;
        x2 += dx2;
        dx2 += 2;
    } /* while */
} /* bh */

/**
 *draw Rectangle. the color format of Framebuffer is ARGB1555
 */
void drawRect_rgb15(int x0, int y0, int width, int height, int color)
{
    const int   bytesPerPixel = 2;
    int         stride        = 0;
    const int   red           = (color & 0xff0000) >> (16 + 3);
    const int   green         = (color & 0xff00) >> (8 + 3);
    const int   blue          = (color & 0xff) >> 3;
    const short color15       = blue | (green << 5) | (red << (5 + 5)) | 0x8000;
    short *     dest          = NULL;
    int         x = 0, y = 0;

    CHECK_FB_INIT;

    stride = finfo.line_length;
    dest   = (short *)((char *)(frameBuffer) + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset) * bytesPerPixel);

    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            dest[x] = color15;
        }
        dest = (short *)((char *)dest + stride);
    }
}

/**
 *draw Rectangle. the color format of Framebuffer is ARGB1444
 */
void drawRect_rgb12(int x0, int y0, int width, int height, int color)
{
    const int   bytesPerPixel = 2;
    int         stride        = 0;
    const int   red           = (color & 0xff0000) >> (16 + 4);
    const int   green         = (color & 0xff00) >> (8 + 4);
    const int   blue          = (color & 0xff) >> 4;
    const short color16       = blue | (green << 4) | (red << (4 + 4)) | 0xf000;
    short *     dest          = NULL;
    int         x, y;

    stride = finfo.line_length;
    dest   = (short *)((char *)(frameBuffer) + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset) * bytesPerPixel);

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            dest[x] = color16;
        }
        dest = (short *)((char *)dest + stride);
    }
}

void drawPoint(int x, int y, int color)
{
    MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);

    switch (fmt)
    {
    case E_MI_FB_COLOR_FMT_ARGB1555: {
        drawPoint_rgb15(x, y, color);
    }
    break;
    default:
        printf("Warning: drawRect() not implemented for color Fmt %i\n", fmt);
    }
}

void drawLine(int x0, int y0, int x1, int y1, int color)
{
    MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);

    switch (fmt)
    {
    case E_MI_FB_COLOR_FMT_ARGB1555: {
        drawLine_rgb15(x0, y0, x1, y1, color);
    }
    break;
    default:
        printf("Warning: drawRect() not implemented for color Fmt %i\n", fmt);
    }
}

void drawCircle(int xc, int yc, int r, int color)
{
    MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);

    switch (fmt)
    {
    case E_MI_FB_COLOR_FMT_ARGB1555: {
        drawCircle_rgb15(xc, yc, r, color);
    }
    break;
    default:
        printf("Warning: drawRect() not implemented for color Fmt %i\n", fmt);
    }
}

/**
 *draw Rectangle. accroding to Framebuffer format
 */
void drawRect(int x0, int y0, int width, int height, int color)
{
    MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);

    switch (fmt)
    {
    case E_MI_FB_COLOR_FMT_ARGB8888: {
        drawRect_rgb32(x0, y0, width, height, color);
    }
    break;
    case E_MI_FB_COLOR_FMT_RGB565: {
        drawRect_rgb16(x0, y0, width, height, color);
    }
    break;
    case E_MI_FB_COLOR_FMT_ARGB4444: {
        drawRect_rgb12(x0, y0, width, height, color);
    }
    break;
    case E_MI_FB_COLOR_FMT_ARGB1555: {
        drawRect_rgb15(x0, y0, width, height, color);
    }
    break;
    default:
        printf("Warning: drawRect() not implemented for color Fmt %i\n", fmt);
    }
}

/**
 *Conver color key value according to color format
 */
void convertColorKeyByFmt(MI_FB_ColorKey_t *colorkey)
{
    MI_FB_ColorFmt_e fmt   = getFBColorFmt(&vinfo);
    MI_U8            red   = colorkey->u8Red;
    MI_U8            green = colorkey->u8Green;
    MI_U8            blue  = colorkey->u8Blue;
    switch (fmt)
    {
    case E_MI_FB_COLOR_FMT_RGB565: {
        colorkey->u8Red   = (red >> 3) & (0x1f);
        colorkey->u8Green = (green >> 2) & (0x3f);
        colorkey->u8Blue  = (blue >> 3) & (0x1f);
    }
    break;
    case E_MI_FB_COLOR_FMT_ARGB4444: {
        colorkey->u8Red   = (red >> 4) & 0xf;
        colorkey->u8Green = (green >> 4) & 0xf;
        colorkey->u8Blue  = (blue >> 4) & 0xf;
    }
    break;
    case E_MI_FB_COLOR_FMT_ARGB1555: {
        colorkey->u8Red   = (red >> 3) & 0x1f;
        colorkey->u8Green = (green >> 3) & 0x1f;
        colorkey->u8Blue  = (blue >> 3) & 0x1f;
    }
    break;
    default:
        printf("convertColorKeyByFmt colorfmt is %d\n", fmt);
        break;
    }
}

MI_S32 ST_Fb_Init(void)
{
    const char *devfile = "/dev/fb0";
    // MI_BOOL bShown = FALSE;

    /* Open the file for reading and writing */
    g_fbFd = open(devfile, O_RDWR);
    if (g_fbFd == -1)
    {
        perror("Error: cannot open framebuffer device");
        assert(0);
    }

    // get fb_fix_screeninfo
    if (ioctl(g_fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error reading fixed information");
        assert(0);
    }
    // printFixedInfo();

    // get fb_var_screeninfo
    if (ioctl(g_fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        assert(0);
    }
    // printVariableInfo();

    /* Figure out the size of the screen in bytes */
    g_screensize = finfo.smem_len;

    /* Map the device to memory */
    frameBuffer = (MI_U8 *)mmap(0, g_screensize, PROT_READ | PROT_WRITE, MAP_SHARED, g_fbFd, 0);
    if (frameBuffer == MAP_FAILED)
    {
        perror("Error: Failed to map framebuffer device to memory");
        assert(0);
    }

    // clear framebuffer
    drawRect(0, 0, vinfo.xres, vinfo.yres, ARGB888_BLUE);
    memset(&stDirtyRect, 0, sizeof(ST_FB_DirtyRectangle_t));
    if (ioctl(g_fbFd, FBIOPAN_DISPLAY, &vinfo) == -1)
    {
        perror("Error: failed to FBIOPAN_DISPLAY");
        assert(0);
    }

// memset(frameBuffer, ARGB1555_RED, g_screensize);
#if 0
    if (ioctl(g_fbFd, FBIOGET_SHOW, &bShown) < 0)
    {
        perror("Error: failed to FBIOGET_SHOW");
        assert(0);
    }
    printf("FBIOGET_SHOW result bShown is %d\n", bShown);

    // test FBIOSET_SHOW
    bShown = TRUE;
    if (ioctl(g_fbFd, FBIOSET_SHOW, &bShown) < 0)
    {
        perror("Error: failed to FBIOSET_SHOW");
        assert(0);
    }
#endif

    return MI_SUCCESS;
}
void ST_Fb_SetColorFmt(MI_FB_ColorFmt_e enFormat)
{
    if (g_fbFd < 0)
    {
        return;
    }

    MI_FB_DisplayLayerAttr_t stDisplayAttr;

    // set color format
    memset(&stDisplayAttr, 0, sizeof(MI_FB_DisplayLayerAttr_t));
    if (ioctl(g_fbFd, FBIOGET_DISPLAYLAYER_ATTRIBUTES, &stDisplayAttr))
    {
        perror("Error: failed to FBIOGET_DISPLAYLAYER_ATTRIBUTES");
        return;
    }

    printf(
        "%s %d, "
        "u32Xpos:%d,u32YPos:%d,u32dstWidth:%d,u32dstHeight:%d,u32DisplayWidth:%d,u32DisplayHeight:%d,u32ScreenWidth:%d,"
        "u32ScreenHeight:%d\n",
        __func__, __LINE__, stDisplayAttr.u32Xpos, stDisplayAttr.u32YPos, stDisplayAttr.u32dstWidth,
        stDisplayAttr.u32dstHeight, stDisplayAttr.u32DisplayWidth, stDisplayAttr.u32DisplayHeight,
        stDisplayAttr.u32ScreenWidth, stDisplayAttr.u32ScreenHeight);

    if (enFormat == E_MI_FB_COLOR_FMT_YUV422)
    {
        printf("not support this format, set default format, ARGB1555\n");
        enFormat = E_MI_FB_COLOR_FMT_ARGB1555;
    }
    stDisplayAttr.u32SetAttrMask = E_MI_FB_DISPLAYLAYER_ATTR_MASK_COLOR_FMB;
    stDisplayAttr.eFbColorFmt    = enFormat;
    if (ioctl(g_fbFd, FBIOSET_DISPLAYLAYER_ATTRIBUTES, &stDisplayAttr))
    {
        perror("Error: failed to FBIOSET_DISPLAYLAYER_ATTRIBUTES");
        return;
    }
}

MI_S32 ST_Fb_DeInit()
{
    if (frameBuffer != NULL)
    {
        munmap(frameBuffer, g_screensize);
        frameBuffer = NULL;
    }

    if (g_fbFd > 0)
    {
        close(g_fbFd);
        g_fbFd = 0;
    }

    return MI_SUCCESS;
}

MI_S32 ST_Fb_FillRect(const MI_SYS_WindowRect_t *pRect, MI_U32 u32ColorVal)
{
    drawRect((MI_S32)pRect->u16X, (MI_S32)pRect->u16Y, (MI_S32)pRect->u16Width, (MI_S32)pRect->u16Height,
             (MI_S32)u32ColorVal);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_DrawRect(const MI_SYS_WindowRect_t *pRect, MI_U32 u32ColorVal)
{
#if 0
    MI_FB_ColorFmt_e fmt = getFBColorFmt(&vinfo);

    switch (fmt)
    {
    case E_MI_FB_COLOR_FMT_ARGB1555:
    {
        drawLine_rgb15(pRect->u16X, pRect->u16Y, pRect->u16X + pRect->u16Width, pRect->u16Y, u32ColorVal);

        drawLine_rgb15(pRect->u16X + pRect->u16Width, pRect->u16Y, pRect->u16X + pRect->u16Width,
                       pRect->u16Y + pRect->u16Height, u32ColorVal);

        printf("dddddddddd\n");
        drawLine_rgb15(pRect->u16X + pRect->u16Width, pRect->u16Y + pRect->u16Height, pRect->u16X,
                       pRect->u16Y + pRect->u16Height, u32ColorVal);
        printf("fffff\n");
        drawLine_rgb15(pRect->u16X, pRect->u16Y + pRect->u16Height, pRect->u16X, pRect->u16Y, u32ColorVal);
    }
    break;
    default:
        printf("Warning: drawRect() not implemented for color Fmt %i\n", fmt);
    }
#endif

    drawLine(pRect->u16X, pRect->u16Y, pRect->u16X + pRect->u16Width, pRect->u16Y, u32ColorVal);

    drawLine(pRect->u16X + pRect->u16Width, pRect->u16Y, pRect->u16X + pRect->u16Width, pRect->u16Y + pRect->u16Height,
             u32ColorVal);

    drawLine(pRect->u16X, pRect->u16Y + pRect->u16Height, pRect->u16X + pRect->u16Width, pRect->u16Y + pRect->u16Height,
             u32ColorVal);

    drawLine(pRect->u16X, pRect->u16Y, pRect->u16X, pRect->u16Y + pRect->u16Height, u32ColorVal);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_FillLine(MI_U16 x0, MI_U16 y0, MI_U16 x1, MI_U16 y1, MI_U32 u32ColorVal)
{
    drawLine(x0, y0, x1, y1, (MI_S32)u32ColorVal);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_FillLine2(MI_U16 x0, MI_U16 y0, MI_U16 x1, MI_U16 y1, MI_U32 u32ColorVal, MI_U16 width)
{
    float xx1 = x0, yy1 = y0;
    float xx2 = x1, yy2 = y1;

    if (xx1 == xx2)
    {
        // CARDV_ERR("Parallel to Y-axis,Not supported \n");
        return -1;
    }

    float ratio = (yy2 - yy1) / (xx2 - xx1);
    float b     = yy1 - (ratio * xx1);
    // CARDV_INFO("ratio:%f b:%f \n",ratio,b);

    if (1) // tend to x-axis
    {
        int x = MIN(x0, x1), p0 = 0, p1 = 0;
        int w  = MAX(x0, x1);
        int ii = 0;
        int jj = 0;
        // CARDV_WARN("fill x:%d w:%d \n",x,w);
        if (width > 1)
        {
            ii = 0;
            for (; ii < width - 1; ii++)
            {
                for (; x <= w; ++x)
                {
                    // printf("line:%d ,x:%d \n",ii,x);
                    p0 = b + ii + ((float)x * ratio);
                    p1 = b + ii + 1 + ((float)x * ratio);
                    jj = 0;
                    for (; jj <= p1 - p0; jj++)
                    {
                        drawPoint_rgb15(x, p0 + jj, u32ColorVal);
                    }
                }
                x = MIN(x0, x1);
            }
        }
        else // pixel equal to 1
        {
            for (; x <= w; ++x)
            {
                // printf("yyj test 1px in  \n");
                p0 = b + ((float)x * ratio);
                drawPoint_rgb15(x, p0, u32ColorVal);
            }
        }
    }

    else // tend to y-axis
    {
        int x = 0, y = MIN(y0, y1);
        int h = MAX(y0, y1);

        for (; y <= h; ++y)
        {
            x = b + ((float)y / ratio);
            //*(dst + y * stride + x) = (uint32_t)color;
            drawPoint_rgb15(x, y, u32ColorVal);
        }
    }

    return MI_SUCCESS;
}

void ST_Fb_DrawNum(CarDV_FontData_t *fontData, BOOL show)
{
    if (fontData->pmt == E_MI_RGN_PIXEL_FORMAT_ARGB1555)
    {
        MI_U16 *dstBuf, *dstBuf1;
        MI_U16 *srcBuf, *srcBuf1;
        int     stride        = finfo.line_length; // 1792 one line bytes  896 * 512
        int     bytesPerPixel = getBytePerPixel(); // rgb1555
        MI_S32  dd            = 0;
        srcBuf                = (MI_U16 *)fontData->buffer;
        dstBuf = (MI_U16 *)((char *)(frameBuffer) + (vinfo.yoffset) * stride + (vinfo.xoffset) * bytesPerPixel);

        // CARDV_WARN("show :%d ,addr:%p ,fontData->height:%d ,font->width :%d
        // \n",show,fontData,fontData->height,fontData->width);

        if (show)
        {
            for (dd = 0; dd < fontData->height; dd++)
            {
                srcBuf1 = srcBuf;
                dstBuf1 = dstBuf;
                memcpy(dstBuf1, srcBuf1, fontData->stride);
                srcBuf += fontData->stride / 2;
                dstBuf += stride / 2;
            }
        }
        else
        {
            for (dd = 0; dd < fontData->height; dd++)
            {
                dstBuf1 = dstBuf;
                for (int ii = 0; ii < fontData->stride / 2; ii++)
                {
                    *dstBuf1 = 0x801F;
                    dstBuf1++;
                }
                dstBuf += stride / 2;
            }
        }

        // CARDV_WARN("show:%d ,fontData:%p  w:%d h:%d stride:%d \n",
        //    show,fontData,fontData->width,fontData->height,stride);
    }
    else
    {
        CARDV_ERR("Only support  argb1555 or I4 ,and now fmt: %d \n", fontData->pmt);
    }
}

MI_S32 ST_Fb_FillPoint(MI_U16 x, MI_U16 y, MI_U32 u32ColorVal)
{
    drawPoint(x, y, (MI_S32)u32ColorVal);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_FillCircle(MI_U16 x, MI_U16 y, MI_U16 r, MI_U32 u32ColorVal)
{
    drawCircle(x, y, r, (MI_S32)u32ColorVal);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_GetColorKey(MI_U32 *pu32ColorKeyVal)
{
    MI_FB_ColorKey_t colorKeyInfo;
    if (ioctl(g_fbFd, FBIOGET_COLORKEY, &colorKeyInfo) < 0)
    {
        printf("Error: failed to FBIOGET_COLORKEY\n");
        assert(0);
    }
    *pu32ColorKeyVal = ((colorKeyInfo.bKeyEnable ? 0xFF : 0) << 24) | (colorKeyInfo.u8Red << 16)
                       | (colorKeyInfo.u8Green << 8) | (colorKeyInfo.u8Blue);

    return MI_SUCCESS;
}

MI_S32 ST_Fb_SetColorKey(MI_U32 u32ColorKeyVal)
{
    MI_FB_ColorKey_t colorKeyInfo;
    if (ioctl(g_fbFd, FBIOGET_COLORKEY, &colorKeyInfo) < 0)
    {
        printf("Error: failed to FBIOGET_COLORKEY\n");
        assert(0);
    }

    colorKeyInfo.bKeyEnable = TRUE;
    colorKeyInfo.u8Red      = PIXEL8888RED(u32ColorKeyVal);
    colorKeyInfo.u8Green    = PIXEL8888GREEN(u32ColorKeyVal);
    colorKeyInfo.u8Blue     = PIXEL8888BLUE(u32ColorKeyVal);

    // convertColorKeyByFmt(&colorKeyInfo);
    if (ioctl(g_fbFd, FBIOSET_COLORKEY, &colorKeyInfo) < 0)
    {
        printf("Error: failed to FBIOGET_COLORKEY");
        assert(0);
    }

    return MI_SUCCESS;
}

MI_S32 ST_Fb_InitMouse(MI_S32 s32MousePicW, MI_S32 s32MousePicH, MI_S32 s32BytePerPixel, MI_U8 *pu8MouseFile)
{
    /*
        MI_FB_CursorAttr_t stCursorAttr;
        FILE *fp = NULL;
        MI_U8 *pbuff = NULL;
        pbuff = malloc(s32BytePerPixel * s32MousePicW * s32MousePicW);
        if (!g_fbFd)
        {
            printf("Please init fb first.\n");
            return -1;
        }
        fp = fopen(pu8MouseFile, "rb");
        if (fp)
        {
            fread(pbuff, 1, s32BytePerPixel * s32MousePicW * s32MousePicW, fp);
            fclose(fp);
        }
        //set curosr Icon && set positon
        stCursorAttr.stCursorImageInfo.u32Width = s32MousePicW;
        stCursorAttr.stCursorImageInfo.u32Height = s32MousePicW;
        stCursorAttr.stCursorImageInfo.u32Pitch = s32MousePicW; //?????
        stCursorAttr.stCursorImageInfo.eColorFmt = E_MI_FB_COLOR_FMT_ARGB8888;
        stCursorAttr.stCursorImageInfo.data = pbuff;
        stCursorAttr.u32HotSpotX = 18;
        stCursorAttr.u32HotSpotY = 9;
        stCursorAttr.u32XPos = 100;
        stCursorAttr.u32YPos = 1080;
        stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_ICON
            | E_MI_FB_CURSOR_ATTR_MASK_SHOW | E_MI_FB_CURSOR_ATTR_MASK_POS;
        if (ioctl(g_fbFd, FBIOSET_CURSOR_ATTRIBUTE, &stCursorAttr)) {
            printf("Error FBIOSET_CURSOR_ATTRIBUTE\n");
            assert(0);
        }

        if (pbuff)
        {
            free(pbuff);
        }

    */
    return MI_SUCCESS;
}

MI_S32 ST_Fb_MouseSet(MI_U32 u32X, MI_U32 u32Y)
{
    MI_FB_CursorAttr_t stCursorAttr;

    stCursorAttr.u32XPos           = u32X;
    stCursorAttr.u32YPos           = u32Y;
    stCursorAttr.u16CursorAttrMask = E_MI_FB_CURSOR_ATTR_MASK_POS;
    if (ioctl(g_fbFd, FBIOSET_CURSOR_ATTRIBUTE, &stCursorAttr))
    {
        printf("Error FBIOSET_CURSOR_ATTRIBUTE\n");
        assert(0);
    }

    return MI_SUCCESS;
}

MI_S32 ST_FB_Show(MI_BOOL bShow)
{
    if (g_fbFd < 0)
    {
        return -1;
    }

    if (ioctl(g_fbFd, FBIOSET_SHOW, &bShow) < 0)
    {
        perror("Error: failed to FBIOSET_SHOW");
    }

    return MI_SUCCESS;
}

void ST_FB_SetAlphaInfo(MI_FB_GlobalAlpha_t *pstAlphaInfo)
{
    if (g_fbFd < 0)
    {
        return;
    }

    if (pstAlphaInfo == NULL)
    {
        return;
    }

    if (ioctl(g_fbFd, FBIOSET_GLOBAL_ALPHA, pstAlphaInfo) < 0)
    {
        perror("Error: failed to FBIOGET_GLOBAL_ALPHA");
    }
}

void ST_FB_GetAlphaInfo(MI_FB_GlobalAlpha_t *pstAlphaInfo)
{
    if (g_fbFd < 0)
    {
        return;
    }

    if (pstAlphaInfo == NULL)
    {
        return;
    }

    if (ioctl(g_fbFd, FBIOGET_GLOBAL_ALPHA, pstAlphaInfo) < 0)
    {
        perror("Error: failed to FBIOGET_GLOBAL_ALPHA");
    }
}

void ST_FB_ChangeResolution(int width, int height)
{
    if (g_fbFd < 0)
    {
        return;
    }

    printf("%s %d, width:%d,height:%d\n", __func__, __LINE__, width, height);
    MI_FB_DisplayLayerAttr_t stDisplayAttr;
    memset(&stDisplayAttr, 0, sizeof(MI_FB_DisplayLayerAttr_t));
    stDisplayAttr.u32SetAttrMask = E_MI_FB_DISPLAYLAYER_ATTR_MASK_SCREEN_SIZE
                                   | E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE
                                   | E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_SIZE | E_MI_FB_DISPLAYLAYER_ATTR_MASK_DISP_POS;
    stDisplayAttr.u32Xpos          = 0;
    stDisplayAttr.u32YPos          = 0;
    stDisplayAttr.u32dstWidth      = width;
    stDisplayAttr.u32dstHeight     = height;
    stDisplayAttr.u32DisplayWidth  = (width > 1920) ? 1920 : width;
    stDisplayAttr.u32DisplayHeight = (height > 1080) ? 1080 : height;
    stDisplayAttr.u32ScreenWidth   = width;
    stDisplayAttr.u32ScreenHeight  = height;
    // E_MI_FB_DISPLAYLAYER_ATTR_MASK_BUFFER_SIZE operaton will change
    // var info and fix.line_lingth, so Need Retrive fixinfo and varinfo
    if (ioctl(g_fbFd, FBIOSET_DISPLAYLAYER_ATTRIBUTES, &stDisplayAttr))
    {
        perror("Error: failed to FBIOSET_DISPLAYLAYER_ATTRIBUTES");
        return;
    }

    // get fb_fix_screeninfo
    if (ioctl(g_fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error reading fixed information");
        assert(0);
    }
    // printFixedInfo();

    // get fb_var_screeninfo
    if (ioctl(g_fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        assert(0);
    }
    // printVariableInfo();

    // clear framebuffer
    drawRect(0, 0, vinfo.xres, vinfo.yres, ARGB888_BLUE);
    // memset(frameBuffer, 0x0, finfo.smem_len);

    // Pandisplay
    if (ioctl(g_fbFd, FBIOPAN_DISPLAY, &vinfo) == -1)
    {
        perror("Error reading variable information");
        assert(0);
    }
}

#endif
