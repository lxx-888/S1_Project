#ifndef _MENU_CONFIG_SDK_H_
#define _MENU_CONFIG_SDK_H_


/* Still */
#define MENU_STILL_PAGE_EN                          (1)
#define MENU_STILL_EN                               (MENU_STILL_PAGE_EN)

#define MENU_STILL_SIZE_EN                          (1)
#define MENU_STILL_SIZE_30M_EN                      (0) // Not Support
#define MENU_STILL_SIZE_14M_EN                      (0) // Not Support
#define MENU_STILL_SIZE_12M_EN                      (0) // Not Support
#define MENU_STILL_SIZE_8M_EN                       (1)
#define MENU_STILL_SIZE_6M_EN                       (0) // Note: Before enable it, please make sure CAMERAITEMID has related setting
#define MENU_STILL_SIZE_6M_WIDE_EN                  (0) // Note: Before enable it, please make sure CAMERAITEMID has related setting
#define MENU_STILL_SIZE_5M_EN                       (0)
#define MENU_STILL_SIZE_3M_EN                       (1)
#define MENU_STILL_SIZE_2M_WIDE_EN                  (1)
#define MENU_STILL_SIZE_1d2M_EN                     (1)
#define MENU_STILL_SIZE_VGA_EN                      (0)

#define MENU_STILL_QUALITY_EN                       (0) // TBD
#define MENU_STILL_QUALITY_SUPER_FINE_EN            (1)
#define MENU_STILL_QUALITY_FINE_EN                  (1)

#define MENU_STILL_BURST_SHOT_EN                    (0) // TBD
#define MENU_STILL_BURST_SHOT_OFF_EN                (1)
#define MENU_STILL_BURST_SHOT_LO_EN                 (1)
#define MENU_STILL_BURST_SHOT_MID_EN                (1)
#define MENU_STILL_BURST_SHOT_HI_EN                 (1)
#define MENU_STILL_BURST_SHOT_LO_NUM                (2)
#define MENU_STILL_BURST_SHOT_MID_NUM               (3)
#define MENU_STILL_BURST_SHOT_HI_NUM                (5)

#define MENU_STILL_TIMELAPSE_TIME_EN                (0)
#define MENU_STILL_TIMELAPSE_TIME1_EN               (0)     // 0s
#define MENU_STILL_TIMELAPSE_TIME2_EN               (0)     // 1s
#define MENU_STILL_TIMELAPSE_TIME3_EN               (0)     // 2s
#define MENU_STILL_TIMELAPSE_TIME4_EN               (1)     // 5s
#define MENU_STILL_TIMELAPSE_TIME5_EN               (1)     // 10s
#define MENU_STILL_TIMELAPSE_TIME6_EN               (1)     // 30s
#define MENU_STILL_TIMELAPSE_TIME7_EN               (1)     // 60s

// Still (copy from Manual)
#define MENU_STILL_SCENE_EN                         (0)
#define MENU_STILL_EV_EN                            (0)
#define MENU_STILL_ISO_EN                           (0)
#define MENU_STILL_WB_EN                            (0)
#define MENU_STILL_COLOR_EN                         (0)
#define MENU_STILL_EFFECT_EN                        (0)
#define MENU_STILL_CONTRAST_EN                      (0)
#define MENU_STILL_SATURATION_EN                    (0)
#define MENU_STILL_SHARPNESS_EN                     (0)
#define MENU_STILL_GAMMA_EN                         (0)

/* Movie */
#define MENU_MOVIE_PAGE_EN                          (1)
#define MENU_MOVIE_EN                               (MENU_MOVIE_PAGE_EN)

#define MENU_MOVIE_SIZE_EN                          (1)
#define MENU_MOVIE_SIZE_4K_25P_EN                   (1)
#define MENU_MOVIE_SIZE_1440_30P_EN                 (1)
#define MENU_MOVIE_SIZE_1080_60P_EN                 (0)
#define MENU_MOVIE_SIZE_1080P_EN                    (1)
#define MENU_MOVIE_SIZE_1080_30P_EN                 (MENU_MOVIE_SIZE_1080P_EN)
#define MENU_MOVIE_SIZE_1080P_30_HDR_EN             (0)   // for HDR
#define MENU_MOVIE_SIZE_1080_25P_EN                 (0)
#define MENU_MOVIE_SIZE_SHD_30P_EN                  (0)
#define MENU_MOVIE_SIZE_SHD_25P_EN                  (0)
#define MENU_MOVIE_SIZE_960P_30P_EN                 (0)
#define MENU_MOVIE_SIZE_900P_30P_EN                 (0)
#define MENU_MOVIE_SIZE_720P_EN                     (1)
#define MENU_MOVIE_SIZE_720_120P_EN                 (0)
#define MENU_MOVIE_SIZE_720_30P_EN                  (MENU_MOVIE_SIZE_720P_EN)
#define MENU_MOVIE_SIZE_720_60P_EN                  (0)
#define MENU_MOVIE_SIZE_720_50P_EN                  (0)
#define MENU_MOVIE_SIZE_VGA30P_EN                   (0)

#define MENU_MOVIE_FRAME_RATE_EN                    (0)
#define MENU_MOVIE_FRAME_RATE_30_EN                 (0)
#define MENU_MOVIE_FRAME_RATE_20_EN                 (0)
#define MENU_MOVIE_FRAME_RATE_15_EN                 (0)
#define MENU_MOVIE_FRAME_RATE_10_EN                 (0)

#define MENU_MOVIE_QUALITY_EN                       (0) // TBD
#define MENU_MOVIE_QUALITY_SUPER_FINE_EN            (1)
#define MENU_MOVIE_QUALITY_FINE_EN                  (1)

#define MENU_MOVIE_MIC_SEN_EN                       (0) // Not Support
#define MENU_MOVIE_MIC_SEN_STANDARD_EN              (1)
#define MENU_MOVIE_MIC_SEN_LOW_EN                   (1)

#define MENU_MOVIE_PRE_RECORD_EN                    (0) // Fisrt recording is pre-record. Not support on / off
#define MENU_MOVIE_PRE_RECORD_ON_EN                 (1)
#define MENU_MOVIE_PRE_RECORD_OFF_EN                (1)

#define MENU_MOVIE_SOUND_RECORD_EN                  (1)
#define MENU_MOVIE_SOUND_RECORD_ON_EN               (1)
#define MENU_MOVIE_SOUND_RECORD_OFF_EN              (1)

#define MENU_MOVIE_RECORD_VOLUME_EN                 (1)

#define MENU_MOVIE_CLIP_TIME_EN                     (1)
#define MENU_MOVIE_CLIP_TIME_OFF_EN                 (0) // Not Support
#define MENU_MOVIE_CLIP_TIME_1MIN_EN                (1)
#define MENU_MOVIE_CLIP_TIME_2MIN_EN                (1)
#define MENU_MOVIE_CLIP_TIME_3MIN_EN                (1)
#define MENU_MOVIE_CLIP_TIME_5MIN_EN                (0)
#define MENU_MOVIE_CLIP_TIME_10MIN_EN               (0)
#define MENU_MOVIE_CLIP_TIME_30MIN_EN               (0)

#define MENU_MOVIE_NIGHT_MODE_EN                    (1)
#define MENU_MOVIE_NIGHT_MODE_ON_EN                 (1)
#define MENU_MOVIE_NIGHT_MODE_OFF_EN                (1)

#define MENU_MOVIE_POWER_OFF_DELAY_EN               (0) // TODO
#define MENU_MOVIE_POWER_OFF_DELAY_0SEC_EN          (1)
#define MENU_MOVIE_POWER_OFF_DELAY_5SEC_EN          (1)
#define MENU_MOVIE_POWER_OFF_DELAY_10SEC_EN         (0)
#define MENU_MOVIE_POWER_OFF_DELAY_15SEC_EN         (0)
#define MENU_MOVIE_POWER_OFF_DELAY_30SEC_EN         (1)
#define MENU_MOVIE_POWER_OFF_DELAY_60SEC_EN         (1)
#define MENU_MOVIE_POWER_OFF_DELAY_1MIN_EN          (MENU_MOVIE_POWER_OFF_DELAY_60SEC_EN)
#define MENU_MOVIE_POWER_OFF_DELAY_2MIN_EN          (0)
#define MENU_MOVIE_POWER_OFF_DELAY_3MIN_EN          (1)
#define MENU_MOVIE_POWER_OFF_DELAY_5MIN_EN          (0)
#define MENU_MOVIE_POWER_OFF_DELAY_10MIN_EN         (0)
#define MENU_MOVIE_POWER_OFF_DELAY_15MIN_EN         (0)
#define MENU_MOVIE_POWER_OFF_DELAY_30MIN_EN         (0)
#define MENU_MOVIE_POWER_OFF_DELAY_60MIN_EN         (0)

#define MENU_MOVIE_VMD_REC_TIME_EN                  (0) // Not Support Motion Detection Now
#define MENU_MOVIE_VMD_REC_TIME_5SEC_EN             (1)
#define MENU_MOVIE_VMD_REC_TIME_10SEC_EN            (1)
#define MENU_MOVIE_VMD_REC_TIME_30SEC_EN            (1)
#define MENU_MOVIE_VMD_REC_TIME_1MIN_EN             (1)

#define MENU_MOVIE_TIMELAPSE_MODE_EN                (1)
#define MENU_VIDEO_TIMELAPSE_MODE_OFF_EN            (1)
#define MENU_VIDEO_TIMELAPSE_MODE_1SEC_EN           (1)
#define MENU_VIDEO_TIMELAPSE_MODE_5SEC_EN           (1)
#define MENU_VIDEO_TIMELAPSE_MODE_10SEC_EN          (1)
#define MENU_VIDEO_TIMELAPSE_MODE_30SEC_EN          (0)
#define MENU_VIDEO_TIMELAPSE_MODE_60SEC_EN          (0)

#define MENU_MOVIE_PARKING_MODE_EN                  (0)

// Movie (copy from Manual)
#define MENU_MOVIE_SCENE_EN                         (0)
#define MENU_MOVIE_EV_EN                            (1)
#define MENU_MOVIE_ISO_EN                           (1)
#define MENU_MOVIE_WB_EN                            (1)
#define MENU_MOVIE_COLOR_EN                         (1)
#define MENU_MOVIE_EFFECT_EN                        (1)
#define MENU_MOVIE_CONTRAST_EN                      (1)
#define MENU_MOVIE_SATURATION_EN                    (1)
#define MENU_MOVIE_SHARPNESS_EN                     (1)
#define MENU_MOVIE_GAMMA_EN                         (1)

#define MENU_MOVIE_SLOWMOTION_MODE_EN               (0) // Not Support

#define MENU_MOVIE_WNR_MODE_EN                      (0)

#define MENU_REARCAM_TYPE_EN                        (0)

// Auto Record when DC in (USB adapter)
#define MENU_MOVIE_AUTO_REC_EN                      (0) // TBD

#define MENU_MOVIE_LDWS_MODE_EN                     (1)
#define MENU_MOVIE_FCWS_MODE_EN                     (1)
#define MENU_MOVIE_SAG_MODE_EN                      (0) // TBD

#define MENU_MOVIE_HDR_MODE_EN                      (0)

/* Manual */
#define MENU_MANUAL_PAGE_EN                         (0)
#define MENU_MANUAL_EN                              (MENU_MANUAL_PAGE_EN)   // TY Miao - TBD

#define MENU_MANUAL_SCENE_EN                        (0)
#define MENU_MANUAL_SCENE_AUTO_EN                   (1)
#define MENU_MANUAL_SCENE_SPORT_EN                  (1)
#define MENU_MANUAL_SCENE_PORTRAIT_EN               (1)
#define MENU_MANUAL_SCENE_LANDSCAPE_EN              (1)
#define MENU_MANUAL_SCENE_TWILIGHT_PORTRAIT_EN      (0)
#define MENU_MANUAL_SCENE_TWILIGHT_EN               (0)//M8
#define MENU_MANUAL_SCENE_SNOW_EN                   (1)
#define MENU_MANUAL_SCENE_BEACH_EN                  (1)
#define MENU_MANUAL_SCENE_FIREWORKS_EN              (1)

#define MENU_MANUAL_FOCUS_EN                        (0)
#define MENU_MANUAL_FOCUS_MULTI_EN                  (1)
#define MENU_MANUAL_FOCUS_CENTERSPOT_EN             (1)

#define MENU_MANUAL_FOCUS_RANGE_EN                  (0)
#define MENU_MANUAL_FOCUS_RANGE_NORMAL_EN           (1)
#define MENU_MANUAL_FOCUS_RANGE_MACRO_EN            (1)

#define MENU_MANUAL_METERING_EN                     (0)
#define MENU_MANUAL_METERING_MULTI_EN               (1)
#define MENU_MANUAL_METERING_WEIGHTED_EN            (1)
#define MENU_MANUAL_METERING_CENTER_SPOT_EN         (1)

#define MENU_MANUAL_EV_EN                           (1)
#define MENU_MANUAL_EV_N20_EN                       (1)
#define MENU_MANUAL_EV_N17_EN                       (1)
#define MENU_MANUAL_EV_N15_EN                       (0)
#define MENU_MANUAL_EV_N13_EN                       (1)
#define MENU_MANUAL_EV_N10_EN                       (1)
#define MENU_MANUAL_EV_N07_EN                       (1)
#define MENU_MANUAL_EV_N05_EN                       (0)
#define MENU_MANUAL_EV_N03_EN                       (1)
#define MENU_MANUAL_EV_00_EN                        (1)
#define MENU_MANUAL_EV_P03_EN                       (1)
#define MENU_MANUAL_EV_P05_EN                       (0)
#define MENU_MANUAL_EV_P07_EN                       (1)
#define MENU_MANUAL_EV_P10_EN                       (1)
#define MENU_MANUAL_EV_P13_EN                       (1)
#define MENU_MANUAL_EV_P15_EN                       (0)
#define MENU_MANUAL_EV_P17_EN                       (1)
#define MENU_MANUAL_EV_P20_EN                       (1)

#define MENU_MANUAL_ISO_EN                          (1)
#define MENU_MANUAL_ISO_AUTO_EN                     (1)
#define MENU_MANUAL_ISO_100_EN                      (1)
#define MENU_MANUAL_ISO_200_EN                      (1)
#define MENU_MANUAL_ISO_400_EN                      (1)
#define MENU_MANUAL_ISO_800_EN                      (1)
#define MENU_MANUAL_ISO_1200_EN                     (0)
#define MENU_MANUAL_ISO_1600_EN                     (1)
#define MENU_MANUAL_ISO_3200_EN                     (1)

#define MENU_MANUAL_WB_EN                           (1)
#define MENU_MANUAL_WB_AUTO_EN                      (1)
#define MENU_MANUAL_WB_FLUORESCENT1_EN              (1)
#define MENU_MANUAL_WB_DAYLIGHT_EN                  (1)
#define MENU_MANUAL_WB_FLUORESCENT2_EN              (0)
#define MENU_MANUAL_WB_CLOUDY_EN                    (1)
#define MENU_MANUAL_WB_FLUORESCENT3_EN              (0)
#define MENU_MANUAL_WB_INCANDESCENT_EN              (1)
#define MENU_MANUAL_WB_FLASH_EN                     (0)
#define MENU_MANUAL_WB_ONEPUSH_EN                   (0)
#define MENU_MANUAL_WB_ONE_PUSH_SET_EN              (0)
#define MENU_MANUAL_WB_UNDERWATER                   (0)

#define MENU_MANUAL_COLOR_EN                        (1)
#define MENU_MANUAL_COLOR_NATURAL_EN                (1)
#define MENU_MANUAL_COLOR_VIVID_EN                  (1)
#define MENU_MANUAL_COLOR_PALE_EN                   (1)

#define MENU_MANUAL_EFFECT_EN                       (1)
#define MENU_MANUAL_EFFECT_NORMAL_EN                (1)
#define MENU_MANUAL_EFFECT_SEPIA_EN                 (1)
#define MENU_MANUAL_EFFECT_BLACK_WHITE_EN           (1)
#define MENU_MANUAL_EFFECT_EMBOSS_EN                (0)
#define MENU_MANUAL_EFFECT_NEGATIVE_EN              (1)
#define MENU_MANUAL_EFFECT_SKETCH_EN                (0)
#define MENU_MANUAL_EFFECT_OIL_EN                   (0)
#define MENU_MANUAL_EFFECT_CRAYON_EN                (0)
#define MENU_MANUAL_EFFECT_BEAUTY_EN                (0)

#define MENU_MANUAL_FACE_DETECT_EN                  (0)
#define MENU_MANUAL_FACE_DETECT_ON_EN               (1)
#define MENU_MANUAL_FACE_DETECT_TOUCHSPOT_EN        (0)
#define MENU_MANUAL_FACE_DETECT_OFF_EN              (1)

#define MENU_MANUAL_SMILE_DETECT_EN                 (0)
#define MENU_MANUAL_SMILE_DETECT_OFF_EN             (1)
#define MENU_MANUAL_SMILE_DETECT_1PEOPLE_EN         (1)
#define MENU_MANUAL_SMILE_DETECT_2PEOPLE_EN         (1)

#define MENU_MANUAL_SMILE_SEN_EN                    (0)
#define MENU_MANUAL_SMILE_SEN_LOW_EN                (1)
#define MENU_MANUAL_SMILE_SEN_HIGH_EN               (1)

// The range of ISP color
#define MENU_MANUAL_CONTRAST_EN                     (1)
#define MENU_MANUAL_SATURATION_EN                   (1)
#define MENU_MANUAL_SHARPNESS_EN                    (1)
#define MENU_MANUAL_GAMMA_EN                        (0) // TBD

/* Playback */
#define MENU_PLAYBACK_PAGE_EN                       (1)

#define MENU_PLAYBACK_SLIDESHOW_FILE_EN             (0)
#define MENU_PLAYBACK_SLIDESHOW_FILE_ALL_EN         (1)
#define MENU_PLAYBACK_SLIDESHOW_FILE_STILL_EN       (1)
#define MENU_PLAYBACK_SLIDESHOW_FILE_MOVIE_EN       (1)

#define MENU_PLAYBACK_SLIDESHOW_EFFECT_EN           (0)
#define MENU_PLAYBACK_SLIDESHOW_EFFECT_SIMPLE_EN    (1)
#define MENU_PLAYBACK_SLIDESHOW_EFFECT_NOSTALGIC_EN (1)
#define MENU_PLAYBACK_SLIDESHOW_EFFECT_STYLISH_EN   (1)
#define MENU_PLAYBACK_SLIDESHOW_EFFECT_ACTIVE_EN    (1)

#define MENU_PLAYBACK_SLIDESHOW_MUSIC_EN            (0)
#define MENU_PLAYBACK_SLIDESHOW_MUSIC_ON_EN         (1)
#define MENU_PLAYBACK_SLIDESHOW_MUSIC_OFF_EN        (1)

#define MENU_PLAYBACK_VOLUME_EN                     (1)
#define MENU_PLAYBACK_VOLUME_LV0_EN                 (1)
#define MENU_PLAYBACK_VOLUME_LV1_EN                 (1)
#define MENU_PLAYBACK_VOLUME_LV2_EN                 (1)
#define MENU_PLAYBACK_VOLUME_LV3_EN                 (1)
#define MENU_PLAYBACK_VOLUME_LV4_EN                 (1)
#define MENU_PLAYBACK_VOLUME_LV5_EN                 (1)
#define MENU_PLAYBACK_VOLUME_LV6_EN                 (1)
#define MENU_PLAYBACK_VOLUME_LV7_EN                 (1)
#define MENU_PLAYBACK_VOLUME_LV8_EN                 (1)
#define MENU_PLAYBACK_VOLUME_LV9_EN                 (1)
#define MENU_PLAYBACK_VOLUME_LV10_EN                (1)

#define MENU_PLAYBACK_VIDEO_TYPE_EN                 (1) //Note: Make sure (DCF_DB_COUNT > 1) before enable it
#define MENU_PLAYBACK_VIDEO_TYPE_NORMAL_EN          (1)
#define MENU_PLAYBACK_VIDEO_TYPE_PARKING_EN         (1)
#define MENU_PLAYBACK_VIDEO_TYPE_EMERGENCY_EN       (1)

#define MENU_PLAYBACK_VIDEO_CAMID_EN                (1)
#define MENU_PLAYBACK_VIDEO_CAMID_1ST_EN            (1)
#define MENU_PLAYBACK_VIDEO_CAMID_2ND_EN            (1)
#define MENU_PLAYBACK_VIDEO_CAMID_3RD_EN            (1)
#define MENU_PLAYBACK_VIDEO_CAMID_4TH_EN            (0)

#define MENU_PLAYBACK_ROTATE_EN                     (0)
#define MENU_PLAYBACK_RESIZE_EN                     (0)
#define MENU_PLAYBACK_TRIMMING_EN                   (0)
#define MENU_PLAYBACK_QM_CUT_EN                     (0)

/* Edit Tool */
#define MENU_EDIT_PAGE_EN                           (0)

#define MENU_EDIT_DELETE_EN                         (1)
#define MENU_EDIT_DELETE_ONE_EN                     (1)
#define MENU_EDIT_DELETE_ALL_EN                     (1)

#define MENU_EDIT_PROTECT_EN                        (1)
#define MENU_EDIT_PROTECT_ONE_EN                    (1)
#define MENU_EDIT_PROTECT_ALL_EN                    (1)

#define MENU_EDIT_UNPROTECT_EN                      (1)
#define MENU_EDIT_UNPROTECT_ONE_EN                  (1)
#define MENU_EDIT_UNPROTECT_ALL_EN                  (1)

/* 1: Select Format (Video/Image) before file */
/* 0: Select file only */
#define MENU_EDIT_DELETE_SELECT_FORMAT              (0)
#define MENU_EDIT_PROTECT_SELECT_FORMAT             (0)
#define MENU_EDIT_UNPROTECT_SELECT_FORMAT           (0)
/* Delete/Protect/Unprotect can do continuously, (repeat to Select/Confirm)*/
#define MENU_EDIT_CONTINUOUSLY                      (0)

/* Media Tool */
#define MENU_MEDIA_PAGE_EN                          (1)

#define MENU_MEDIA_SELECT_MDEIA_EN                  (0)
#define MENU_MEDIA_FORMAT_SD_EN                     (1)
#define MENU_MEDIA_FORMAT_INT_MEM_EN                (0)
#define MENU_MEDIA_SHOW_SD_INFO                     (1)
#define MENU_MEDIA_SHOW_INT_MEM_INFO                (0)

/* General */
#define MENU_GENERAL_PAGE_EN                        (1)
#define MENU_GENERAL_EN                             (MENU_GENERAL_PAGE_EN)
#define MENU_GENERAL_BEEP_EN                        (0) // TBD
#define MENU_GENERAL_BEEP_ON_EN                     (1)
#define MENU_GENERAL_BEEP_OFF_EN                    (1)
#define MENU_GENERAL_LCD_BRIGHTNESS_EN              (1)

#define MENU_GENERAL_AUTO_POWEROFF_EN               (0) // TBD
#define MENU_GENERAL_AUTO_POWEROFF_NEVER_EN         (1)
#define MENU_GENERAL_AUTO_POWEROFF_15SEC_EN         (0)
#define MENU_GENERAL_AUTO_POWEROFF_30SEC_EN         (0)
#define MENU_GENERAL_AUTO_POWEROFF_1MIN_EN          (1)
#define MENU_GENERAL_AUTO_POWEROFF_2MIN_EN          (0)
#define MENU_GENERAL_AUTO_POWEROFF_3MIN_EN          (0)
#define MENU_GENERAL_AUTO_POWEROFF_5MIN_EN          (1)

#define MENU_GENERAL_CLOCK_SETTING_EN               (1)

#define MENU_GENERAL_DRIVER_ID_SETTING_EN           (0)

#define MENU_GENERAL_DATE_FORMAT_EN                 (1)
#define MENU_GENERAL_DATE_FORMAT_NONE_EN            (1)
#define MENU_GENERAL_DATE_FORMAT_YMD_EN             (1)
#define MENU_GENERAL_DATE_FORMAT_MDY_EN             (1)
#define MENU_GENERAL_DATE_FORMAT_DMY_EN             (1)

#define MENU_GENERAL_DATE_LOGO_STAMP_EN             (1)
#define MENU_GENERAL_DATE_LOGO_STAMP_DATE_LOGO_EN   (1)
#define MENU_GENERAL_DATE_LOGO_STAMP_DATE_EN        (1)
#define MENU_GENERAL_DATE_LOGO_STAMP_LOGO_EN        (1)
#define MENU_GENERAL_DATE_LOGO_STAMP_OFF_EN         (1)

#define MENU_GENERAL_GPS_STAMP_EN                   (0)
#define MENU_GENERAL_GPS_STAMP_ON_EN                (1)
#define MENU_GENERAL_GPS_STAMP_OFF_EN               (1)

#define MENU_GENERAL_GPS_RECORD_EN                  (1)

#define MENU_GENERAL_SPEED_STAMP_EN                 (0)
#define MENU_GENERAL_SPEED_STAMP_ON_EN              (1)
#define MENU_GENERAL_SPEED_STAMP_OFF_EN             (1)

#define MENU_GENERAL_LANGUAGE_EN                    (1)
#define MENU_GENERAL_LANGUAGE_ENGLISH_EN            (1)
#define MENU_GENERAL_LANGUAGE_SPANISH_EN            (0)
#define MENU_GENERAL_LANGUAGE_PORTUGUESE_EN         (0)
#define MENU_GENERAL_LANGUAGE_RUSSIAN_EN            (0)
#define MENU_GENERAL_LANGUAGE_SCHINESE_EN           (1)
#define MENU_GENERAL_LANGUAGE_TCHINESE_EN           (0)
#define MENU_GENERAL_LANGUAGE_GERMAN_EN             (0)
#define MENU_GENERAL_LANGUAGE_ITALIAN_EN            (0)
#define MENU_GENERAL_LANGUAGE_LATVIAN_EN            (0)
#define MENU_GENERAL_LANGUAGE_POLISH_EN             (0)
#define MENU_GENERAL_LANGUAGE_ROMANIAN_EN           (0)
#define MENU_GENERAL_LANGUAGE_SLOVAK_EN             (0)
#define MENU_GENERAL_LANGUAGE_UKRANINIAN_EN         (0)
#define MENU_GENERAL_LANGUAGE_FRENCH_EN             (0)
#define MENU_GENERAL_LANGUAGE_JAPANESE_EN           (0)
#define MENU_GENERAL_LANGUAGE_KOREAN_EN             (0)
#define MENU_GENERAL_LANGUAGE_CZECH_EN              (0)
#define MENU_GENERAL_LANGUAGE_TURKISH_EN            (0) // TBD
#define MENU_GENERAL_LANGUAGE_DUTCH_EN              (0) // TBD
#define MENU_GENERAL_LANGUAGE_DANISH_EN             (0) // TBD
#define MENU_GENERAL_LANGUAGE_GREEK_EN              (0) // TBD
#define MENU_GENERAL_LANGUAGE_ARABIC_EN             (0) // TBD
#define MENU_GENERAL_LANGUAGE_THAI_EN               (0) // TBD

#define MENU_GENERAL_RESET_SETUP_EN                 (1)
#define MENU_GENERAL_RESET_SETUP_YES_EN             (1)
#define MENU_GENERAL_RESET_SETUP_NO_EN              (1)

#define MENU_GENERAL_FLICKER_EN                     (1)
#define MENU_GENERAL_FLICKER_50HZ_EN                (1)
#define MENU_GENERAL_FLICKER_60HZ_EN                (1)

#define MENU_GENERAL_USB_FUNCTION_EN                (0) // TBD
#define MENU_GENERAL_USB_FUNCTION_MSDC_EN           (1)
#define MENU_GENERAL_USB_FUNCTION_PCAM_EN           (1)

#define MENU_GENERAL_LCD_POWER_SAVE_EN              (0) // TBD
#define MENU_GENERAL_LCD_POWER_SAVE_OFF_EN          (1)
#define MENU_GENERAL_LCD_POWER_SAVE_10SEC_EN        (0)
#define MENU_GENERAL_LCD_POWER_SAVE_30SEC_EN        (0)
#define MENU_GENERAL_LCD_POWER_SAVE_1MIN_EN         (1)
#define MENU_GENERAL_LCD_POWER_SAVE_3MIN_EN         (1)

#define GSENSOR_5_LEVEL                             (1) // OFF, Level0 ~ 4
#define GSENSOR_3_LEVEL                             (2) // OFF, HIGH,MIDDLE,LOW
#define GSENSOR_4_LEVEL                             (3) // OFF, HIGH,MIDDLE,LOW,STANDARD
#define MENU_GENERAL_GSENSOR_EN                     (1) // TBD

#define MENU_MOVIE_GSENSOR_SENSITIVITY_XYZ_EN       (0)

#define MENU_GENERAL_POWERON_GSENSOR_EN             (1) // TBD
#define MENU_GENERAL_POWERON_GSENSOR_OFF_EN         (1)
#define MENU_GENERAL_POWER_ON_GSENSOR_LEVEL0_EN     (1)
#define MENU_GENERAL_POWER_ON_GSENSOR_LEVEL1_EN     (1)
#define MENU_GENERAL_POWER_ON_GSENSOR_LEVEL2_EN     (1)

#define MENU_GENERAL_GSENSOR_OFF_EN                 (1) // TBD
#if (MENU_GENERAL_GSENSOR_EN==GSENSOR_5_LEVEL)
#define MENU_GENERAL_GSENSOR_LEVEL0_EN              (1)
#define MENU_GENERAL_GSENSOR_LEVEL1_EN              (1)
#define MENU_GENERAL_GSENSOR_LEVEL2_EN              (1)
#define MENU_GENERAL_GSENSOR_LEVEL3_EN              (1)
#define MENU_GENERAL_GSENSOR_LEVEL4_EN              (1)
#elif (MENU_GENERAL_GSENSOR_EN==GSENSOR_4_LEVEL)
#define MENU_GENERAL_GSENSOR_LEVEL0_EN              (1)
#define MENU_GENERAL_GSENSOR_LEVEL1_EN              (1)
#define MENU_GENERAL_GSENSOR_LEVEL2_EN              (1)
#define MENU_GENERAL_GSENSOR_LEVEL3_EN              (0)
#define MENU_GENERAL_GSENSOR_LEVEL4_EN              (1)
#else
#define MENU_GENERAL_GSENSOR_LEVEL0_EN              (1)
#define MENU_GENERAL_GSENSOR_LEVEL1_EN              (0)
#define MENU_GENERAL_GSENSOR_LEVEL2_EN              (1)
#define MENU_GENERAL_GSENSOR_LEVEL3_EN              (0)
#define MENU_GENERAL_GSENSOR_LEVEL4_EN              (1)
#endif

#define MENU_GENERAL_MOTION_DTC_EN                  (1) // TBD
#define MENU_GENERAL_MOTION_DTC_OFF_EN              (1)
#define MENU_GENERAL_MOTION_DTC_LOW_EN              (1)
#define MENU_GENERAL_MOTION_DTC_MID_EN              (1)
#define MENU_GENERAL_MOTION_DTC_HIGH_EN             (1)

#define MENU_GENERAL_GPSINFO_EN                     (1)
#define MENU_GENERAL_TIME_ZONE_EN                   (1)

#define MENU_GENERAL_FW_VERSION_EN                  (1)

#define MENU_GENERAL_NIGHT_MODE_EN                  (0)

#define MENU_GENERAL_WIFI_EN                        (1)
#define MENU_GENERAL_WIFI_OFF_EN                    (1)
#define MENU_GENERAL_WIFI_ON_EN                     (1)

/////////////////////////////////////////////////////////////
#define MENU_WIFI_PAGE_EN                               (0)

#define MENU_WIFI_MODE_EN                               (0)
#define MENU_WIFI_MODE_AP_EN                            (0)
#define MENU_WIFI_MODE_STATION_EN                       (0)

#define MENU_WIFI_AP_SSID_EN                            (0)
#define MENU_WIFI_AP_PSWD_EN                            (1)

// Do not enable STATION1
#define MENU_WIFI_STATION1_EN                           (0)
#define MENU_WIFI_STATION1_SSID_EN                      (0)
#define MENU_WIFI_STATION1_PSWD_EN                      (0)

#define MENU_WIFI_STATION2_EN                           (0)
#define MENU_WIFI_STATION2_SSID_EN                      (0)
#define MENU_WIFI_STATION2_PSWD_EN                      (0)

#define MENU_WIFI_STATION3_EN                           (0)
#define MENU_WIFI_STATION3_SSID_EN                      (0)
#define MENU_WIFI_STATION3_PSWD_EN                      (0)

#define MENU_WIFI_STATION4_EN                           (0)
#define MENU_WIFI_STATION4_SSID_EN                      (0)
#define MENU_WIFI_STATION4_PSWD_EN                      (0)

#define MENU_WIFI_STATION5_EN                           (0)
#define MENU_WIFI_STATION5_SSID_EN                      (0)
#define MENU_WIFI_STATION5_PSWD_EN                      (0)

#define MENU_WIFI_STATION6_EN                           (0)
#define MENU_WIFI_STATION6_SSID_EN                      (0)
#define MENU_WIFI_STATION6_PSWD_EN                      (0)

/////////////////////////////////////////////////////////////
#define MENU_GENERAL_PQ_CONTRAST_EN                     (0)// Not Support
#define MENU_GENERAL_PQ_BRIGHTNESS_EN                   (0)// Not Support
#define MENU_GENERAL_PQ_SATURATION_EN                   (0)// Not Support
#define MENU_GENERAL_PQ_COLORTEMP_EN                    (0)// Not Support
#define MENU_GENERAL_PQ_COLORTEMP_OFF_EN                (0)
#define MENU_GENERAL_PQ_COLORTEMP_MODE1_EN              (0)
#define MENU_GENERAL_PQ_COLORTEMP_MODE2_EN              (0)
#define MENU_GENERAL_PQ_COLORTEMP_MODE3_EN              (0)
#define MENU_GENERAL_PQ_COLORTEMP_MODE4_EN              (0)

#define MENU_GENERAL_SYSTEM_SETTINGS_EN                 (0)
#endif//_MENU_CONFIG_SDK_H_
