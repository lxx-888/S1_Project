/*
 * module_uvc.h- Sigmastar
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
#ifndef _MODULE_UVC_H_
#define _MODULE_UVC_H_

#include <linux/videodev2.h>
#include "video.h"
#include "uvc.h"

#define UVC_SUPPORT_DEBUG

#define UVC_VC_INPUT_TERMINAL_ID  1
#define UVC_VC_OUTPUT_TERMINAL_ID 3
#define UVC_VC_PROCESSING_UNIT_ID 2
#define UVC_VC_EXTENSION_UNIT_ID  0xff
#define UVC_VC_SELECTOR_UNIT_ID   0xfe

typedef FILE *FILE_HANDLE;
#define CarDV_UVC_SUCCESS 0

typedef signed char   MI_S8;
typedef signed int    MI_S32;
typedef unsigned char MI_U8;
typedef unsigned int  MI_U32;
typedef unsigned int  size_t;
typedef MI_U32        CarDV_UVC_CHANNEL;

#define V4L2_PIX_FMT_H265 v4l2_fourcc('H', '2', '6', '5') /* add claude.rao */

#define MaxFrameSize 320 * 240 * 2 // define in kernel
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) ((sizeof(a) / sizeof(a[0])))
#endif
#define CLEAR(x)                     memset(&(x), 0, sizeof(x))
#define FrameInterval2FrameRate(val) ((int)(1.0 / val * 10000000))
#define FrameRate2FrameInterval(val) ((int)(1.0 / val * 10000000))

#define clamp(val, min, max)                                \
    ({                                                      \
        unsigned int __val = (val);                         \
        unsigned int __min = (min);                         \
        unsigned int __max = (max);                         \
        __val              = __val < __min ? __min : __val; \
        __val > __max ? __max : __val;         \
    })

#define IS_NULL(val)         ((val == NULL) ? true : false)
#define IS_TRUE(val)         ((val != 0) ? true : false)
#define IS_ZERO(val)         ((val == 0) ? true : false)
#define IS_NOZERO(val)       ((val != 0) ? true : false)
#define IS_EQUAL(vala, valb) ((vala == valb) ? true : false)

/*FOR UVC DEVICE STATUS*/
#define UVC_DEVICE_MASK       0
#define UVC_DEVICE_INITIAL    ((1 << 7) << UVC_DEVICE_MASK)
#define UVC_DEVICE_ENUMURATED ((1 << 6) << UVC_DEVICE_MASK)
#define UVC_DEVICE_REQBUFS    ((1 << 5) << UVC_DEVICE_MASK)
#define UVC_DEVICE_STREAMON   ((1 << 4) << UVC_DEVICE_MASK)
#define UVC_INTDEV_INITIAL    ((1 << 3) << UVC_DEVICE_MASK)
#define UVC_INTDEV_STARTED    ((1 << 2) << UVC_DEVICE_MASK)
#define UVC_INTDEV_STREAMON   ((1 << 1) << UVC_DEVICE_MASK)
#define UVC_INTDEV_UNDEFINE   ((1 << 0) << UVC_DEVICE_MASK)

#define UVC_CHANGE_STATUS(val, mask, bit)           \
    do                                              \
    {                                               \
        val = bit ? (mask | val) : (val & (~mask)); \
    } while (0)
#define UVC_SET_STATUS(val, mask)   UVC_CHANGE_STATUS(val, mask, 1)
#define UVC_UNSET_STATUS(val, mask) UVC_CHANGE_STATUS(val, mask, 0)
#define UVC_GET_STATUS(val, mask)   IS_TRUE((val & mask))
#define UVC_INPUT_ISENABLE(val)                                                         \
    (UVC_GET_STATUS(val, UVC_INTDEV_INITIAL) && UVC_GET_STATUS(val, UVC_INTDEV_STARTED) \
     && UVC_GET_STATUS(val, UVC_INTDEV_STREAMON))
#define UVC_OUTPUT_ISENABLE(val)                                                           \
    (UVC_GET_STATUS(val, UVC_DEVICE_INITIAL) && UVC_GET_STATUS(val, UVC_DEVICE_ENUMURATED) \
     && UVC_GET_STATUS(val, UVC_DEVICE_REQBUFS) && UVC_GET_STATUS(val, UVC_DEVICE_STREAMON))
#define UVC_DEVICE_ISREADY(val)                                                         \
    (UVC_GET_STATUS(val, UVC_DEVICE_INITIAL) && UVC_GET_STATUS(val, UVC_INTDEV_INITIAL) \
     && UVC_GET_STATUS(val, UVC_INTDEV_STARTED))

typedef enum
{
    UVC_DBG_NONE = 0,
    UVC_DBG_ERR,
    UVC_DBG_WRN,
    UVC_DBG_INFO,
    UVC_DBG_DEBUG,
    UVC_DBG_ALL
} UVC_DBG_LEVEL_e;

#define ASCII_COLOR_RED    "\033[1;31m"
#define ASCII_COLOR_WHITE  "\033[1;37m"
#define ASCII_COLOR_YELLOW "\033[1;33m"
#define ASCII_COLOR_BLUE   "\033[1;36m"
#define ASCII_COLOR_GREEN  "\033[1;32m"
#define ASCII_COLOR_END    "\033[0m"

extern UVC_DBG_LEVEL_e uvc_debug_level;
extern bool            uvc_func_trace;

#ifdef UVC_SUPPORT_DEBUG
#define UVC_DEBUG(fmt, args...)                                                                                      \
    ({                                                                                                               \
        do                                                                                                           \
        {                                                                                                            \
            if (uvc_debug_level >= UVC_DBG_DEBUG)                                                                    \
            {                                                                                                        \
                printf(ASCII_COLOR_GREEN "[APP INFO]:%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args); \
            }                                                                                                        \
        } while (0);                                                                                                 \
    })
#define UVC_INFO(fmt, args...)                                                                                       \
    ({                                                                                                               \
        do                                                                                                           \
        {                                                                                                            \
            if (uvc_debug_level >= UVC_DBG_INFO)                                                                     \
            {                                                                                                        \
                printf(ASCII_COLOR_GREEN "[APP INFO]:%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args); \
            }                                                                                                        \
        } while (0);                                                                                                 \
    })
#define UVC_WRN(fmt, args...)                                                                                          \
    ({                                                                                                                 \
        do                                                                                                             \
        {                                                                                                              \
            if (uvc_debug_level >= UVC_DBG_WRN)                                                                        \
            {                                                                                                          \
                printf(ASCII_COLOR_YELLOW "[APP WRN ]: %s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args); \
            }                                                                                                          \
        } while (0);                                                                                                   \
    })
#define UVC_ERR(fmt, args...)                                                                                       \
    ({                                                                                                              \
        do                                                                                                          \
        {                                                                                                           \
            if (uvc_debug_level >= UVC_DBG_ERR)                                                                     \
            {                                                                                                       \
                printf(ASCII_COLOR_RED "[APP ERR ]: %s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args); \
            }                                                                                                       \
        } while (0);                                                                                                \
    })
#define UVC_EXIT_ERR(fmt, args...)                                                                    \
    ({                                                                                                \
        do                                                                                            \
        {                                                                                             \
            printf(ASCII_COLOR_RED "<<<%s[%d] " fmt ASCII_COLOR_END, __FUNCTION__, __LINE__, ##args); \
        } while (0);                                                                                  \
    })
#define UVC_ENTER()                                                                              \
    ({                                                                                           \
        do                                                                                       \
        {                                                                                        \
            if (uvc_func_trace)                                                                  \
            {                                                                                    \
                printf(ASCII_COLOR_BLUE ">>>%s[%d] \n" ASCII_COLOR_END, __FUNCTION__, __LINE__); \
            }                                                                                    \
        } while (0);                                                                             \
    })
#define UVC_EXIT_OK()                                                                            \
    ({                                                                                           \
        do                                                                                       \
        {                                                                                        \
            if (uvc_func_trace)                                                                  \
            {                                                                                    \
                printf(ASCII_COLOR_BLUE "<<<%s[%d] \n" ASCII_COLOR_END, __FUNCTION__, __LINE__); \
            }                                                                                    \
        } while (0);                                                                             \
    })
#else
#define UVC_DEBUG(fmt, args...) NULL
#define UVC_ERR(fmt, args...)   NULL
#define UVC_INFO(fmt, args...)  NULL
#define UVC_WRN(fmt, args...)   NULL
#endif

typedef enum
{
    USB_ISOC_MODE,
    USB_BULK_MODE,
} Transfer_Mode_e;

typedef enum
{
    UVC_CONTROL_INTERFACE = 1,
    UVC_STREAMING_INTERFACE,
} Control_Type_e;

typedef struct UVC_Control_s
{
    Control_Type_e ctype;
    unsigned char  control;
    unsigned char  entity;
} UVC_Control_t;

typedef enum
{
    BUFFER_FREE,
    BUFFER_DEQUEUE,
    BUFFER_FILLING,
    BUFFER_QUEUE,
} buffer_status_e;

struct buffer
{
    struct v4l2_buffer buf;
    void *             start;
    /* for buff to finish, differ from mmap and userptr */
    int handle;
    /* for mmap */
    size_t length;
    /* buffer tail flags, default tail(1) */
    buffer_status_e status;
    bool            is_tail;
};

struct uvc_frame_info
{
    MI_U32 width;
    MI_U32 height;
    MI_U32 intervals[8];
};

struct uvc_format_info
{
    MI_U32                       fcc;
    const struct uvc_frame_info *frames;
};

static const struct uvc_frame_info uvc_frames_yuyv[] = {
    {
        320,
        240,
        {333333, 666666, 1000000, 0},
    },
    {
        640,
        480,
        {333333, 666666, 1000000, 0},
    },
    {
        1280,
        720,
        {333333, 666666, 1000000, 0},
    },
    {
        1920,
        1080,
        {333333, 666666, 1000000, 0},
    },
    {
        0,
        0,
        {
            0,
        },
    },
};

static const struct uvc_frame_info uvc_frames_nv12[] = {
    {
        320,
        240,
        {333333, 666666, 1000000, 0},
    },
    {
        640,
        480,
        {333333, 666666, 1000000, 0},
    },
    {
        1280,
        720,
        {333333, 666666, 1000000, 0},
    },
    {
        1920,
        1080,
        {333333, 666666, 1000000, 0},
    },
    {
        0,
        0,
        {
            0,
        },
    },
};

static const struct uvc_frame_info uvc_frames_mjpeg[] = {
    {
        320,
        240,
        {333333, 666666, 1000000, 0},
    },
    {
        640,
        480,
        {333333, 666666, 1000000, 0},
    },
    {
        1280,
        720,
        {333333, 666666, 1000000, 0},
    },
    {
        1920,
        1080,
        {333333, 666666, 1000000, 0},
    },
    {
        2560,
        1440,
        {333333, 666666, 1000000, 0},
    },
    {
        3840,
        2160,
        {333333, 666666, 1000000, 0},
    },
    {
        0,
        0,
        {
            0,
        },
    },
};

static const struct uvc_frame_info uvc_frames_h264[] = {
    {
        320,
        240,
        {333333, 666666, 1000000, 0},
    },
    {
        640,
        480,
        {333333, 666666, 1000000, 0},
    },
    {
        1280,
        720,
        {333333, 666666, 1000000, 0},
    },
    {
        1920,
        1080,
        {333333, 666666, 1000000, 0},
    },
    {
        2560,
        1440,
        {333333, 666666, 1000000, 0},
    },
    {
        3840,
        2160,
        {333333, 666666, 1000000, 0},
    },
    {
        0,
        0,
        {
            0,
        },
    },
};

static const struct uvc_frame_info uvc_frames_h265[] = {
    {
        320,
        240,
        {333333, 666666, 1000000, 0},
    },
    {
        640,
        480,
        {333333, 666666, 1000000, 0},
    },
    {
        1280,
        720,
        {333333, 666666, 1000000, 0},
    },
    {
        1920,
        1080,
        {333333, 666666, 1000000, 0},
    },
    {
        2560,
        1440,
        {333333, 666666, 1000000, 0},
    },
    {
        3840,
        2160,
        {333333, 666666, 1000000, 0},
    },
    {
        0,
        0,
        {
            0,
        },
    },
};

static const struct uvc_format_info uvc_formats[] = {
    //  { V4L2_PIX_FMT_YUYV,  uvc_frames_yuyv },
    {V4L2_PIX_FMT_NV12, uvc_frames_nv12},
    {V4L2_PIX_FMT_MJPEG, uvc_frames_mjpeg},
    {V4L2_PIX_FMT_H264, uvc_frames_h264},
    {V4L2_PIX_FMT_H265, uvc_frames_h265},
};

typedef struct Stream_Params_s
{
    /* video format framerate */
    MI_U32 fcc;
    MI_U32 iformat;
    MI_U32 iframe;
    MI_U32 width;
    MI_U32 height;
    double frameRate;
    MI_U32 maxframesize;
} Stream_Params_t;

typedef enum
{
    UVC_MEMORY_MMAP,
    UVC_MEMORY_USERPTR,
} UVC_IO_MODE_e;

typedef struct CarDV_UVC_Setting_s
{
    /* buffer related*/
    MI_U8 nbuf;
    /* v4l2 memory type */
    UVC_IO_MODE_e io_mode;
    /* transfer mode : bulk or isoc */
    Transfer_Mode_e mode;
} CarDV_UVC_Setting_t;

typedef struct CarDV_UVC_BufInfo_s
{
    unsigned long length;
    union
    {
        unsigned long start; // for userptr
        void *        buf;   // for mmap
    } b;
    bool is_tail;
} CarDV_UVC_BufInfo_t;

typedef struct CarDV_UVC_USERPTR_BufOpts_s
{
    MI_S32 (*UVC_DevFillBuffer)(void *uvc, CarDV_UVC_BufInfo_t *bufInfo, int *handle);
    MI_S32 (*UVC_FinishBuffer)(int handle);
} CarDV_UVC_USERPTR_BufOpts_t;

typedef struct CarDV_UVC_MMAP_BufOpts_s
{
    MI_S32 (*UVC_DevFillBuffer)(void *uvc, CarDV_UVC_BufInfo_t *bufInfo);
} CarDV_UVC_MMAP_BufOpts_t;

typedef struct CarDV_UVC_OPS_s
{
    MI_S32 (*UVC_Inputdev_Init)(void *uvc);
    MI_S32 (*UVC_Inputdev_Deinit)(void *uvc);
    union
    {
        CarDV_UVC_MMAP_BufOpts_t    m;
        CarDV_UVC_USERPTR_BufOpts_t u;
    };
    MI_S32 (*UVC_StartCapture)(void *uvc, Stream_Params_t format);
    MI_S32 (*UVC_StopCapture)(void *uvc);
} CarDV_UVC_OPS_t;

typedef struct CarDV_UVC_ChnAttr_s
{
    CarDV_UVC_Setting_t setting;
    CarDV_UVC_OPS_t     fops;
} CarDV_UVC_Attr_t;

typedef struct CarDV_UVC_Device_s
{
    MI_S32 uvc_fd;

    /* UVC thread Releated */
    pthread_t       event_handle_thread;
    pthread_t       video_process_thread;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;

    /* UVC Setting */
    CarDV_UVC_Attr_t ChnAttr;

    /* Input Video Specific */
    void *Input_Device;

    /* UVC Stream Control Specific */
    Stream_Params_t              stream_param;
    struct uvc_streaming_control probe;
    struct uvc_streaming_control commit;

    /* UVC Control Request Specific */
    UVC_Control_t           control;
    struct uvc_request_data request_error_code;

    /* UVC Device buffer */
    struct buffer *mem;

    /* UVC Specific flags */
    unsigned char status;

    int exit_request;
} CarDV_UVC_Device_t;

MI_S32 CarDV_UVC_Init(char *uvc_name);
MI_S32 CarDV_UVC_Uninit(void);
MI_S32 CarDV_UVC_CreateDev(const CarDV_UVC_Attr_t *pstAttr);
MI_S32 CarDV_UVC_DestroyDev();
MI_S32 CarDV_UVC_StartDev(void);
MI_S32 CarDV_UVC_StopDev(void);

#endif //_MODULE_UVC_H_