/*
 * IPC_cardvInfo.h - Sigmastar
 *
 * Copyright (C) 2019 Sigmastar Technology Corp.
 *
 * Author: jeff.li <jeff.li@sigmastar.com.cn>
 * Desc: for Inter-Process Communication
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

#ifndef _IPC_CARDV_INFO_H_
#define _IPC_CARDV_INFO_H_

#include "mi_sys_datatype.h"
#include "DCF.h"
#include <stddef.h>

#include "ldws.h"
#include "fcws.h"
#include "dms/dms_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define IPC_CarInfo_Write_RecInfo(P) \
    IPC_CarInfo_Write_PartInfo(P, sizeof(struct CarDV_RecordInfo), offsetof(struct CarDV_Info, stRecInfo))

#define IPC_CarInfo_Write_CapInfo(P) \
    IPC_CarInfo_Write_PartInfo(P, sizeof(struct CarDV_CaptureInfo), offsetof(struct CarDV_Info, stCapInfo))

#define IPC_CarInfo_Write_DispInfo(P) \
    IPC_CarInfo_Write_PartInfo(P, sizeof(struct CarDV_DisplayInfo), offsetof(struct CarDV_Info, stDispInfo))

#define IPC_CarInfo_Write_SdInfo(P) \
    IPC_CarInfo_Write_PartInfo(P, sizeof(struct CarDV_SDInfo), offsetof(struct CarDV_Info, stSdInfo))

#define IPC_CarInfo_Write_BrowserInfo(P) \
    IPC_CarInfo_Write_PartInfo(P, sizeof(struct CarDV_BrowserInfo), offsetof(struct CarDV_Info, stBrowserInfo))

#define IPC_CarInfo_Write_UIModeInfo(P) \
    IPC_CarInfo_Write_PartInfo(P, sizeof(struct CarDV_UIModeInfo), offsetof(struct CarDV_Info, stUIModeInfo))

#define IPC_CarInfo_Write_AdasInfo(P) \
    IPC_CarInfo_Write_PartInfo(P, sizeof(struct CarDV_AdasInfo), offsetof(struct CarDV_Info, stAdasInfo))

#define IPC_CarInfo_Write_DmsInfo(P) \
    IPC_CarInfo_Write_PartInfo(P, sizeof(struct CarDV_DmsInfo), offsetof(struct CarDV_Info, stDmsInfo))

/* #define IPC_CarInfo_Write_MenuInfo(P) \
    IPC_CarInfo_Write_PartInfo(P, sizeof(struct CarDV_MenuInfo), offsetof(struct CarDV_Info, stMenuInfo)) */

#define IPC_CarInfo_Read_RecInfo(P) \
    IPC_CarInfo_Read_PartInfo(P, sizeof(struct CarDV_RecordInfo), offsetof(struct CarDV_Info, stRecInfo))

#define IPC_CarInfo_Read_CapInfo(P) \
    IPC_CarInfo_Read_PartInfo(P, sizeof(struct CarDV_CaptureInfo), offsetof(struct CarDV_Info, stCapInfo))

#define IPC_CarInfo_Read_DispInfo(P) \
    IPC_CarInfo_Read_PartInfo(P, sizeof(struct CarDV_DisplayInfo), offsetof(struct CarDV_Info, stDispInfo))

#define IPC_CarInfo_Read_SdInfo(P) \
    IPC_CarInfo_Read_PartInfo(P, sizeof(struct CarDV_SDInfo), offsetof(struct CarDV_Info, stSdInfo))

#define IPC_CarInfo_Read_BrowserInfo(P) \
    IPC_CarInfo_Read_PartInfo(P, sizeof(struct CarDV_BrowserInfo), offsetof(struct CarDV_Info, stBrowserInfo))

#define IPC_CarInfo_Read_UIModeInfo(P) \
    IPC_CarInfo_Read_PartInfo(P, sizeof(struct CarDV_UIModeInfo), offsetof(struct CarDV_Info, stUIModeInfo))

#define IPC_CarInfo_Read_AdasInfo(P) \
    IPC_CarInfo_Read_PartInfo(P, sizeof(struct CarDV_AdasInfo), offsetof(struct CarDV_Info, stAdasInfo))

#define IPC_CarInfo_Read_DmsInfo(P) \
    IPC_CarInfo_Read_PartInfo(P, sizeof(struct CarDV_DmsInfo), offsetof(struct CarDV_Info, stDmsInfo))

/* #define IPC_CarInfo_Read_MenuInfo(P) \
    IPC_CarInfo_Read_PartInfo(P, sizeof(struct CarDV_MenuInfo), offsetof(struct CarDV_Info, stMenuInfo)) */

//==============================================================================
//
//                              ENUM & STRUCT DEFINES
//
//==============================================================================

struct CarDV_RecordInfo
{
    // TODO : add what you want.
    MI_U32  u32CurDuration[3];     // Sync MUXER_TYPE_NUM
    MI_U32  u32PreRecdDuration[3]; // Sync MUXER_TYPE_NUM
    MI_BOOL bAudioTrack;
    MI_BOOL bMuxing;      // writing Normal file to storage
    MI_BOOL bMuxingEmerg; // writing Emergency file to storage
    MI_BOOL bMuxingShare; // writing Share file to storage
    MI_BOOL bMuxingMD;    // writing Motion-detect file to storage
};

struct CarDV_CaptureInfo
{
    // TODO : add what you want.
    MI_U32 u32FileCnt;
};

struct CarDV_DisplayInfo
{
    // TODO : add what you want.
    MI_SYS_ChnPort_t stPlayBackChnPort;
    MI_U8            u8SwitchNum;
};

struct CarDV_SDInfo
{
    // TODO : add what you want.
    MI_S32  s32ErrCode;
    MI_U64  u64FreeSize;
    MI_U64  u64TotalSize;
    MI_U64  u64OtherSpace;
    MI_U64  u64ReservedSpace;
    MI_U32  u32TotalLifeTime;
    MI_BOOL bStorageMount;
    MI_BOOL bStorageInsert;
};

struct CarDV_BrowserInfo
{
    // TODO : add what you want.
    MI_U8  u8CamId;
    MI_U8  u8DBId;
    MI_U32 u32CurFileIdx[DB_NUM][CAM_NUM];
};

struct CarDV_UIModeInfo
{
#define UI_VIDEO_MODE    (0)
#define UI_CAPTURE_MODE  (1)
#define UI_BROWSER_MODE  (2)
#define UI_PLAYBACK_MODE (3)
#define UI_MENU_MODE     (4)
    // TODO : add what you want.
    MI_U8 u8Mode;
};

struct CarDV_AdasInfo
{
    // TODO : add what you want.
    MI_BOOL            bCalibVaild;
    ldws_tuning_params stCalibInfo;
    MI_BOOL            bLdwsVaild;
    ldws_params_t      stLdwsInfo;
    MI_BOOL            bFcwsVaild;
    fcws_info_t        stFcwsInfo;
};

struct CarDV_MenuInfo
{
    // Still
    MI_U32 uiIMGSize; // 0
    MI_U32 uiIMGQuality;
    MI_U32 uiBurstShot;
    // Movie
    MI_U32 uiMOVSize;
    MI_U32 uiMOVQuality;
    MI_U32 uiMicSensitivity;
    MI_U32 uiMOVPreRecord;
    MI_U32 uiMOVClipTime;
    MI_U32 uiMOVPowerOffTime; // 8
    MI_U32 uiMOVSoundRecord;
    MI_U32 uiVMDRecTime;
    MI_U32 uiAutoRec;
    MI_U32 uiTimeLapseTime; // 12
    MI_U32 uiSlowMotion;
    MI_U32 uiHDR;
    MI_U32 uiWNR;
    MI_U32 uiNightMode;
    MI_U32 uiParkingMode;
    MI_U32 uiLDWS;
    MI_U32 uiFCWS;
    MI_U32 uiSAG;
    MI_U32 uiContrast;
    MI_U32 uiSaturation;
    MI_U32 uiSharpness;
    MI_U32 uiGamma;
    // Manual
    MI_U32 uiScene;
    MI_U32 uiEV;
    MI_U32 uiISO;
    MI_U32 uiWB;
    MI_U32 uiColor;
    MI_U32 uiEffect; // 30
                     // Playback
    MI_U32 uiPlaybackVideoType;
    MI_U32 uiSlideShowStart;
    MI_U32 uiSlideShowFile; // 33
    MI_U32 uiSlideShowEffect;
    MI_U32 uiSlideShowMusic;
    MI_U32 uiVolume;
    // Edit Tool
    MI_U32 uiFileEdit;
    // Media Tool
    MI_U32 uiMediaSelect; // 38
                          // General
    MI_U32 uiBeep;
    MI_U32 uiAutoPowerOff;
    MI_U32 uiDateTimeFormat;
    MI_U32 uiDateLogoStamp;
    MI_U32 uiGPSStamp;
    MI_U32 uiSpeedStamp;
    MI_U32 uiLanguage;
    MI_U32 uiResetSetting;
    MI_U32 uiFlickerHz;
    MI_U32 uiUSBFunction;
    MI_U32 uiLCDPowerSave;
    MI_U32 uiGsensorSensitivity;
    MI_U32 uiPowerOnGsensorSensitivity;
    MI_U32 uiTimeZone;
    MI_U32 uiMotionDtcSensitivity;
    // Battery
    MI_U32 uiBatteryVoltage;
    // Wifi
    MI_U32 uiWifi;
    // version
    MI_U32 uiVersion; // 56
};

struct CarDV_DmsInfo
{
    // TODO : add what you want.
    STATUS        status;
    bool          bLandmarkValid;
    dms_point     landmarkPoints[DMS_LANDMARK_POINTS];
    dms_face_rect faceRect;
    dms_face_rect preFaceRect;
};

struct CarDV_Info
{
    // TODO : add what you want.
    struct CarDV_RecordInfo  stRecInfo;
    struct CarDV_CaptureInfo stCapInfo;
    struct CarDV_DisplayInfo stDispInfo;
    struct CarDV_SDInfo      stSdInfo;
    struct CarDV_BrowserInfo stBrowserInfo;
    struct CarDV_UIModeInfo  stUIModeInfo;
    struct CarDV_AdasInfo    stAdasInfo;
    struct CarDV_DmsInfo     stDmsInfo;
    // struct CarDV_MenuInfo     stMenuInfo;
};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int IPC_CarInfo_Open(void);
int IPC_CarInfo_Close(void);
int IPC_CarInfo_Write(struct CarDV_Info *pCarInfo);
int IPC_CarInfo_Read(struct CarDV_Info *pCarInfo);
int IPC_CarInfo_Write_PartInfo(void *pCarPartInfo, ssize_t size, ssize_t offset);
int IPC_CarInfo_Read_PartInfo(void *pCarPartInfo, ssize_t size, ssize_t offset);

#ifdef __cplusplus
}
#endif

#endif // _IPC_CARDV_INFO_H_
