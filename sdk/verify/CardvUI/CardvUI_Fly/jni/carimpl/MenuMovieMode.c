#include "MenuCommon.h"
#include "carimpl.h"

/*===========================================================================
 * Local function
 *===========================================================================*/

bool MenuItemMovieMode(PSMENUITEM pItem);
bool MenuItemMovieQuality(PSMENUITEM pItem);
bool MenuItemMICSensitivity(PSMENUITEM pItem);
bool MenuItemMoviePreRecord(PSMENUITEM pItem);
bool MenuItemMovieClipTime(PSMENUITEM pItem);
bool MenuItemMovieNightMode(PSMENUITEM pItem);
bool MenuItemMoviePowerOffTime(PSMENUITEM pItem);
bool MenuItemMovieSoundRecord(PSMENUITEM pItem);
bool MenuItemMovieRecordVolumeAdjust(PSMENUITEM pItem);
bool MenuItemMovieVMDRecTime(PSMENUITEM pItem);
bool MenuItemMovieAutoRec(PSMENUITEM pItem);
bool MenuItemMovieTimelapse(PSMENUITEM pItem);   // VIDEO_REC_TIMELAPSE_EN
bool MenuItemMovieParkingMode(PSMENUITEM pItem);
bool MenuItemMovieLDWSMode(PSMENUITEM pItem);
bool MenuItemMovieFCWSMode(PSMENUITEM pItem);
bool MenuItemMovieSAGMode(PSMENUITEM pItem);
bool MenuItemMovieHDRMode(PSMENUITEM pItem);
bool MenuItemMovieSlowMotionMode(PSMENUITEM pItem);
bool MenuItemMovieWNRMode(PSMENUITEM pItem);

int  MenuGetDefault_Movie(PSMENUSTRUCT pMenu);

/*===========================================================================
 * Extern varible
 *===========================================================================*/

extern SMENUITEM sItemScence;
extern SMENUITEM sItemEV;
extern SMENUITEM sItemWB;
extern SMENUITEM sItemISO;
extern SMENUITEM sItemColor;
extern SMENUITEM sItemEffect;
extern SMENUITEM sItemContrast;
extern SMENUITEM sItemSaturation;
extern SMENUITEM sItemSharpness;
extern SMENUITEM sItemGamma;

 /*===========================================================================
 * Global varible : Menu
 *===========================================================================*/

extern SMENUSTRUCT sMainMenuVideo;

extern SMENUSTRUCT sSubMovieMode;
extern SMENUSTRUCT sSubMovieQuality;
extern SMENUSTRUCT sSubMICSensitivity;
extern SMENUSTRUCT sSubMoviePreRecord;
extern SMENUSTRUCT sSubMovieClipTime;
extern SMENUSTRUCT sSubMovieNightMode;
extern SMENUSTRUCT sSubMoviePowerOffTime;
extern SMENUSTRUCT sSubMovieSoundRecord;
extern SMENUSTRUCT sSubMovieRecordVolume;
extern SMENUSTRUCT sSubMovieVMDRecTime;
extern SMENUSTRUCT sSubMovieAutoRec;
extern SMENUSTRUCT sSubMovieTimelapse;     // VIDEO_REC_TIMELAPSE_EN
extern SMENUSTRUCT sSubMovieParkingMode;
extern SMENUSTRUCT sSubMovieLDWSMode;
extern SMENUSTRUCT sSubMovieFCWSMode;
extern SMENUSTRUCT sSubMovieSAGMode;
extern SMENUSTRUCT sSubMovieHDRMode;
extern SMENUSTRUCT sSubMovieSlowMotionMode;
extern SMENUSTRUCT sSubMovieWNRMode;

 /*===========================================================================
 * Global varible : Item Structure
 *===========================================================================*/


// Movie Mode
#if (MENU_MOVIE_SIZE_EN)
SMENUITEM sItemMovieMode                = { ITEMID_MOVIE_MODE,           IDS_DS_MOVIE_MODE,          &sSubMovieMode, 0 };
#if (MENU_MOVIE_SIZE_4K_25P_EN)
SMENUITEM sItemMovieMode_4K_25P         = { ITEMID_MOVIE_MODE_4K_25P,    IDS_DS_MOVIE_MODE_4K_25,      NULL, MenuItemMovieMode };
#endif
#if (MENU_MOVIE_SIZE_1440_30P_EN)
SMENUITEM sItemMovieMode_1440_30P       = { ITEMID_MOVIE_MODE_1440_30P,  IDS_DS_MOVIE_MODE_1440_30,      NULL, MenuItemMovieMode };
#endif
#if (MENU_MOVIE_SIZE_SHD_30P_EN)
SMENUITEM sItemMovieMode_SHD            = { ITEMID_MOVIE_MODE_SHD,       IDS_DS_MOVIE_MODE_SHD,      NULL, MenuItemMovieMode };
#endif
#if (MENU_MOVIE_SIZE_SHD_25P_EN)
SMENUITEM sItemMovieMode_SHD_25P        = { ITEMID_MOVIE_MODE_SHD_25P,   IDS_DS_MOVIE_MODE_SHD_25P,  NULL, MenuItemMovieMode };
#endif
#if (MENU_MOVIE_SIZE_1080_60P_EN)
SMENUITEM sItemMovieMode_FHD_60P        = { ITEMID_MOVIE_MODE_FHD_60P,   IDS_DS_MOVIE_MODE_FHD_60P,  NULL, MenuItemMovieMode };
#endif
#if (MENU_MOVIE_SIZE_1080P_EN)
SMENUITEM sItemMovieMode_FHD            = { ITEMID_MOVIE_MODE_FHD,       IDS_DS_MOVIE_MODE_FHD,      NULL, MenuItemMovieMode };
#endif
#if (MENU_MOVIE_SIZE_1080P_30_HDR_EN)
SMENUITEM sItemMovieMode_FHD_HDR        = { ITEMID_MOVIE_MODE_FHD_HDR,   IDS_DS_MOVIE_MODE_FHD_HDR,  NULL, MenuItemMovieMode };
#endif
#if (MENU_MOVIE_SIZE_720P_EN)
SMENUITEM sItemMovieMode_HD             = { ITEMID_MOVIE_MODE_HD,        IDS_DS_MOVIE_MODE_HD,       NULL, MenuItemMovieMode };
#endif
#if (MENU_MOVIE_SIZE_720_60P_EN)
SMENUITEM sItemMovieMode_HD_60P         = { ITEMID_MOVIE_MODE_HD_60P,    IDS_DS_MOVIE_MODE_HD_60P,   NULL, MenuItemMovieMode };
#endif
#if (MENU_MOVIE_SIZE_VGA30P_EN)
SMENUITEM sItemMovieMode_VGA_30P        = { ITEMID_MOVIE_MODE_VGA_30P,   IDS_DS_MOVIE_MODE_VGA_30P,  NULL, MenuItemMovieMode };
#endif
#endif

// Movie Quality
#if (MENU_MOVIE_QUALITY_EN)
SMENUITEM sItemMovieQuality             = { ITEMID_MOVIE_QUALITY,      IDS_DS_MOVIE_QUALITY,      &sSubMovieQuality, 0 };
#if (MENU_MOVIE_QUALITY_SUPER_FINE_EN)
SMENUITEM sItemMovieQuality_Super_Fine  = { ITEMID_MQUALITY_SUPERFINE, IDS_DS_QUALITY_SUPERFINE,  NULL, MenuItemMovieQuality };
#endif
#if (MENU_MOVIE_QUALITY_FINE_EN)
SMENUITEM sItemMovieQuality_Fine        = { ITEMID_MQUALITY_FINE,      IDS_DS_QUALITY_FINE,       NULL, MenuItemMovieQuality };
#endif
#endif

// MIC Sensitivity
#if (MENU_MOVIE_MIC_SEN_EN)
SMENUITEM sItemMICSensitivity           = { ITEMID_MIC_SENSITIVITY,      IDS_DS_MIC_SENSITIVITY, &sSubMICSensitivity, 0 };
#if (MENU_MOVIE_MIC_SEN_STANDARD_EN)
SMENUITEM sItemMICSensitivity_Standard  = { ITEMID_MIC_SENSITI_STANDARD, IDS_DS_STANDARD,   NULL, MenuItemMICSensitivity };
#endif
#if (MENU_MOVIE_MIC_SEN_LOW_EN)
SMENUITEM sItemMICSensitivity_Low       = { ITEMID_MIC_SENSITI_LOW,      IDS_DS_LOW,        NULL, MenuItemMICSensitivity };
#endif
#endif

// Movie PreRecord
#if (MENU_MOVIE_PRE_RECORD_EN)
SMENUITEM sItemMoviePreRecord           = { ITEMID_MOVIE_PRE_RECORD,     IDS_DS_MOVIE_PRE_RECORD, &sSubMoviePreRecord, 0 };
#if (MENU_MOVIE_PRE_RECORD_ON_EN)
SMENUITEM sItemMoviePreRecord_On        = { ITEMID_MOVIE_PRE_RECORD_ON,  IDS_DS_ON,         NULL, MenuItemMoviePreRecord };
#endif
#if (MENU_MOVIE_PRE_RECORD_OFF_EN)
SMENUITEM sItemMoviePreRecord_Off       = { ITEMID_MOVIE_PRE_RECORD_OFF, IDS_DS_OFF,        NULL, MenuItemMoviePreRecord };
#endif
#endif

// Movie Clip Time
#if (MENU_MOVIE_CLIP_TIME_EN)
SMENUITEM sItemMovieClipTime            = { ITEMID_MOVIE_CLIPTIME,       IDS_MOVIE_CLIPTIME,     &sSubMovieClipTime, 0 };
#if (MENU_MOVIE_CLIP_TIME_OFF_EN)
SMENUITEM sItemMovieClipTime_off        = { ITEMID_MOVIE_CLIPTIME_OFF,   IDS_DS_OFF,             NULL, MenuItemMovieClipTime };
#endif
#if (MENU_MOVIE_CLIP_TIME_1MIN_EN)
SMENUITEM sItemMovieClipTime_1min       = { ITEMID_MOVIE_CLIPTIME_1MIN,  IDS_TIME_1MIN,          NULL, MenuItemMovieClipTime };
#endif
#if (MENU_MOVIE_CLIP_TIME_2MIN_EN)
SMENUITEM sItemMovieClipTime_2min       = { ITEMID_MOVIE_CLIPTIME_2MIN,  IDS_TIME_2MIN,          NULL, MenuItemMovieClipTime };
#endif
#if (MENU_MOVIE_CLIP_TIME_3MIN_EN)
SMENUITEM sItemMovieClipTime_3min       = { ITEMID_MOVIE_CLIPTIME_3MIN,  IDS_TIME_3MIN,          NULL, MenuItemMovieClipTime };
#endif
#if (MENU_MOVIE_CLIP_TIME_5MIN_EN)
SMENUITEM sItemMovieClipTime_5min       = { ITEMID_MOVIE_CLIPTIME_5MIN,  IDS_TIME_5MIN,          NULL, MenuItemMovieClipTime };
#endif
#if (MENU_MOVIE_CLIP_TIME_10MIN_EN)
SMENUITEM sItemMovieClipTime_10min      = { ITEMID_MOVIE_CLIPTIME_10MIN, IDS_TIME_10MIN,         NULL, MenuItemMovieClipTime };
#endif
#if (MENU_MOVIE_CLIP_TIME_30MIN_EN)
SMENUITEM sItemMovieClipTime_30min      = { ITEMID_MOVIE_CLIPTIME_30MIN, IDS_TIME_1MIN,          NULL, MenuItemMovieClipTime };
#endif
#endif

// Movie Night mode
#if (MENU_MOVIE_NIGHT_MODE_EN)
SMENUITEM sItemMovieNightMode           = { ITEMID_MOVIE_NIGHT_MODE,      IDS_DS_NIGHT_MODE,     &sSubMovieNightMode };
SMENUITEM sItemMovieNightMode_On        = { ITEMID_MOVIE_NIGHT_MODE_ON,   IDS_DS_ON,             NULL, MenuItemMovieNightMode };
SMENUITEM sItemMovieNightMode_Off       = { ITEMID_MOVIE_NIGHT_MODE_OFF,  IDS_DS_OFF,            NULL, MenuItemMovieNightMode };
#endif

// Movie PowerOff Time
#if (MENU_MOVIE_POWER_OFF_DELAY_EN)
SMENUITEM sItemMoviePowerOffTime        = { ITEMID_MOVIE_POWEROFF_TIME,          IDS_MOVIE_POWEROFF_TIME,   &sSubMoviePowerOffTime, 0 };
#if (MENU_MOVIE_POWER_OFF_DELAY_0SEC_EN)
SMENUITEM sItemMoviePowerOffTime_0min   = { ITEMID_MOVIE_POWEROFF_TIME_0MIN,     IDS_TIME_0MIN,  NULL, MenuItemMoviePowerOffTime };
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_5SEC_EN)
SMENUITEM sItemMoviePowerOffTime_5sec   = { ITEMID_MOVIE_POWEROFF_TIME_5SEC,     IDS_TIME_5SEC,  NULL, MenuItemMoviePowerOffTime };
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_10SEC_EN)
SMENUITEM sItemMoviePowerOffTime_10sec  = { ITEMID_MOVIE_POWEROFF_TIME_10SEC,    IDS_TIME_10SEC, NULL, MenuItemMoviePowerOffTime };
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_15SEC_EN)
SMENUITEM sItemMoviePowerOffTime_15sec  = { ITEMID_MOVIE_POWEROFF_TIME_15SEC,    IDS_TIME_15SEC, NULL, MenuItemMoviePowerOffTime };
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_30SEC_EN)
SMENUITEM sItemMoviePowerOffTime_30sec  = { ITEMID_MOVIE_POWEROFF_TIME_30SEC,    IDS_TIME_30SEC, NULL, MenuItemMoviePowerOffTime };
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_1MIN_EN)
SMENUITEM sItemMoviePowerOffTime_1min   = { ITEMID_MOVIE_POWEROFF_TIME_1MIN,     IDS_TIME_1MIN,  NULL, MenuItemMoviePowerOffTime };
#endif
#endif

// Movie Sound Record
#if (MENU_MOVIE_SOUND_RECORD_EN)
SMENUITEM sItemMovieSoundRecord         = { ITEMID_MOVIE_SOUND_RECORD,           IDS_MOVIE_SOUND_RECORD, &sSubMovieSoundRecord,  0 };
SMENUITEM sItemMovieSoundRecord_On      = { ITEMID_MOVIE_SOUND_RECORD_ON,        IDS_DS_ON,      NULL, MenuItemMovieSoundRecord };
SMENUITEM sItemMovieSoundRecord_Off     = { ITEMID_MOVIE_SOUND_RECORD_OFF,       IDS_DS_OFF,     NULL, MenuItemMovieSoundRecord };
#endif

#if (MENU_MOVIE_RECORD_VOLUME_EN)
SMENUITEM sItemMovieRecordVolume        = { ITEMID_MOVIE_RECORD_VOLUME,          IDS_MOVIE_RECORD_VOLUME, &sSubMovieRecordVolume, 0 };
SMENUITEM sItemMovieRecordVolume_DEC    = { ITEMID_MOVIE_RECORD_VOLUME_DEC,      IDS_DS_DEC,     NULL, MenuItemMovieRecordVolumeAdjust };
SMENUITEM sItemMovieRecordVolume_INC    = { ITEMID_MOVIE_RECORD_VOLUME_INC,      IDS_DS_INC,     NULL, MenuItemMovieRecordVolumeAdjust };
#endif

// Motion Record Time
#if (MENU_MOVIE_VMD_REC_TIME_EN)
SMENUITEM sItemMovieVMDRecTime          = { ITEMID_MOVIE_VMD_REC_TIME,       IDS_VMD_REC_TIME,   &sSubMovieVMDRecTime, 0 };
#if (MENU_MOVIE_VMD_REC_TIME_5SEC_EN)
SMENUITEM sItemMovieVMDRecTime_5sec     = { ITEMID_MOVIE_VMD_REC_TIME_5SEC,  IDS_TIME_5SEC,      NULL, MenuItemMovieVMDRecTime };
#endif
#if (MENU_MOVIE_VMD_REC_TIME_10SEC_EN)
SMENUITEM sItemMovieVMDRecTime_10sec    = { ITEMID_MOVIE_VMD_REC_TIME_10SEC, IDS_TIME_10SEC,     NULL, MenuItemMovieVMDRecTime };
#endif
#if (MENU_MOVIE_VMD_REC_TIME_30SEC_EN)
SMENUITEM sItemMovieVMDRecTime_30sec    = { ITEMID_MOVIE_VMD_REC_TIME_30SEC, IDS_TIME_30SEC,     NULL, MenuItemMovieVMDRecTime };
#endif
#if (MENU_MOVIE_VMD_REC_TIME_1MIN_EN)
SMENUITEM sItemMovieVMDRecTime_1min     = { ITEMID_MOVIE_VMD_REC_TIME_1MIN,  IDS_TIME_1MIN,      NULL, MenuItemMovieVMDRecTime };
#endif
#endif

// Movie Auto Rec
#if (MENU_MOVIE_AUTO_REC_EN)
SMENUITEM sItemMovieAutoRec             = { ITEMID_MOVIE_AUTO_REC,          IDS_AUTO_REC,     &sSubMovieAutoRec, 0 };
SMENUITEM sItemMovieAutoRec_On          = { ITEMID_MOVIE_AUTO_REC_ON,       IDS_DS_ON,        NULL,    MenuItemMovieAutoRec };
SMENUITEM sItemMovieAutoRec_Off         = { ITEMID_MOVIE_AUTO_REC_OFF,      IDS_DS_OFF,       NULL,    MenuItemMovieAutoRec };
#endif

#if (MENU_MOVIE_TIMELAPSE_MODE_EN)  // VIDEO_REC_TIMELAPSE_EN
SMENUITEM sItemMovieTimelapse           = { ITEMID_MOVIE_TIMELAPSE,          IDS_DS_VIDEO_TIMELAPSE,       &sSubMovieTimelapse, 0 };
#if (MENU_VIDEO_TIMELAPSE_MODE_OFF_EN)
SMENUITEM sItemMovieTimelapse_Off       = { ITEMID_MOVIE_TIMELAPSE_OFF,      IDS_DS_OFF,           NULL,   MenuItemMovieTimelapse };
#endif
#if (MENU_VIDEO_TIMELAPSE_MODE_1SEC_EN)
SMENUITEM sItemMovieTimelapse_1Sec      = { ITEMID_MOVIE_TIMELAPSE_1SEC,     IDS_TIME_1SEC,        NULL,   MenuItemMovieTimelapse };
#endif
#if (MENU_VIDEO_TIMELAPSE_MODE_5SEC_EN)
SMENUITEM sItemMovieTimelapse_5Sec      = { ITEMID_MOVIE_TIMELAPSE_5SEC,     IDS_TIME_5SEC,        NULL,   MenuItemMovieTimelapse };
#endif
#if (MENU_VIDEO_TIMELAPSE_MODE_10SEC_EN)
SMENUITEM sItemMovieTimelapse_10Sec     = { ITEMID_MOVIE_TIMELAPSE_10SEC,    IDS_TIME_10SEC,       NULL,   MenuItemMovieTimelapse };
#endif
#if (MENU_VIDEO_TIMELAPSE_MODE_30SEC_EN)
SMENUITEM sItemMovieTimelapse_30Sec     = { ITEMID_MOVIE_TIMELAPSE_30SEC,    IDS_TIME_30SEC,       NULL,   MenuItemMovieTimelapse };
#endif
#if (MENU_VIDEO_TIMELAPSE_MODE_60SEC_EN)
SMENUITEM sItemMovieTimelapse_60Sec     = { ITEMID_MOVIE_TIMELAPSE_60SEC,    IDS_TIME_60SEC,       NULL,   MenuItemMovieTimelapse };
#endif
#endif

#if (MENU_MOVIE_PARKING_MODE_EN)
SMENUITEM sItemMovieParkingMode         = { ITEMID_MOVIE_PARKING_MODE,       IDS_DS_POWERON_GSENSOR_SENSITIVETY,  &sSubMovieParkingMode, 0 };
SMENUITEM sItemMovieParkingMode_On      = { ITEMID_MOVIE_PARKING_MODE_ON,    IDS_DS_ON,            NULL,       MenuItemMovieParkingMode };
SMENUITEM sItemMovieParkingMode_Off     = { ITEMID_MOVIE_PARKING_MODE_OFF,   IDS_DS_OFF,           NULL,       MenuItemMovieParkingMode };
#endif

// LDWS On/Off
#if (MENU_MOVIE_LDWS_MODE_EN)
SMENUITEM sItemMovieLDWSMode            = { ITEMID_MOVIE_LDWS_MODE,          IDS_DS_LDWS,          &sSubMovieLDWSMode, 0 };
SMENUITEM sItemMovieLDWSMode_On         = { ITEMID_MOVIE_LDWS_MODE_ON,       IDS_DS_ON,            NULL,       MenuItemMovieLDWSMode };
SMENUITEM sItemMovieLDWSMode_Off        = { ITEMID_MOVIE_LDWS_MODE_OFF,      IDS_DS_OFF,           NULL,       MenuItemMovieLDWSMode };
#endif

// FCWS On/Off
#if (MENU_MOVIE_FCWS_MODE_EN)
SMENUITEM sItemMovieFCWSMode            = { ITEMID_MOVIE_FCWS_MODE,          IDS_DS_FCWS,          &sSubMovieFCWSMode, 0 };
SMENUITEM sItemMovieFCWSMode_On         = { ITEMID_MOVIE_FCWS_MODE_ON,       IDS_DS_ON,            NULL,       MenuItemMovieFCWSMode };
SMENUITEM sItemMovieFCWSMode_Off        = { ITEMID_MOVIE_FCWS_MODE_OFF,      IDS_DS_OFF,           NULL,       MenuItemMovieFCWSMode };
#endif

// SAG On/Off
#if (MENU_MOVIE_SAG_MODE_EN)
SMENUITEM sItemMovieSAGMode            = { ITEMID_MOVIE_SAG_MODE,            IDS_DS_SAG,           &sSubMovieSAGMode, 0 };
SMENUITEM sItemMovieSAGMode_On         = { ITEMID_MOVIE_SAG_MODE_ON,         IDS_DS_ON,            NULL,       MenuItemMovieSAGMode };
SMENUITEM sItemMovieSAGMode_Off        = { ITEMID_MOVIE_SAG_MODE_OFF,        IDS_DS_OFF,           NULL,       MenuItemMovieSAGMode };
#endif

// HDR On/Off
#if (MENU_MOVIE_HDR_MODE_EN)
SMENUITEM sItemMovieHDRMode            = { ITEMID_MOVIE_HDR_MODE,            IDS_DS_HDR,           &sSubMovieHDRMode, 0 };
SMENUITEM sItemMovieHDRMode_On         = { ITEMID_MOVIE_HDR_MODE_ON,         IDS_DS_ON,            NULL,       MenuItemMovieHDRMode };
SMENUITEM sItemMovieHDRMode_Off        = { ITEMID_MOVIE_HDR_MODE_OFF,        IDS_DS_OFF,           NULL,       MenuItemMovieHDRMode };
#endif

// SLOWMOTION
#if (MENU_MOVIE_SLOWMOTION_MODE_EN)
SMENUITEM sItemMovieSlowMotionMode     = { ITEMID_MOVIE_SLOWMOTION_MODE,     IDS_DS_SLOWMOTION,    &sSubMovieSlowMotionMode, 0 };
SMENUITEM sItemMovieSlowMotionMode_X1  = { ITEMID_MOVIE_SLOWMOTION_MODE_X1,  IDS_DS_SLOWMOTION_X1, NULL,       MenuItemMovieSlowMotionMode };
SMENUITEM sItemMovieSlowMotionMode_X2  = { ITEMID_MOVIE_SLOWMOTION_MODE_X2,  IDS_DS_SLOWMOTION_X2, NULL,       MenuItemMovieSlowMotionMode };
SMENUITEM sItemMovieSlowMotionMode_X4  = { ITEMID_MOVIE_SLOWMOTION_MODE_X4,  IDS_DS_SLOWMOTION_X4, NULL,       MenuItemMovieSlowMotionMode };
SMENUITEM sItemMovieSlowMotionMode_X8  = { ITEMID_MOVIE_SLOWMOTION_MODE_X8,  IDS_DS_SLOWMOTION_X8, NULL,       MenuItemMovieSlowMotionMode };
#endif

// WNR On/Off
#if (MENU_MOVIE_WNR_MODE_EN)
SMENUITEM sItemMovieWNRMode            = { ITEMID_MOVIE_WNR_MODE,            IDS_DS_WNR,           &sSubMovieWNRMode, 0 };
SMENUITEM sItemMovieWNRMode_On         = { ITEMID_MOVIE_WNR_MODE_ON,         IDS_DS_ON,            NULL,       MenuItemMovieWNRMode };
SMENUITEM sItemMovieWNRMode_Off        = { ITEMID_MOVIE_WNR_MODE_OFF,        IDS_DS_OFF,           NULL,       MenuItemMovieWNRMode };
#endif

/*===========================================================================
 * Global varible : Item List
 *===========================================================================*/

 #if (MENU_MOVIE_SIZE_EN)
PSMENUITEM sMenuListMovieMode[] =
{
#if (MENU_MOVIE_SIZE_4K_25P_EN)
    &sItemMovieMode_4K_25P,
#endif
#if (MENU_MOVIE_SIZE_1440_30P_EN)
    &sItemMovieMode_1440_30P,
#endif
#if (MENU_MOVIE_SIZE_SHD_30P_EN)
    &sItemMovieMode_SHD,
#endif
#if (MENU_MOVIE_SIZE_SHD_25P_EN)
    &sItemMovieMode_SHD_25P,
#endif
#if (MENU_MOVIE_SIZE_1080_60P_EN)
    &sItemMovieMode_FHD_60P,
#endif
#if (MENU_MOVIE_SIZE_1080P_30_HDR_EN)
    &sItemMovieMode_FHD_HDR,
#endif
#if (MENU_MOVIE_SIZE_1080P_EN)
    &sItemMovieMode_FHD,
#endif
#if (MENU_MOVIE_SIZE_720P_EN)
    &sItemMovieMode_HD,
#endif
#if (MENU_MOVIE_SIZE_720_60P_EN)
    &sItemMovieMode_HD_60P,
#endif
#if (MENU_MOVIE_SIZE_VGA30P_EN)
    &sItemMovieMode_VGA_30P,
#endif
};
#endif

#if (MENU_MOVIE_QUALITY_EN)
PSMENUITEM sMenuListMovieQuality[] =
{
#if (MENU_MOVIE_QUALITY_SUPER_FINE_EN)
    &sItemMovieQuality_Super_Fine,
#endif
#if (MENU_MOVIE_QUALITY_FINE_EN)
    &sItemMovieQuality_Fine
#endif
};
#endif

#if (MENU_MOVIE_MIC_SEN_EN)
PSMENUITEM sMenuListMICSensitivity[] =
{
#if (MENU_MOVIE_MIC_SEN_STANDARD_EN)
    &sItemMICSensitivity_Standard,
#endif
#if (MENU_MOVIE_MIC_SEN_LOW_EN)
    &sItemMICSensitivity_Low
#endif
};
#endif

#if (MENU_MOVIE_PRE_RECORD_EN)
PSMENUITEM sMenuListMoviePreRecord[] =
{
#if (MENU_MOVIE_PRE_RECORD_ON_EN)
    &sItemMoviePreRecord_On,
#endif
#if (MENU_MOVIE_PRE_RECORD_OFF_EN)
    &sItemMoviePreRecord_Off
#endif
};
#endif

#if (MENU_MOVIE_CLIP_TIME_EN)
PSMENUITEM sMenuListMovieClipTime[] =
{
#if (MENU_MOVIE_CLIP_TIME_OFF_EN)
    &sItemMovieClipTime_off,
#endif
#if (MENU_MOVIE_CLIP_TIME_1MIN_EN)
    &sItemMovieClipTime_1min,
#endif
#if (MENU_MOVIE_CLIP_TIME_2MIN_EN)
    &sItemMovieClipTime_2min,
#endif
#if (MENU_MOVIE_CLIP_TIME_3MIN_EN)
    &sItemMovieClipTime_3min,
#endif
#if (MENU_MOVIE_CLIP_TIME_5MIN_EN)
    &sItemMovieClipTime_5min,
#endif
#if (MENU_MOVIE_CLIP_TIME_10MIN_EN)
    &sItemMovieClipTime_10min,
#endif
#if (MENU_MOVIE_CLIP_TIME_30MIN_EN)
    &sItemMovieClipTime_30min,
#endif
};
#endif

#if (MENU_MOVIE_NIGHT_MODE_EN)
PSMENUITEM sMenuListMovieNightMode[] =
{
#if (MENU_MOVIE_NIGHT_MODE_ON_EN)
    &sItemMovieNightMode_On,
#endif
#if (MENU_MOVIE_NIGHT_MODE_OFF_EN)
    &sItemMovieNightMode_Off
#endif
};
#endif

#if (MENU_MOVIE_POWER_OFF_DELAY_EN)
PSMENUITEM sMenuListMoviePowerOffTime[] =
{
#if (MENU_MOVIE_POWER_OFF_DELAY_0SEC_EN)
    &sItemMoviePowerOffTime_0min,
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_5SEC_EN)
    &sItemMoviePowerOffTime_5sec,
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_10SEC_EN)
    &sItemMoviePowerOffTime_10sec,
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_15SEC_EN)
    &sItemMoviePowerOffTime_15sec,
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_30SEC_EN)
    &sItemMoviePowerOffTime_30sec,
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_1MIN_EN)
    &sItemMoviePowerOffTime_1min,
#endif
};
#endif

#if (MENU_MOVIE_SOUND_RECORD_EN)
PSMENUITEM sMenuListMovieSoundRecord[] =
{
#if (MENU_MOVIE_SOUND_RECORD_ON_EN)
    &sItemMovieSoundRecord_On,
#endif
#if (MENU_MOVIE_SOUND_RECORD_OFF_EN)
    &sItemMovieSoundRecord_Off,
#endif
};
#endif

#if (MENU_MOVIE_RECORD_VOLUME_EN)
PSMENUITEM sMenuListMovieRecordVolume[] =
{
    &sItemMovieRecordVolume_DEC,
    &sItemMovieRecordVolume_INC,
};
#endif

#if (MENU_MOVIE_VMD_REC_TIME_EN)
PSMENUITEM sMenuListMovieVMDRecTime[] =
{
#if (MENU_MOVIE_VMD_REC_TIME_5SEC_EN)
    &sItemMovieVMDRecTime_5sec,
#endif
#if (MENU_MOVIE_VMD_REC_TIME_10SEC_EN)
    &sItemMovieVMDRecTime_10sec,
#endif
#if (MENU_MOVIE_VMD_REC_TIME_30SEC_EN)
    &sItemMovieVMDRecTime_30sec,
#endif
#if (MENU_MOVIE_VMD_REC_TIME_1MIN_EN)
    &sItemMovieVMDRecTime_1min,
#endif
};
#endif

#if (MENU_MOVIE_AUTO_REC_EN)
PSMENUITEM sMenuMovieAutoRec[] =
{
    &sItemMovieAutoRec_On,
    &sItemMovieAutoRec_Off
};
#endif

#if (MENU_MOVIE_TIMELAPSE_MODE_EN)  // VIDEO_REC_TIMELAPSE_EN
PSMENUITEM sMenuMovieTimelapse[] =
{
#if (MENU_VIDEO_TIMELAPSE_MODE_OFF_EN)
    &sItemMovieTimelapse_Off,
#endif
#if (MENU_VIDEO_TIMELAPSE_MODE_1SEC_EN)
    &sItemMovieTimelapse_1Sec,
#endif
#if (MENU_VIDEO_TIMELAPSE_MODE_5SEC_EN)
    &sItemMovieTimelapse_5Sec,
#endif
#if (MENU_VIDEO_TIMELAPSE_MODE_10SEC_EN)
    &sItemMovieTimelapse_10Sec,
#endif
#if (MENU_VIDEO_TIMELAPSE_MODE_30SEC_EN)
    &sItemMovieTimelapse_30Sec,
#endif
#if (MENU_VIDEO_TIMELAPSE_MODE_60SEC_EN)
    &sItemMovieTimelapse_60Sec,
#endif
};
#endif

#if (MENU_MOVIE_PARKING_MODE_EN)
PSMENUITEM sMenuMovieParkingMode[] =
{
    &sItemMovieParkingMode_On,
    &sItemMovieParkingMode_Off,
};
#endif

#if (MENU_MOVIE_LDWS_MODE_EN)
PSMENUITEM sMenuMovieLDWSMode[] =
{
    &sItemMovieLDWSMode_On,
    &sItemMovieLDWSMode_Off,
};
#endif

#if (MENU_MOVIE_FCWS_MODE_EN)
PSMENUITEM sMenuMovieFCWSMode[] =
{
    &sItemMovieFCWSMode_On,
    &sItemMovieFCWSMode_Off,
};
#endif

#if (MENU_MOVIE_SAG_MODE_EN)
PSMENUITEM sMenuMovieSAGMode[] =
{
    &sItemMovieSAGMode_On,
    &sItemMovieSAGMode_Off,
};
#endif

#if (MENU_MOVIE_HDR_MODE_EN)
PSMENUITEM sMenuMovieHDRMode[] =
{
    &sItemMovieHDRMode_On,
    &sItemMovieHDRMode_Off,
};
#endif

#if (MENU_MOVIE_SLOWMOTION_MODE_EN)
PSMENUITEM sMenuMovieSlowMotionMode[] =
{
    &sItemMovieSlowMotionMode_X1,
    &sItemMovieSlowMotionMode_X2,
    &sItemMovieSlowMotionMode_X4,
    &sItemMovieSlowMotionMode_X8,
};
#endif

#if (MENU_MOVIE_WNR_MODE_EN)
PSMENUITEM sMenuMovieWNRMode[] =
{
    &sItemMovieWNRMode_Off,
    &sItemMovieWNRMode_On,
};
#endif

PSMENUITEM sMenuListMainVideo[] =
{
#if (MENU_MOVIE_SIZE_EN)
    &sItemMovieMode,
#endif
#if (MENU_MOVIE_QUALITY_EN)
    &sItemMovieQuality,
#endif
#if (MENU_MOVIE_CLIP_TIME_EN)
    &sItemMovieClipTime,
#endif
#if (MENU_MOVIE_NIGHT_MODE_EN)
    &sItemMovieNightMode,
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_EN)
    &sItemMoviePowerOffTime,
#endif
#if (MENU_MOVIE_VMD_REC_TIME_EN)
    &sItemMovieVMDRecTime,
#endif
#if (MENU_MOVIE_AUTO_REC_EN)
    &sItemMovieAutoRec,
#endif
#if (MENU_MOVIE_SOUND_RECORD_EN)
    &sItemMovieSoundRecord,
#endif
#if (MENU_MOVIE_RECORD_VOLUME_EN)
    &sItemMovieRecordVolume,
#endif 
#if (MENU_MOVIE_PRE_RECORD_EN)
    &sItemMoviePreRecord,
#endif
#if (MENU_MOVIE_MIC_SEN_EN)
    &sItemMICSensitivity,
#endif
#if (MENU_MOVIE_SCENE_EN)
    &sItemScence,
#endif
#if (MENU_MOVIE_EV_EN)
    &sItemEV,
#endif
#if (MENU_MOVIE_WB_EN)
    &sItemWB,
#endif
#if (MENU_MOVIE_ISO_EN)
    &sItemISO,
#endif
#if (MENU_MOVIE_COLOR_EN)
    &sItemColor,
#endif
#if (MENU_MOVIE_EFFECT_EN)
    &sItemEffect,
#endif
#if (MENU_MOVIE_PARKING_MODE_EN)
    &sItemMovieParkingMode,
#endif
#if (MENU_MOVIE_LDWS_MODE_EN)
    &sItemMovieLDWSMode,
#endif
#if (MENU_MOVIE_FCWS_MODE_EN)
    &sItemMovieFCWSMode,
#endif
#if (MENU_MOVIE_SAG_MODE_EN)
    &sItemMovieSAGMode,
#endif
#if (MENU_MOVIE_HDR_MODE_EN)
    &sItemMovieHDRMode,
#endif
#if (MENU_MOVIE_CONTRAST_EN)
    &sItemContrast,
#endif
#if (MENU_MOVIE_SATURATION_EN)
    &sItemSaturation,
#endif
#if (MENU_MOVIE_SHARPNESS_EN)
    &sItemSharpness,
#endif
#if (MENU_MOVIE_GAMMA_EN)
    &sItemGamma,
#endif
#if (MENU_MOVIE_TIMELAPSE_MODE_EN)  // VIDEO_REC_TIMELAPSE_EN
    &sItemMovieTimelapse,
#endif
#if (MENU_MOVIE_SLOWMOTION_MODE_EN)
    &sItemMovieSlowMotionMode,
#endif
#if (MENU_MOVIE_WNR_MODE_EN)
    &sItemMovieWNRMode,
#endif
};

/*===========================================================================
 * Global variable : Menu Structure
 *===========================================================================*/
SMENUSTRUCT   sMainMenuVideo =
{
    MENUID_MAIN_MENU_VIDEO,
    IDS_DS_MOVIE_MODE,
    NULL,
    sizeof(sMenuListMainVideo)/sizeof(PSMENUITEM),
    sMenuListMainVideo,
    NULL,
};
//--------------SUB MENU-------------------

#if (MENU_MOVIE_SIZE_EN)
SMENUSTRUCT sSubMovieMode =
{
    MENUID_SUB_MENU_MOVIE_MODE,
    IDS_DS_MOVIE_MODE,
    &sMainMenuVideo,
    sizeof(sMenuListMovieMode)/sizeof(PSMENUITEM),
    sMenuListMovieMode,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_QUALITY_EN)
SMENUSTRUCT sSubMovieQuality =
{
    MENUID_SUB_MENU_MOVIE_QUALITY,
    IDS_DS_MOVIE_QUALITY,
    &sMainMenuVideo,
    sizeof(sMenuListMovieQuality)/sizeof(PSMENUITEM),
    sMenuListMovieQuality,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_MIC_SEN_EN)
SMENUSTRUCT sSubMICSensitivity =
{
    MENUID_SUB_MENU_MIC_SENSITIVITY,
    IDS_DS_MIC_SENSITIVITY,
    &sMainMenuVideo,
    sizeof(sMenuListMICSensitivity)/sizeof(PSMENUITEM),
    sMenuListMICSensitivity,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_PRE_RECORD_EN)
SMENUSTRUCT sSubMoviePreRecord =
{
    MENUID_SUB_MENU_MOVIE_PRERECORD,
    IDS_DS_MOVIE_PRE_RECORD,
    &sMainMenuVideo,
    sizeof(sMenuListMoviePreRecord)/sizeof(PSMENUITEM),
    sMenuListMoviePreRecord,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_CLIP_TIME_EN)
SMENUSTRUCT sSubMovieClipTime =
{
    MENUID_SUB_MENU_MOVIE_CLIPTIME,
    IDS_MOVIE_CLIPTIME,
    &sMainMenuVideo,
    sizeof(sMenuListMovieClipTime)/sizeof(PSMENUITEM),
    sMenuListMovieClipTime,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_NIGHT_MODE_EN)
SMENUSTRUCT sSubMovieNightMode =
{
    MENUID_SUB_MENU_MOVIE_NIGHT_MODE,
    IDS_DS_NIGHT_MODE,
    &sMainMenuVideo,
    sizeof(sMenuListMovieNightMode)/sizeof(PSMENUITEM),
    sMenuListMovieNightMode,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_POWER_OFF_DELAY_EN)
SMENUSTRUCT sSubMoviePowerOffTime =
{
    MENUID_SUB_MENU_MOVIE_POWER_OFF_TIME,
    IDS_MOVIE_POWEROFF_TIME,
    &sMainMenuVideo,
    sizeof(sMenuListMoviePowerOffTime)/sizeof(PSMENUITEM),
    sMenuListMoviePowerOffTime,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_SOUND_RECORD_EN)
SMENUSTRUCT sSubMovieSoundRecord =
{
    MENUID_SUB_MENU_MOVIE_SOUND_RECORD,
    IDS_MOVIE_SOUND_RECORD,
    &sMainMenuVideo,
    sizeof(sMenuListMovieSoundRecord)/sizeof(PSMENUITEM),
    sMenuListMovieSoundRecord,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_RECORD_VOLUME_EN)
SMENUSTRUCT sSubMovieRecordVolume =
{
    MENUID_SUB_MENU_MOVIE_RECORD_VOLUME,
    IDS_MOVIE_RECORD_VOLUME,
    &sMainMenuVideo,
    sizeof(sMenuListMovieRecordVolume)/sizeof(PSMENUITEM),
    sMenuListMovieRecordVolume,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_VMD_REC_TIME_EN)
SMENUSTRUCT sSubMovieVMDRecTime =
{
    MENUID_SUB_MENU_MOVIE_VMD_REC_TIME,
    IDS_VMD_REC_TIME,
    &sMainMenuVideo,
    sizeof(sMenuListMovieVMDRecTime)/sizeof(PSMENUITEM),
    sMenuListMovieVMDRecTime,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_AUTO_REC_EN)
SMENUSTRUCT sSubMovieAutoRec =
{
    MENUID_SUB_MENU_MOVIE_AUTO_REC,
    IDS_AUTO_REC,
    &sMainMenuVideo,
    sizeof(sMenuMovieAutoRec)/sizeof(PSMENUITEM),
    sMenuMovieAutoRec,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_TIMELAPSE_MODE_EN)  // VIDEO_REC_TIMELAPSE_EN
SMENUSTRUCT sSubMovieTimelapse =
{
    MENUID_SUB_MENU_MOVIE_TIMELAPSE,
    IDS_DS_TIMELAPSE,
    &sMainMenuVideo,
    sizeof(sMenuMovieTimelapse)/sizeof(PSMENUITEM),
    sMenuMovieTimelapse,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_PARKING_MODE_EN)
SMENUSTRUCT sSubMovieParkingMode =
{
    MENUID_SUB_MENU_MOVIE_PARKING_MODE,
    IDS_DS_POWERON_GSENSOR_SENSITIVETY,
    &sMainMenuVideo,
    sizeof(sMenuMovieParkingMode)/sizeof(PSMENUITEM),
    sMenuMovieParkingMode,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_LDWS_MODE_EN)
SMENUSTRUCT sSubMovieLDWSMode =
{
    MENUID_SUB_MENU_MOVIE_LDWS_MODE,
    IDS_DS_LDWS,
    &sMainMenuVideo,
    sizeof(sMenuMovieLDWSMode)/sizeof(PSMENUITEM),
    sMenuMovieLDWSMode,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_FCWS_MODE_EN)
SMENUSTRUCT sSubMovieFCWSMode =
{
    MENUID_SUB_MENU_MOVIE_FCWS_MODE,
    IDS_DS_FCWS,
    &sMainMenuVideo,
    sizeof(sMenuMovieFCWSMode)/sizeof(PSMENUITEM),
    sMenuMovieFCWSMode,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_SAG_MODE_EN)
SMENUSTRUCT sSubMovieSAGMode =
{
    MENUID_SUB_MENU_MOVIE_SAG_MODE,
    IDS_DS_SAG,
    &sMainMenuVideo,
    sizeof(sMenuMovieSAGMode)/sizeof(PSMENUITEM),
    sMenuMovieSAGMode,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_HDR_MODE_EN)
SMENUSTRUCT sSubMovieHDRMode =
{
    MENUID_SUB_MENU_MOVIE_HDR_MODE,
    IDS_DS_HDR,
    &sMainMenuVideo,
    sizeof(sMenuMovieHDRMode)/sizeof(PSMENUITEM),
    sMenuMovieHDRMode,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_SLOWMOTION_MODE_EN)
SMENUSTRUCT sSubMovieSlowMotionMode =
{
    MENUID_SUB_MENU_MOVIE_SLOWMOTION_MODE,
    IDS_DS_SLOWMOTION,
    &sMainMenuVideo,
    sizeof(sMenuMovieSlowMotionMode)/sizeof(PSMENUITEM),
    sMenuMovieSlowMotionMode,
    MenuGetDefault_Movie,
};
#endif

#if (MENU_MOVIE_WNR_MODE_EN)
SMENUSTRUCT sSubMovieWNRMode =
{
    MENUID_SUB_MENU_MOVIE_WNR_MODE,
    IDS_DS_WNR,
    &sMainMenuVideo,
    sizeof(sMenuMovieWNRMode)/sizeof(PSMENUITEM),
    sMenuMovieWNRMode,
    MenuGetDefault_Movie,
};
#endif

/*===========================================================================
 * Main body
 *===========================================================================*/
#if (MENU_MOVIE_SIZE_EN)
bool MenuItemMovieMode(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiMOVSize= pItem->iItemId - ITEMID_MOVIE_MODE -1;
    carimpl_VideoFunc_SetResolution();
    return true;
}
#endif

#if (MENU_MOVIE_QUALITY_EN)
bool MenuItemMovieQuality(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiMOVQuality = pItem->iItemId - ITEMID_MOVIE_QUALITY -1;
    carimpl_VideoFunc_SetQuality();
    return true;
}
#endif

 #if (MENU_MOVIE_MIC_SEN_EN)
bool MenuItemMICSensitivity(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiMicSensitivity = pItem->iItemId - ITEMID_MIC_SENSITIVITY -1;
    carimpl_VideoFunc_SetMicSensitivity();
    return true;
}
#endif

#if (MENU_MOVIE_PRE_RECORD_EN)
bool MenuItemMoviePreRecord(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiMOVPreRecord = pItem->iItemId - ITEMID_MOVIE_PRE_RECORD -1;
    carimpl_VideoFunc_SetPreRecord();
    return true;
}
#endif

#if (MENU_MOVIE_CLIP_TIME_EN)
bool MenuItemMovieClipTime(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiMOVClipTime = pItem->iItemId - ITEMID_MOVIE_CLIPTIME -1;
    carimpl_VideoFunc_SetClipTime();
    return true;
}
#endif

#if (MENU_MOVIE_NIGHT_MODE_EN)
bool MenuItemMovieNightMode(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiNightMode = pItem->iItemId - ITEMID_MOVIE_NIGHT_MODE -1;
    carimpl_VideoFunc_SetNightMode();
    return true;
}
#endif

#if (MENU_MOVIE_POWER_OFF_DELAY_EN)
bool MenuItemMoviePowerOffTime(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiMOVPowerOffTime = pItem->iItemId - ITEMID_MOVIE_POWEROFF_TIME -1;
    carimpl_VideoFunc_SetPowerOffTime();
    return true;
}
#endif

#if (MENU_MOVIE_SOUND_RECORD_EN)
bool MenuItemMovieSoundRecord(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiMOVSoundRecord = pItem->iItemId - ITEMID_MOVIE_SOUND_RECORD -1;
    carimpl_VideoFunc_SetSoundRecord();
    return true;
}
#endif

#if (MENU_MOVIE_RECORD_VOLUME_EN)
bool MenuItemMovieRecordVolumeAdjust(PSMENUITEM pItem)
{
    if (pItem->iItemId == ITEMID_MOVIE_RECORD_VOLUME_INC)
    {
        MenuSettingConfig()->uiMOVRecordVolume += 10;
        if (MenuSettingConfig()->uiMOVRecordVolume > 100)
            MenuSettingConfig()->uiMOVRecordVolume = 100;
    }
    else if (pItem->iItemId == ITEMID_MOVIE_RECORD_VOLUME_DEC)
    {
        MenuSettingConfig()->uiMOVRecordVolume -= 10;
        if (MenuSettingConfig()->uiMOVRecordVolume < 0)
            MenuSettingConfig()->uiMOVRecordVolume = 0;
    }
    carimpl_VideoFunc_SetRecordVolume();
    return true;
}
#endif

#if (MENU_MOVIE_VMD_REC_TIME_EN)
bool MenuItemMovieVMDRecTime(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiVMDRecTime = pItem->iItemId - ITEMID_MOVIE_VMD_REC_TIME -1;
    carimpl_VideoFunc_SetVMDRecordTime();
    return true;
}
#endif

#if (MENU_MOVIE_AUTO_REC_EN)
bool MenuItemMovieAutoRec(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiAutoRec = pItem->iItemId - ITEMID_MOVIE_AUTO_REC -1;
    carimpl_VideoFunc_SetAutoReord();
    return true;
}
#endif

#if (MENU_MOVIE_TIMELAPSE_MODE_EN)  // VIDEO_REC_TIMELAPSE_EN
bool MenuItemMovieTimelapse(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiTimeLapseTime =  pItem->iItemId - ITEMID_MOVIE_TIMELAPSE -1;
    carimpl_VideoFunc_SetTimelapse();
    return true;
}
#endif

#if (MENU_MOVIE_PARKING_MODE_EN)
bool MenuItemMovieParkingMode(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiParkingMode = pItem->iItemId - ITEMID_MOVIE_PARKING_MODE -1;
    carimpl_VideoFunc_SetParkingMode();
    return true;
}
#endif

#if (MENU_MOVIE_LDWS_MODE_EN)
bool MenuItemMovieLDWSMode(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiLDWS = pItem->iItemId - ITEMID_MOVIE_LDWS_MODE -1;
    carimpl_VideoFunc_SetLDWSMode();
    return true;
}
#endif

#if (MENU_MOVIE_FCWS_MODE_EN)
bool MenuItemMovieFCWSMode(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiFCWS = pItem->iItemId - ITEMID_MOVIE_FCWS_MODE -1;
    carimpl_VideoFunc_SetFCWSMode();
    return true;
}
#endif

#if (MENU_MOVIE_SAG_MODE_EN)
bool MenuItemMovieSAGMode(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiSAG = pItem->iItemId - ITEMID_MOVIE_SAG_MODE -1;
    carimpl_VideoFunc_SetSAGMode();
    return true;
}
#endif

#if (MENU_MOVIE_HDR_MODE_EN)
bool MenuItemMovieHDRMode(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiHDR = pItem->iItemId - ITEMID_MOVIE_HDR_MODE -1;
    carimpl_VideoFunc_SetHDRMode();
    return true;
}
#endif

#if (MENU_MOVIE_SLOWMOTION_MODE_EN)
bool MenuItemMovieSlowMotionMode(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiSlowMotion = pItem->iItemId - ITEMID_MOVIE_SLOWMOTION_MODE -1;
    carimpl_VideoFunc_SetSlowMotion();
    return true;
}
#endif

#if (MENU_MOVIE_WNR_MODE_EN)
bool MenuItemMovieWNRMode(PSMENUITEM pItem)
{
    MenuSettingConfig()->uiWNR = pItem->iItemId - ITEMID_MOVIE_WNR_MODE -1;
    carimpl_VideoFunc_SetWNRMode();
    return true;
}
#endif

int MenuGetDefault_Movie(PSMENUSTRUCT pMenu)
{
    int  DefaultValue = 0;

    switch (pMenu->iMenuId)
    {
#if (MENU_MOVIE_SIZE_EN)
    case MENUID_SUB_MENU_MOVIE_MODE:
        DefaultValue = MenuSettingConfig()->uiMOVSize ;
        break;
#endif
#if (MENU_MOVIE_QUALITY_EN)
    case MENUID_SUB_MENU_MOVIE_QUALITY:
        DefaultValue = MenuSettingConfig()->uiMOVQuality ;
        break;
#endif
#if (MENU_MOVIE_MIC_SEN_EN)
    case MENUID_SUB_MENU_MIC_SENSITIVITY:
        DefaultValue = MenuSettingConfig()->uiMicSensitivity;
        break;
#endif
#if (MENU_MOVIE_PRE_RECORD_EN)
    case MENUID_SUB_MENU_MOVIE_PRERECORD:
        DefaultValue = MenuSettingConfig()->uiMOVPreRecord;
        break;
#endif
#if (MENU_MOVIE_CLIP_TIME_EN)
    case MENUID_SUB_MENU_MOVIE_CLIPTIME:
        DefaultValue = MenuSettingConfig()->uiMOVClipTime;
        break;
#endif
#if (MENU_MOVIE_POWER_OFF_DELAY_EN)
    case MENUID_SUB_MENU_MOVIE_POWER_OFF_TIME:
        DefaultValue = MenuSettingConfig()->uiMOVPowerOffTime;
        break;
#endif
#if (MENU_MOVIE_SOUND_RECORD_EN)
    case MENUID_SUB_MENU_MOVIE_SOUND_RECORD:
        DefaultValue = MenuSettingConfig()->uiMOVSoundRecord;
        break;
#endif
#if (MENU_MOVIE_RECORD_VOLUME_EN)
    case MENUID_SUB_MENU_MOVIE_RECORD_VOLUME:
        DefaultValue = MenuSettingConfig()->uiMOVRecordVolume;
        break;
#endif
#if (MENU_MOVIE_VMD_REC_TIME_EN)
    case MENUID_SUB_MENU_MOVIE_VMD_REC_TIME:
        DefaultValue = MenuSettingConfig()->uiVMDRecTime;
        break;
#endif
#if (MENU_MOVIE_AUTO_REC_EN)
    case MENUID_SUB_MENU_MOVIE_AUTO_REC:
        DefaultValue = MenuSettingConfig()->uiAutoRec;
        break;
#endif
#if (MENU_MOVIE_TIMELAPSE_MODE_EN)
    case MENUID_SUB_MENU_MOVIE_TIMELAPSE:
        DefaultValue = MenuSettingConfig()->uiTimeLapseTime;
        break;
#endif
#if (MENU_MOVIE_PARKING_MODE_EN)
    case MENUID_SUB_MENU_MOVIE_PARKING_MODE:
        DefaultValue = MenuSettingConfig()->uiParkingMode;
        break;
#endif
#if (MENU_MOVIE_LDWS_MODE_EN)
    case MENUID_SUB_MENU_MOVIE_LDWS_MODE:
        DefaultValue = MenuSettingConfig()->uiLDWS;
        break;
#endif
#if (MENU_MOVIE_FCWS_MODE_EN)
    case MENUID_SUB_MENU_MOVIE_FCWS_MODE:
        DefaultValue = MenuSettingConfig()->uiFCWS;
        break;
#endif
#if (MENU_MOVIE_SAG_MODE_EN)
    case MENUID_SUB_MENU_MOVIE_SAG_MODE:
        DefaultValue = MenuSettingConfig()->uiSAG;
        break;
#endif
#if (MENU_MOVIE_HDR_MODE_EN)
    case MENUID_SUB_MENU_MOVIE_HDR_MODE:
        DefaultValue = MenuSettingConfig()->uiHDR;
        break;
#endif
#if (MENU_MOVIE_SLOWMOTION_MODE_EN)
    case MENUID_SUB_MENU_MOVIE_SLOWMOTION_MODE:
        DefaultValue = MenuSettingConfig()->uiSlowMotion;
        break;
#endif
#if (MENU_MOVIE_WNR_MODE_EN)
    case MENUID_SUB_MENU_MOVIE_WNR_MODE:
        DefaultValue = MenuSettingConfig()->uiWNR;
        break;
#endif
#if (MENU_MOVIE_NIGHT_MODE_EN)
    case MENUID_SUB_MENU_MOVIE_NIGHT_MODE:
        DefaultValue = MenuSettingConfig()->uiNightMode;
        break;
#endif
#if (MENU_MOVIE_CONTRAST_EN)
    case MENUID_SUB_MENU_CONTRAST:
        DefaultValue = MenuSettingConfig()->uiContrast;
        break;
#endif
#if (MENU_MOVIE_SATURATION_EN)
    case MENUID_SUB_MENU_SATURATION:
        DefaultValue = MenuSettingConfig()->uiSaturation;
        break;
#endif
#if (MENU_MOVIE_SHARPNESS_EN)
    case MENUID_SUB_MENU_SHARPNESS:
        DefaultValue = MenuSettingConfig()->uiSharpness;
        break;
#endif
#if (MENU_MOVIE_GAMMA_EN)
    case MENUID_SUB_MENU_GAMMA:
        DefaultValue = MenuSettingConfig()->uiGamma;
        break;
#endif
    }

    return DefaultValue;
}