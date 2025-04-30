/*
 * mid_common.h- Sigmastar
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
#ifndef _MID_COMMON_H_
#define _MID_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <fcntl.h>
#include <error.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "mi_sys.h"
#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"
#include "mi_aio_datatype.h"
#include "mi_rgn_datatype.h"
#include "mi_venc_datatype.h"
#include "mi_vif_datatype.h"
#include "mi_vpe_datatype.h"
#include "mi_sensor_datatype.h"
#include "mi_mipitx_datatype.h"
//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#ifndef S8
#define S8 signed char
#endif

#ifndef U8
#define U8 unsigned char
#endif

#ifndef S16
#define S16 signed short
#endif

#ifndef U16
#define U16 unsigned short
#endif

#ifndef S32
#define S32 signed int
#endif

#ifndef U32
#define U32 unsigned int
#endif

#ifndef S64
#define S64 signed long long
#endif

#ifndef U64
#define U64 unsigned long long
#endif

/*
#ifndef MI_RET
#define MI_RET      signed int
#endif
*/

#ifndef BOOL
#define BOOL unsigned int
#endif
// typedef int BOOL;

#ifndef VOID
#define VOID void
#endif
// typedef void    VOID;

typedef signed char    CHAR;
typedef unsigned char  BYTE;
typedef signed short   SHORT;
typedef unsigned short WORD;
typedef signed long    LONG;
typedef unsigned int   DWORD;
typedef signed int     INT;
typedef unsigned int   UINT;
// typedef int BOOL;
typedef signed char    INT8;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;
typedef unsigned short WCHAR;
typedef signed int     INTEGER;

#ifndef F32
#define F32 float
#endif

#ifndef HANDLE
#define HANDLE void *
#endif

#ifndef MALLOC
#define MALLOC(s) malloc(s)
#endif

#ifndef FREEIF
#define FREEIF(m) \
    if (m != 0)   \
    {             \
        free(m);  \
        m = NULL; \
    }
#endif

#ifndef CARDV_ALIGN_UP
#define CARDV_ALIGN_UP(value, align) ((value + align - 1) / align * align)
#endif

#ifndef CARDV_ALIGN_DOWN
#define CARDV_ALIGN_DOWN(value, align) (value / align * align)
#endif

#ifndef ExecFuncNoExit
#define ExecFuncNoExit(_func_, _ret_)                                                        \
    do                                                                                       \
    {                                                                                        \
        MI_S32 s32Ret = MI_SUCCESS;                                                          \
        s32Ret        = _func_;                                                              \
        if (s32Ret != _ret_)                                                                 \
        {                                                                                    \
            printf("[%s %d]exec func fail, NOT exit, err:%x\n", __func__, __LINE__, s32Ret); \
        }                                                                                    \
        else                                                                                 \
        {                                                                                    \
            /* printf("[%s %d]exec func pass\n", __func__, __LINE__); */                     \
        }                                                                                    \
    } while (0)
#endif

#ifndef ExecFunc
#ifdef DUAL_OS
#define ExecFunc(_func_, _ret_) ExecFuncNoExit(_func_, _ret_)
#else
#define ExecFunc(_func_, _ret_)                                                    \
    do                                                                             \
    {                                                                              \
        MI_S32 s32Ret = MI_SUCCESS;                                                \
        s32Ret        = _func_;                                                    \
        if (s32Ret != _ret_)                                                       \
        {                                                                          \
            printf("[%s %d]exec func fail, err:%x\n", __func__, __LINE__, s32Ret); \
            return s32Ret;                                                         \
        }                                                                          \
        else                                                                       \
        {                                                                          \
            /* printf("[%s %d]exec func pass\n", __func__, __LINE__); */           \
        }                                                                          \
    } while (0)
#endif
#endif

#ifndef ExecFunc_OS
#ifdef DUAL_OS
extern BOOL g_bVideoStart;
#define ExecFunc_OS(_func_, _ret_)                                                     \
    if (!g_bVideoStart)                                                                \
    {                                                                                  \
        /* printf("DUAL_OS:%s todo %d!\n", __FUNCTION__, __LINE__); */                 \
    }                                                                                  \
    else                                                                               \
    {                                                                                  \
        do                                                                             \
        {                                                                              \
            MI_S32 s32Ret = MI_SUCCESS;                                                \
            s32Ret        = _func_;                                                    \
            if (s32Ret != _ret_)                                                       \
            {                                                                          \
                printf("[%s %d]exec func fail, err:%x\n", __func__, __LINE__, s32Ret); \
                return s32Ret;                                                         \
            }                                                                          \
            else                                                                       \
            {                                                                          \
                /* printf("[%s %d]exec func pass\n", __func__, __LINE__); */           \
            }                                                                          \
        } while (0);                                                                   \
    }
#else
#define ExecFunc_OS(_func_, _ret_) ExecFunc(_func_, _ret_)
#endif
#endif

#ifndef CARDVCHECKRESULT
#define CARDVCHECKRESULT(_func_)                                                   \
    do                                                                             \
    {                                                                              \
        MI_S32 s32Ret = MI_SUCCESS;                                                \
        s32Ret        = _func_;                                                    \
        if (s32Ret != MI_SUCCESS)                                                  \
        {                                                                          \
            printf("(%s %d)exec func fail, err:%x\n", __func__, __LINE__, s32Ret); \
            return s32Ret;                                                         \
        }                                                                          \
        else                                                                       \
        {                                                                          \
            /* printf("(%s %d)exec func pass\n", __FUNCTION__,__LINE__); */        \
        }                                                                          \
    } while (0)
#endif

#ifndef CARDVCHECKRESULT_OS
#ifdef DUAL_OS
#define CARDVCHECKRESULT_OS(_func_)                                                        \
    {                                                                                      \
        if (!g_bVideoStart)                                                                \
        {                                                                                  \
            /* printf("DUAL_OS:%s todo %d!\n", __FUNCTION__, __LINE__); */                 \
        }                                                                                  \
        else                                                                               \
        {                                                                                  \
            do                                                                             \
            {                                                                              \
                MI_S32 s32Ret = MI_SUCCESS;                                                \
                s32Ret        = _func_;                                                    \
                if (s32Ret != MI_SUCCESS)                                                  \
                {                                                                          \
                    printf("(%s %d)exec func fail, err:%x\n", __func__, __LINE__, s32Ret); \
                    return s32Ret;                                                         \
                }                                                                          \
                else                                                                       \
                {                                                                          \
                    /* printf("(%s %d)exec func pass\n", __FUNCTION__,__LINE__); */        \
                }                                                                          \
            } while (0);                                                                   \
        }                                                                                  \
    }
#else
#define CARDVCHECKRESULT_OS(_func_) CARDVCHECKRESULT(_func_)
#endif
#endif

#ifndef CarDV_API_ISVALID_POINT
#define CarDV_API_ISVALID_POINT(X)                        \
    {                                                     \
        if (X == NULL)                                    \
        {                                                 \
            printf("cardv input point param is null!\n"); \
            return MI_SUCCESS;                            \
        }                                                 \
    }
#endif

#define CARDVDBG_ENTER()                                            \
    printf("\n");                                                   \
    printf("[IN] [%s:%s:%d] \n", __FILE__, __FUNCTION__, __LINE__); \
    printf("\n");

#define CARDVDBG_LEAVE()                                             \
    printf("\n");                                                    \
    printf("[OUT] [%s:%s:%d] \n", __FILE__, __FUNCTION__, __LINE__); \
    printf("\n");

#define CARDV_RUN()                                                     \
    printf("\n");                                                       \
    printf("[RUN] ok [%s:%s:%d] \n", __FILE__, __FUNCTION__, __LINE__); \
    printf("\n");

#define CARDV_THREAD() /*                                                               \
    printf("\n");                                                                       \
    printf("[THREAD] [%s:%d:pid:%ld] \n", __FUNCTION__, __LINE__, syscall(SYS_gettid)); \
    printf("\n"); \ */

extern int g_dbglevel;

#define CARDV_DBGLV_NONE    0 // disable all the debug message
#define CARDV_DBGLV_INFO    1 // information
#define CARDV_DBGLV_NOTICE  2 // normal but significant condition
#define CARDV_DBGLV_DEBUG   3 // debug-level messages
#define CARDV_DBGLV_WARNING 4 // warning conditions
#define CARDV_DBGLV_ERR     5 // error conditions
#define CARDV_DBGLV_CRIT    6 // critical conditions
#define CARDV_DBGLV_ALERT   7 // action must be taken immediately
#define CARDV_DBGLV_EMERG   8 // system is unusable

#define COLOR_NONE   "\033[0m"
#define COLOR_BLACK  "\033[0;30m"
#define COLOR_BLUE   "\033[0;34m"
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_CYAN   "\033[0;36m"
#define COLOR_RED    "\033[0;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_WHITE  "\033[1;37m"

#define CARDV_NOP(fmt, args...)
#define CARDV_DBG(fmt, args...)                                                      \
    if (g_dbglevel <= CARDV_DBGLV_DEBUG)                                             \
        do                                                                           \
        {                                                                            \
            printf(COLOR_GREEN "[DBG]:%s[%d]: " COLOR_NONE, __FUNCTION__, __LINE__); \
            printf(fmt, ##args);                                                     \
    } while (0)

#define CARDV_WARN(fmt, args...)                                                       \
    if (g_dbglevel <= CARDV_DBGLV_WARNING)                                             \
        do                                                                             \
        {                                                                              \
            printf(COLOR_YELLOW "[WARN]:%s[%d]: " COLOR_NONE, __FUNCTION__, __LINE__); \
            printf(fmt, ##args);                                                       \
    } while (0)

#define CARDV_INFO(fmt, args...)                                 \
    if (g_dbglevel <= CARDV_DBGLV_INFO)                          \
        do                                                       \
        {                                                        \
            printf("[INFO]:%s[%d]: \n", __FUNCTION__, __LINE__); \
            printf(fmt, ##args);                                 \
    } while (0)

#define CARDV_ERR(fmt, args...)                                                    \
    if (g_dbglevel <= CARDV_DBGLV_ERR)                                             \
        do                                                                         \
        {                                                                          \
            printf(COLOR_RED "[ERR]:%s[%d]: " COLOR_NONE, __FUNCTION__, __LINE__); \
            printf(fmt, ##args);                                                   \
    } while (0)

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#define ALIGN_UP(x, align)   (((x) + ((align)-1)) & ~((align)-1))
#define ALIGN_DOWN(x, align) (((x) / (align)) * (align))

#define BOND_SSM613D 0x00 // QFN  1G
#define BOND_SSM613Q 0x01 // QFN  2G
#define BOND_SSM616Q 0x31 // LQFP 2G
#define BOND_SSM633Q 0x11 // BGA1 2G
#define BOND_SSM650G 0x27 // BGA2

//==============================================================================
//
//                              STRUCT & ENUM DEFINES
//
//==============================================================================

typedef struct CarDV_BindParam_s
{
    MI_SYS_ChnPort_t  stChnPort;
    MI_SYS_BindType_e eBindType;
    MI_U32            u32BindParam;
    MI_U32            u32SrcFrmrate;
    MI_U32            u32DstFrmrate;
} CarDV_BindParam_t;

typedef struct _Point_t
{
    S32 x;
    S32 y;
} Point_t;

typedef struct _Size_t
{
    U32 width;
    U32 height;
} Size_t;

typedef struct _Rect_s
{
    U16 x;
    U16 y;
    U16 width;
    U16 height;
} Rect_t;

typedef struct _YUVColor_t
{
    U8 y;
    U8 u;
    U8 v;
    U8 transparent;
} YUVColor_t;

typedef enum _Rotate_e
{
    ROTATE_0   = 0,
    ROTATE_90  = 1,
    ROTATE_180 = 2,
    ROTATE_270 = 3,
} Rotate_e;

typedef struct _Color_t
{
    U8 a;
    U8 r;
    U8 g;
    U8 b;
} Color_t;

typedef struct OsdInvertColor_s
{
    U32  nLumThresh; // The threshold to decide whether invert the OSD's color or not.
    BOOL bInvColEn;  // The switch of inverting color.
} OsdInvertColor_t;

typedef enum _CarDVCmdIdtype
{
    CMD_VIDEO      = 0x0100,
    CMD_AUDIO      = 0x0200,
    CMD_OSD        = 0x0300,
    CMD_ISP        = 0x0400,
    CMD_VDF        = 0x0500,
    CMD_SYSTEM     = 0x0600,
    CMD_PIPE       = 0x0700,
    CMD_DISP       = 0x0800,
    CMD_SYS        = 0x0900,
    CMD_MUX        = 0x0A00,
    CMD_ADAS       = 0x0B00,
    CMD_DMS        = 0x0C00,
    CMD_LD         = 0x0D00,
    CMD_UVC        = 0x0E00,
    CARDV_CMD_TYPE = 0xFF00,
} CarDVCmdType;

typedef enum _CarDVCmdId
{
    CMD_VIDEO_INIT_H26X      = 0x01 | CMD_VIDEO,
    CMD_VIDEO_DEINIT_H26X    = 0x02 | CMD_VIDEO,
    CMD_VIDEO_INIT_JPEG      = 0x03 | CMD_VIDEO,
    CMD_VIDEO_DEINIT_JPEG    = 0x04 | CMD_VIDEO,
    CMD_VIDEO_SET_RESOLUTION = 0x05 | CMD_VIDEO,
    CMD_VIDEO_SET_FRAMERATE  = 0x06 | CMD_VIDEO,
    CMD_VIDEO_START          = 0x07 | CMD_VIDEO,
    CMD_VIDEO_STOP           = 0x08 | CMD_VIDEO,
    CMD_VIDEO_CAPTURE        = 0x09 | CMD_VIDEO,
    CMD_VIDEO_CAPTURE_THUMB  = 0x0A | CMD_VIDEO,
    CMD_VIDEO_CAPTURE_RAW    = 0x0B | CMD_VIDEO,
    CMD_VIDEO_SET_BITRATE    = 0x0C | CMD_VIDEO,
    CMD_VIDEO_SET_CODEC      = 0x0D | CMD_VIDEO,

    CMD_AUDIO_IN_OPEN        = 0x01 | CMD_AUDIO,
    CMD_AUDIO_IN_CLOSE       = 0x02 | CMD_AUDIO,
    CMD_AUDIO_IN_START       = 0x03 | CMD_AUDIO,
    CMD_AUDIO_IN_STOP        = 0x04 | CMD_AUDIO,
    CMD_AUDIO_IN_MUTE        = 0x05 | CMD_AUDIO,
    CMD_AUDIO_OUT_OPEN       = 0x06 | CMD_AUDIO,
    CMD_AUDIO_OUT_CLOSE      = 0x07 | CMD_AUDIO,
    CMD_AUDIO_OUT_PLAY       = 0x08 | CMD_AUDIO,
    CMD_AUDIO_OUT_MULTI_PLAY = 0x09 | CMD_AUDIO,
    CMD_AUDIO_OUT_STOP       = 0x0A | CMD_AUDIO,
    CMD_AUDIO_IN_SET_VOLUME  = 0x0B | CMD_AUDIO,
    CMD_AUDIO_IN_START_FILE  = 0x0C | CMD_AUDIO,
    CMD_AUDIO_IN_STOP_FILE   = 0x0D | CMD_AUDIO,

    CMD_OSD_OPEN               = 0x01 | CMD_OSD,
    CMD_OSD_CLOSE              = 0x02 | CMD_OSD,
    CMD_OSD_SET_OD             = 0x03 | CMD_OSD,
    CMD_OSD_GET_OD             = 0x04 | CMD_OSD,
    CMD_OSD_IE_OPEN            = 0x07 | CMD_OSD,
    CMD_OSD_IE_CLOSE           = 0x08 | CMD_OSD,
    CMD_OSD_FULL_OPEN          = 0x0F | CMD_OSD,
    CMD_OSD_RESET_RESOLUTION   = 0x14 | CMD_OSD,
    CMD_OSD_RESTART_RESOLUTION = 0x15 | CMD_OSD,
    CMD_OSD_SET_STAMP          = 0x16 | CMD_OSD,
    CMD_OSD_SET_TIMEFORMAT     = 0x17 | CMD_OSD,

    CMD_ISP_OPEN                 = 0x01 | CMD_ISP,
    CMD_ISP_CLOSE                = 0x02 | CMD_ISP,
    CMD_ISP_SET_SATURATION       = 0x03 | CMD_ISP,
    CMD_ISP_SET_SHARPENESS       = 0x04 | CMD_ISP,
    CMD_ISP_SET_LIGHTSENSITIVITY = 0x05 | CMD_ISP,
    CMD_ISP_SET_SCENE            = 0x06 | CMD_ISP,
    CMD_ISP_SET_EXPOSURE_MODE    = 0x07 | CMD_ISP,
    CMD_ISP_SET_IRCUT            = 0x08 | CMD_ISP,
    CMD_ISP_SET_ROTATION         = 0x09 | CMD_ISP,
    CMD_ISP_SET_EXPOSURE_LIMIT   = 0x0a | CMD_ISP,
    CMD_ISP_SET_SHUTTER          = 0x0b | CMD_ISP,
    CMD_ISP_SET_CONTRAST         = 0x12 | CMD_ISP,
    CMD_ISP_SET_BRIGHTNESS       = 0x13 | CMD_ISP,
    CMD_ISP_SET_INPUTCROP        = 0x14 | CMD_ISP,
    CMD_ISP_SET_FLICKER          = 0x15 | CMD_ISP,
    CMD_ISP_SET_AFWIN            = 0x16 | CMD_ISP,
    CMD_ISP_QUERY_AFINFO         = 0x17 | CMD_ISP,
    CMD_ISP_LOAD_CALI_DATA       = 0x18 | CMD_ISP,
    CMD_ISP_LOAD_CMDBIN          = 0x19 | CMD_ISP,
    CMD_ISP_SET_WB_MODE          = 0x1a | CMD_ISP,
    CMD_ISP_SET_ISO              = 0x1b | CMD_ISP,
    CMD_ISP_SET_GAMMA            = 0x1c | CMD_ISP,
    CMD_ISP_SET_EFFECT           = 0x1d | CMD_ISP,
    CMD_ISP_SET_WINWGT_TYPE      = 0x1f | CMD_ISP,

    CMD_VDF_OPEN      = 0x01 | CMD_VDF,
    CMD_VDF_CLOSE     = 0x02 | CMD_VDF,
    CMD_VDF_MD_OPEN   = 0x03 | CMD_VDF,
    CMD_VDF_MD_CLOSE  = 0x04 | CMD_VDF,
    CMD_VDF_MD_CHANGE = 0x05 | CMD_VDF,

    CMD_SYSTEM_WATCHDOG_OPEN         = 0x01 | CMD_SYSTEM,
    CMD_SYSTEM_WATCHDOG_CLOSE        = 0x02 | CMD_SYSTEM,
    CMD_SYSTEM_IQSERVER_OPEN         = 0x03 | CMD_SYSTEM,
    CMD_SYSTEM_IQSERVER_CLOSE        = 0x04 | CMD_SYSTEM,
    CMD_SYSTEM_LIVE555_OPEN          = 0x09 | CMD_SYSTEM,
    CMD_SYSTEM_LIVE555_CLOSE         = 0x0a | CMD_SYSTEM,
    CMD_SYSTEM_LIVE555_CLOSE_SESSION = 0x0b | CMD_SYSTEM,
    CMD_SYSTEM_RESET_TO_UBOOT        = 0x0c | CMD_SYSTEM,
    CMD_SYSTEM_VENDOR_GPS            = 0x0d | CMD_SYSTEM,
    CMD_SYSTEM_VENDOR_EDOG_CTRL      = 0x0e | CMD_SYSTEM,
    CMD_SYSTEM_EXIT_QUEUE            = 0x10 | CMD_SYSTEM,

    CMD_SYSTEM_INIT                  = 0x11 | CMD_SYSTEM,
    CMD_SYSTEM_UNINIT                = 0x12 | CMD_SYSTEM,
    CMD_SYSTEM_CORE_BACKTRACE        = 0x13 | CMD_SYSTEM,
    CMD_SYSTEM_SETCHNOUTPUTPORTDEPTH = 0x16 | CMD_SYSTEM,
    CMD_SYSTEM_DEMUX_THUMB           = 0x19 | CMD_SYSTEM,
    CMD_SYSTEM_DEMUX_TOTALTIME       = 0x1A | CMD_SYSTEM,

    CMD_UVC_INIT  = 0x01 | CMD_UVC,
    CMD_UVC_CLOSE = 0x02 | CMD_UVC,

    CMD_PIPE_INIT                  = 0x01 | CMD_PIPE,
    CMD_PIPE_UNINIT                = 0x02 | CMD_PIPE,
    CMD_PIPE_SWITCH_HDR            = 0x03 | CMD_PIPE,
    CMD_PIPE_SWITCH_SENSOR_RES_IDX = 0x04 | CMD_PIPE,
    CMD_PIPE_SWITCH_ANADEC_RES     = 0x05 | CMD_PIPE,
    CMD_PIPE_ISP_ZOOM              = 0x06 | CMD_PIPE,
    CMD_PIPE_SENSOR_ENABLE         = 0x07 | CMD_PIPE,
    CMD_PIPE_SENSOR_DISABLE        = 0x08 | CMD_PIPE,
    CMD_PIPE_LDC_ENABLE            = 0x09 | CMD_PIPE,
    CMD_PIPE_LDC_DISABLE           = 0x0a | CMD_PIPE,

    // CMD_DISP_INIT                           = 0x01 | CMD_DISP,
    // CMD_DISP_UNINIT                         = 0x02 | CMD_DISP,
    CMD_DISP_PREVIEW_START      = 0x03 | CMD_DISP,
    CMD_DISP_PREVIEW_STOP       = 0x04 | CMD_DISP,
    CMD_DISP_PLAYBACK_START     = 0x05 | CMD_DISP,
    CMD_DISP_PLAYBACK_STOP      = 0x06 | CMD_DISP,
    CMD_DISP_PREVIEW_MOVE       = 0x07 | CMD_DISP,
    CMD_DISP_SET_LCD_LUMA       = 0x08 | CMD_DISP,
    CMD_DISP_SET_HDMI_TIMING    = 0x09 | CMD_DISP,
    CMD_DISP_SET_LCD_CONTRAST   = 0x0A | CMD_DISP,
    CMD_DISP_SET_LCD_HUE        = 0x0B | CMD_DISP,
    CMD_DISP_SET_LCD_SATURATION = 0x0C | CMD_DISP,

    CMD_MUX_OPEN               = 0x01 | CMD_MUX,
    CMD_MUX_CLOSE              = 0x02 | CMD_MUX,
    CMD_MUX_PRE_START          = 0x03 | CMD_MUX,
    CMD_MUX_START              = 0x04 | CMD_MUX,
    CMD_MUX_STOP               = 0x05 | CMD_MUX,
    CMD_MUX_STOP_KEEP_ENCODING = 0x06 | CMD_MUX,
    CMD_MUX_PAUSE              = 0x07 | CMD_MUX,
    CMD_MUX_RESUME             = 0x08 | CMD_MUX,
    CMD_MUX_SET_FRAMERATE      = 0x09 | CMD_MUX,
    CMD_MUX_SET_REC_LIMIT_TIME = 0x0a | CMD_MUX,
    CMD_MUX_SET_PRE_REC_TIME   = 0x0b | CMD_MUX,

    CMD_DMS_INIT   = 0x01 | CMD_DMS,
    CMD_DMS_DEINIT = 0x02 | CMD_DMS,

    CMD_ADAS_INIT        = 0x01 | CMD_ADAS,
    CMD_ADAS_DEINIT      = 0x02 | CMD_ADAS,
    CMD_ADAS_REAR_INIT   = 0x03 | CMD_ADAS,
    CMD_ADAS_REAR_DEINIT = 0x04 | CMD_ADAS,

    CMD_LD_INIT      = 0x01 | CMD_LD,
    CMD_LD_DEINIT    = 0x02 | CMD_LD,
    CMD_LD_SET_PARAM = 0x03 | CMD_LD,

} CarDVCmdId;

typedef enum _EncoderType_e
{
    VE_H264,
    VE_H265,
    VE_JPG,
} CarDV_EncoderType_e;

typedef struct CarDV_Sys_BindInfo_s
{
    MI_SYS_ChnPort_t  stSrcChnPort;
    MI_SYS_ChnPort_t  stDstChnPort;
    MI_U32            u32SrcFrmrate;
    MI_U32            u32DstFrmrate;
    MI_SYS_BindType_e eBindType;
    MI_U32            u32BindParam;
} CarDV_Sys_BindInfo_T;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void segfault_sigaction(int signo, siginfo_t *si, void *arg);

MI_S32 CarDV_Sys_Init(void);
MI_S32 CarDV_Sys_Exit(void);
MI_S32 CarDV_Sys_Bind(CarDV_Sys_BindInfo_T *pstBindInfo);
MI_S32 CarDV_Sys_UnBind(CarDV_Sys_BindInfo_T *pstBindInfo);
MI_U16 CarDV_Sys_ReadBondID(void);

int cardv_send_cmd(CarDVCmdId cmdId, MI_S8 *param, MI_S32 paramLen);
int cardv_send_cmd_HP(CarDVCmdId cmdId, MI_S8 *param, MI_S32 paramLen);
int cardv_send_cmd_and_wait(CarDVCmdId cmdId, MI_S8 *param, MI_S32 paramLen);
int cardv_message_queue_init();
int cardv_message_queue_uninit();

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //#define _MID_COMMON_H_
