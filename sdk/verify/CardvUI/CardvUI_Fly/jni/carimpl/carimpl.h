/*
 * Carimpl.h
 *
 *  Created on: 2019-10-10
 *      Author: lei.qin
 */

#ifndef _CARDV_IMPL_H_
#define _CARDV_IMPL_H_

#include <stdint.h>
#include "usbdetect.h"
#include "mi_common_datatype.h"
#include "IPC_msg.h"
#include "IPC_cardvInfo.h"
#include "DCF.h"
#include <linux/input.h>
#include "MenuCommon.h"

#ifndef BYTE
typedef unsigned char   BYTE;
#endif
#ifndef UINT
typedef unsigned int    UINT;
#endif
#ifndef UINT16
typedef unsigned short  UINT16;
#endif
/* MAX/MIN/ABS macors */
/**
 * \def MAX(x, y)
 * \brief A macro returns the maximum of \a x and \a y.
 */
#ifndef MAX
#define MAX(x, y) (((x) > (y))?(x):(y))
#endif
/**
 * \def MIN(x, y)
 * \brief A macro returns the minimum of \a x and \a y.
 */
#ifndef MIN
#define MIN(x, y) (((x) < (y))?(x):(y))
#endif
/**
 * \def ABS(x)
 * \brief A macro returns the absolute value of \a x.
 */
#ifndef ABS
#define ABS(x) (((x)<0) ? -(x) : (x))
#endif

#define FB0 "/dev/fb0"
#define FB1 "/dev/fb1"

#define FIFO_NAME                           "/tmp/cardv_fifo"
#define REC_STATUS                          "/tmp/rec_status"
#define MMC_STATUS                          "/tmp/mmc_status"
#define CGI_PROCESS_PATH                    "/customer/wifi/webserver/www/CGI_PROCESS.sh"

#define CARDV_CMD_STOP_REC                  "rec 0"
#define CARDV_CMD_START_REC                 "rec 1"
#define CARDV_CMD_EMERG_REC                 "rec 2"
#define CARDV_CMD_CAPTURE                   "capture"
#define CARDV_CMD_START_PREVIEW             "disp start"
#define CARDV_CMD_STOP_PREVIEW              "disp stop"
#define CARDV_CMD_START_PLAYBACK            "disp playstart"
#define CARDV_CMD_STOP_PLAYBACK             "disp playstop"
#define CARDV_CMD_EXIT_APP                  "quit"
#define CARDV_CMD_SYNC_DCF                  "dcf sync"
#define CARDV_CMD_VIDEO_H26X_FLOW           "video 0"
#define CARDV_CMD_VIDEO_JPEG_FLOW           "video 1"
#define CARDV_CMD_VIDEO_H26X_JPEG_FLOW      "video 2"
#define CARDV_CMD_VIDEO_SENSOR_FLOW         "video 3"

typedef enum {
    CGI_CMD_TYPE = 0,
    FIFO_CMD_TYPE,
    SCRIPT_CMD_TYPE,
    MAZ_CMD_TYPE
} CARIMPL_CMD_TYPE;

typedef enum {
    NET_STATUS_INIT_FAIL, // no interface
    NET_STATUS_UP,        // interface up, remote disconnect
    NET_STATUS_READY,     // remote connected
    NET_STATUS_WEAK,      // weak signal, not stable
    NET_STATUS_NOIP,      // Cannot get IP for DHCP server, or Crypto TYPE/KEY not match
    NET_STATUS_DOWN,      // interface is down or cann't transfer data
    NET_STATUS_UNKNOW,
} NET_STATUS_TYPE;

#define SHOW_BAT_ICON(PTR)                               \
{                                                        \
    static char u8Idx = 0;                               \
    char str[32] = {0};                                  \
    u8Idx++;                                             \
    if (u8Idx > 3) {                                     \
        u8Idx = 1;                                       \
        sprintf(str, "charge/%d-battery.png", 5);        \
    } else if(u8Idx == 2) {                              \
        sprintf(str, "charge/%d-battery.png", 7);        \
    } else {                                             \
        sprintf(str, "charge/%d-battery.png", 10);       \
    }                                                    \
    PTR->setBackgroundPic(str);                          \
}                                                        \

#define SHOW_SD_ICON(PTR, bSdStatus)                     \
{                                                        \
    IPC_CarInfo_Read_SdInfo(&carimpl.stSdInfo);          \
    if (!bSdStatus && IMPL_SD_ISINSERT) {                \
        bSdStatus = TRUE;                                \
        PTR->setBackgroundPic("cardv/video_sd_in.png");  \
    } else if (bSdStatus && !IMPL_SD_ISINSERT) {         \
        bSdStatus = FALSE;                               \
        PTR->setBackgroundPic("cardv/video_sd_out.png"); \
    }                                                    \
}                                                        \

/*******************************************************/
/*******************************************************/

extern MI_U8 gu8LastUIMode;
extern CarDV_Info carimpl;
#define IMPL_REC_STATUS         (carimpl.stRecInfo.bMuxing)
#define IMPL_REC_DURATION       (carimpl.stRecInfo.u32CurDuration[0])
#define IMPL_EMERG_REC_STATUS   (carimpl.stRecInfo.bMuxingEmerg)
#define IMPL_EMERG_REC_DURATION (carimpl.stRecInfo.u32CurDuration[1])
#define IMPL_MD_REC_STATUS      (carimpl.stRecInfo.bMuxingMD)
#define IMPL_SD_ISMOUNT         (carimpl.stSdInfo.bStorageMount)
#define IMPL_SD_ISINSERT        (carimpl.stSdInfo.bStorageInsert)
#define IMPL_CAP_PICNUM         (carimpl.stCapInfo.u32FileCnt)

bool          Carimpl_SyncAllSetting(void);
 /*===========================================================================
 * DCF related
 *===========================================================================*/
int           Carimpl_DcfMount(void);
int           Carimpl_DcfUnmount(void);
bool          Carimpl_IsSubFileEnable(void);
unsigned int  Carimpl_GetTotalFiles(unsigned char u8DB, unsigned char u8CamId);
unsigned int  Carimpl_GetTotalFiles(void);
unsigned int  Carimpl_GetCurFileIdx(unsigned char u8DB, unsigned char u8CamId);
unsigned int  Carimpl_GetCurFileIdx(void);
char*         Carimpl_GetCurFileName(unsigned int u32FileIdx);
char*         Carimpl_GetCurSubFileName(unsigned int u32FileIdx);
char*         Carimpl_GetPairFileName(char* pszFileName, unsigned char u8CamId);
bool          Carimpl_SetCurFileIdx(unsigned char u8DB, unsigned char u8CamId, unsigned int u32FileIdx);
bool          Carimpl_SetCurFileIdx(unsigned int u32FileIdx);
bool          Carimpl_SetCurFolder(unsigned char u8DB, unsigned char u8CamId);
unsigned char Carimpl_GetCurDB(void);
unsigned char Carimpl_GetCurCamId(void);
int           Carimpl_DeleteFile(unsigned int u32FileIdx);
int           Carimpl_ProtectFile(unsigned int u32FileIdx);
int           Carimpl_UnProtectFile(unsigned int u32FileIdx);
bool          Carimpl_IsProtectFile(unsigned int u32FileIdx);
void          carimpl_SyncDCF(void);
 /*===========================================================================
 * Demux related
 *===========================================================================*/
int64_t       Carimpl_GetFileDuration(char *pFileName);
int           Carimpl_GetFileThumbnail(char *pFileName, char *pThumbFileName);
int           Carimpl_GetJpegFileThumbnail(char *pFileName, char *pThumbFileName);
int           Carimpl_GetJpegFileResolution(const char *szFileName, unsigned int *pu32Width, unsigned int *pu32Height);
 /*===========================================================================
 * Video related
 *===========================================================================*/
void          carimpl_VideoFunc_StartRecording(bool bStart);
void          carimpl_VideoFunc_StartEmergRecording(void);
void          carimpl_VideoFunc_Capture(void);
void          carimpl_VideoFunc_SetResolution(void);
void          carimpl_VideoFunc_SetQuality(void);
void          carimpl_VideoFunc_SetMicSensitivity(void);
void          carimpl_VideoFunc_SetPreRecord(void);
void          carimpl_VideoFunc_SetClipTime(void);
void          carimpl_VideoFunc_SetNightMode(void);
void          carimpl_VideoFunc_SetPowerOffTime(void);
void          carimpl_VideoFunc_SetSoundRecord(void);
void          carimpl_VideoFunc_SetRecordVolume(void);
void          carimpl_VideoFunc_SetVMDRecordTime(void);
void          carimpl_VideoFunc_SetAutoReord(void);
void          carimpl_VideoFunc_SetTimelapse(void);
void          carimpl_VideoFunc_SetSlowMotion(void);
void          carimpl_VideoFunc_SetParkingMode(void);
void          carimpl_VideoFunc_SetLDWSMode(void);
void          carimpl_VideoFunc_SetFCWSMode(void);
void          carimpl_VideoFunc_SetSAGMode(void);
void          carimpl_VideoFunc_SetHDRMode(void);
void          carimpl_VideoFunc_SetWNRMode(void);
 /*===========================================================================
 * Capture related
 *===========================================================================*/
void          carimpl_CaptureFunc_SetResolution(void);
void          carimpl_CaptureFunc_SetBurstShot(void);
 /*===========================================================================
 * Playback related
 *===========================================================================*/
void          carimpl_PlaybackFunc_setVolume(void);
void          carimpl_PlaybackFunc_setVideoType(void);
void          carimpl_PlaybackFunc_setImageType(void);
void          carimpl_PlaybackFunc_DeleteOne(void);
void          carimpl_PlaybackFunc_DeleteAll_Video(void);
void          carimpl_PlaybackFunc_DeleteAll_Image(void);
void          carimpl_PlaybackFunc_ProtectOne(void);
void          carimpl_PlaybackFunc_ProtectAll_Video(void);
void          carimpl_PlaybackFunc_ProtectAll_Image(void);
void          carimpl_PlaybackFunc_UnProtectOne(void);
void          carimpl_PlaybackFunc_UnProtectAll_Video(void);
void          carimpl_PlaybackFunc_UnProtectAll_Image(void);
 /*===========================================================================
 * Media tool related
 *===========================================================================*/
void          carimpl_MediaToolFunc_FormatSDCard(void);
 /*===========================================================================
 * Manual setting related
 *===========================================================================*/
void          carimpl_IQFunc_SetEV(void);
void          carimpl_IQFunc_SetISO(void);
void          carimpl_IQFunc_SetWB(void);
void          carimpl_IQFunc_SetColor(void);
void          carimpl_IQFunc_SetEffect(void);
void          carimpl_IQFunc_SetContrastAdjust(void);
void          carimpl_IQFunc_SetSaturationAdjust(void);
void          carimpl_IQFunc_SetSharpnessAdjust(void);
void          carimpl_IQFunc_SetGammaAdjust(void);
void          carimpl_IQFunc_SetFlicker(void);
 /*===========================================================================
 * Genera setting related
 *===========================================================================*/
void          carimpl_GeneralFunc_SetBeep(void);
void          carimpl_GeneralFunc_SetLcdBrightness(void);
void          carimpl_GeneralFunc_SetAutoPowerOff(void);
void          carimpl_GeneralFunc_SetDatetimeFormat(void);
void          carimpl_GeneralFunc_SetDatelogoStamp(void);
void          carimpl_GeneralFunc_SetGpsStamp(void);
void          carimpl_GeneralFunc_SetSpeedStamp(void);
void          carimpl_GeneralFunc_SetLanguage(void);
void          carimpl_GeneralFunc_SetResetSetting(void);
void          carimpl_GeneralFunc_SetUsbFunction(void);
void          carimpl_GeneralFunc_SetLcdPowerSave(void);
void          carimpl_GeneralFunc_setGSensorSensitivity(void);
void          carimpl_GeneralFunc_SetPowerOnGSensorSensitivity(void);
void          carimpl_SysFunc_SetPowerOnGSensorSensitivity(void);
void          carimpl_GeneralFunc_SetMotionDetect(void);
void          carimpl_GeneralFunc_SetTimeZone(void);
void          carimpl_GeneralFunc_SetWifiOnOff(void);
#ifdef SUPPORT_CARLIFE
void          carimpl_GeneralFunc_SetCarLifeOnOff(void);
#endif
void          carimpl_GeneralFunc_GetSoftwareVersion(void);
 /*===========================================================================
 * Warning message related
 *===========================================================================*/
void          carimpl_show_wmsg(bool bEnable, WMSG_INFO WMSGInfo);
WMSG_INFO     carimpl_get_wmsginfo(void);
 /*===========================================================================
 * Other control related
 *===========================================================================*/
MI_S32        carimpl_refresh_display_yuv420p(MI_S32 s32DispWidth, MI_S32 s32DispHeight, void *pYData, void *pUData, void *pVData);
MI_S32        carimpl_refresh_display_yuv422(MI_S32 s32DispWidth, MI_S32 s32DispHeight, void *pYuvData);
MI_S32        carimpl_refresh_display_yuv422p(MI_S32 s32DispWidth, MI_S32 s32DispHeight, void *pYData, void *pUData, void *pVData);
void          Carimpl_Send2Fifo(const void *cmd, size_t size);
void          carimpl_update_status(const char *status_name, const char *status, size_t size);
void          carimpl_MenuSetting_Init(void);
void          carimpl_GeneralFunc_SetPreviewCamIdx(void);
char*         carimpl_GetSettingCmdBuf(void);
void          carimpl_set_colorkey(const char * dev, bool bEnable);
void          carimpl_set_alpha_blending(const char * dev, bool bEnable, MI_U8 u8GlobalAlpha);
void          carimpl_msg_handler(struct IPCMsgBuf *pMsgBuf);
void          carimpl_Poweroff_handler(void);
void          carimpl_VideoH26xInit(bool bInit);
void          carimpl_VideoJpegInit(bool bInit);
void          carimpl_VideoAllInit(bool bInit);
void          carimpl_VideoSensorInit(bool bInit);

NET_STATUS_TYPE check_wifi_status(void);

#endif /* _CARDV_IMPL_H_ */
