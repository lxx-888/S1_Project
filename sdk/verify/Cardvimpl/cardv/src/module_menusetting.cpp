/*************************************************
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 * All rights reserved.
 *
 **************************************************
 * File name:  module_menusetting.c
 * Author:     jeff.li@sigmastar.com.cn
 * Version:    Initial Draft
 * Date:       2019/9/4
 * Description: menu setting module source file
 *
 *
 *
 * History:
 *
 *    1. Date  :        2019/9/4
 *       Author:        jeff.li@sigmastar.com.cn
 *       Modification:  Created file
 *
 **************************************************/

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "module_common.h"
#include "module_config.h"
#include "module_gsensor.h"
#include "module_video.h"
#include "module_scl.h"
#include "module_osd.h"
#if (CARDV_VDF_ENABLE)
#include "module_md.h"
#endif
#if (CARDV_ADAS_ENABLE)
#include "module_adas.h"
#endif

extern "C" {
#include "nvconf.h"
}

//==============================================================================
//
//                              EXTERN GLOBAL
//
//==============================================================================

extern BOOL   g_bHdr;
extern BOOL   g_bAudioOut;
extern BOOL   g_bAudioInMute;
extern MI_U64 g_u64NormalRecTime;
extern MI_U32 g_u32TimeLapse;
extern MI_S32 g_recordWidth[];
extern MI_S32 g_recordHeight[];
extern MI_S8  g_s8volume;
extern MI_S8  g_s8AudioInVolume;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

BOOL cardv_menu_getResult(const char *cmd, char *outBuf, int inLen)
{
    FILE *fp  = NULL;
    char *str = NULL;
    BOOL  ret = TRUE;

    fp = popen(cmd, "r");
    if (fp == NULL)
        return FALSE;

    if (outBuf != NULL)
    {
        str = fgets(outBuf, inLen, fp);
        if (str == NULL)
            ret = FALSE;
        else
        {
            for (int i = 0; i < inLen; i++)
            {
                if (outBuf[i] == '\n')
                {
                    outBuf[i] = '\0';
                    break;
                }
            }
        }
    }

    pclose(fp);
    return ret;
}

void cardv_menu_updateVideoRecordSize(MI_U32 u32Width, MI_U32 u32Height)
{
    if (u32Width == 0 || u32Height == 0)
        return;

    for (MI_U32 u32VencChn = 0; u32VencChn < MAX_VIDEO_NUMBER; u32VencChn++)
    {
        CarDV_VencAttr_t *pstVencAttr = &gstVencAttr[u32VencChn];
        if (pstVencAttr->bUsed && pstVencAttr->u8CamId == 0 && pstVencAttr->eVideoType == VIDEO_STREAM_RECORD)
        {
            MI_U32        u32BindDevId = pstVencAttr->stVencInBindParam.stChnPort.u32DevId;
            MI_U32        u32BindChn   = pstVencAttr->stVencInBindParam.stChnPort.u32ChnId;
            MI_U32        u32BindPort  = pstVencAttr->stVencInBindParam.stChnPort.u32PortId;
            MI_ModuleId_e eBindModule  = pstVencAttr->stVencInBindParam.stChnPort.eModId;
            if (E_MI_MODULE_ID_SCL == eBindModule)
            {
                MI_SYS_WindowSize_t stPortSize;
                stPortSize.u16Width  = u32Width;
                stPortSize.u16Height = u32Height;
                CarDV_SclChangeOutputPortSize(u32BindDevId, u32BindChn, u32BindPort, &stPortSize);
                return;
            }
        }
    }
}

void cardv_menu_updateSettingFromConfig(void)
{
    char *str = NULL;
    int   val;

    if (semaphore_open() < 0)
    {
        return;
    }

    if (load_nvconf((char *)"/customer/wifi/cgi_config.bin") != 0)
    {
        goto EXIT;
    }

    str = nvconf_get_val((char *)"Camera.Menu.VideoRes");
    if (str)
    {
        if (strncasecmp(str, "1080P", 5) == 0)
        {
            g_recordWidth[0]  = 1920;
            g_recordHeight[0] = 1080;
        }
        else if (strncasecmp(str, "720P", 4) == 0)
        {
            g_recordWidth[0]  = 1280;
            g_recordHeight[0] = 720;
        }
        else if (strncasecmp(str, "2K", 2) == 0 || strncasecmp(str, "1440P", 5) == 0)
        {
            g_recordWidth[0]  = 2560;
            g_recordHeight[0] = 1440;
        }
        else if (strncasecmp(str, "4K", 2) == 0 || strncasecmp(str, "2160P", 5) == 0)
        {
            g_recordWidth[0]  = 3840;
            g_recordHeight[0] = 2160;
        }
        else
        {
            printf("TODO set VideoRes [%s]\n", str);
        }
    }

    str = nvconf_get_val((char *)"Camera.Menu.HDR");
    if (str)
    {
        if (strcmp(str, "ON") == 0)
        {
            g_bHdr = TRUE;
        }
    }

    str = nvconf_get_val((char *)"Camera.Menu.LoopingVideo");
    if (str)
    {
        sscanf(str, "%dMIN", &val);
        g_u64NormalRecTime = val * 60 * 1000000;
    }

    str = nvconf_get_val((char *)"Camera.Menu.VideoQuality");
    if (str)
    {
        printf("TODO set VideoQuality [%s]\n", str);
    }

    str = nvconf_get_val((char *)"Camera.Menu.RecordWithAudio");
    if (str)
    {
        if (strcmp(str, "OFF") == 0)
        {
            g_bAudioInMute = TRUE;
        }
    }

    str = nvconf_get_val((char *)"Camera.Menu.MicrophoneSensitivity");
    if (str)
    {
        g_s8AudioInVolume = atoi(str);
    }

    str = nvconf_get_val((char *)"Camera.Menu.Timelapse");
    if (str)
    {
        if (strcmp(str, "OFF") == 0)
        {
            g_u32TimeLapse = 0;
        }
        else if (strstr(str, "SEC"))
        {
            sscanf(str, "%dSEC", &val);
            g_u32TimeLapse = val;
        }
        else
        {
            g_u32TimeLapse = 0;
        }
    }

    str = nvconf_get_val((char *)"Camera.Menu.ParkingMonitor");
    if (str)
    {
        val = 0;
        if (strcmp(str, "ENABLE") == 0)
            val = 1;
        GsensorParkingMonitor(val);
    }

    str = nvconf_get_val((char *)"Camera.Menu.GSensor");
    if (str)
    {
        val = 0;
        if (strncmp(str, "LEVEL", 5) == 0)
        {
            sscanf(str, "LEVEL%d", &val);
            val += 1;
        }
        GsensorSetSensitivity(val);
    }

    str = nvconf_get_val((char *)"Camera.Menu.VoiceSwitch");
    if (str)
    {
        if (strcmp(str, "OFF") == 0)
        {
            g_bAudioOut = FALSE;
        }
    }

    str = nvconf_get_val((char *)"Camera.Menu.PlaybackVolume");
    if (str)
    {
        g_s8volume = 5;
        if (strcmp(str, "00") == 0)
        {
            g_s8volume = 0;
        }
        else if (strcmp(str, "01") == 0)
        {
            g_s8volume = 1;
        }
        else if (strcmp(str, "02") == 0)
        {
            g_s8volume = 2;
        }
        else if (strcmp(str, "03") == 0)
        {
            g_s8volume = 3;
        }
        else if (strcmp(str, "04") == 0)
        {
            g_s8volume = 4;
        }
        else if (strcmp(str, "05") == 0)
        {
            g_s8volume = 5;
        }
        else if (strcmp(str, "06") == 0)
        {
            g_s8volume = 6;
        }
        else if (strcmp(str, "07") == 0)
        {
            g_s8volume = 7;
        }
        else if (strcmp(str, "08") == 0)
        {
            g_s8volume = 8;
        }
        else if (strcmp(str, "09") == 0)
        {
            g_s8volume = 9;
        }
        else if (strcmp(str, "10") == 0)
        {
            g_s8volume = 10;
        }
        printf("init volumeLvl= %d \n", g_s8volume);
    }

#if (CARDV_ADAS_ENABLE)
    str = nvconf_get_val((char *)"Camera.Preview.Adas.LDWS");
    if (str)
    {
        if (strcmp(str, "ON") == 0)
            gFrontAdasAttr.feature.ldws = 1;
        else
            gFrontAdasAttr.feature.ldws = 0;
    }

    str = nvconf_get_val((char *)"Camera.Preview.Adas.FCWS");
    if (str)
    {
        if (strcmp(str, "ON") == 0)
            gFrontAdasAttr.feature.fcws = 1;
        else
            gFrontAdasAttr.feature.fcws = 0;
    }

    str = nvconf_get_val((char *)"Camera.Preview.AdasRear.LDWS");
    if (str)
    {
        if (strcmp(str, "ON") == 0)
            gRearAdasAttr.feature.ldws = 1;
        else
            gRearAdasAttr.feature.ldws = 0;
    }

    str = nvconf_get_val((char *)"Camera.Preview.AdasRear.BCWS");
    if (str)
    {
        if (strcmp(str, "ON") == 0)
            gRearAdasAttr.feature.fcws = 1;
        else
            gRearAdasAttr.feature.fcws = 0;
    }
#endif

#if (CARDV_VDF_ENABLE)
    str = nvconf_get_val((char *)"Camera.Menu.MotionSensitivity");
    if (str)
    {
        if (strcmp(str, "LOW") == 0)
            g_MdtLevel = 1;
        else if (strcmp(str, "MID") == 0)
            g_MdtLevel = 2;
        else if (strcmp(str, "HIGH") == 0)
            g_MdtLevel = 3;
        else
            g_MdtLevel = 0;
    }
#endif

    str = nvconf_get_val((char *)"Camera.Menu.TimeStampLogoTXT");
    if (str)
    {
        if (strcmp(str, "off") == 0)
            osd_SetStampMode(STAMP_MODE_OFF);
        else if (strcmp(str, "date") == 0)
            osd_SetStampMode(STAMP_MODE_DATE);
        else if (strcmp(str, "date+logo") == 0)
            osd_SetStampMode(STAMP_MODE_DATELOGO);
        else if (strcmp(str, "logo") == 0)
            osd_SetStampMode(STAMP_MODE_LOGO);
    }

    str = nvconf_get_val((char *)"Camera.Menu.DateTimeFormat");
    if (str)
    {
        if (strcmp(str, "NONE") == 0)
            osd_SetTimeFormat(DATE_FORMAT_OFF);
        else if (strcmp(str, "YMD") == 0)
            osd_SetTimeFormat(DATE_FORMAT_YMD);
        else if (strcmp(str, "MDY") == 0)
            osd_SetTimeFormat(DATE_FORMAT_MDY);
        else if (strcmp(str, "DMY") == 0)
            osd_SetTimeFormat(DATE_FORMAT_DMY);
    }

EXIT:
    semaphore_close();
}