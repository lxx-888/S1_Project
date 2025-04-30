/*
 * module_adas.cpp- Sigmastar
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
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <pthread.h>

#include "module_common.h"
#include "module_adas.h"
#include "module_scl.h"
#include "mid_common.h"
#include "adas.h"
#include "ldws.h"
#include "bcam_adas.h"
#include "module_fb.h"
#include "module_font.h"
#include "IPC_cardvInfo.h"

#if CARDV_ADAS_ENABLE

#ifdef DEBUG_BUILD
#define __adas_print_str(x) printf(#x "\n");
#define __adas_print_int(x) printf(#x ": %d\n", x)
#elif RELEASE_BUILD
#define __adas_print_str(x)
#define __adas_print_int(x)
#else
#define __adas_print_str(x)
#define __adas_print_int(x)
#endif

typedef struct _ADAS_Context
{
    MI_SYS_ChnPort_t src_chn_port;

    struct
    {
        char      name[32];
        BOOL      thread_exit;
        pthread_t thread;
    } thread_param;

    struct
    {
        MI_BOOL ldws;
        MI_BOOL fcws;
        MI_BOOL sag;
        MI_BOOL bsd;
        MI_BOOL bcws;
    } feature;

    struct
    {
        int (*adas_get_buf_info)(unsigned short width, unsigned short height);

        ADAS_init_error_type (*adas_init)(unsigned short width, unsigned short height, unsigned char *working_buf_ptr,
                                          int working_buf_len, ADAS_init_params *params);

        ADAS_error_type (*adas_process)(const unsigned char *src, ADAS_set_info *params);

        int (*adas_enable)(ADAS_enable_params *ADAS_en);

        int (*adas_get_result)(ADAS_results *result);

        int (*adas_set_params)(ADAS_process_params *params);

        int (*adas_wait_process_done)(void);

        int (*adas_abort_process)(void);

        unsigned int (*adas_GetLibVersion)(unsigned int *ver);

        void (*adas_set_calibration)(int calibration_status);

        void (*adas_set_symmetry)(int symmetry_status);
    } api;
} ADAS_Context;

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

#if (ADAS_DRAW_INFO)
MI_SYS_WindowRect_t last_stRect    = {0, 0, 0, 0};
MI_SYS_WindowRect_t current_stRect = {0, 0, 0, 0};

ldws_params_t last_lane_info;
ldws_params_t current_lane_info;

Color_t white_Color = {255, 255, 255, 255};
Color_t black_Color = {255, 0, 0, 0};
Color_t red_Color   = {255, 255, 0, 0};
Color_t blue_Color  = {255, 0, 0, 255};

CarDV_FontData_t g_fontData_last;
CarDV_FontData_t g_fontData_new;
#endif

int                  adas_print_msg = 0;
static ADAS_Context *f_adas_ctx;
static ADAS_Context *r_adas_ctx;
ADAS_ATTR_t          gFrontAdasAttr = {.feature{.ldws = 0, .fcws = 0, .sag = 0, .bsd = 0, .bcws = 0}};
ADAS_ATTR_t          gRearAdasAttr  = {.feature{.ldws = 0, .fcws = 0, .sag = 0, .bsd = 0, .bcws = 0}};

//==============================================================================
//
//                              EXTERN VARIABLES
//
//==============================================================================

extern CarDV_Info carInfo;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

static void * Adas_Task(void *args);
static MI_S32 ADAS_SetFeature(ADAS_Context *adas_ctx, MI_U8 ldws, MI_U8 fcws, MI_U8 sag, MI_U8 bsd, MI_U8 bcws);
static MI_S32 ADAS_SetChannelPort(ADAS_Context *adas_ctx, MI_SYS_ChnPort_t *chanel_port);

#if (ADAS_DRAW_INFO)
void rotate(int origin_w, int origin_h, int origin_x, int origin_y, int *x, int *y)
{
#if (0) // 90 degree
    *x = origin_h - 1 - origin_y;
    *y = origin_x;
#elif (1) // 270 degree
    *x = origin_y;
    *y = origin_w - 1 - origin_x;
#else
    *x = origin_x;
    *y = origin_y;
#endif
}

void Show_FCar_Distance(MI_U32 distance)
{
    char   text_tt[2]  = {0};
    MI_U32 u32BuffSize = 0;
    MI_U8  text[2]     = {0};

    sprintf(text_tt, "%d", distance);
    strcpy((char *)text, text_tt);
    CARDV_WARN("distance:%d text is %s text_tt:%s \n", distance, text, text_tt);

    Color_t fColor;
    Color_t bColor;
    memset(&fColor, 0, sizeof(Color_t));
    memset(&bColor, 0, sizeof(Color_t));

    g_fontData_new.offset = 0;
    g_fontData_new.pmt    = E_MI_RGN_PIXEL_FORMAT_ARGB1555;
    int hzLen             = cardv_Font_Strlen(text); // calculate bytes

    // nFontSize = font width *nMultiple, 8, 12, 16, 24, 32, 48...
    int nFontSize = cardv_Font_GetSizeByType(FONT_SIZE_32);

    g_fontData_new.height = nFontSize;
    if (E_MI_RGN_PIXEL_FORMAT_ARGB1555 == g_fontData_new.pmt)
    {
        g_fontData_new.stride = nFontSize * hzLen * 2; // 1555 so,2bytes
        memcpy(&fColor, &white_Color, sizeof(Color_t));
    }
    else if (E_MI_RGN_PIXEL_FORMAT_I4 == g_fontData_new.pmt)
    {
        g_fontData_new.stride = nFontSize * hzLen / 2;
        fColor.a              = 0x00;
        fColor.r              = 15 & 0x0F; // refer to module_osd:OSD_DrawVideoStamp
        fColor.g              = 0x00;
        fColor.b              = 0x00;
    }
    CARDV_WARN("fmt:%d,len:%d,nFontsize:%d,height:%d,width:%d \n", g_fontData_new.pmt, hzLen, nFontSize,
               g_fontData_new.height, g_fontData_new.stride);

    if (g_fontData_new.height != 0)
    {
        u32BuffSize = g_fontData_new.stride * g_fontData_new.height;

        if (NULL == (g_fontData_new.buffer = (MI_U8 *)malloc(u32BuffSize)))
        {
            CARDV_ERR("malloc buffer err\n");
            return;
        }
        // CARDV_WARN("malloc buffer :%p size:%d \n",g_fontData_new.buffer,u32BuffSize);

        MI_U16 *pt  = (MI_U16 *)g_fontData_new.buffer;
        MI_U16  end = (u32BuffSize) / 2;

        for (int ii = 0; ii < end; ii++)
        {
            *pt = 0x801F;
            pt++;
        }
    }

    memcpy(&bColor, &blue_Color, sizeof(Color_t));

    if (MI_SUCCESS != cardv_Font_DrawText(&g_fontData_new, text, 0, FONT_SIZE_32, &fColor, &bColor, FALSE))
    {
        CARDV_WARN("cardv_Font_DrawText error \n");
    }

#if (CARDV_FB_ENABLE)
    ST_Fb_DrawNum(&g_fontData_new, TRUE);
#endif

    if (NULL != g_fontData_new.buffer)
    {
        free(g_fontData_new.buffer);
        g_fontData_new.buffer = NULL;
    }
}

void Draw_FCar_Distance(MI_U8 distance, BOOL show)
{
#if (CARDV_FB_ENABLE)
    if (show == FALSE)
    {
        ST_FB_SyncDirtyDown();
        ST_Fb_DrawNum(&g_fontData_last, FALSE);
        memset(&g_fontData_new, 0, sizeof(g_fontData_new));
        memcpy(&g_fontData_last, &g_fontData_new, sizeof(g_fontData_last));
    }
    else
    {
        ST_FB_SyncDirtyDown();
        ST_Fb_DrawNum(&g_fontData_last, FALSE);
        Show_FCar_Distance(distance);
        memcpy(&g_fontData_last, &g_fontData_new, sizeof(g_fontData_last));
        // memset(&g_fontData_new,0,sizeof(g_fontData_new));//note this !
    }

    MI_SYS_WindowRect_t canvas;
    canvas.u16X      = 0;
    canvas.u16Y      = 0;
    canvas.u16Width  = 896;
    canvas.u16Height = 512;
    ST_FB_SyncDirtyUp(&canvas);
    ST_Fb_SetColorKey(ARGB888_BLUE);
    ST_FB_Show(TRUE);
#endif
}

void Draw_Lane_Line(ldws_params_t lane_info, BOOL isDraw)
{
#if (CARDV_FB_ENABLE)
    MI_SYS_WindowRect_t canvas;
    MI_U32              u32Color = 0;
    int                 x1 = 0, y1 = 0, x2 = 0, y2 = 0;

    // Fixme: less than zero mean what?
    if ((lane_info.left_lane[0].x < 0) || (lane_info.left_lane[0].y < 0) || (lane_info.left_lane[1].x < 0)
        || (lane_info.left_lane[1].y < 0) || (lane_info.right_lane[0].x < 0) || (lane_info.right_lane[0].y < 0)
        || (lane_info.right_lane[1].x < 0) || (lane_info.right_lane[1].y < 0))
    {
        // CARDV_ERR("Please tell me what is this mean and how do deal with it ! \n");
        return;
    }

    if ((lane_info.left_lane[0].x >= 432) || (lane_info.left_lane[0].y >= 240) || (lane_info.left_lane[1].x >= 432)
        || (lane_info.left_lane[1].y >= 240) || (lane_info.right_lane[0].x >= 432) || (lane_info.right_lane[0].y >= 240)
        || (lane_info.right_lane[1].x >= 432) || (lane_info.right_lane[1].y >= 240))
    {
        return;
    }

    if (isDraw == FALSE)
    {
        // if not need to draw ,so clean up last Rectangle!
        ST_FB_SyncDirtyDown();
        if ((lane_info.left_lane[0].x == lane_info.left_lane[1].x) && (lane_info.left_lane[0].x == 0))
        {
            // doing nothing;
        }
        else if ((lane_info.right_lane[0].x == lane_info.right_lane[1].x) && (lane_info.right_lane[0].x == 0))
        {
            // doing nothing;
        }
        else
        {
            rotate(896, 512, last_lane_info.left_lane[0].x * 896 / 432, last_lane_info.left_lane[0].y * 512 / 240, &x1,
                   &y1);
            rotate(896, 512, last_lane_info.left_lane[1].x * 896 / 432, last_lane_info.left_lane[1].y * 512 / 240, &x2,
                   &y2);
            printf("L%d [%d,%d]->[%d,%d]\n", __LINE__, x1, y1, x2, y2);
            ST_Fb_FillLine2(x1, y1, x2, y2, ARGB888_BLUE, 1); // left line
            rotate(896, 512, last_lane_info.right_lane[0].x * 896 / 432, last_lane_info.right_lane[0].y * 512 / 240,
                   &x1, &y1);
            rotate(896, 512, last_lane_info.right_lane[1].x * 896 / 432, last_lane_info.right_lane[1].y * 512 / 240,
                   &x2, &y2);
            printf("L%d [%d,%d]->[%d,%d]\n", __LINE__, x1, y1, x2, y2);
            ST_Fb_FillLine2(x1, y1, x2, y2, ARGB888_BLUE, 1); // right line
        }
        memset(&current_lane_info, 0, sizeof(ldws_params_t));
    }
    else
    {
        ST_FB_SyncDirtyDown();
        // clean up last lane
        rotate(896, 512, last_lane_info.left_lane[0].x * 896 / 432, last_lane_info.left_lane[0].y * 512 / 240, &x1,
               &y1);
        rotate(896, 512, last_lane_info.left_lane[1].x * 896 / 432, last_lane_info.left_lane[1].y * 512 / 240, &x2,
               &y2);
        printf("L%d [%d,%d]->[%d,%d]\n", __LINE__, x1, y1, x2, y2);
        ST_Fb_FillLine2(x1, y1, x2, y2, ARGB888_BLUE, 1); // left line
        rotate(896, 512, last_lane_info.right_lane[0].x * 896 / 432, last_lane_info.right_lane[0].y * 512 / 240, &x1,
               &y1);
        rotate(896, 512, last_lane_info.right_lane[1].x * 896 / 432, last_lane_info.right_lane[1].y * 512 / 240, &x2,
               &y2);
        printf("L%d [%d,%d]->[%d,%d]\n", __LINE__, x1, y1, x2, y2);
        ST_Fb_FillLine2(x1, y1, x2, y2, ARGB888_BLUE, 1); // right line

        // update lane
        u32Color = 0xFFFF00; // yellow color
        memcpy(&current_lane_info, &lane_info, sizeof(ldws_params_t));
        rotate(896, 512, current_lane_info.left_lane[0].x * 896 / 432, current_lane_info.left_lane[0].y * 512 / 240,
               &x1, &y1);
        rotate(896, 512, current_lane_info.left_lane[1].x * 896 / 432, current_lane_info.left_lane[1].y * 512 / 240,
               &x2, &y2);
        printf("L%d [%d,%d]->[%d,%d]\n", __LINE__, x1, y1, x2, y2);
        ST_Fb_FillLine2(x1, y1, x2, y2, u32Color, 1); // left line
        rotate(896, 512, current_lane_info.right_lane[0].x * 896 / 432, current_lane_info.right_lane[0].y * 512 / 240,
               &x1, &y1);
        rotate(896, 512, current_lane_info.right_lane[1].x * 896 / 432, current_lane_info.right_lane[1].y * 512 / 240,
               &x2, &y2);
        printf("L%d [%d,%d]->[%d,%d]\n", __LINE__, x1, y1, x2, y2);
        ST_Fb_FillLine2(x1, y1, x2, y2, u32Color, 1); // right line
    }

    canvas.u16X      = 0;
    canvas.u16Y      = 0;
    canvas.u16Width  = 896;
    canvas.u16Height = 512;
    ST_FB_SyncDirtyUp(&canvas);
    ST_Fb_SetColorKey(ARGB888_BLUE);
    ST_FB_Show(TRUE);
    // update data
    memcpy(&last_lane_info, &current_lane_info, sizeof(ldws_params_t));
#endif
}

void Draw_FCar_Rectangle(fcws_info_t fcws_info, BOOL isDraw)
{
#if (CARDV_FB_ENABLE)
    MI_SYS_WindowRect_t canvas;
    MI_U32              u32Color = 0;

    if (isDraw == FALSE)
    {
        // if not need to draw ,so clean up last Rectangle!
        ST_FB_SyncDirtyDown();
        ST_Fb_DrawRect(&last_stRect, ARGB888_BLUE);
        memset(&current_stRect, 0, sizeof(MI_SYS_WindowRect_t));
    }
    else
    {
        // need to clean last Firstly!
        ST_FB_SyncDirtyDown();
        ST_Fb_DrawRect(&last_stRect, ARGB888_BLUE);
        u32Color                 = 0xFFFFFF; // white color
        current_stRect.u16X      = fcws_info.car_x * 896 / 432;
        current_stRect.u16Y      = fcws_info.car_y * 512 / 240;
        current_stRect.u16Width  = fcws_info.car_width * 896 / 432;
        current_stRect.u16Height = fcws_info.car_height * 512 / 240;
        ST_Fb_DrawRect(&current_stRect, u32Color);
    }

    canvas.u16X      = 0;
    canvas.u16Y      = 0;
    canvas.u16Width  = 896;
    canvas.u16Height = 512;
    ST_FB_SyncDirtyUp(&canvas);
    ST_Fb_SetColorKey(ARGB888_BLUE);
    ST_FB_Show(TRUE);

    // update data
    memcpy(&last_stRect, &current_stRect, sizeof(MI_SYS_WindowRect_t));
#endif
}
#endif

static ADAS_Context *ADAS_Create(MI_BOOL bIsForntAdas, ADAS_ATTR_t *pAttr)
{
    ADAS_Context *p_adas_ctx = (ADAS_Context *)malloc(sizeof(ADAS_Context));

    if (p_adas_ctx)
    {
        p_adas_ctx->thread_param.thread_exit = 0;
        p_adas_ctx->thread_param.thread      = 0;

        if (pAttr)
        {
            ADAS_SetFeature(p_adas_ctx, pAttr->feature.ldws, pAttr->feature.fcws, pAttr->feature.sag,
                            pAttr->feature.bsd, pAttr->feature.bcws);

            ADAS_SetChannelPort(p_adas_ctx, &pAttr->stSrcChnPort);
        }

        if (bIsForntAdas)
        {
            strcpy(p_adas_ctx->thread_param.name, "front_adas_task");
            p_adas_ctx->api.adas_abort_process     = ADAS_abort_process;
            p_adas_ctx->api.adas_enable            = ADAS_enable;
            p_adas_ctx->api.adas_GetLibVersion     = ADAS_GetLibVersion;
            p_adas_ctx->api.adas_get_buf_info      = ADAS_get_buffer_info;
            p_adas_ctx->api.adas_get_result        = ADAS_get_result;
            p_adas_ctx->api.adas_init              = ADAS_init;
            p_adas_ctx->api.adas_process           = ADAS_process;
            p_adas_ctx->api.adas_set_calibration   = ADAS_set_calibration;
            p_adas_ctx->api.adas_set_params        = ADAS_set_params;
            p_adas_ctx->api.adas_set_symmetry      = ADAS_set_symmetry;
            p_adas_ctx->api.adas_wait_process_done = ADAS_wait_process_done;
        }
        else
        {
            strcpy(p_adas_ctx->thread_param.name, "rear_adas_task");
            p_adas_ctx->api.adas_abort_process     = BCam_ADAS_abort_process;
            p_adas_ctx->api.adas_enable            = BCam_ADAS_enable;
            p_adas_ctx->api.adas_GetLibVersion     = BCam_ADAS_GetLibVersion;
            p_adas_ctx->api.adas_get_buf_info      = BCam_ADAS_get_buffer_info;
            p_adas_ctx->api.adas_get_result        = BCam_ADAS_get_result;
            p_adas_ctx->api.adas_init              = BCam_ADAS_init;
            p_adas_ctx->api.adas_process           = BCam_ADAS_process;
            p_adas_ctx->api.adas_set_calibration   = BCam_ADAS_set_calibration;
            p_adas_ctx->api.adas_set_params        = BCam_ADAS_set_params;
            p_adas_ctx->api.adas_set_symmetry      = BCam_ADAS_set_symmetry;
            p_adas_ctx->api.adas_wait_process_done = BCam_ADAS_wait_process_done;
        }
    }
    return p_adas_ctx;
}

static MI_S32 ADAS_Destory(ADAS_Context **pp_adas_ctx)
{
    free(*pp_adas_ctx);
    *pp_adas_ctx = NULL;
    return 0;
}

static MI_S32 ADAS_SetFeature(ADAS_Context *adas_ctx, MI_U8 ldws, MI_U8 fcws, MI_U8 sag, MI_U8 bsd, MI_U8 bcws)
{
    adas_ctx->feature.ldws = ldws;
    adas_ctx->feature.fcws = fcws;
    adas_ctx->feature.sag  = sag;
    adas_ctx->feature.bsd  = bsd;
    adas_ctx->feature.bcws = bcws;
    return 0;
}

static MI_S32 ADAS_SetChannelPort(ADAS_Context *adas_ctx, MI_SYS_ChnPort_t *chanel_port)
{
    adas_ctx->src_chn_port = *chanel_port;
    return 0;
}

static MI_S32 ADAS_Run(ADAS_Context *adas_ctx)
{
    int ret;

    ret = pthread_create(&adas_ctx->thread_param.thread, NULL, Adas_Task, adas_ctx);
    if (0 == ret)
    {
        pthread_setname_np(adas_ctx->thread_param.thread, adas_ctx->thread_param.name);
        return 0;
    }
    else
    {
        return -1;
    }
}

static MI_S32 ADAS_Stop(ADAS_Context *adas_ctx)
{
    adas_ctx->thread_param.thread_exit = TRUE;
    pthread_join(adas_ctx->thread_param.thread, NULL);
    adas_ctx->api.adas_abort_process();
    adas_ctx->api.adas_wait_process_done();
    return 0;
}

static S32 ADAS_SourceImageWH(ADAS_Context *adas_ctx, U32 *width, U32 *height)
{
    MI_SYS_ChnPort_t *pSrcChnPort = &adas_ctx->src_chn_port;

    if (E_MI_MODULE_ID_SCL == pSrcChnPort->eModId)
    {
        MI_SCL_OutPortParam_t stSclOutputParam;
        memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
        MI_SCL_GetOutputPortParam((MI_SCL_DEV)pSrcChnPort->u32DevId, pSrcChnPort->u32ChnId, pSrcChnPort->u32PortId,
                                  &stSclOutputParam);
        *width  = stSclOutputParam.stSCLOutputSize.u16Width;
        *height = stSclOutputParam.stSCLOutputSize.u16Height;

        return 0;
    }
    else
    {
        printf("NOT support ADAS input module [%d]\n", pSrcChnPort->eModId);
        *width  = 0;
        *height = 0;

        return -1;
    }
}

static S32 ADAS_GetSrcImage(ADAS_Context *adas_ctx, U8 **source_image, MI_SYS_BUF_HANDLE *hHandle)
{
    MI_S32           s32Ret = 0;
    MI_SYS_BufInfo_t stBufInfo;

    s32Ret = MI_SYS_ChnOutputPortGetBuf(&adas_ctx->src_chn_port, &stBufInfo, hHandle);
    if (MI_SUCCESS == s32Ret)
    {
        void *pViraddr = NULL;
        if (stBufInfo.eBufType == E_MI_SYS_BUFDATA_FRAME)
        {
            pViraddr      = stBufInfo.stFrameData.pVirAddr[0];
            *source_image = static_cast<unsigned char *>(pViraddr);
            return 0;
        }
    }
    *source_image = 0;
    return -1;
}

static S32 ADAS_ReturnSrcImage(MI_SYS_BUF_HANDLE hHandle)
{
    MI_SYS_ChnOutputPortPutBuf(hHandle);
    return 0;
}

bool ADAS_IsAnyFeatureEnable(ADAS_ATTR_t *adas_attr)
{
    if (adas_attr->feature.ldws || adas_attr->feature.fcws || adas_attr->feature.sag || adas_attr->feature.bsd
        || adas_attr->feature.bcws)
        return TRUE;
    else
        return FALSE;
}

static ADAS_error_type ADAS_DoOneFrame(ADAS_Context *adas_ctx, unsigned char *source_image)
{
    ADAS_error_type ADAS_err;
    ADAS_results    ADAS_result;
    ADAS_set_info   ADAS_info;

#if 0 //(GPS_CONNECT_ENABLE)
    if (GPSCtrl_GPS_IsValidValue())
    {
        ADAS_info.gps_params.gps_state = 1; // With GPS and GPS signal
        ADAS_info.gps_params.gps_speed = GPSCtrl_GetSpeed(0);
    }
    else
    {
        ADAS_info.gps_params.gps_state = 0; // With GPS but no GPS signal
        ADAS_info.gps_params.gps_speed = -1;
    }
#else
    ADAS_info.gps_params.gps_state = 0; // Without GPS
    ADAS_info.gps_params.gps_speed = -1;
#endif
    ADAS_info.day_or_night = 1;

    // ADAS_err = ADAS_process(source_image, &ADAS_info);
    ADAS_err = adas_ctx->api.adas_process(source_image, &ADAS_info);
    if (ADAS_err == ADAS_ERR_NONE)
    {
        // ADAS_get_result(&ADAS_result);
        adas_ctx->api.adas_get_result(&ADAS_result);
        // if user disable certain function, the pointer of the function result will be NULL.
        if (ADAS_result.LDWS_result != NULL)
        {
            if ((ADAS_result.LDWS_result->ldws_params.state != LDWS_STATE_NODETECT)
                && (ADAS_result.LDWS_result->ldws_err_state == LDWS_ERR_NONE))
            {
                ADAS_DBG_MSG(adas_print_msg & 0x2,
                             "[LDWS] Err State [%d] State [%d] Left [%03d, %03d] to [%03d, %03d] Right [%03d, %03d] to "
                             "[%03d, %03d]\n",
                             ADAS_result.LDWS_result->ldws_err_state, ADAS_result.LDWS_result->ldws_params.state,
                             ADAS_result.LDWS_result->ldws_params.left_lane[0].x,
                             ADAS_result.LDWS_result->ldws_params.left_lane[0].y,
                             ADAS_result.LDWS_result->ldws_params.left_lane[1].x,
                             ADAS_result.LDWS_result->ldws_params.left_lane[1].y,
                             ADAS_result.LDWS_result->ldws_params.right_lane[0].x,
                             ADAS_result.LDWS_result->ldws_params.right_lane[0].y,
                             ADAS_result.LDWS_result->ldws_params.right_lane[1].x,
                             ADAS_result.LDWS_result->ldws_params.right_lane[1].y);
            }
#if (ADAS_DRAW_INFO)
            ldws_params_t cur_line_info;
            memcpy(&cur_line_info, &(ADAS_result.LDWS_result->ldws_params), sizeof(ldws_params_t));
            Draw_Lane_Line(cur_line_info, TRUE);
#else
            memcpy(&carInfo.stAdasInfo.stLdwsInfo, &(ADAS_result.LDWS_result->ldws_params), sizeof(ldws_params_t));
            carInfo.stAdasInfo.bLdwsVaild = TRUE;
#endif
        }
        else
        {
#if (ADAS_DRAW_INFO)
            ldws_params_t cur_line_info;
            memset(&cur_line_info, 0, sizeof(ldws_params_t));
            Draw_Lane_Line(cur_line_info, FALSE);
#else
            carInfo.stAdasInfo.bLdwsVaild = FALSE;
            memset(&carInfo.stAdasInfo.stLdwsInfo, 0, sizeof(ldws_params_t));
#endif
        }

        if (ADAS_result.FCWS_result_C != NULL)
        {
            if (ADAS_result.FCWS_result_C->state == FCWS_STATE_FIND && ADAS_result.FCWS_result_C->distance != 99)
            {
                ADAS_DBG_MSG(adas_print_msg & 0x2,
                             "[FCWS] State [%d] Pos [%03d, %03d] WH [%03d, %03d] Distance [%03d]\n",
                             ADAS_result.FCWS_result_C->state, ADAS_result.FCWS_result_C->car_x,
                             ADAS_result.FCWS_result_C->car_y, ADAS_result.FCWS_result_C->car_width,
                             ADAS_result.FCWS_result_C->car_height, ADAS_result.FCWS_result_C->distance);
#if (ADAS_DRAW_INFO)
                fcws_info_t cur_fcws_info;
                memcpy(&cur_fcws_info, ADAS_result.FCWS_result_C, sizeof(fcws_info_t));
                Draw_FCar_Rectangle(cur_fcws_info, TRUE);
                Draw_FCar_Distance(ADAS_result.FCWS_result_C->distance, TRUE);
#else
                memcpy(&carInfo.stAdasInfo.stFcwsInfo, ADAS_result.FCWS_result_C, sizeof(fcws_info_t));
                carInfo.stAdasInfo.bFcwsVaild = TRUE;
#endif
            }
            else if (ADAS_result.FCWS_result_C->state == FCWS_STATE_LOSE || ADAS_result.FCWS_result_C->distance == 99)
            {
#if (ADAS_DRAW_INFO)
                // find front car, but distance(99) is too long
                fcws_info_t cur_fcws_info;
                memset(&cur_fcws_info, 0, sizeof(fcws_info_t));
                Draw_FCar_Rectangle(cur_fcws_info, FALSE);
                Draw_FCar_Distance(99, FALSE);
#else
                carInfo.stAdasInfo.bFcwsVaild = FALSE;
                memset(&carInfo.stAdasInfo.stFcwsInfo, 0, sizeof(fcws_info_t));
#endif
            }
        }
        else
        {
#if (ADAS_DRAW_INFO)
            // not find front car
            fcws_info_t cur_fcws_info;
            Draw_FCar_Rectangle(cur_fcws_info, FALSE);
#else
            carInfo.stAdasInfo.bFcwsVaild = FALSE;
            memset(&carInfo.stAdasInfo.stFcwsInfo, 0, sizeof(fcws_info_t));
#endif
        }

        if (ADAS_result.SAG_result != NULL)
        {
            ADAS_DBG_MSG(adas_print_msg & 0x2, "[SAG] State [%d]\n", ADAS_result.SAG_result->true_state);
        }

#if !(ADAS_DRAW_INFO)
        IPC_CarInfo_Write_AdasInfo(&carInfo.stAdasInfo);
#endif
    }

    return ADAS_err;
}

static void *Adas_Task(void *args)
{
    int                buf_len;
    unsigned char *    buf_ptr;
    ADAS_init_params   stAdasInitParam;
    ADAS_enable_params stAdasEnableParam;
    adas_input_t       adas_input;
    LdwsSetupPosi      ldws_setup_posi;
    int                m_Calibrated_y_start, m_Calibrated_y_end, m_Camera_height;
    MI_U32             u32AdasWidth, u32AdasHeight;
    int                ret;
    ADAS_Context *     adas_ctx = (ADAS_Context *)args;

    CARDV_THREAD();

#if (ADAS_DRAW_INFO)
    ret = ST_Fb_Init();
    if (ret != 0)
    {
        CARDV_ERR("ST_Fb_Init err [%d]\n", ret);
    }

    // Test only begin!
    if (0)
    {
        MI_SYS_WindowRect_t stRect;
        MI_U32              u32Color = 0;
        u32Color                     = 0xFFFFFF; // white
        // u32Color = 0x0000FF;//blue
        stRect.u16X      = 0;
        stRect.u16Y      = 0;
        stRect.u16Width  = 896; // 432  -->896
        stRect.u16Height = 512; // 240 -->512
        ST_FB_SyncDirtyDown();
        ST_Fb_FillLine2(202 * 896 / 432, 134 * 512 / 240, 124 * 896 / 432, 195 * 512 / 240, u32Color,
                        3); //(202,134) (124,195) left line
        ST_Fb_FillLine2(202 * 896 / 432, 134 * 512 / 240, 293 * 896 / 432, 195 * 512 / 240, u32Color,
                        3); //(202,134) (293,195) right line
        ST_Fb_FillLine2(124 * 896 / 432, 195 * 512 / 240, 293 * 896 / 432, 195 * 512 / 240, u32Color,
                        3); //(124,195) (293,195) base line
        ST_FB_SyncDirtyUp(&stRect);
        ST_Fb_SetColorKey(ARGB888_BLUE);
        ST_FB_Show(TRUE);

        // Test only begin !
        // CARDV_WARN("yyj test car distance car\n");
        // Draw_FCar_Distance(79,TRUE);
        // Test only end !
    }
// Test only end !
#endif

    ret = ADAS_SourceImageWH(adas_ctx, &u32AdasWidth, &u32AdasHeight);
    if (ret < 0)
    {
        printf("ADAS: get width, height error\n"); // goto err_exit;
        pthread_exit(NULL);
    }

    //////////////////////// value from user calibration program ////////////////////////
    m_Calibrated_y_start                     = 120;
    m_Calibrated_y_end                       = 210;
    m_Camera_height                          = 120; // cetimeter
    stAdasInitParam.user_calib_y_start       = m_Calibrated_y_start;
    stAdasInitParam.user_calib_y_end         = m_Calibrated_y_end;
    stAdasInitParam.user_calib_camera_height = m_Camera_height;
    //////////////////////// value from user calibration program ////////////////////////

    //////////////////////// value from LDWS calibration ////////////////////////
    ldws_setup_posi                                           = LDWS_SETUP_CENTER;
    carInfo.stAdasInfo.stCalibInfo.laneCalibrationImageWidth  = 432;  // laneCalibrationImageWidth
    carInfo.stAdasInfo.stCalibInfo.laneCalibrationImageHeight = 240;  // laneCalibrationImageHeight
    carInfo.stAdasInfo.stCalibInfo.laneCalibrationUpPointX    = 202;  // laneCalibrationUpPointX
    carInfo.stAdasInfo.stCalibInfo.laneCalibrationUpPointY    = 134;  // laneCalibrationUpPointY
    carInfo.stAdasInfo.stCalibInfo.laneCalibrationLeftPointX  = 124;  // laneCalibrationLeftPointX
    carInfo.stAdasInfo.stCalibInfo.laneCalibrationLeftPointY  = 195;  // laneCalibrationLeftPointY
    carInfo.stAdasInfo.stCalibInfo.laneCalibrationRightPointX = 293;  // laneCalibrationRightPointX
    carInfo.stAdasInfo.stCalibInfo.laneCalibrationRightPointY = 195;  // laneCalibrationRightPointY
    carInfo.stAdasInfo.stCalibInfo.departureHighThr1          = 768;  // departureHighThr1
    carInfo.stAdasInfo.stCalibInfo.departureHighThr2          = 1024; // departureHighThr2
    carInfo.stAdasInfo.stCalibInfo.departureMiddleThr1        = 307;  // departureMiddleThr1
    carInfo.stAdasInfo.stCalibInfo.departureMiddleThr2        = 332;  // departureMiddleThr2
    carInfo.stAdasInfo.stCalibInfo.departureLowThr1           = 230;  // departureLowThr1
    carInfo.stAdasInfo.stCalibInfo.departureLowThr2           = 282;  // departureLowThr2
    carInfo.stAdasInfo.stCalibInfo.minLaneRatioX256           = 204;  // minLaneRatioX256
    carInfo.stAdasInfo.stCalibInfo.maxLaneRatioX256           = 512;  // maxLaneRatioX256
    carInfo.stAdasInfo.stCalibInfo.alarmShowCnt               = 10;   // alarmShowCnt

#if !(ADAS_DRAW_INFO)
    carInfo.stAdasInfo.bCalibVaild = TRUE;
    IPC_CarInfo_Write_AdasInfo(&carInfo.stAdasInfo);
#endif

    adas_input.image_width       = u32AdasWidth;  // input width of source image
    adas_input.image_height      = u32AdasHeight; // input height of source image
    adas_input.dz_N              = 9;             // downsample ratio N
    adas_input.dz_M              = 40;            // downsample ratio M
    adas_input.camera_focal      = 330;           // focal length (mm * 100)
    adas_input.sensor_cell_width = 280;           // sensor cell size (um * 100)

    stAdasInitParam.LDWS_input_params.LDWS_params = carInfo.stAdasInfo.stCalibInfo;
    stAdasInitParam.LDWS_input_params.LDWS_pos    = ldws_setup_posi;
    stAdasInitParam.ADAS_input_params             = adas_input;

    memset(&stAdasEnableParam, 0x00, sizeof(ADAS_enable_params));
    stAdasEnableParam.LDWS_enable = adas_ctx->feature.ldws;
    stAdasEnableParam.FCWS_enable = adas_ctx->feature.fcws;
    stAdasEnableParam.SAG_enable  = adas_ctx->feature.sag;
    stAdasEnableParam.BSD_enable  = adas_ctx->feature.bsd;
    stAdasEnableParam.BCWS_enable = adas_ctx->feature.bcws;
    ret                           = adas_ctx->api.adas_enable(&stAdasEnableParam);
    ADAS_DBG_MSG(adas_print_msg & 0x1, "ADAS Enable Ret [%d]\n", ret);

    buf_len = adas_ctx->api.adas_get_buf_info(u32AdasWidth, u32AdasHeight);
    ADAS_DBG_MSG(adas_print_msg & 0x1, "Buf Len [%d]\n", buf_len);

    buf_ptr = (unsigned char *)malloc(buf_len);
    ADAS_DBG_MSG(adas_print_msg & 0x1, "Buf Ptr [%p]\n", buf_ptr);

    adas_ctx->api.adas_set_calibration(1);

    ADAS_init_error_type ADAS_init_err =
        adas_ctx->api.adas_init(u32AdasWidth, u32AdasHeight, buf_ptr, buf_len, &stAdasInitParam);
    ADAS_DBG_MSG(adas_print_msg & 0x1, "ADAS Init Ret [%d]\n", ADAS_init_err);

    // get image data
    MI_S32            s32Ret = 0;
    MI_SYS_BUF_HANDLE hHandle;
    unsigned char *   source_image = NULL;

    MI_S32         s32Fd = -1;
    fd_set         read_fds;
    struct timeval TimeoutVal;

    s32Ret = MI_SYS_GetFd(&adas_ctx->src_chn_port, &s32Fd);
    if (MI_SUCCESS != s32Ret)
    {
        ADAS_DBG_ERR(1, "MI_SYS_GetFd err [%x]\n", s32Ret);
    }

    while (adas_ctx->thread_param.thread_exit == FALSE)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32Fd, &read_fds);

        TimeoutVal.tv_sec  = 0;
        TimeoutVal.tv_usec = 1000 * 10;

        s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            ADAS_DBG_ERR(1, "select failed!\n");
            continue;
        }
        else if (s32Ret == 0)
        {
            // Time Out
            continue;
        }
        else
        {
            if (FD_ISSET(s32Fd, &read_fds))
            {
                ret = ADAS_GetSrcImage(adas_ctx, &source_image, &hHandle);
                if (ret >= 0)
                {
                    ADAS_error_type ADAS_err;

                    // printf("ADAS_DoOneFrame\n");

                    if ((ADAS_err = ADAS_DoOneFrame(adas_ctx, source_image)) != 0)
                    {
                        ADAS_DBG_ERR(1, "ADAS Run Error [%d]\n", ADAS_err);
                    }

                    // MI_SYS_ChnOutputPortPutBuf(hHandle);
                    //}
                    ADAS_ReturnSrcImage(hHandle);
                }
            }
            else
            {
                printf("fd_isset fail\n");
            }
        }
    }

#if (ADAS_DRAW_INFO)
    ret = ST_Fb_DeInit();
    if (ret != 0)
    {
        CARDV_ERR("ST_Fb_Init err [%d]\n", ret);
    }
#endif

    MI_SYS_CloseFd(s32Fd);
    pthread_exit(NULL);
}

MI_S32 adas_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    MI_S32         s32ret       = 0;
    MI_BOOL        bIsFrontAdas = 1;
    ADAS_ATTR_t    attr;
    ADAS_Context **pp_adas_ctx = &f_adas_ctx;

    // if (adas_attr->chn_port.eModId == E_MI_MODULE_ID_MAX)
    //    return -1;

    switch (id)
    {
    case CMD_ADAS_REAR_INIT:
        bIsFrontAdas = 0;
        pp_adas_ctx  = &r_adas_ctx;
    case CMD_ADAS_INIT:
        if (*pp_adas_ctx == NULL)
        {
            memcpy(&attr, param, sizeof(ADAS_ATTR_t));

            if (attr.stSrcChnPort.eModId == E_MI_MODULE_ID_MAX)
                return -1;

            *pp_adas_ctx = ADAS_Create(bIsFrontAdas, &attr);

            if (*pp_adas_ctx)
            {
                // ADAS_SetFeature(*pp_adas_ctx, attr.feature.ldws, attr.feature.fcws, attr.feature.sag,
                // attr.feature.bsd,
                //                attr.feature.bcws);
                // ADAS_SetChannelPort(*pp_adas_ctx, &attr.stSrcChnPort);
                s32ret = ADAS_Run(*pp_adas_ctx);
                if (s32ret < 0)
                {
                    ADAS_Destory(pp_adas_ctx);
                }
            }
        }
        break;

    case CMD_ADAS_REAR_DEINIT:
        pp_adas_ctx = &r_adas_ctx;
    case CMD_ADAS_DEINIT:
        if (*pp_adas_ctx)
        {
            ADAS_Stop(*pp_adas_ctx);
            ADAS_Destory(pp_adas_ctx);
        }
        break;

    default:
        break;
    }

    return 0;
}
#endif