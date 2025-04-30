/*
 * module_common.h- Sigmastar
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
#ifndef _MODULE_COMMON_
#define _MODULE_COMMON_

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/queue.h>

#include "mid_common.h"
#include "mid_VideoEncoder.h"
#include "module_config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================
#ifdef DUAL_OS
#define WAV_PATH_POWER_OFF     "/bootconfig/audio/Power_Off.wav"
#define WAV_PATH_CARD_ERROR    "/bootconfig/audio/Card_Err.wav"
#define WAV_PATH_CARD_FORMAT_D "/bootconfig/audio/Card_Format_Done.wav"
#define WAV_PATH_CARD_FORMAT   "/bootconfig/audio/ConnectPhone_FormatCard.wav"
#define WAV_PATH_CARD_EJECT    "/bootconfig/audio/Insert_Card.wav"
#define WAV_PATH_CAPTURE       "/bootconfig/audio/Shutter.wav"
#define WAV_PATH_REC_START     "/bootconfig/audio/Rec.wav"
#define WAV_PATH_CARD_FORMAT_F "/bootconfig/audio/Card_Format_Fail.wav"
#define CONFIG_PATH            "/misc" // "/config"
#define WAV_PATH_POWER_ON      CONFIG_PATH "/Power_On.wav"
#define OSD_PATH               CONFIG_PATH "/SigmaStar.bmp"
#define CARDV_FONT_PATH        CONFIG_PATH
#else
#define WAV_PATH_POWER_OFF     "/customer/audio/Power_Off.wav"
#define WAV_PATH_CARD_ERROR    "/customer/audio/Card_Err.wav"
#define WAV_PATH_CARD_FORMAT_D "/customer/audio/Card_Format_Done.wav"
#define WAV_PATH_CARD_FORMAT   "/customer/audio/ConnectPhone_FormatCard.wav"
#define WAV_PATH_CARD_EJECT    "/customer/audio/Insert_Card.wav"
#define WAV_PATH_CAPTURE       "/customer/audio/Shutter.wav"
#define WAV_PATH_REC_START     "/customer/audio/Rec.wav"
#define WAV_PATH_CARD_FORMAT_F "/customer/audio/Card_Format_Fail.wav"
#define CONFIG_PATH            "/customer/" // "/config"
#define WAV_PATH_POWER_ON      CONFIG_PATH "cardv/Power_On.wav"
#define OSD_PATH               CONFIG_PATH "cardv/SigmaStar.bmp"
#define CARDV_FONT_PATH        CONFIG_PATH "cardv"
#endif

#define CARDV_MAX_FPS     60
#define CARDV_DEFAULT_FPS 30

#define FIFO_NAME           "/tmp/cardv_fifo"
#define FIFO_NAME_HIGH_PRIO "/tmp/cardv_internal"

#define REC_STATUS "/tmp/rec_status"
#define MMC_STATUS "/tmp/mmc_status"

#define CARDV_CMD_SEAMLESS    "seamless"
#define CARDV_CMD_STOP_REC    "rec 0"
#define CARDV_CMD_START_REC   "rec 1"
#define CARDV_CMD_EMERG_REC   "rec 2"
#define CARDV_CMD_MD_REC_STOP "rec 4"
#define CARDV_CMD_MD_REC      "rec 5"
#define CARDV_CMD_CAPTURE     "capture"
#define CARDV_CMD_CAPTURE5    "capture 5"
#define CARDV_CMD_ISP_API_BIN "ispapi"
#define CARDV_CMD_SRC_CHANGE  "srcchg"

#define CARDV_MAX_MSG_BUFFER_SIZE 128

#define CARDV_VIDEO_SUFFIX "mp4" // support "ts", "mp4" or "mov"

#define MAX_CAM_NUM (4)

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

typedef struct CarDV_InPortAttr_s
{
    MI_SYS_WindowRect_t stInputCropWin;
    CarDV_BindParam_t   stBindParam;
} CarDV_InPortAttr_t;

typedef struct CarDV_PortAttr_s
{
    MI_BOOL               bUsed;
    MI_BOOL               bCreate;
    MI_BOOL               bEnable;
    MI_BOOL               bMirror;
    MI_BOOL               bFlip;
    MI_SYS_PixelFormat_e  ePixelFormat;
    MI_SYS_CompressMode_e eCompressMode;
    MI_SYS_WindowSize_t   stOrigPortSize;
    MI_SYS_WindowRect_t   stOrigPortCrop;
    MI_U16                u16Depth;
    MI_U16                u16UserDepth;
} CarDV_PortAttr_t;

struct CarDV_StreamLinkNode_t
{
    MI_U8            u8Chn;
    MI_S32           s32Sock;
    MI_VideoEncoder *pEncoder;
    MI_BOOL          bRequstIDR;
    MI_BOOL          bStart;
    SLIST_ENTRY(CarDV_StreamLinkNode_t) next;
};

SLIST_HEAD(stream_Link_head, CarDV_StreamLinkNode_t);

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int isp_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int audio_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int video_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int osd_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int vdf_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int video_pipe_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int display_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int system_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int uvc_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int mux_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int dms_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int adas_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);
int lightdetect_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen);

MI_S32 CarDV_VideoPipeSwitchSensorResIdx(MI_SNR_PADID u32SnrPad, MI_U8 u8ResIndex);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_MODULE_COMMON_
