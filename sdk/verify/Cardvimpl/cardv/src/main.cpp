/*
 * main.cpp- Sigmastar
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
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <error.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/msg.h>
#include "ms_version.h"

#include "module_common.h"
#include "module_system.h"
#include "module_audio.h"
#include "module_sensor.h"
#include "module_vif.h"
#include "module_isp.h"
#if (CARDV_LDC_ENABLE)
#include "module_ldc.h"
#endif
#include "module_scl.h"
#include "module_osd.h"
#if (CARDV_DISPLAY_ENABLE)
#include "module_display.h"
#endif
#include "module_vdisp.h"
#include "module_jpd.h"
#include "module_vdec.h"
#include "module_mux.h"
#include "module_dcf.h"
#include "module_gsensor.h"
#include "module_gps.h"
#include "module_storage.h"
#include "module_video.h"
#include "module_menusetting.h"
#if (CARDV_VDF_ENABLE)
#include "module_vdf.h"
#include "module_md.h"
#endif
#if (CARDV_ADAS_ENABLE)
#include "module_adas.h"
#endif
#if (CARDV_LD_ENABLE)
#include "module_lightdetect.h"
#endif
#if (CARDV_MIPITX_ENABLE)
#include "module_mipitx.h"
#endif
#if (CARDV_RTSP_INPUT_ENABLE || CARDV_WS_INPUT_ENABLE)
#include "module_rtspclient.h"
#include "module_ws.h"
#include "module_ipc.h"
#endif

#include "mid_common.h"
#include "mid_AudioEncoder.h"
#include "module_AudioDecoder.h"
#include "mid_utils.h"

#include "iniparser.h"
#include "DCF.h"
#include "IPC_cardvInfo.h"
#include "IPC_msg.h"
#if (CARDV_ALINK_ENABLE)
#include "alink.h"
#endif

#if (CARDV_SPEECH_ENABLE)
#include "speech.h"
#include "txz_engine.h"
#endif
#define MVXV_V2 "MVX" MVXV_HEAD_VER " " MVXV_LIB_TYPE " " MVXV_CHIP_ID " " MVXV_CHANGELIST " " MVXV_EXT "XVM"

#define COMPBUF2SEC (7) // unit:SEC cal:MUXER_FRAME_QUEUE_SIZE > COMPBUF2SEC*30

#define FIFO_DATA_SIZE (256)

#define CREATE_FIFO(X_FIFO_NAME)                                                     \
    if (0 == access(X_FIFO_NAME, F_OK))                                              \
    {                                                                                \
        if (0 != remove(X_FIFO_NAME))                                                \
        {                                                                            \
            printf("%s:%d removed %s.", __func__, __LINE__, X_FIFO_NAME);            \
            exit(1);                                                                 \
        }                                                                            \
    }                                                                                \
    if (0 != (mkfifo(X_FIFO_NAME, 0777)))                                            \
    {                                                                                \
        printf("%s:%d Could not create fifo %s\n", __func__, __LINE__, X_FIFO_NAME); \
        exit(1);                                                                     \
    }

const MI_S8 *CARDV_VER                            = (MI_S8 *)MVXV_V2;
MI_S32       g_s32SocId                           = 0;
MI_S32       g_displayOsd                         = FALSE;
MI_S32       g_openUVCDevice                      = FALSE;
BOOL         g_camExisted[MAX_CAM_NUM]            = {0};
MI_S32       g_recordWidth[CARDV_MAX_SENSOR_NUM]  = {1920, 0, 0, 0};
MI_S32       g_recordHeight[CARDV_MAX_SENSOR_NUM] = {1080, 0, 0, 0};

BOOL  g_bAudioIn        = TRUE;
BOOL  g_bAudioOut       = TRUE;
BOOL  g_bAudioInMute    = FALSE;
BOOL  g_bSetBitrate     = FALSE;
BOOL  g_bHdr            = FALSE;
MI_S8 g_s8AudioInVolume = 10;

BOOL          g_bVideoStart       = FALSE;
MuxerStatus_e g_eMuxerStatus      = MUXER_STATUS_IDLE;
MI_U32        g_u32TimeLapse      = 0;
MI_U32        g_u32SlowMotion     = 0;
BOOL          g_bLockFile         = FALSE;
BOOL          g_bRtspEnable       = FALSE;
BOOL          g_bSpeechEnable     = FALSE;
BOOL          g_bPowerOnByGsensor = FALSE;
BOOL          g_bRecMdtTrigger    = FALSE;

#if (CARDV_ALINK_ENABLE)
MI_S8     g_alinkpemPath[128] = "/bootconfig/bin/alink/alink.conf";
AlinkDemo AlinkSever;
#endif

CarDVAudioInParam  g_audioInParam[MAX_AUDIO_INPUT];
CarDVAudioOutParam g_audioOutParam[MAX_AUDIO_OUTPUT];
MI_S8              g_s8volume = 5;

MI_U64 g_u64NormalRecTime     = 60 * 1000000;
MI_U64 g_u64ShareEmergRecTime = 20 * 1000000;

CarDV_Info carInfo        = {0};
BOOL       bMessageLoop   = TRUE;
MI_S8      g_UIInitCmd[8] = "zkgui &";

extern int g_muxerDbgLog;

void cardvSuspend(void);
void cardvResume(MI_BOOL bPowerOn);
void cardv_parserIni(char *pIniPath);

MI_S32 cardv_process_display_help(void)
{
    printf("video process useage: echo cmd value > /tmp/cardv_info \n");
    printf("Options: \n");
    /* audio play */
    // echo audioplay 1 1 live  > /tmp/cardv_fifo
    // echo audioplay 1 1 /bootconfig/a.wav  > /tmp/cardv_fifo
    printf("\t'aplaystop 0/1 dev_id', 0:stop immediately 1:stop wait for complete\n");
    printf("\t'audioplay 0/1 dev_id <live/name>', live:audio live streaming name:play audio file\n");
    /* audio record mute or not */
    printf("\t'audiorec <value[0, 1]>', audio record & rtsp mute or not\n");
    /* resolution */
    printf("\t'res <stream id> <width> <height>',   'res 0 1920 1080' as full HD\n");
    /* bitrate */
    printf("\t'bitrate <channel id> <value>',   'bitrate 0 10000000' set channel 0 10M bps.\n");
    // /* frame rate */
    // printf(  "\t'framerate <stream id> <frame_rate> ', 'framerate 0 30 ' as 30fps.\n");
    // /* bitrate ctrl */
    // printf(  "\t'bitrate <stream id> <ctrl> <bitrate> ', bitrate is bits per second.\n"
    //  "\t\t<ctrl>: [0]cbr, [1]vbr, [2]fixqp\n"
    //  "\t\tcbr: [JPG/MJPEG]<bitrate[100*1024, 8*1024*1024]> <maxQfactor[20,90]> <MinQfactor[20,90]>,\n"
    //  "\t\tcbr: [H264/H265]<bitrate> <maxQp[12,48]> <minQp[12,48]> <maxIQp[12,48]> <minIQp[12,48]>
    //  <IPQPDelta[-12,12]>,\n"
    //  "\t\tvbr: <max bitrate> <maxQp[12,48]> <minQp[12,48]> <maxIQp[12,48]> <minIQp[12,48]> <IPQPDelta[-12,12]>
    //  <ChangePos[50,100]>,\n"
    //  "\t\tfixqp: [JPG/MJPEG]<Qfactor[20,90]>,\n"
    //  "\t\tfixqp: [H264/H265]<IQP>_<PQP>,\n"
    //  "\t\t'bitrate 0 0 5000000 48 22 48 20 2 ' as cbr 5M bps.\n");
    // /* ie */
    // printf(  "\t'ie <enable>', (0-disable, 1-enable)\n");
    // /* ie md param*/
    // printf(  "\t'md sensitivity ', [10,20,30,...100]\n");
    /* record */
    printf("\t'rec <value[0, 1]>', (0-stop, 1-start)\n");
    /* lock file */
    printf("\t'lock <value[0, 1]>', lock file\n");
    /* timelapse record */
    printf("\t'timelapse', timelapse record start\n");
    /* capture */
    printf("\t'capture', capture jpg\n");
    /* mux debug */
    printf("\t'muxdebug', open mux debug log\n");
    /* parkingMonitor */ /*gsensorpwronsens*/
    printf("\t'park <value[0, 1, 2, 3]>', 0:OFF, 1:level1, 2:level2...\n");
    /* voice */
    printf("\t'voice <value[0, 1]>', 0:OFF, 1:ON\n");
    /* gsensor sensitivity*/
    printf("\t'gsensor <value[0, 3]>', 0:OFF, 1:LOW, 2:MID, 3:HIGH\n");
    /* sensor rotate */
    printf("\t'flip <value[0, 1]>', 0:Normal, 1:Rotate\n");
    /* isp */
    printf("\t'bri <value[0, 100]> ', '-bri 60' brightness set 60.\n");
    printf("\t'con <value[0, 100]> ', '-con 60' Contrast set 60.\n");
    printf("\t'sat <value[0, 127]> ', '-sat 60' Saturation set 60.\n");
    printf("\t'sha <value[0, 1023]> ', '-sha 60' Sharpness set 60.\n");
    printf("\t'wb <value[0, 4]> ', '[0] auto, [1] daylight [2] cloudy [3] fluorescent1 [4]incandescent\n");
    printf("\t'ev <value[-6, 6]>' , '-ev 0' EvBiasMode set 0.\n");
    printf("\t'flicker <value[50, 60]> ', '-flicker 60', flicker set 60hz.\n");
// printf("\t'-nr <Level> ',\n"
//  "\t\tLevel 0: 0 set\n"
//  "\t\tLevel 1: 1 set,  8-bit\n"
//  "\t\tLevel 2: 1 set, 10-bit\n"
//  "\t\tLevel 3: 1 set, 12-bit\n"
//  "\t\tLevel 4: 2 set,  8-bit,  8-bit\n"
//  "\t\tLevel 5: 2 set,  8-bit, 10-bit\n"
//  "\t\tLevel 6: 2 set,  8-bit, 12-bit\n"
//  "\t\tLevel 7: 2 set, 12-bit, 12-bit(default if enable 3DNR)\n");
// printf(  "\t'hdr', switch hdr mode\n");
#if (CARDV_GPS_ENABLE)
    /* gps edog */
    printf("\t'gps <value[0, 1]> ', 'gps 1' enable gps.\n");
#endif
#if (CARDV_EDOG_ENABLE)
    printf(
        "\t'edog <value[0, 7]> ', edog operation.\n"
        "\t\t[0] init [1] list info [2] add Manual POI [3] restore Manual POI [4] clear Manual POI\n"
        "\t\t[5] parase header [6] delete Manual POI [7] update speedcam info [8] open edog log.\n"
        "\t\t'edog 2_1' add 1 POI mannual, max five once,\n"
        "\t\t'edog 6_0' delete index 0 POI in manual,\n"
        "\t\t'edog 8_1' open edog log\n");
#endif
#if (CARDV_DMS_ENABLE)
    printf("\t'dms <value[0, 1]> ', 'dms 1' enable dms.\n");
#endif
#if (CARDV_SPEECH_ENABLE)
    printf("\t'speech', 'speech 1' enable speech.\n");
#endif
#if (CARDV_LIVE555_ENABLE)
    printf("\t'rtsp', 'rtsp 1' enable live555.\n");
#endif
#if (CARDV_ALINK_ENABLE)
    printf("\t'alink <value[0, 1]> ', 'alink 1' enable alink.\n");
#endif
#if (CARDV_LD_ENABLE)
    printf("\t'LD moduleId dev chnl port debug ', LD connect to which module.res=480x320\n");
    printf("\t'LD <onoff[1, 0]> ', LD init/deinit\n");
#endif
    printf("\t lcdbri <value[0, 100]> ', lcdbri 100, set LCD brightness 100.\n");
    /* quit / useage */
    printf("\t, 'help', list useage\n");

    return 0;
}

void cardv_process_capture(MI_S32 s32FrameCnt, BOOL bRaw, MI_U32 u32ShutterUs)
{
    if (StorageIsMounted() == FALSE)
    {
        if (g_bAudioOut)
        {
            MI_S8 audioparam[64];
            audioparam[0] = 1;
            memcpy(&audioparam[1], WAV_PATH_CARD_EJECT, sizeof(WAV_PATH_CARD_EJECT));
            cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_CARD_EJECT) + 1);
            IPC_MsgToUI_SendMsg(IPC_MSG_UI_SD_NOCARD, NULL, 0);
        }
        printf("Cap Fail => SD Not Exist\n");
        return;
    }

    if (!DCF_IsDBFormatFree(DB_PHOTO) && (StorageGetOtherTotalSize() >> 10) * 100 / (StorageGetTotalSize() >> 10) > 30)
    {
        if (g_bAudioOut)
        {
            MI_S8 audioparam[64];
            audioparam[0] = 1;
            memcpy(&audioparam[1], WAV_PATH_CARD_FORMAT, sizeof(WAV_PATH_CARD_FORMAT));
            cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_CARD_FORMAT) + 1);
            IPC_MsgToUI_SendMsg(IPC_MSG_UI_SD_NEED_FORMAT, NULL, 0);
        }
        printf("Cap Fail => Format SD first\n");
        return;
    }

    if (g_bAudioOut)
    {
        MI_S8 audioparam[64];
        audioparam[0] = 1;
        memcpy(&audioparam[1], WAV_PATH_CAPTURE, sizeof(WAV_PATH_CAPTURE));
        cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_CAPTURE) + 1);
    }

    if (bRaw)
    {
        MI_S32 param[1];
        param[0] = u32ShutterUs;
        cardv_send_cmd(CMD_VIDEO_CAPTURE_RAW, (MI_S8 *)param, sizeof(param));
    }
    else
    {
        MI_S32 param[2];
        param[0] = s32FrameCnt;
        param[1] = u32ShutterUs;
        cardv_send_cmd(CMD_VIDEO_CAPTURE, (MI_S8 *)param, sizeof(param));
    }
}

void cardv_process_pre_record_start(void)
{
    if (g_eMuxerStatus == MUXER_STATUS_IDLE)
    {
        if (g_bAudioIn == TRUE)
        {
            cardv_send_cmd_HP(CMD_AUDIO_IN_START, NULL, 0);
        }

        for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
        {
            if (gstVencAttr[i].bUsed == FALSE)
                continue;
            if (gstVencAttr[i].eVideoType == VIDEO_STREAM_RECORD || gstVencAttr[i].eVideoType == VIDEO_STREAM_SUBREC
                || gstVencAttr[i].eVideoType == VIDEO_STREAM_SHARE)
            {
                MI_S8 param[1];
                param[0] = i;
                cardv_send_cmd_HP(CMD_VIDEO_START, (MI_S8 *)param, sizeof(param));
            }
        }

        cardv_send_cmd_and_wait(CMD_MUX_PRE_START, NULL, 0); // Must call CMD_MUX_PRE_START before CMD_MUX_START
        g_eMuxerStatus = MUXER_STATUS_PRE_RECORD;
    }
}

void cardv_process_record_start(void)
{
    MI_S8 param[1];

    if (g_eMuxerStatus == MUXER_STATUS_PRE_RECORD || g_eMuxerStatus == MUXER_STATUS_IDLE)
    {
        if (StorageIsMounted() == FALSE)
        {
            if (g_bAudioOut)
            {
                MI_S8 audioparam[64];
                audioparam[0] = 0;
                memcpy(&audioparam[1], WAV_PATH_CARD_EJECT, sizeof(WAV_PATH_CARD_EJECT));
                cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_CARD_EJECT) + 1);
                IPC_MsgToUI_SendMsg(IPC_MSG_UI_SD_NOCARD, NULL, 0);
            }
            printf("Rec Fail => SD Not Exist\n");
            return;
        }

        if (!DCF_IsDBFormatFree(DB_NORMAL)
            && (StorageGetOtherTotalSize() >> 10) * 100 / (StorageGetTotalSize() >> 10) > 30)
        {
            if (g_bAudioOut)
            {
                MI_S8 audioparam[64];
                audioparam[0] = 0;
                memcpy(&audioparam[1], WAV_PATH_CARD_FORMAT, sizeof(WAV_PATH_CARD_FORMAT));
                cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_CARD_FORMAT) + 1);
                IPC_MsgToUI_SendMsg(IPC_MSG_UI_SD_NEED_FORMAT, NULL, 0);
            }
            printf("Rec Fail => Format SD first\n");
            return;
        }

        if (DCF_CheckDeleteFilesForSpace(DB_NORMAL) == TRUE)
        {
            if (g_eMuxerStatus == MUXER_STATUS_IDLE)
            {
                if (g_bAudioIn == TRUE)
                {
                    cardv_send_cmd_HP(CMD_AUDIO_IN_START, NULL, 0);
                }

                for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
                {
                    if (gstVencAttr[i].bUsed == FALSE)
                        continue;
                    if (gstVencAttr[i].eVideoType == VIDEO_STREAM_RECORD
                        || gstVencAttr[i].eVideoType == VIDEO_STREAM_SUBREC
                        || gstVencAttr[i].eVideoType == VIDEO_STREAM_SHARE)
                    {
                        param[0] = i;
                        cardv_send_cmd_HP(CMD_VIDEO_START, (MI_S8 *)param, sizeof(param));
                    }
                }

                cardv_send_cmd_HP(CMD_MUX_PRE_START, NULL, 0); // Must call CMD_MUX_PRE_START before CMD_MUX_START
                g_eMuxerStatus = MUXER_STATUS_PRE_RECORD;
            }

            if (g_bAudioOut)
            {
                MI_S8 audioparam[64];
                audioparam[0] = 0;
                memcpy(&audioparam[1], WAV_PATH_REC_START, sizeof(WAV_PATH_REC_START));
                cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_REC_START) + 1);
            }

            param[0] = MUXER_NORMAL;
            cardv_send_cmd_and_wait(CMD_MUX_START, (MI_S8 *)param, sizeof(param));
            g_eMuxerStatus = MUXER_STATUS_RECORD;
        }
    }
}

void cardv_process_record_motion_detect_start(void)
{
#define MD_TRIGGER_PLAY_AUDIO (0)
    MI_S8 param[1];

    if (g_eMuxerStatus == MUXER_STATUS_PRE_RECORD || g_eMuxerStatus == MUXER_STATUS_IDLE)
    {
        if (StorageIsMounted() == FALSE)
        {
            if (g_bAudioOut && MD_TRIGGER_PLAY_AUDIO)
            {
                MI_S8 audioparam[64];
                audioparam[0] = 0;
                memcpy(&audioparam[1], WAV_PATH_CARD_EJECT, sizeof(WAV_PATH_CARD_EJECT));
                cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_CARD_EJECT) + 1);
                IPC_MsgToUI_SendMsg(IPC_MSG_UI_SD_NOCARD, NULL, 0);
            }
            printf("Rec Fail => SD Not Exist\n");
            return;
        }

        if (!DCF_IsDBFormatFree(DB_PARKING)
            && (StorageGetOtherTotalSize() >> 10) * 100 / (StorageGetTotalSize() >> 10) > 30)
        {
            if (g_bAudioOut && MD_TRIGGER_PLAY_AUDIO)
            {
                MI_S8 audioparam[64];
                audioparam[0] = 0;
                memcpy(&audioparam[1], WAV_PATH_CARD_FORMAT, sizeof(WAV_PATH_CARD_FORMAT));
                cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_CARD_FORMAT) + 1);
                IPC_MsgToUI_SendMsg(IPC_MSG_UI_SD_NEED_FORMAT, NULL, 0);
            }
            printf("Rec Fail => Format SD first\n");
            return;
        }

        if (DCF_CheckDeleteFilesForSpace(DB_PARKING) == TRUE)
        {
            if (g_eMuxerStatus == MUXER_STATUS_IDLE)
            {
                if (g_bAudioIn == TRUE)
                {
                    cardv_send_cmd_HP(CMD_AUDIO_IN_START, NULL, 0);
                }

                for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
                {
                    if (gstVencAttr[i].bUsed == FALSE)
                        continue;
                    if (gstVencAttr[i].eVideoType == VIDEO_STREAM_RECORD
                        || gstVencAttr[i].eVideoType == VIDEO_STREAM_SUBREC
                        || gstVencAttr[i].eVideoType == VIDEO_STREAM_SHARE)
                    {
                        param[0] = i;
                        cardv_send_cmd_HP(CMD_VIDEO_START, (MI_S8 *)param, sizeof(param));
                    }
                }

                cardv_send_cmd_HP(CMD_MUX_PRE_START, NULL, 0); // Must call CMD_MUX_PRE_START before CMD_MUX_START
                g_eMuxerStatus = MUXER_STATUS_PRE_RECORD;
            }

            if (g_bAudioOut && MD_TRIGGER_PLAY_AUDIO)
            {
                MI_S8 audioparam[64];
                audioparam[0] = 0;
                memcpy(&audioparam[1], WAV_PATH_REC_START, sizeof(WAV_PATH_REC_START));
                cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_REC_START) + 1);
            }

            param[0]         = MUXER_NORMAL;
            g_bRecMdtTrigger = TRUE;
            cardv_send_cmd_and_wait(CMD_MUX_START, (MI_S8 *)param, sizeof(param));
            g_eMuxerStatus = MUXER_STATUS_RECORD;
        }
    }
}

void cardv_process_record_share_start(void)
{
    MI_S8 param[1];

    if ((g_eMuxerStatus == MUXER_STATUS_PRE_RECORD || g_eMuxerStatus == MUXER_STATUS_RECORD)
        && carInfo.stRecInfo.bMuxingShare == FALSE)
    {
        if (StorageIsMounted() == FALSE)
        {
            printf("Share Rec Fail => SD Not Exist\n");
            return;
        }

        DCF_CheckDeleteFilesForSpace(DB_SHARE);
        param[0] = MUXER_SHARE;
        cardv_send_cmd_and_wait(CMD_MUX_START, (MI_S8 *)param, sizeof(param));
    }
}

void cardv_process_record_emerg_start(void)
{
    MI_S8 param[1];

    if ((g_eMuxerStatus == MUXER_STATUS_PRE_RECORD || g_eMuxerStatus == MUXER_STATUS_RECORD)
        && carInfo.stRecInfo.bMuxingEmerg == FALSE)
    {
        if (StorageIsMounted() == FALSE)
        {
            printf("Emerg Rec Fail => SD Not Exist\n");
            return;
        }

        DCF_CheckDeleteFilesForSpace(DB_EVENT);
        param[0] = MUXER_EMERG;
        cardv_send_cmd_and_wait(CMD_MUX_START, (MI_S8 *)param, sizeof(param));
    }
}

void cardv_process_record_stop(BOOL bKeepEncoding)
{
#if !(CARDV_DISPLAY_ENABLE)
    for (int i = MAX_VIDEO_NUMBER - 1; i >= 0; i--)
    {
        if (gstVencAttr[i].bUsed && gstVencAttr[i].eVideoType == VIDEO_STREAM_RTSP
            && gstVencAttr[i].stVencInBindParam.stChnPort.eModId == E_MI_MODULE_ID_VENC)
        {
            // because venc0 is rtsp stream's input source,so,when venc0 shouldn't be stopped,just stop muxer
            bKeepEncoding = TRUE;
            break;
        }
    }
#endif
    if (g_eMuxerStatus == MUXER_STATUS_PRE_RECORD || g_eMuxerStatus == MUXER_STATUS_RECORD
        || g_eMuxerStatus == MUXER_STATUS_PAUSE)
    {
        if (bKeepEncoding == FALSE && g_eMuxerStatus != MUXER_STATUS_PAUSE)
        {
            for (int i = MAX_VIDEO_NUMBER - 1; i >= 0; i--)
            {
                if (gstVencAttr[i].bUsed == FALSE)
                    continue;
                if (gstVencAttr[i].eVideoType == VIDEO_STREAM_RECORD || gstVencAttr[i].eVideoType == VIDEO_STREAM_SUBREC
                    || gstVencAttr[i].eVideoType == VIDEO_STREAM_SHARE)
                {
                    MI_S32 param[1];
                    param[0] = i;
                    cardv_send_cmd_HP(CMD_VIDEO_STOP, (MI_S8 *)param, sizeof(param));
                }
            }

            if (g_bAudioIn == TRUE)
            {
                cardv_send_cmd_HP(CMD_AUDIO_IN_STOP, NULL, 0);
            }
        }

        if (g_eMuxerStatus == MUXER_STATUS_RECORD)
        {
            if (bKeepEncoding)
            {
                cardv_send_cmd_HP(CMD_MUX_STOP_KEEP_ENCODING, NULL, 0);
                g_eMuxerStatus = MUXER_STATUS_PRE_RECORD;
            }
            else
            {
                cardv_send_cmd_and_wait(CMD_MUX_STOP, NULL, 0);
                g_eMuxerStatus = MUXER_STATUS_IDLE;
            }

            carInfo.stRecInfo.bMuxing      = FALSE;
            carInfo.stRecInfo.bMuxingEmerg = FALSE;
            carInfo.stRecInfo.bMuxingShare = FALSE;
            carInfo.stRecInfo.bMuxingMD    = FALSE;
            IPC_CarInfo_Write_RecInfo(&carInfo.stRecInfo);

            cardv_update_status(REC_STATUS, "0", 2);

            if (g_bLockFile)
            {
                DCF_CheckDeleteFilesForSpace(DB_EVENT);
                DCF_MoveLastFile(DB_NORMAL, DB_EVENT);
                g_bLockFile = FALSE;
            }
        }
        else
        {
            if (bKeepEncoding == FALSE)
            {
                // Because pre-muxing is working, stop it.
                cardv_send_cmd_and_wait(CMD_MUX_STOP, NULL, 0);
                g_eMuxerStatus = MUXER_STATUS_IDLE;

                carInfo.stRecInfo.bMuxing      = FALSE;
                carInfo.stRecInfo.bMuxingEmerg = FALSE;
                carInfo.stRecInfo.bMuxingShare = FALSE;
                carInfo.stRecInfo.bMuxingMD    = FALSE;
                IPC_CarInfo_Write_RecInfo(&carInfo.stRecInfo);

                cardv_update_status(REC_STATUS, "0", 2);

                if (g_bLockFile)
                {
                    DCF_CheckDeleteFilesForSpace(DB_EVENT);
                    DCF_MoveLastFile(DB_NORMAL, DB_EVENT);
                    g_bLockFile = FALSE;
                }
            }
        }
    }
}

void cardv_process_record_pause(void)
{
    if (g_eMuxerStatus == MUXER_STATUS_RECORD && carInfo.stRecInfo.bMuxing == TRUE)
    {
        for (int i = MAX_VIDEO_NUMBER - 1; i >= 0; i--)
        {
            if (gstVencAttr[i].bUsed == FALSE)
                continue;
            if (gstVencAttr[i].eVideoType == VIDEO_STREAM_RECORD || gstVencAttr[i].eVideoType == VIDEO_STREAM_SUBREC
                || gstVencAttr[i].eVideoType == VIDEO_STREAM_SHARE)
            {
                MI_S32 param[1];
                param[0] = i;
                cardv_send_cmd_HP(CMD_VIDEO_STOP, (MI_S8 *)param, sizeof(param));
            }
        }

        if (g_bAudioIn == TRUE)
        {
            cardv_send_cmd_HP(CMD_AUDIO_IN_STOP, NULL, 0);
        }

        cardv_send_cmd_and_wait(CMD_MUX_PAUSE, NULL, 0);
        g_eMuxerStatus = MUXER_STATUS_PAUSE;
    }
}

void cardv_process_record_resume(void)
{
    if (g_eMuxerStatus == MUXER_STATUS_PAUSE)
    {
        if (g_bAudioIn == TRUE)
        {
            cardv_send_cmd_HP(CMD_AUDIO_IN_START, NULL, 0);
        }

        for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
        {
            if (gstVencAttr[i].bUsed == FALSE)
                continue;
            if (gstVencAttr[i].eVideoType == VIDEO_STREAM_RECORD || gstVencAttr[i].eVideoType == VIDEO_STREAM_SUBREC
                || gstVencAttr[i].eVideoType == VIDEO_STREAM_SHARE)
            {
                MI_S32 param[1];
                param[0] = i;
                cardv_send_cmd_HP(CMD_VIDEO_START, (MI_S8 *)param, sizeof(param));
            }
        }

        cardv_send_cmd_and_wait(CMD_MUX_RESUME, NULL, 0);
        g_eMuxerStatus = MUXER_STATUS_RECORD;
    }
}

void cardv_process_record_audio_start(void)
{
    if (g_bAudioIn == TRUE)
    {
        cardv_send_cmd_HP(CMD_AUDIO_IN_START_FILE, NULL, 0);
    }
}

void cardv_process_record_audio_stop(void)
{
    if (g_bAudioIn == TRUE)
    {
        cardv_send_cmd_HP(CMD_AUDIO_IN_STOP_FILE, NULL, 0);
    }
}

void cardv_process_record_seamless_restart(void)
{
    MI_S8 param[1];
    if (DCF_CheckDeleteFilesForSpace(DB_NORMAL) == TRUE)
    {
        if (g_bLockFile)
        {
            DCF_CheckDeleteFilesForSpace(DB_EVENT);
            DCF_MoveLastFile(DB_NORMAL, DB_EVENT);
            g_bLockFile = FALSE;
            cardv_update_status(REC_STATUS, "1", 2);
        }

        param[0] = MUXER_NORMAL;
        cardv_send_cmd_HP(CMD_MUX_START, (MI_S8 *)param, sizeof(param));
    }
    else
    {
        cardv_process_record_stop(FALSE);
    }
}

void cardv_process_set_VideoFps(MI_S32 s32Fps)
{
    for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
    {
        if (gstVencAttr[i].bUsed == FALSE)
            continue;
        if (gstVencAttr[i].eVideoType == VIDEO_STREAM_RECORD || gstVencAttr[i].eVideoType == VIDEO_STREAM_SUBREC)
        {
            MI_S32 param[2];
            param[0] = i;
            param[1] = s32Fps;
            cardv_send_cmd_HP(CMD_VIDEO_SET_FRAMERATE, (MI_S8 *)param, sizeof(param));
        }
    }
}

void cardv_process_set_HDR_Mode(BOOL bHdr)
{
    MI_S32 param[1];
    param[0] = bHdr;
    cardv_send_cmd(CMD_PIPE_SWITCH_HDR, (MI_S8 *)param, sizeof(param));
}

void cardv_process_set_sensor_res_idx(MI_U8 u8SnrPad, MI_U8 u8SnrIdx)
{
    MI_S32 param[2];
    param[0] = u8SnrPad;
    param[1] = u8SnrIdx;
    cardv_send_cmd(CMD_PIPE_SWITCH_SENSOR_RES_IDX, (MI_S8 *)param, sizeof(param));
}

void cardv_process_set_MuxerFps(MI_U8 u8MuxerType, MI_U16 s32FpsNum, MI_U16 s32FpsDen, MI_BOOL bTimeLapse,
                                MuxerSlowMotionStatus eSlowMotion)
{
    MI_S32 param[5];
    param[0] = u8MuxerType;
    param[1] = s32FpsNum;
    param[2] = s32FpsDen;
    param[3] = bTimeLapse;
    param[4] = eSlowMotion;
    cardv_send_cmd_HP(CMD_MUX_SET_FRAMERATE, (MI_S8 *)param, sizeof(param));
}

void cardv_process_set_MuxerLimitTime(MI_U8 u8MuxerType, MI_U64 u64Time)
{
    MI_U64 param[2];
    param[0] = u8MuxerType;
    param[1] = u64Time;
    cardv_send_cmd_HP(CMD_MUX_SET_REC_LIMIT_TIME, (MI_S8 *)param, sizeof(param));
}

void cardv_process_set_MuxerPreRecordTime(MI_U8 u8MuxerType, MI_U64 u64Time)
{
    MI_U64 param[2];
    param[0] = u8MuxerType;
    param[1] = u64Time;
    cardv_send_cmd_HP(CMD_MUX_SET_PRE_REC_TIME, (MI_S8 *)param, sizeof(param));
}

static void cardv_cmd_handler_zoom(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_S32 param[3];
        param[0] = atoi(argv[1]); // isp dev
        param[1] = atoi(argv[2]); // isp chn
        param[2] = atoi(argv[3]); // 0 : Zoom Out 1 : Zoom In 2 : Zoom Stop
        cardv_send_cmd(CMD_PIPE_ISP_ZOOM, (MI_S8 *)param, sizeof(param));
    }
}

static void cardv_cmd_handler_seamless(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    cardv_process_record_seamless_restart();
}

static void cardv_cmd_handler_audioPlay(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    // echo audioplay 1 1 live  > /tmp/cardv_fifo
    // echo audioplay 1 1 filename  > /tmp/cardv_fifo
    if (argc == u8MaxPara)
    {
        if (g_bAudioOut)
        {
            MI_S8 audioparam[128] = {0};
            audioparam[0]         = atoi(argv[1]); // 1 : Stop immediately  0 : Wait for playback done
            audioparam[1]         = atoi(argv[2]); // 0 : dev0 1 : dev1
            memcpy(&audioparam[2], argv[3], 126);
            cardv_send_cmd(CMD_AUDIO_OUT_MULTI_PLAY, (MI_S8 *)audioparam, 128);
        }
    }
}

static void cardv_cmd_handler_audioStop(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        if (g_bAudioOut)
        {
            MI_S8 audioparam[64];
            audioparam[0] = atoi(argv[1]); // 1 : Stop immediately  0 : Wait for playback done
            audioparam[1] = atoi(argv[2]); // 0 : dev0 1 : dev1
            cardv_send_cmd(CMD_AUDIO_OUT_STOP, (MI_S8 *)audioparam, 64);
        }
    }
}

static void cardv_cmd_handler_audioRecord(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S8 param[1];

    if (argc == u8MaxPara)
    {
        param[0] = atoi(argv[1]);
        printf("audio %s\n", param[0] ? "mute" : "un-mute");
        cardv_send_cmd(CMD_AUDIO_IN_MUTE, (MI_S8 *)param, sizeof(param));
    }
}

static void cardv_cmd_handler_setAudioPlayVol(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S8  param;
    MI_S32 s32Volume = 0;

    if (argc == u8MaxPara)
    {
        param = atoi(argv[1]);
#ifdef CHIP_IFORD
        s32Volume = (param * (MAX_AO_VOLUME - MIN_AO_VOLUME) / 10 + MIN_AO_VOLUME) * 8;
#else
        s32Volume = param * (MAX_AO_VOLUME - MIN_AO_VOLUME) / 10 + MIN_AO_VOLUME;
#endif
        printf("Audio Out DB [%d]\n", s32Volume);
        g_audioOutParam[0].s8VolumeOutDb = s32Volume;
    }
}

#if (CARDV_LIVE555_ENABLE)
static void cardv_cmd_handler_RtspSwitch(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (g_bRtspEnable == TRUE)
    {
        MI_S8 param = atoi(argv[1]);
        if (param == 1)
            cardv_send_cmd_HP(CMD_SYSTEM_LIVE555_OPEN, NULL, 0);
        else if (param == 0)
            cardv_send_cmd_HP(CMD_SYSTEM_LIVE555_CLOSE, NULL, 0);
        else
            cardv_send_cmd_HP(CMD_SYSTEM_LIVE555_CLOSE_SESSION, NULL, 0);
    }
}
#endif

#if (CARDV_DISPLAY_ENABLE)
static void cardv_cmd_handler_display(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    /*
        echo disp pnl on > /tmp/cardv_fifo
        echo disp start > /tmp/cardv_fifo
        echo disp stop > /tmp/cardv_fifo
        echo disp pnl off > /tmp/cardv_fifo

        note: call "pnl on" first, than "start", "stop".
     */

    if (strcmp(argv[1], "start") == 0)
    {
        if (argc == 3)
        {
            MI_S8 param[1];
            param[0] = atoi(argv[2]);
            cardv_send_cmd_HP(CMD_DISP_PREVIEW_START, &param[0], sizeof(param));
        }
        else
        {
            cardv_send_cmd_HP(CMD_DISP_PREVIEW_START, NULL, 0);
        }
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        cardv_send_cmd_HP(CMD_DISP_PREVIEW_STOP, NULL, 0);
    }
    else if (strcmp(argv[1], "switch") == 0)
    {
        MI_S8 param[1];
        param[0] = atoi(argv[2]);
        cardv_send_cmd_HP(CMD_DISP_PREVIEW_STOP, NULL, 0);
        cardv_send_cmd_HP(CMD_DISP_PREVIEW_START, &param[0], sizeof(param));
    }
    else if (strcmp(argv[1], "move") == 0)
    {
        MI_S32 param[3];
        param[0] = atoi(argv[2]);
        param[1] = atoi(argv[3]);
        param[2] = atoi(argv[4]);
        cardv_send_cmd(CMD_DISP_PREVIEW_MOVE, (MI_S8 *)param, sizeof(param));
    }
    else if (strcmp(argv[1], "playstart") == 0)
    {
        cardv_send_cmd_HP(CMD_DISP_PREVIEW_STOP, NULL, 0);
        cardv_send_cmd_HP(CMD_DISP_PLAYBACK_START, NULL, 0);
    }
    else if (strcmp(argv[1], "playstop") == 0)
    {
        cardv_send_cmd_HP(CMD_DISP_PLAYBACK_STOP, NULL, 0);
        cardv_send_cmd_HP(CMD_DISP_PREVIEW_START, NULL, 0);
    }
}
#endif

static void cardv_cmd_handler_ParkingMonitor(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S8 param[1];
    if (argc == u8MaxPara)
    {
        param[0] = atoi(argv[1]);
        printf("ParkingMonitor %d\n", param[0]);
        GsensorParkingMonitor(param[0]);
    }
}

static void cardv_cmd_handler_GsensorSensitivity(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S8 param[1];
    if (argc == u8MaxPara)
    {
        param[0] = atoi(argv[1]);
        printf("GsensorSensitivity %d\n", param[0]);
        GsensorSetSensitivity(param[0]);
    }
}

#if (CARDV_VDF_ENABLE)
static void cardv_process_set_MdtOnOff(MI_BOOL bEnable)
{
    CarDV_VdfModAttr_t *pstVdfModule = &gstVdfModule;

    if (bEnable && pstVdfModule->bUsed)
    {
        cardv_send_cmd_HP(CMD_VDF_OPEN, NULL, 0);
        cardv_send_cmd_HP(CMD_VDF_MD_OPEN, NULL, 0);
    }
    else if (!bEnable && pstVdfModule->bCreate)
    {
        cardv_send_cmd_HP(CMD_VDF_MD_CLOSE, NULL, 0);
        cardv_send_cmd_HP(CMD_VDF_CLOSE, NULL, 0);
    }
}

static void cardv_process_set_MdtLevel(MI_S8 mdtlevel)
{
    // printf("%s: level%d\n",__FUNCTION__, mdtlevel);
    switch (mdtlevel)
    {
    case 1:
        g_MdSens = 150; // low sensitivity
        break;
    case 2:
        g_MdSens = 50;
        break;
    case 3:
        g_MdSens = 10; // high sensitivity
        break;
    default:
        g_MdSens = 0x7FFFFFFF;
        break;
    }
    cardv_process_set_MdtOnOff((g_MdSens < 0xff) ? TRUE : FALSE);
}

static void cardv_cmd_handler_MotionSensitivity(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S8 param[1];
    if (argc == u8MaxPara)
    {
        param[0] = atoi(argv[1]);
        cardv_process_set_MdtLevel(param[0]);
    }
}
#endif

static void cardv_cmd_handler_VoiceSwitch(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S8 param[1];

    if (argc == u8MaxPara)
    {
        param[0] = atoi(argv[1]);
        printf("VoiceSwitch %s\n", param[0] ? "Voice ON" : "Voice OFF");
        g_bAudioOut = param[0] ? TRUE : FALSE;
    }
}
static void cardv_cmd_handler_SensorFlip(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    // MI_S8 param[1];

    // if (argc == u8MaxPara) {
    //     param[0] = atoi(argv[1]);
    //     printf("flip %s\n", param[0] ? "Normal" : "flip");
    //     MI_SNR_SetOrien(E_MI_SNR_PAD_ID_0, param[0], param[0]);
    // }
}

static void cardv_BitrateCheck(MI_U8 i, MI_U32 u32Width, MI_U32 u32Height, MI_U32 maxBitRate, MI_U32 minBitRate,
                               MI_U8 currentFps, MI_U32 *currentBitRate)
{
    if ((u32Width == gstVencAttr[i].u32Width) && (u32Height == gstVencAttr[i].u32Height))
    {
        if (*currentBitRate < minBitRate)
        {
            *currentBitRate = minBitRate;
            return;
        }
        else if (*currentBitRate > maxBitRate)
        {
            *currentBitRate = maxBitRate;
            return;
        }
    }
}

static void cardv_cmd_handler_setBitrate(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_U32 param[2];

    if (argc == u8MaxPara)
    {
        param[0] = atoi(argv[1]);
        param[1] = atoi(argv[2]);

        //(u32PicWidth/u32PicHeight) and profile is static Attr.
        // if (g_bEncoding == FALSE)
        {
            // printf("set channel[%d] bitrate: %d \n", param[0], param[1]);
            // check bitrate
            if ((640 <= gstVencAttr[param[0]].u32Width && 1920 >= gstVencAttr[param[0]].u32Width)
                || (480 <= gstVencAttr[param[0]].u32Height && 1088 >= gstVencAttr[param[0]].u32Height))
            {
                cardv_BitrateCheck(param[0], 1920, 1088, 12000000, 1000000, gstVencAttr[param[0]].u32VencFrameRate,
                                   (MI_U32 *)&param[1]);
                cardv_BitrateCheck(param[0], 1920, 1080, 12000000, 1000000, gstVencAttr[param[0]].u32VencFrameRate,
                                   (MI_U32 *)&param[1]);
                // RTCP
                cardv_BitrateCheck(param[0], 1280, 736, 5000000, 100000, gstVencAttr[param[0]].u32VencFrameRate,
                                   (MI_U32 *)&param[1]);
                cardv_BitrateCheck(param[0], 1280, 720, 5000000, 100000, gstVencAttr[param[0]].u32VencFrameRate,
                                   (MI_U32 *)&param[1]);

                cardv_BitrateCheck(param[0], 720, 576, 2500000, 500000, gstVencAttr[param[0]].u32VencFrameRate,
                                   (MI_U32 *)&param[1]);
                cardv_BitrateCheck(param[0], 704, 576, 2500000, 500000, gstVencAttr[param[0]].u32VencFrameRate,
                                   (MI_U32 *)&param[1]);

                cardv_BitrateCheck(param[0], 640, 480, 2500000, 500000, gstVencAttr[param[0]].u32VencFrameRate,
                                   (MI_U32 *)&param[1]);
            }
            // printf("width:%d  height:%d  u32VencFrameRate:%d  bitrate:%d \n", g_videoParam[param[0]].u32Width,
            // g_videoParam[param[0]].u32Height, g_videoParam[param[0]].u32VencFrameRate, param[1]);
            cardv_send_cmd(CMD_VIDEO_SET_BITRATE, (MI_S8 *)param, sizeof(param));
        }
    }
}

static void cardv_cmd_handler_setCodec(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S32 param[2];
    if (argc == u8MaxPara)
    {
        param[0] = atoi(argv[1]);
        param[1] = atoi(argv[2]);
        if (g_eMuxerStatus == MUXER_STATUS_IDLE)
        {
            cardv_send_cmd(CMD_VIDEO_SET_CODEC, (MI_S8 *)param, sizeof(param));
        }
        else
        {
            printf("Stop rec/pre_rec first\n");
        }
    }
}

static void cardv_cmd_handler_setResolution(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S32 param[3];

    if (argc == u8MaxPara)
    {
        param[0] = atoi(argv[1]); // venc channel
        param[1] = atoi(argv[2]); // width
        param[2] = atoi(argv[3]); // height

        if (g_eMuxerStatus == MUXER_STATUS_IDLE)
        {
            printf("set venc chn%d res %dx%d\n", param[0], param[1], param[2]);
            if (g_displayOsd)
            {
                cardv_send_cmd_HP(CMD_OSD_RESET_RESOLUTION, (MI_S8 *)param, sizeof(param));
            }
            cardv_send_cmd_HP(CMD_VIDEO_SET_RESOLUTION, (MI_S8 *)param, sizeof(param));
            usleep(1000);
            if (g_displayOsd)
            {
                cardv_send_cmd_HP(CMD_OSD_RESTART_RESOLUTION, (MI_S8 *)param, sizeof(param));
            }
        }
        else
        {
            printf("Stop rec/pre_rec first\n");
        }
    }
}

static void cardv_cmd_handler_setVideoResolution(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S32 param[3];

    if (argc == u8MaxPara)
    {
        param[1] = atoi(argv[1]); // width
        param[2] = atoi(argv[2]); // height

        if (g_eMuxerStatus == MUXER_STATUS_IDLE)
        {
            param[0] = VideoMapToIndex(0, VIDEO_STREAM_RECORD); // Find Cam0 Venc channel
            if (param[0] < 0)
                return;

            printf("set venc chn%d res %dx%d\n", param[0], param[1], param[2]);
            if (g_displayOsd)
            {
                cardv_send_cmd_HP(CMD_OSD_RESET_RESOLUTION, (MI_S8 *)param, sizeof(param));
            }
            cardv_send_cmd_HP(CMD_VIDEO_SET_RESOLUTION, (MI_S8 *)param, sizeof(param));
            usleep(1000);
            if (g_displayOsd)
            {
                cardv_send_cmd_HP(CMD_OSD_RESTART_RESOLUTION, (MI_S8 *)param, sizeof(param));
            }
        }
        else
        {
            printf("Stop rec/pre_rec first\n");
        }
    }
}

static void cardv_cmd_handler_setCaptureResolution(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S32 param[3];

    if (argc == u8MaxPara)
    {
        param[1] = atoi(argv[1]); // width
        param[2] = atoi(argv[2]); // height

        param[0] = VideoMapToIndex(0, VIDEO_STREAM_CAPTURE); // Find Cam0 Venc channel
        if (param[0] < 0)
            return;

        printf("set venc chn%d res %dx%d\n", param[0], param[1], param[2]);
        if (g_displayOsd)
        {
            cardv_send_cmd_HP(CMD_OSD_RESET_RESOLUTION, (MI_S8 *)param, sizeof(param));
        }
        cardv_send_cmd_HP(CMD_VIDEO_SET_RESOLUTION, (MI_S8 *)param, sizeof(param));
        usleep(1000);
        if (g_displayOsd)
        {
            cardv_send_cmd_HP(CMD_OSD_RESTART_RESOLUTION, (MI_S8 *)param, sizeof(param));
        }
    }
}

static void cardv_cmd_handler_setRecLimitTime(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_U8 u8Min;

    if (argc == u8MaxPara)
    {
        u8Min = atoi(argv[1]);
        printf("set loop time %d minutes\n", u8Min);
        g_u64NormalRecTime     = u8Min * 60ULL * 1000000ULL;
        g_u64ShareEmergRecTime = u8Min * 60ULL * 1000000ULL;
        cardv_process_set_MuxerLimitTime(MUXER_NORMAL, g_u64NormalRecTime); // unit: real time usec
        cardv_process_set_MuxerLimitTime(MUXER_EMERG, g_u64ShareEmergRecTime);
        IPC_MsgToUI_SendMsg(IPC_MSG_UI_SETTING_UPDATE, NULL, 0);
    }
}

#if (CARDV_SPEECH_ENABLE)

void Speech_VoiceAnalyzeprocess(MI_U8 cmdIndex)
{
    switch (cmdIndex)
    {
    case 1:
        cardv_send_to_fifo(CARDV_CMD_START_REC, sizeof(CARDV_CMD_START_REC));
        break;
    case 2:
        cardv_send_to_fifo(CARDV_CMD_STOP_REC, sizeof(CARDV_CMD_STOP_REC));
        break;
    case 3:
        cardv_send_to_fifo(CARDV_CMD_CAPTURE, sizeof(CARDV_CMD_CAPTURE));
        break;
    case 5:
        cardv_send_to_fifo(CARDV_CMD_STOP_REC, sizeof(CARDV_CMD_STOP_REC));
        // IPC_MsgToUI_SendMsg(IPC_MSG_UI_SPEECH_POWEROFF, NULL, 0);
        break;
    case 4:
        // cardv_send_to_fifo(CARDV_CMD_CAPTURE5, sizeof(CARDV_CMD_CAPTURE5));
        break;
    default:
        printf("TODO:Speech Result: %d\n", cmdIndex);
        break;
    }
}

void Speech_txzActivateStatusNotify(MI_S32 cmdIndex)
{
    switch (cmdIndex)
    {
    case TXZAS_WAIT_USB_MOUNT:
        // TODO
        break;
    case TXZAS_CREATE_PUSH:
        // TODO
        break;
    case TXZAS_WAIT_POP:
        // TODO
        break;
    case TXZAS_SUCCESS:
        // TODO
        break;
    case TXZAS_FAILED:
        // TODO
        break;
    case TXZAS_GET_UUID_ERROR:
        // TODO
        break;
    case TXZAS_ACTIVATE_INFO_ERROR:
        // TODO
        break;
    case TXZAS_POP_DATA_PARSE_ERROR:
        // TODO
        break;
    default:
        // TODO
        break;
    }
}

static void cardv_cmd_handler_setSpeech(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        BOOL bEnable = atoi(argv[1]);
        if (bEnable && !g_bSpeechEnable)
        {
            if (MI_SUCCESS == Speech_VoiceAnalyzeInit())
            {
                if (MI_SUCCESS == Speech_VoiceAnalyzeStart())
                {
                    g_bSpeechEnable = TRUE;
                    Speech_VoiceAnalyzeprocessCallback(Speech_VoiceAnalyzeprocess);
                    Speech_txzActivateStatusNotifyCallback(Speech_txzActivateStatusNotify);
                    printf("cardv speech enable\n");
                }
            }
        }
        else if (!bEnable && g_bSpeechEnable)
        {
            g_bSpeechEnable = FALSE;
            // panic_test();
            if (MI_SUCCESS == Speech_VoiceAnalyzeStop())
                printf("cardv speech disable\n");
            Speech_VoiceAnalyzeDeInit();
        }
    }
}
#endif

static void cardv_cmd_handler_setLockFile(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_U32 u32Lock = atoi(argv[1]);
        if (carInfo.stRecInfo.bMuxing)
        {
            g_bLockFile = u32Lock;
            cardv_update_status(REC_STATUS, "2", 2);
        }
    }
}

static void cardv_cmd_handler_record(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_U32 u32Record = atoi(argv[1]);
        switch (u32Record)
        {
        case 0:
            cardv_process_record_stop(FALSE);
            break;
        case 1:
            cardv_process_record_start();
            break;
        case 2:
            cardv_process_record_emerg_start();
            break;
        case 3:
            cardv_process_record_share_start();
            break;
        case 4:
            // Stop muxing, keep encoding.(pre-record)
            cardv_process_record_stop(TRUE);
            break;
        case 5:
            IPC_CarInfo_Read_UIModeInfo(&carInfo.stUIModeInfo);
            if (carInfo.stUIModeInfo.u8Mode == UI_VIDEO_MODE)
            {
                cardv_process_record_motion_detect_start();
            }
            break;
        case 6:
            cardv_process_record_pause();
            break;
        case 7:
            cardv_process_record_resume();
            break;
        case 8:
            cardv_process_record_audio_start();
            break;
        case 9:
            cardv_process_record_audio_stop();
            break;
        default:
            break;
        }
    }
}

static void cardv_cmd_handler_videoFlow(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_U32 u32VideoType = atoi(argv[1]); // 0 : h26x, 1 : jpeg, 2 : h26x & jpeg 3: sensor
        MI_U32 u32InitVideo = atoi(argv[2]); // 0 : disable, 1 : enable

        if (u32VideoType == 0)
        {
            // H26X
            if (u32InitVideo)
            {
                cardv_send_cmd_HP(CMD_MUX_OPEN, NULL, 0);
                cardv_send_cmd_HP(CMD_VIDEO_INIT_H26X, NULL, 0);
            }
            else
            {
                cardv_process_record_stop(FALSE);
                cardv_send_cmd_HP(CMD_VIDEO_DEINIT_H26X, NULL, 0);
                cardv_send_cmd_HP(CMD_MUX_CLOSE, NULL, 0);
            }
        }
        else if (u32VideoType == 1)
        {
            // JPEG
            if (u32InitVideo)
            {
                cardv_send_cmd_HP(CMD_VIDEO_INIT_JPEG, NULL, 0);
            }
            else
            {
                cardv_send_cmd_HP(CMD_VIDEO_DEINIT_JPEG, NULL, 0);
            }
        }
        else if (u32VideoType == 2)
        {
            // H26X & JPEG
            if (u32InitVideo)
            {
                cardv_send_cmd_HP(CMD_MUX_OPEN, NULL, 0);
                cardv_send_cmd_HP(CMD_VIDEO_INIT_H26X, NULL, 0);
                cardv_send_cmd_HP(CMD_VIDEO_INIT_JPEG, NULL, 0);
            }
            else
            {
                cardv_process_record_stop(FALSE);
                cardv_send_cmd_HP(CMD_VIDEO_DEINIT_JPEG, NULL, 0);
                cardv_send_cmd_HP(CMD_VIDEO_DEINIT_H26X, NULL, 0);
                cardv_send_cmd_HP(CMD_MUX_CLOSE, NULL, 0);
            }
        }
        else if (u32VideoType == 3)
        {
            // SENSOR
            if (u32InitVideo)
            {
                cardv_send_cmd_HP(CMD_PIPE_SENSOR_ENABLE, NULL, 0);
            }
            else
            {
                cardv_send_cmd_HP(CMD_PIPE_SENSOR_DISABLE, NULL, 0);
            }
        }
    }
}

static void cardv_cmd_handler_setTimeLapse(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (g_eMuxerStatus != MUXER_STATUS_IDLE)
        cardv_process_record_stop(FALSE);

    if (argc == u8MaxPara)
    {
        g_u32TimeLapse = atoi(argv[1]);
        if (!g_u32TimeLapse)
        {
            printf("Normal Fps\n");
            cardv_process_set_VideoFps(CARDV_DEFAULT_FPS);
            cardv_process_set_MuxerFps(MUXER_NORMAL, MUXER_FPS_NUM, MUXER_FPS_DEN, FALSE, MUXER_SLOW_MOTION_NONE);
            cardv_process_set_MuxerFps(MUXER_EMERG, MUXER_FPS_NUM, MUXER_FPS_DEN, FALSE, MUXER_SLOW_MOTION_NONE);
            cardv_process_set_MuxerLimitTime(MUXER_NORMAL, g_u64NormalRecTime); // unit: real time usec
            cardv_process_set_MuxerLimitTime(MUXER_EMERG, g_u64ShareEmergRecTime);
            cardv_process_set_MuxerPreRecordTime(MUXER_NORMAL, 5000000);
            cardv_process_set_MuxerPreRecordTime(MUXER_EMERG, 5000000);
            g_bAudioIn = TRUE;
        }
        else
        {
            printf("TimeLapse [1/%d] Fps\n", g_u32TimeLapse);
            cardv_process_set_VideoFps((g_u32TimeLapse & 0xffff) << 16 | 1);
            cardv_process_set_MuxerFps(MUXER_NORMAL, CARDV_DEFAULT_FPS, 1, TRUE, MUXER_SLOW_MOTION_NONE);
            cardv_process_set_MuxerFps(MUXER_EMERG, CARDV_DEFAULT_FPS, 1, TRUE, MUXER_SLOW_MOTION_NONE);
            cardv_process_set_MuxerLimitTime(MUXER_NORMAL, g_u32TimeLapse * CARDV_DEFAULT_FPS
                                                               * g_u64NormalRecTime); // unit: real time usec
            cardv_process_set_MuxerLimitTime(MUXER_EMERG, g_u32TimeLapse * CARDV_DEFAULT_FPS * g_u64ShareEmergRecTime);
            cardv_process_set_MuxerPreRecordTime(MUXER_NORMAL, g_u32TimeLapse * CARDV_DEFAULT_FPS * 5000000);
            cardv_process_set_MuxerPreRecordTime(MUXER_EMERG, g_u32TimeLapse * CARDV_DEFAULT_FPS * 5000000);
            g_bAudioIn = FALSE;
        }
    }
}

static void cardv_cmd_handler_setSlowMotion(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (g_eMuxerStatus != MUXER_STATUS_IDLE)
        cardv_process_record_stop(FALSE);

    if (argc == u8MaxPara)
    {
        g_u32SlowMotion = atoi(argv[1]);
        if (!g_u32SlowMotion)
        {
            printf("SlowMotion Off\n");
            cardv_process_set_MuxerFps(MUXER_NORMAL, 0, 0, FALSE, MUXER_SLOW_MOTION_NONE);
            cardv_process_set_MuxerFps(MUXER_EMERG, 0, 0, FALSE, MUXER_SLOW_MOTION_NONE);
            cardv_process_set_MuxerLimitTime(MUXER_NORMAL, g_u64NormalRecTime); // unit: real time usec
            cardv_process_set_MuxerLimitTime(MUXER_EMERG, g_u64ShareEmergRecTime);
            g_bAudioIn = TRUE;
        }
        else
        {
            if (g_u32SlowMotion == MUXER_SLOW_MOTION_X2 || g_u32SlowMotion == MUXER_SLOW_MOTION_X4
                || g_u32SlowMotion == MUXER_SLOW_MOTION_X8)
            {
                printf("SlowMotion X%d\n", g_u32SlowMotion);
                cardv_process_set_MuxerFps(MUXER_NORMAL, 0, 0, FALSE, (MuxerSlowMotionStatus)g_u32SlowMotion);
                cardv_process_set_MuxerFps(MUXER_EMERG, 0, 0, FALSE, (MuxerSlowMotionStatus)g_u32SlowMotion);
                cardv_process_set_MuxerLimitTime(MUXER_NORMAL,
                                                 g_u64NormalRecTime / g_u32SlowMotion); // unit: real time usec
                cardv_process_set_MuxerLimitTime(MUXER_EMERG, g_u64ShareEmergRecTime / g_u32SlowMotion);
                g_bAudioIn = FALSE;
            }
            else
            {
                g_u32SlowMotion = 0;
            }
        }
    }
}

static void cardv_cmd_handler_capture(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == 2)
    {
        // continuous capture
        MI_S32 s32Num = atoi(argv[1]);
        cardv_process_capture(s32Num, FALSE, 0);
    }
    else if (argc == 3)
    {
        // long exposure capture + continuous capture(if need)
        MI_S32 s32Num       = atoi(argv[1]);
        MI_U32 u32ShutterUs = atoi(argv[2]);
        cardv_process_capture(s32Num, FALSE, u32ShutterUs);
    }
    else
    {
        // single capture
        cardv_process_capture(1, FALSE, 0);
    }
}

static void cardv_cmd_handler_capture_raw(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == 2)
    {
        // long exposure capture
        MI_U32 u32ShutterUs = atoi(argv[1]);
        cardv_process_capture(1, TRUE, u32ShutterUs);
    }
    else
    {
        // single capture
        cardv_process_capture(1, TRUE, 0);
    }
}

#if (CARDV_DMS_ENABLE)
static void cardv_cmd_handler_setDMS(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_U32 u32Init = atoi(argv[1]);
        switch (u32Init)
        {
        case 0:
            cardv_send_cmd_HP(CMD_DMS_DEINIT, NULL, 0);
            break;
        case 1:
            cardv_send_cmd_HP(CMD_DMS_INIT, NULL, 0);
            break;
        default:
            break;
        }
    }
}
#endif

static void cardv_cmd_handler_setMuxerDebug(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        g_muxerDbgLog = atoi(argv[1]);
    }
}

static void cardv_cmd_handler_sourceChange(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    // #if (CARDV_DISPLAY_ENABLE)
    // BOOL bStopPreview = FALSE;
    // #endif
    MuxerStatus_e eMuxerStatus = g_eMuxerStatus;

    if (eMuxerStatus != MUXER_STATUS_IDLE)
    {
        cardv_process_record_stop(FALSE);
        if (eMuxerStatus == MUXER_STATUS_RECORD)
        {
            cardv_process_record_start();
        }
        else if (eMuxerStatus == MUXER_STATUS_PRE_RECORD)
        {
            cardv_process_pre_record_start();
        }
    }

    // #if (CARDV_DISPLAY_ENABLE)
    // if (!g_camExisted[0]) {
    //     if (eDispMode == Disp_Prm_Cam ||
    //         eDispMode == Disp_Prm_Snd_Cam ||
    //         eDispMode == Disp_Prm_Trd_Cam) {
    //         cardv_send_cmd_HP(CMD_DISP_PREVIEW_STOP, NULL, 0);
    //         bStopPreview = TRUE;
    //     }
    // }
    // if (!g_camExisted[1]) {
    //     if (eDispMode == Disp_Snd_Cam ||
    //         eDispMode == Disp_Prm_Snd_Cam ||
    //         eDispMode == Disp_Snd_Trd_Cam) {
    //         cardv_send_cmd_HP(CMD_DISP_PREVIEW_STOP, NULL, 0);
    //         bStopPreview = TRUE;
    //     }
    // }
    // if (!g_camExisted[2]) {
    //     if (eDispMode == Disp_Trd_Cam ||
    //         eDispMode == Disp_Prm_Trd_Cam ||
    //         eDispMode == Disp_Snd_Trd_Cam) {
    //         cardv_send_cmd_HP(CMD_DISP_PREVIEW_STOP, NULL, 0);
    //         bStopPreview = TRUE;
    //     }
    // }
    // if (bStopPreview) {
    //     MI_S8 param[1];
    //     param[0] = 0;
    //     cardv_send_cmd_HP(CMD_DISP_PREVIEW_START, &param[0], sizeof(param));
    // }
    // #endif
}

#if (CARDV_ADAS_ENABLE)
static void cardv_cmd_handler_setADAS(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    // echo adas ldws 1/0 > /tmp/cardv_fifo
    // echo adas fcws 1/0 > /tmp/cardv_fifo
    if (argc == 3)
    {
        ADAS_ATTR_t old_attr;

        memcpy(&old_attr, &gFrontAdasAttr, sizeof(ADAS_ATTR_t));

        if (strcmp(argv[1], "ldws") == 0)
        {
            if (atoi(argv[2]) > 0)
                gFrontAdasAttr.feature.ldws = 1;
            else
                gFrontAdasAttr.feature.ldws = 0;
        }
        else if (strcmp(argv[1], "fcws") == 0)
        {
            if (atoi(argv[2]) > 0)
                gFrontAdasAttr.feature.fcws = 1;
            else
                gFrontAdasAttr.feature.fcws = 0;
        }
        else if (strcmp(argv[1], "sag") == 0)
        {
            if (atoi(argv[2]) > 0)
                gFrontAdasAttr.feature.sag = 1;
            else
                gFrontAdasAttr.feature.sag = 0;
        }
        else
        {
            return;
        }

        if (0 != memcmp(&old_attr, &gFrontAdasAttr, sizeof(ADAS_ATTR_t)))
        {
            if (ADAS_IsAnyFeatureEnable(&gFrontAdasAttr) == TRUE)
            {
                cardv_send_cmd_HP(CMD_ADAS_DEINIT, NULL, 0);
                cardv_send_cmd_HP(CMD_ADAS_INIT, (MI_S8 *)&gFrontAdasAttr, sizeof(ADAS_ATTR_t));
            }
            else
            {
                // all features disable.
                cardv_send_cmd_HP(CMD_ADAS_DEINIT, NULL, 0);
            }
        }
    }
}

static void cardv_cmd_handler_setADAS_Rear(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    // echo adas ldws 1/0 > /tmp/cardv_fifo
    // echo adas fcws 1/0 > /tmp/cardv_fifo
    if (argc == 3)
    {
        ADAS_ATTR_t old_attr;

        memcpy(&old_attr, &gRearAdasAttr, sizeof(ADAS_ATTR_t));

        if (strcmp(argv[1], "ldws") == 0)
        {
            if (atoi(argv[2]) > 0)
                gRearAdasAttr.feature.ldws = 1;
            else
                gRearAdasAttr.feature.ldws = 0;
        }
        else if (strcmp(argv[1], "bcws") == 0)
        {
            if (atoi(argv[2]) > 0)
                gRearAdasAttr.feature.bcws = 1;
            else
                gRearAdasAttr.feature.bcws = 0;
        }
        else if (strcmp(argv[1], "bsd") == 0)
        {
            if (atoi(argv[2]) > 0)
                gRearAdasAttr.feature.bsd = 1;
            else
                gRearAdasAttr.feature.bsd = 0;
        }
        else
        {
            return;
        }

        if (0 != memcmp(&old_attr, &gRearAdasAttr, sizeof(ADAS_ATTR_t)))
        {
            if (gRearAdasAttr.feature.ldws || gRearAdasAttr.feature.bcws || gRearAdasAttr.feature.bsd)
            {
                cardv_send_cmd_HP(CMD_ADAS_REAR_DEINIT, NULL, 0);
                cardv_send_cmd_HP(CMD_ADAS_REAR_INIT, (MI_S8 *)&gRearAdasAttr, sizeof(gRearAdasAttr));
            }
            else
            {
                cardv_send_cmd_HP(CMD_ADAS_REAR_DEINIT, NULL, 0);
            }
        }
    }
}
#endif

#if (CARDV_LD_ENABLE)
static void cardv_cmd_handler_setLD(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_U32 param[5];
    // echo LD moduleId dev chnl port debug > /tmp/cardv_fifo
    if (argc == 6)
    {
        MI_U32 moduleId = atoi(argv[1]);
        MI_U32 dev      = atoi(argv[2]);
        MI_U32 chnl     = atoi(argv[3]);
        MI_U32 port     = atoi(argv[4]);
        MI_U32 debug    = atoi(argv[5]);
        param[0]        = moduleId;
        param[1]        = dev;
        param[2]        = chnl;
        param[3]        = port;
        param[4]        = debug;

        cardv_send_cmd_HP(CMD_LD_SET_PARAM, (MI_S8 *)param, sizeof(param));
    }
    if (argc == 2)
    {
        // echo LD onoff > /tmp/cardv_fifo
        MI_U32 onoff = atoi(argv[1]);
        if (onoff)
            cardv_send_cmd_HP(CMD_LD_INIT, NULL, 0);
        else
            cardv_send_cmd_HP(CMD_LD_DEINIT, NULL, 0);
    }
}
#endif
#if (VIDEO_ENABLE_USER)
static void cardv_cmd_handler_userStream(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S8 param[1];

    if (argc == u8MaxPara)
    {
        param[0] = atoi(argv[1]);
        if (param[0])
        {
            for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
            {
                if (gstVencAttr[i].bUsed == FALSE)
                    continue;
                if (gstVencAttr[i].eVideoType == VIDEO_STREAM_USER)
                {
                    param[0] = i;
                    cardv_send_cmd_HP(CMD_VIDEO_START, (MI_S8 *)param, sizeof(param));
                }
            }
        }
        else
        {
            for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
            {
                if (gstVencAttr[i].bUsed == FALSE)
                    continue;
                if (gstVencAttr[i].eVideoType == VIDEO_STREAM_USER)
                {
                    param[0] = i;
                    cardv_send_cmd_HP(CMD_VIDEO_STOP, (MI_S8 *)param, sizeof(param));
                }
            }
        }
    }
}
#endif

#if (CARDV_ALINK_ENABLE)
static void cardv_AlinkStream_Callback(bool bRec)
{
    MI_S8 param[1];
    printf("==========  alink callback : %d  =========== \n", bRec);
    if (bRec)
    {
        for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
        {
            if (gstVencAttr[i].bUsed == FALSE)
                continue;
            if (gstVencAttr[i].eVideoType == VIDEO_STREAM_RTSP)
            {
                param[0] = i;
                cardv_send_cmd_HP(CMD_VIDEO_START, (MI_S8 *)param, sizeof(param));
            }
        }
    }
    else
    {
        for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
        {
            if (gstVencAttr[i].bUsed == FALSE)
                continue;
            if (gstVencAttr[i].eVideoType == VIDEO_STREAM_RTSP)
            {
                param[0] = i;
                cardv_send_cmd_HP(CMD_VIDEO_STOP, (MI_S8 *)param, sizeof(param));
            }
        }
    }
}

static void cardv_cmd_handler_alinkStream(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S8       param[1];
    static BOOL bStart = FALSE;
    if (argc == u8MaxPara)
    {
        param[0] = atoi(argv[1]);
        if (param[0] == 1 && !bStart)
        {
            // init alink
            std::string   json = "{}";
            std::ifstream configfs((char *)g_alinkpemPath);
            if (!configfs.good())
            {
                // std::cout << "Failed to open config file: " << std::endl;
                printf("Failed to open config file: %s \n", g_alinkpemPath);
                return;
            }
            std::stringstream buffer;
            buffer << configfs.rdbuf();
            json = buffer.str();
            buffer.str("");
            configfs.close();

            static auto &server = alink::ALinkServerNative::getInstance(json.c_str(), json.size());
            AlinkSever.mServer  = &server;

            // start alink
            AlinkSever.start();
            AlinkSever.g_AlinkStream_Callback = cardv_AlinkStream_Callback;
            bStart                            = TRUE;
        }
        else if (param[0] == 0 && bStart)
        {
            bStart = FALSE;
            // stop alink
            AlinkSever.stop();
            alink::ALinkServerNative::releaseInstance(*AlinkSever.mServer);
            // google::protobuf::ShutdownProtobufLibrary();
        }
    }
}
#endif

static void cardv_cmd_handler_renameStorage(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argv[1])
    {
        StorageSetVolume(argv[1]);
    }
}

static void cardv_cmd_handler_formatStorage(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    int ret;

#if (CARDV_EXFAT_ENABLE)
    if (argc > 1)
    {
        ret = StorageFormat(FS_TYPE_EXFAT);
    }
    else
#endif
    {
        ret = StorageFormat(FS_TYPE_FAT32);
    }

    if (g_bAudioOut)
    {
        MI_S8 audioparam[64];
        audioparam[0] = 1;
        if (ret != 0)
        {
            memcpy(&audioparam[1], WAV_PATH_CARD_FORMAT_F, sizeof(WAV_PATH_CARD_FORMAT_F));
            cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_CARD_FORMAT_F) + 1);
        }
        else
        {
            memcpy(&audioparam[1], WAV_PATH_CARD_FORMAT_D, sizeof(WAV_PATH_CARD_FORMAT_D));
            cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_CARD_FORMAT_D) + 1);
        }
    }

    if (ret == 0)
    {
        IPC_CarInfo_Read_UIModeInfo(&carInfo.stUIModeInfo);
        if (carInfo.stUIModeInfo.u8Mode == UI_VIDEO_MODE)
            cardv_process_record_start();
    }
}

static void cardv_cmd_handler_setBrightness(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_BRIGHTNESS, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setContrast(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_CONTRAST, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setSaturation(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_SATURATION, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setSharpness(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_SHARPENESS, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setAWB(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_WB_MODE, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setScene(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_SCENE, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setEvMode(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_EXPOSURE_MODE, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setFlicker(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_FLICKER, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setShutter(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_SHUTTER, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setISO(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_ISO, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setGamma(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_GAMMA, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setEffect(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_EFFECT, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setWinWgtType(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_WINWGT_TYPE, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_setNightMode(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        // = 0: off
        // > 0: on
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_ISP_SET_EXPOSURE_LIMIT, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

#if (CARDV_HDMI_ENABLE)
static void cardv_cmd_handler_HdmiTimingSwitch(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_S8 szHdmiTiming[64] = {
            0,
        };
        memcpy(szHdmiTiming, argv[1], sizeof(szHdmiTiming));
        cardv_send_cmd(CMD_DISP_SET_HDMI_TIMING, (MI_S8 *)&szHdmiTiming, sizeof(szHdmiTiming));
    }
}
#endif

#if (CARDV_GPS_ENABLE)
static void cardv_cmd_handler_setGps(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd(CMD_SYSTEM_VENDOR_GPS, (MI_S8 *)&nValue, sizeof(nValue));
    }
}
#endif

#if (CARDV_EDOG_ENABLE)
static void cardv_cmd_handler_setEdog(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc <= u8MaxPara)
    {
        MI_S32 param[2];
        param[0] = atoi(argv[1]);
        if (param[0] == 2 || param[0] == 6 || param[0] == 8)
        {
            param[1] = atoi(argv[2]);
        }
        cardv_send_cmd(CMD_SYSTEM_VENDOR_EDOG_CTRL, (MI_S8 *)param, sizeof(param));
    }
}
#endif

static void cardv_cmd_handler_demuxThumbnail(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_S8 filename[64] = {
            0,
        };
        memcpy(filename, argv[1], sizeof(filename));
        cardv_send_cmd(CMD_SYSTEM_DEMUX_THUMB, (MI_S8 *)&filename, sizeof(filename));
    }
}

static void cardv_cmd_handler_demuxTotalTime(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_S8 filename[64] = {
            0,
        };
        memcpy(filename, argv[1], sizeof(filename));
        cardv_send_cmd(CMD_SYSTEM_DEMUX_TOTALTIME, (MI_S8 *)&filename, sizeof(filename));
    }
}

static void cardv_cmd_handler_removeFile(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        printf("remove %s\n", argv[1]);
        DCF_DeleteFileByFileName(argv[1]);
    }
}

static void cardv_cmd_handler_Dcf(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        if (strcmp(argv[1], "sync") == 0)
        {
            // printf("cardv remount DCF\n");
            DCF_UnMount();
            DCF_Mount();
        }
    }
    else
    {
        DCF_DumpInfo();
    }
}

static void cardv_cmd_handler_MenuTimeZone(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        char strTZ[12] = {0};
        memcpy(strTZ, argv[1], sizeof(strTZ));
        setenv("TZ", strTZ, 1);
        tzset();
    }
}

static void cardv_cmd_handler_MenuVideoQuality(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    printf("cardv_cmd_handler_MenuVideoQuality is TBD!\r\n");
}

static void cardv_cmd_handler_MenuMicSensitivity(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    printf("cardv_cmd_handler_MenuMicSensitivity is TBD!\r\n");
}

static void cardv_cmd_handler_MenuVideoPreRecord(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    printf("cardv_cmd_handler_MenuVideoPreRecord is TBD!\r\n");
}

static void cardv_cmd_handler_MenuVideoVMDRecordTime(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    printf("cardv_cmd_handler_MenuVideoVMDRecordTime is TBD!\r\n");
}

static void cardv_cmd_handler_MenuVideoAutoRecord(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    printf("cardv_cmd_handler_MenuVideoAutoRecord is TBD!\r\n");
}

static void cardv_cmd_handler_MenuVideoHDRMode(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        BOOL bHdr = atoi(argv[1]);
        cardv_process_set_HDR_Mode(bHdr);
    }
}

static void cardv_cmd_handler_MenuVideoWNRMode(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    printf("cardv_cmd_handler_MenuVideoWNRMode is TBD!\r\n");
}

static void cardv_cmd_handler_MenuCaptureBurstShot(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    printf("cardv_cmd_handler_MenuCaptureBurstShot is TBD!\r\n");
}

static void cardv_cmd_handler_SensorResIdx(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_U8 u8SnrPad = atoi(argv[1]);
        MI_U8 u8SnrIdx = atoi(argv[2]);
        cardv_process_set_sensor_res_idx(u8SnrPad, u8SnrIdx);
    }
}

static void cardv_cmd_handler_SclRotate(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_SCL_ChnParam_t stSclChnParam;

        MI_U32 dev         = atoi(argv[1]);
        MI_U32 chn         = atoi(argv[2]);
        stSclChnParam.eRot = (MI_SYS_Rotate_e)atoi(argv[3]);
        MI_SCL_StopChannel(dev, chn);
        MI_SCL_SetChnParam(dev, chn, &stSclChnParam);
        MI_SCL_StartChannel(dev, chn);
    }
}

static void cardv_cmd_handler_SetAudioInAttr(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_AUDIO_Format_e     Format;
    MI_AUDIO_SoundMode_e  Soundmode;
    MI_AUDIO_SampleRate_e Samplerate;

    cardv_send_cmd_HP(CMD_AUDIO_IN_CLOSE, NULL, 0);
    if (argc == u8MaxPara)
    {
        switch (atoi(argv[1]))
        {
        case 0:
            Format = E_MI_AUDIO_FORMAT_PCM_S16_LE;
            printf("Ai Media Format (16bit)\n");
            break;
        default:
            Format = E_MI_AUDIO_FORMAT_PCM_S16_LE;
            printf("Ai Media Format (set error, and reset it to 16bit)\n");
            break;
        }

        if (g_audioInParam[0].eAudioInEncType == AUDIO_IN_AAC)
        {
            Format = E_MI_AUDIO_FORMAT_PCM_S16_LE;
            printf("AAC only support AV_SAMPLE_FMT_S16 now\n");
        }

        switch (atoi(argv[2]))
        {
        case 0:
            Samplerate = E_MI_AUDIO_SAMPLE_RATE_8000;
            printf("Ai Media Samplerate (16000)\n");
            break;
        case 1:
            Samplerate = E_MI_AUDIO_SAMPLE_RATE_11025;
            printf("Ai Media Samplerate (11025)\n");
            break;
        case 2:
            Samplerate = E_MI_AUDIO_SAMPLE_RATE_12000;
            printf("Ai Media Samplerate (12000)\n");
            break;
        case 3:
            Samplerate = E_MI_AUDIO_SAMPLE_RATE_16000;
            printf("Ai Media Samplerate (16000)\n");
            break;
        case 4:
            Samplerate = E_MI_AUDIO_SAMPLE_RATE_22050;
            printf("Ai Media Samplerate (22050)\n");
            break;
        case 5:
            Samplerate = E_MI_AUDIO_SAMPLE_RATE_24000;
            printf("Ai Media Samplerate (24000)\n");
            break;
        case 6:
            Samplerate = E_MI_AUDIO_SAMPLE_RATE_32000;
            printf("Ai Media Samplerate (32000)\n");
            break;
        case 7:
            Samplerate = E_MI_AUDIO_SAMPLE_RATE_44100;
            printf("Ai Media Samplerate (44100)\n");
            break;
        case 8:
            Samplerate = E_MI_AUDIO_SAMPLE_RATE_48000;
            printf("Ai Media Samplerate (48000)\n");
            break;
        case 9:
            Samplerate = E_MI_AUDIO_SAMPLE_RATE_96000;
            printf("Ai Media Samplerate (96000)\n");
            break;
        default:
            Samplerate = E_MI_AUDIO_SAMPLE_RATE_16000;
            printf("Ai Media Samplerate (set error, and reset it to 16bit)\n");
            break;
        }

        switch (atoi(argv[3]))
        {
        case 0:
            Soundmode = E_MI_AUDIO_SOUND_MODE_MONO;
            printf("Ai Media Soundmode (MONO)\n");
            break;
        case 1:
            Soundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
            printf("Ai Media Soundmode (STERO)\n");
            break;
        default:
            Soundmode = E_MI_AUDIO_SOUND_MODE_MONO;
            printf("Ai Media Soundmode (Set wrong Soundmode,and reset it to MONO)\n");
            break;
        }

        g_audioInParam[0].stAudioAttr.enFormat     = Format;
        g_audioInParam[0].stAudioAttr.enSampleRate = Samplerate;
        g_audioInParam[0].stAudioAttr.enSoundMode  = Soundmode;

        cardv_send_cmd_HP(CMD_AUDIO_IN_OPEN, NULL, 0);
    }
}

static void cardv_cmd_handler_AnadecRes(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_S32 param[5];
        param[0] = atoi(argv[1]); // cam ID
        param[1] = atoi(argv[2]); // width
        param[2] = atoi(argv[3]); // height
        param[3] = atoi(argv[4]); // fps
        param[4] = atoi(argv[5]); // AHD or TVI, refs MI_SNR_Anadec_TransferMode_e
        cardv_send_cmd(CMD_PIPE_SWITCH_ANADEC_RES, (MI_S8 *)param, sizeof(param));
    }
}

static void cardv_cmd_handler_DumpCardvImpl(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    printf("**** %s begin****\n", __FUNCTION__);
    printf("carInfo.stRecInfo:\n");
    printf("\t stRecInfo.bMuxing:[%d,%d,%d]\n", carInfo.stRecInfo.u32CurDuration[0],
           carInfo.stRecInfo.u32CurDuration[1], carInfo.stRecInfo.u32CurDuration[2]);
    printf("\t stRecInfo.bAudioTrack:%d\n", carInfo.stRecInfo.bAudioTrack);
    printf("\t stRecInfo.bMuxing:%d\n", carInfo.stRecInfo.bMuxing);
    printf("\t stRecInfo.bMuxingEmerg:%d\n", carInfo.stRecInfo.bMuxingEmerg);
    printf("\t stRecInfo.bMuxingShare:%d\n", carInfo.stRecInfo.bMuxingShare);

    printf("carInfo.stSdInfo:\n");
    printf("\t stSdInfo.s32ErrCode:%d\n", carInfo.stSdInfo.s32ErrCode);
    printf("\t stSdInfo.u64FreeSize:%lld\n", carInfo.stSdInfo.u64FreeSize);
    printf("\t stSdInfo.u64TotalSize:%lld\n", carInfo.stSdInfo.u64TotalSize);
    printf("\t stSdInfo.u64OtherSpace:%lld\n", carInfo.stSdInfo.u64OtherSpace);
    printf("\t stSdInfo.u64ReservedSpace:%lld\n", carInfo.stSdInfo.u64ReservedSpace);
    printf("\t stSdInfo.u32TotalLifeTime:%d\n", carInfo.stSdInfo.u32TotalLifeTime);
    printf("\t stSdInfo.bStorageMount:%d\n", carInfo.stSdInfo.bStorageMount);
    printf("\t stSdInfo.bStorageInsert:%d\n", carInfo.stSdInfo.bStorageInsert);

    printf("**** %s end*******\n", __FUNCTION__);
}

static void cardv_cmd_handler_exit(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    bMessageLoop = FALSE;
}

static void cardv_cmd_handler_help(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    cardv_process_display_help();
}

static void cardv_cmd_handler_setdatelogoStamp(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd_HP(CMD_OSD_SET_STAMP, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_settimeformat(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        int nValue = atoi(argv[1]);
        cardv_send_cmd_HP(CMD_OSD_SET_TIMEFORMAT, (MI_S8 *)&nValue, sizeof(nValue));
    }
}

static void cardv_cmd_handler_suspend(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    cardvSuspend();
    printf("cardv suspend\n");
    system("echo mem > /sys/power/state");
    printf("cardv resume\n");
    cardvResume(FALSE);
}

static void cardv_cmd_handler_reset_pipe_line(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        cardvSuspend();
        cardv_parserIni(argv[1]);
        cardvResume(FALSE);
    }
}

static void cardv_cmd_handler_setLcdBrightness(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_S32 Luma = atoi(argv[1]);
        cardv_send_cmd_HP(CMD_DISP_SET_LCD_LUMA, (MI_S8 *)&Luma, sizeof(Luma));
    }
}

static void cardv_cmd_handler_setLcdContrast(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_S32 Contrast = atoi(argv[1]);
        cardv_send_cmd_HP(CMD_DISP_SET_LCD_CONTRAST, (MI_S8 *)&Contrast, sizeof(Contrast));
    }
}

static void cardv_cmd_handler_setLcdHue(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_S32 Hue = atoi(argv[1]);
        cardv_send_cmd_HP(CMD_DISP_SET_LCD_HUE, (MI_S8 *)&Hue, sizeof(Hue));
    }
}

static void cardv_cmd_handler_setLcdSaturation(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (argc == u8MaxPara)
    {
        MI_S32 Saturation = atoi(argv[1]);
        cardv_send_cmd_HP(CMD_DISP_SET_LCD_SATURATION, (MI_S8 *)&Saturation, sizeof(Saturation));
    }
}

static void cardv_cmd_handler_MenuRecordVolme(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    MI_S32 AiVolume = atoi(argv[1]);
    cardv_send_cmd_HP(CMD_AUDIO_IN_SET_VOLUME, (MI_S8 *)&AiVolume, sizeof(AiVolume));
}

#if (CARDV_LDC_ENABLE)
static void cardv_cmd_handler_Eis(MI_U8 u8MaxPara, MI_U8 argc, char **argv)
{
    if (strcmp(argv[1], "on") == 0)
    {
        cardv_send_cmd_HP(CMD_PIPE_LDC_ENABLE, NULL, 0);
    }
    else if (strcmp(argv[1], "off") == 0)
    {
        cardv_send_cmd_HP(CMD_PIPE_LDC_DISABLE, NULL, 0);
    }
}
#endif

void cardvInputMessageProcess(char *cmd, MI_U32 len, bool bHighPriority)
{
    MI_U8  u8Argc;
    char * pu8Argv[MAX_ARGC];
    char * pNext;
    char * pStart       = cmd;
    MI_S32 s32ExeMaxCnt = 2;
    MI_S32 s32Ret       = 0;

L_NEXT_CMD:
    s32ExeMaxCnt--;
    u8Argc = CarDVParseStringsStart(pStart, pu8Argv, &pNext);
    if (u8Argc)
    {
        s32Ret = CarDVExecCmdHandler(pu8Argv, u8Argc);
        if (s32Ret < 0)
        {
            printf("\n====\n");
            for (int i = 0; i < u8Argc; i++)
            {
                printf("%s ", pu8Argv[i]);
            }
            printf("\n====\n");
            cardv_process_display_help();
        }
        CarDVParseStringsEnd(u8Argc, pu8Argv);

        if (cmd + len > pNext)
        {
            if (bHighPriority || (!bHighPriority && s32ExeMaxCnt > 0))
            {
                pStart = pNext;
                goto L_NEXT_CMD;
            }
        }
    }
}

MI_S32 cardvInputMessageLoop()
{
    fd_set read_fds;
    char   fifo_data[FIFO_DATA_SIZE] = {0};
    int    ret                       = 0;
    int    fd_out_r = -1, fd_internal_r = -1, maxfds = 0;

    CarDVInitCmdHandler();
    /*                         cmd name    max para num    function */
    CarDVAddCmdHandler((char *)"seamless", 1, cardv_cmd_handler_seamless);
    CarDVAddCmdHandler((char *)"rec", 2, cardv_cmd_handler_record);
    CarDVAddCmdHandler((char *)"capture", 2, cardv_cmd_handler_capture);
    CarDVAddCmdHandler((char *)"capraw", 2, cardv_cmd_handler_capture_raw);
    CarDVAddCmdHandler((char *)"lock", 2, cardv_cmd_handler_setLockFile);
    CarDVAddCmdHandler((char *)"res", 4, cardv_cmd_handler_setResolution);
    CarDVAddCmdHandler((char *)"vidres", 3, cardv_cmd_handler_setVideoResolution);
    CarDVAddCmdHandler((char *)"capres", 3, cardv_cmd_handler_setCaptureResolution);
    CarDVAddCmdHandler((char *)"loop", 2, cardv_cmd_handler_setRecLimitTime);
    CarDVAddCmdHandler((char *)"timelapse", 2, cardv_cmd_handler_setTimeLapse);
    CarDVAddCmdHandler((char *)"slowmotion", 2, cardv_cmd_handler_setSlowMotion);
    CarDVAddCmdHandler((char *)"rm", 2, cardv_cmd_handler_removeFile);
    CarDVAddCmdHandler((char *)"format", 2, cardv_cmd_handler_formatStorage);
    CarDVAddCmdHandler((char *)"sdrename", 2, cardv_cmd_handler_renameStorage);
    CarDVAddCmdHandler((char *)"thumb", 2, cardv_cmd_handler_demuxThumbnail);
    CarDVAddCmdHandler((char *)"tt", 2, cardv_cmd_handler_demuxTotalTime);
    CarDVAddCmdHandler((char *)"bri", 2, cardv_cmd_handler_setBrightness);
    CarDVAddCmdHandler((char *)"con", 2, cardv_cmd_handler_setContrast);
    CarDVAddCmdHandler((char *)"sat", 2, cardv_cmd_handler_setSaturation);
    CarDVAddCmdHandler((char *)"sha", 2, cardv_cmd_handler_setSharpness);
    CarDVAddCmdHandler((char *)"wb", 2, cardv_cmd_handler_setAWB);
    CarDVAddCmdHandler((char *)"scene", 2, cardv_cmd_handler_setScene);
    CarDVAddCmdHandler((char *)"ev", 2, cardv_cmd_handler_setEvMode);
    CarDVAddCmdHandler((char *)"flicker", 2, cardv_cmd_handler_setFlicker);
    CarDVAddCmdHandler((char *)"shutter", 2, cardv_cmd_handler_setShutter);
    CarDVAddCmdHandler((char *)"iso", 2, cardv_cmd_handler_setISO);
    CarDVAddCmdHandler((char *)"gamma", 2, cardv_cmd_handler_setGamma);
    CarDVAddCmdHandler((char *)"effect", 2, cardv_cmd_handler_setEffect);
    CarDVAddCmdHandler((char *)"winweight", 2, cardv_cmd_handler_setWinWgtType);
    CarDVAddCmdHandler((char *)"night", 2, cardv_cmd_handler_setNightMode);
    CarDVAddCmdHandler((char *)"audioplay", 4, cardv_cmd_handler_audioPlay);
    CarDVAddCmdHandler((char *)"aplaystop", 3, cardv_cmd_handler_audioStop);
    CarDVAddCmdHandler((char *)"audiorec", 2, cardv_cmd_handler_audioRecord);
    CarDVAddCmdHandler((char *)"pbvolume", 2, cardv_cmd_handler_setAudioPlayVol);
    CarDVAddCmdHandler((char *)"park", 2, cardv_cmd_handler_ParkingMonitor);
    CarDVAddCmdHandler((char *)"gsensor", 2, cardv_cmd_handler_GsensorSensitivity);
    CarDVAddCmdHandler((char *)"zoom", 4, cardv_cmd_handler_zoom);
    CarDVAddCmdHandler((char *)"video", 3, cardv_cmd_handler_videoFlow);
#if (CARDV_VDF_ENABLE)
    CarDVAddCmdHandler((char *)"mdt", 2, cardv_cmd_handler_MotionSensitivity);
#endif
    CarDVAddCmdHandler((char *)"voice", 2, cardv_cmd_handler_VoiceSwitch);
    CarDVAddCmdHandler((char *)"flip", 2, cardv_cmd_handler_SensorFlip);
    CarDVAddCmdHandler((char *)"bitrate", 3, cardv_cmd_handler_setBitrate);
    CarDVAddCmdHandler((char *)"codec", 3, cardv_cmd_handler_setCodec);
#if (CARDV_HDMI_ENABLE)
    CarDVAddCmdHandler((char *)"hdmitiming", 2, cardv_cmd_handler_HdmiTimingSwitch);
#endif

#if (CARDV_GPS_ENABLE)
    CarDVAddCmdHandler((char *)"gps", 2, cardv_cmd_handler_setGps);
#endif
#if (CARDV_EDOG_ENABLE)
    CarDVAddCmdHandler((char *)"edog", 3, cardv_cmd_handler_setEdog);
#endif
#if (CARDV_SPEECH_ENABLE)
    CarDVAddCmdHandler((char *)"speech", 2, cardv_cmd_handler_setSpeech);
#endif
#if (CARDV_DMS_ENABLE)
    CarDVAddCmdHandler((char *)"dms", 2, cardv_cmd_handler_setDMS);
#endif
#if (CARDV_ADAS_ENABLE)
    CarDVAddCmdHandler((char *)"adas", 3, cardv_cmd_handler_setADAS);
    CarDVAddCmdHandler((char *)"radas", 3, cardv_cmd_handler_setADAS_Rear);
#endif
#if (CARDV_LD_ENABLE)
    CarDVAddCmdHandler((char *)"LD", 4, cardv_cmd_handler_setLD);
#endif
#if (CARDV_LIVE555_ENABLE)
    CarDVAddCmdHandler((char *)"rtsp", 2, cardv_cmd_handler_RtspSwitch);
#endif
#if (CARDV_DISPLAY_ENABLE)
    CarDVAddCmdHandler((char *)"disp", 2, cardv_cmd_handler_display);
#endif
    CarDVAddCmdHandler((char *)"muxdebug", 2, cardv_cmd_handler_setMuxerDebug);
    CarDVAddCmdHandler((char *)"dcf", 2, cardv_cmd_handler_Dcf);
    CarDVAddCmdHandler((char *)"srcchg", 3, cardv_cmd_handler_sourceChange);
#if (VIDEO_ENABLE_USER)
    CarDVAddCmdHandler((char *)"usrstream", 2, cardv_cmd_handler_userStream);
#endif
#if (CARDV_ALINK_ENABLE)
    CarDVAddCmdHandler((char *)"alink", 2, cardv_cmd_handler_alinkStream);
#endif
    CarDVAddCmdHandler((char *)"timezone", 2, cardv_cmd_handler_MenuTimeZone);
    CarDVAddCmdHandler((char *)"quality", 2, cardv_cmd_handler_MenuVideoQuality);
    CarDVAddCmdHandler((char *)"micsen", 2, cardv_cmd_handler_MenuMicSensitivity);
    CarDVAddCmdHandler((char *)"prerec", 2, cardv_cmd_handler_MenuVideoPreRecord);
    CarDVAddCmdHandler((char *)"vmdrectime", 2, cardv_cmd_handler_MenuVideoVMDRecordTime);
    CarDVAddCmdHandler((char *)"autorec", 2, cardv_cmd_handler_MenuVideoAutoRecord);
    CarDVAddCmdHandler((char *)"hdr", 2, cardv_cmd_handler_MenuVideoHDRMode);
    CarDVAddCmdHandler((char *)"wnr", 2, cardv_cmd_handler_MenuVideoWNRMode);
    CarDVAddCmdHandler((char *)"snr", 3, cardv_cmd_handler_SensorResIdx);
    CarDVAddCmdHandler((char *)"anadec", 6, cardv_cmd_handler_AnadecRes);
    CarDVAddCmdHandler((char *)"burstshot", 2, cardv_cmd_handler_MenuCaptureBurstShot);
    CarDVAddCmdHandler((char *)"dumpimpl", 2, cardv_cmd_handler_DumpCardvImpl);
    CarDVAddCmdHandler((char *)"quit", 1, cardv_cmd_handler_exit);
    CarDVAddCmdHandler((char *)"help", 1, cardv_cmd_handler_help);
    CarDVAddCmdHandler((char *)"datelogoStamp", 2, cardv_cmd_handler_setdatelogoStamp);
    CarDVAddCmdHandler((char *)"timeformat", 2, cardv_cmd_handler_settimeformat);
    CarDVAddCmdHandler((char *)"str", 1, cardv_cmd_handler_suspend);
    CarDVAddCmdHandler((char *)"reset", 2, cardv_cmd_handler_reset_pipe_line);
    CarDVAddCmdHandler((char *)"lcdbri", 2, cardv_cmd_handler_setLcdBrightness);
    CarDVAddCmdHandler((char *)"lcdcon", 2, cardv_cmd_handler_setLcdContrast);
    CarDVAddCmdHandler((char *)"lcdhue", 2, cardv_cmd_handler_setLcdHue);
    CarDVAddCmdHandler((char *)"lcdsat", 2, cardv_cmd_handler_setLcdSaturation);
    CarDVAddCmdHandler((char *)"recvol", 2, cardv_cmd_handler_MenuRecordVolme);
#if (CARDV_LDC_ENABLE)
    CarDVAddCmdHandler((char *)"eis", 2, cardv_cmd_handler_Eis);
#endif
    CarDVAddCmdHandler((char *)"sclrotate", 4, cardv_cmd_handler_SclRotate);
    CarDVAddCmdHandler((char *)"aiattr", 4, cardv_cmd_handler_SetAudioInAttr);

    printf("cardv Loop begin\n");

    if (-1 == (fd_out_r = open(FIFO_NAME, O_RDWR)))
    {
        printf("Process %d open fifo error\n", getpid());
        goto L_OPEN_ERR;
    }
    if (-1 == (fd_internal_r = open(FIFO_NAME_HIGH_PRIO, O_RDWR)))
    {
        printf("Process %d open fifo high error\n", getpid());
        close(fd_out_r);
        goto L_OPEN_ERR;
    }

    maxfds = MAX(fd_out_r, fd_internal_r);

    while (bMessageLoop)
    {
        FD_ZERO(&read_fds);
        FD_SET(fd_out_r, &read_fds);
        FD_SET(fd_internal_r, &read_fds);

        ret = select(maxfds + 1, &read_fds, NULL, NULL, NULL);
        if (ret > 0)
        {
            // first:FIFO_NAME_HIGH_PRIO
            if (FD_ISSET(fd_internal_r, &read_fds))
            {
                ret = read(fd_internal_r, fifo_data, FIFO_DATA_SIZE);
                if (ret > 0)
                {
                    cardvInputMessageProcess(fifo_data, ret, TRUE);
                    memset(fifo_data, 0x00, FIFO_DATA_SIZE);
                }
            }
            FD_CLR(fd_internal_r, &read_fds);

            // second:FIFO_NAME
            if (FD_ISSET(fd_out_r, &read_fds))
            {
                ret = read(fd_out_r, fifo_data, FIFO_DATA_SIZE);
                if (ret > 0)
                {
                    cardvInputMessageProcess(fifo_data, ret, FALSE);
                    memset(fifo_data, 0x00, FIFO_DATA_SIZE);
                }
            }
            FD_CLR(fd_out_r, &read_fds);
        }
        else if (ret < 0)
        {
            printf("FIFO select failed\n");
            goto L_SELECT_ERR;
        }
        else if (0 == ret)
        {
            printf("FIFO select timeout\n");
            goto L_SELECT_ERR;
        }
    }

L_SELECT_ERR:
    close(fd_out_r);
    close(fd_internal_r);
L_OPEN_ERR:
    CarDVRemoveCmdHandler();
    printf("cardv Loop exit\n");
    return 0;
}

void cardvSuspend(void)
{
    if (g_bAudioOut)
    {
        MI_S8 audioparam[64];
        audioparam[0] = 1;
        memcpy(&audioparam[1], WAV_PATH_POWER_OFF, sizeof(WAV_PATH_POWER_OFF));
        cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_POWER_OFF) + 1);
    }

#if (CARDV_GUI_ENABLE)
    IPC_MsgToUI_SendMsg(IPC_MSG_UI_STOP, NULL, 0);
#endif

    StorageStopDetectThread();

    // Need stop encoding before CMD_VIDEO_DEINIT_H26X & CMD_AUDIO_IN_CLOSE & CMD_MUX_CLOSE.
    cardv_process_record_stop(FALSE);

#if (CARDV_LD_ENABLE)
    cardv_send_cmd_HP(CMD_LD_DEINIT, NULL, 0);
#endif

#if (CARDV_ADAS_ENABLE)
    cardv_send_cmd_HP(CMD_ADAS_DEINIT, NULL, 0);
#endif

#if (CARDV_DISPLAY_ENABLE)
    cardv_send_cmd_HP(CMD_DISP_PREVIEW_STOP, NULL, 0);
#endif

#if (CARDV_IQSERVER_ENABLE)
    cardv_send_cmd_HP(CMD_SYSTEM_IQSERVER_CLOSE, NULL, 0);
#endif

    if (g_openUVCDevice)
    {
        cardv_send_cmd_HP(CMD_UVC_CLOSE, NULL, 0);
    }

#if (CARDV_VDF_ENABLE)
    cardv_process_set_MdtOnOff(FALSE);
#endif

    cardv_send_cmd_HP(CMD_AUDIO_OUT_CLOSE, NULL, 0);

#if (CARDV_LIVE555_ENABLE)
    if (g_bRtspEnable)
    {
        // live555 should close before video and audio close,
        // because live555 used video & audio global variable.
        cardv_send_cmd_HP(CMD_SYSTEM_LIVE555_CLOSE, NULL, 0);
    }
#endif

    if (g_displayOsd)
    {
        cardv_send_cmd_HP(CMD_OSD_CLOSE, NULL, 0);
    }

#if (CARDV_SPEECH_ENABLE)
    if (g_bSpeechEnable)
    {
        g_bSpeechEnable = FALSE;
        if (MI_SUCCESS == Speech_VoiceAnalyzeStop())
            printf("cardv speech disable\n");

        Speech_VoiceAnalyzeDeInit();
    }
#endif

    GsensorSetPowerOnByInt(); // CHECK : CLEAR INT STATUS BEFORE PWR_EN DOWN

#if (CARDV_DMS_ENABLE)
    cardv_send_cmd_HP(CMD_DMS_DEINIT, NULL, 0);
#endif

    cardv_send_cmd_HP(CMD_VIDEO_DEINIT_JPEG, NULL, 0);
    cardv_send_cmd_HP(CMD_VIDEO_DEINIT_H26X, NULL, 0);
    cardv_send_cmd_HP(CMD_AUDIO_IN_CLOSE, NULL, 0);
    cardv_send_cmd_HP(CMD_MUX_CLOSE, NULL, 0);

    cardv_send_cmd_HP(CMD_PIPE_UNINIT, NULL, 0);
    cardv_send_cmd_and_wait(CMD_SYSTEM_UNINIT, NULL, 0);
}

void cardvResume(MI_BOOL bPowerOn)
{
    // system init
    cardv_send_cmd_HP(CMD_SYSTEM_INIT, NULL, 0);
    cardv_send_cmd_HP(CMD_PIPE_INIT, NULL, 0);

    cardv_send_cmd_HP(CMD_MUX_OPEN, NULL, 0);
    cardv_send_cmd_HP(CMD_AUDIO_IN_OPEN, NULL, 0);
    cardv_send_cmd_HP(CMD_VIDEO_INIT_H26X, NULL, 0);
    cardv_send_cmd_HP(CMD_VIDEO_INIT_JPEG, NULL, 0);

    if (g_displayOsd)
        cardv_send_cmd_HP(CMD_OSD_OPEN, NULL, 0);

#ifdef DUAL_OS
#if (CARDV_DISPLAY_ENABLE)
    cardv_send_cmd_HP(CMD_DISP_PREVIEW_START, NULL, 0);
#endif
#endif

    if (g_u32TimeLapse)
    {
        printf("TimeLapse [1/%d] Fps\n", g_u32TimeLapse);
        cardv_process_set_VideoFps((g_u32TimeLapse & 0xffff) << 16 | 1);
        cardv_process_set_MuxerFps(MUXER_NORMAL, CARDV_DEFAULT_FPS, 1, TRUE, MUXER_SLOW_MOTION_NONE);
        cardv_process_set_MuxerFps(MUXER_EMERG, CARDV_DEFAULT_FPS, 1, TRUE, MUXER_SLOW_MOTION_NONE);
        cardv_process_set_MuxerLimitTime(MUXER_NORMAL, g_u32TimeLapse * CARDV_DEFAULT_FPS
                                                           * g_u64NormalRecTime); // unit: real time usec
        cardv_process_set_MuxerLimitTime(MUXER_EMERG, g_u32TimeLapse * CARDV_DEFAULT_FPS * g_u64ShareEmergRecTime);
        cardv_process_set_MuxerPreRecordTime(MUXER_NORMAL, g_u32TimeLapse * CARDV_DEFAULT_FPS * 5000000);
        cardv_process_set_MuxerPreRecordTime(MUXER_EMERG, g_u32TimeLapse * CARDV_DEFAULT_FPS * 5000000);
        g_bAudioIn = FALSE;
    }
    else if (g_u32SlowMotion == MUXER_SLOW_MOTION_X2 || g_u32SlowMotion == MUXER_SLOW_MOTION_X4
             || g_u32SlowMotion == MUXER_SLOW_MOTION_X8)
    {
        printf("SlowMotion X%d\n", g_u32SlowMotion);
        cardv_process_set_MuxerFps(MUXER_NORMAL, 0, 0, FALSE, (MuxerSlowMotionStatus)g_u32SlowMotion);
        cardv_process_set_MuxerFps(MUXER_EMERG, 0, 0, FALSE, (MuxerSlowMotionStatus)g_u32SlowMotion);
        cardv_process_set_MuxerLimitTime(MUXER_NORMAL, g_u64NormalRecTime / g_u32SlowMotion); // unit: real time usec
        cardv_process_set_MuxerLimitTime(MUXER_EMERG, g_u64ShareEmergRecTime / g_u32SlowMotion);
        g_bAudioIn = FALSE;
    }

    cardv_process_pre_record_start();

    if (bPowerOn)
    {
        while (g_bVideoStart == FALSE)
        {
            usleep(1000);
        }
        // Is recording now, run demo.sh continuse.
        int cardv_rec_fd = open("/tmp/cardv_rec", O_RDONLY | O_CREAT | O_EXCL, 0444);
        if (cardv_rec_fd >= 0)
        {
            close(cardv_rec_fd);
        }
    }

    cardv_send_cmd_HP(CMD_AUDIO_OUT_OPEN, NULL, 0);
    if (bPowerOn && g_bAudioOut)
    {
        MI_S8 param[64];
        param[0] = 1; // Stop Immediately
        memcpy(&param[1], WAV_PATH_POWER_ON, sizeof(WAV_PATH_POWER_ON));
        cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)param, sizeof(WAV_PATH_POWER_ON) + 1);
    }

#if (CARDV_VDF_ENABLE)
    cardv_process_set_MdtLevel(g_MdtLevel);
#endif

    if (g_openUVCDevice)
    {
        cardv_send_cmd_HP(CMD_UVC_INIT, NULL, 0);
    }

#if (CARDV_IQSERVER_ENABLE)
    cardv_send_cmd_HP(CMD_SYSTEM_IQSERVER_OPEN, NULL, 0);
#endif

#ifndef DUAL_OS
#if (CARDV_DISPLAY_ENABLE)
    cardv_send_cmd_HP(CMD_DISP_PREVIEW_START, NULL, 0);
#endif
#endif

    StorageStartDetectThread();

#if (CARDV_ADAS_ENABLE)
    if (ADAS_IsAnyFeatureEnable(&gFrontAdasAttr) == TRUE)
    {
        cardv_send_cmd_HP(CMD_ADAS_INIT, (MI_S8 *)&gFrontAdasAttr, sizeof(ADAS_ATTR_t));
    }
    if (ADAS_IsAnyFeatureEnable(&gRearAdasAttr) == TRUE)
    {
        cardv_send_cmd_HP(CMD_ADAS_REAR_INIT, (MI_S8 *)&gRearAdasAttr, sizeof(ADAS_ATTR_t));
    }
#endif

#if (CARDV_LD_ENABLE)
    cardv_send_cmd_HP(CMD_LD_INIT, NULL, 0);
#endif

#if (CARDV_GUI_ENABLE)
    if (!bPowerOn)
    {
        system((char *)g_UIInitCmd);
    }
#endif
}

void cardvExit()
{
    cardvSuspend();

    GsensorStopThread();
#if (CARDV_GPS_ENABLE)
    GpsStopThread();
#endif

#if (CARDV_WATCHDOG_ENABLE)
    cardv_send_cmd_and_wait(CMD_SYSTEM_WATCHDOG_CLOSE, NULL, 0);
#endif
    cardv_message_queue_uninit();
    IPC_CarInfo_Close();

#if (CARDV_RTSP_INPUT_ENABLE || CARDV_WS_INPUT_ENABLE)
    IpcServerStop();
#endif
}

void signalStop(MI_S32 signo)
{
    printf("\nsignalStop(signal code: %d) !!!\n", signo);
    cardvExit();
    printf("cardv app exit\n");
    _exit(0);
}

#if (CARDV_RTSP_INPUT_ENABLE)
void cardv_init_inputRtspParam(dictionary *pstDict, MI_U8 u8CamId)
{
    char                    key[64];
    char *                  string      = NULL;
    MI_U8                   u8RtspNum   = 0;
    MI_U8                   u8RtspId    = 0;
    MI_U8                   u8RtspChn   = 0;
    CarDV_RtspClientAttr_t *pstRtspAttr = NULL;

    sprintf(key, "Cam%d:RtspNum", u8CamId);
    u8RtspNum = iniparser_getint(pstDict, key, 0);
    for (u8RtspId = 0; u8RtspId < u8RtspNum; u8RtspId++)
    {
        sprintf(key, "Cam%d:Rtsp%dChn", u8CamId, u8RtspId);
        u8RtspChn = iniparser_getint(pstDict, key, -1);
        if (u8RtspChn >= RTSP_MAX_STREAM_CNT)
        {
            printf("ini key [%s] error, check chn setting\n", key);
            exit(-1);
        }

        pstRtspAttr = &g_stRtspAttr[u8RtspChn];
        if (pstRtspAttr->bUsed)
        {
            printf("rtsp chn%u is used, check chn setting\n", u8RtspChn);
            exit(-1);
        }

        pstRtspAttr->bUsed = TRUE;

        sprintf(key, "Cam%d:Rtsp%dUrl", u8CamId, u8RtspId);
        string = iniparser_getstring(pstDict, key, (char *)"NULL");
        strcpy(pstRtspAttr->cUrl, string);

        sprintf(key, "Cam%d:Rtsp%dEsBufSize", u8CamId, u8RtspId);
        pstRtspAttr->u32EsBufSize = iniparser_getint(pstDict, key, 2 * 1024 * 1024);

        FREEIF(pstRtspAttr->linkHead);
        pstRtspAttr->linkHead = (stream_Link_head *)MALLOC(sizeof(stream_Link_head));
        if (pstRtspAttr->linkHead == NULL)
        {
            printf("rtsp chn%u link error\n", u8RtspChn);
            exit(-1);
        }
        SLIST_INIT(pstRtspAttr->linkHead);

#if (CARDV_VDEC_ENABLE)
        sprintf(key, "Cam%d:Rtsp%dUseVdec", u8CamId, u8RtspId);
        pstRtspAttr->bUseVdec = iniparser_getint(pstDict, key, 0);
        if (pstRtspAttr->bUseVdec)
        {
            sprintf(key, "Cam%d:Rtsp%dVdecChn", u8CamId, u8RtspId);
            pstRtspAttr->u32VdecChn = iniparser_getint(pstDict, key, -1);
            if (pstRtspAttr->u32VdecChn == 0 || pstRtspAttr->u32VdecChn > CARDV_MAX_VDEC_CHN_NUM)
            {
                printf("rtsp chn%u vdec chn error\n", u8RtspChn);
                exit(-1);
            }
        }
#endif
    }
}
#endif

#if (CARDV_WS_INPUT_ENABLE)
void cardv_init_inputWsParam(dictionary *pstDict, MI_U8 u8CamId)
{
    char                  key[64];
    char *                string    = NULL;
    MI_U8                 u8WsNum   = 0;
    MI_U8                 u8WsId    = 0;
    MI_U8                 u8WsChn   = 0;
    CarDV_WsClientAttr_t *pstWsAttr = NULL;

    sprintf(key, "Cam%d:WsNum", u8CamId);
    u8WsNum = iniparser_getint(pstDict, key, 0);
    for (u8WsId = 0; u8WsId < u8WsNum; u8WsId++)
    {
        sprintf(key, "Cam%d:Ws%dChn", u8CamId, u8WsId);
        u8WsChn = iniparser_getint(pstDict, key, -1);
        if (u8WsChn >= WS_MAX_STREAM_CNT)
        {
            printf("ini key [%s] error, check chn setting\n", key);
            exit(-1);
        }

        pstWsAttr = &g_stWsAttr[u8WsChn];
        if (pstWsAttr->bUsed)
        {
            printf("Ws chn%u is used, check chn setting\n", u8WsChn);
            exit(-1);
        }

        pstWsAttr->bUsed = TRUE;

        sprintf(key, "Cam%d:Ws%dHost", u8CamId, u8WsId);
        string = iniparser_getstring(pstDict, key, (char *)"NULL");
        strcpy(pstWsAttr->cHost, string);

        sprintf(key, "Cam%d:Ws%dPath", u8CamId, u8WsId);
        string = iniparser_getstring(pstDict, key, (char *)"/video0");
        strcpy(pstWsAttr->cPath, string);

        sprintf(key, "Cam%d:Ws%dPort", u8CamId, u8WsId);
        pstWsAttr->u32Port = iniparser_getint(pstDict, key, BASE_LISTEN_PORT);

        sprintf(key, "Cam%d:Ws%dEsBufSize", u8CamId, u8WsId);
        pstWsAttr->u32EsBufSize = iniparser_getint(pstDict, key, 2 * 1024 * 1024);

        FREEIF(pstWsAttr->linkHead);
        pstWsAttr->linkHead = (stream_Link_head *)MALLOC(sizeof(stream_Link_head));
        if (pstWsAttr->linkHead == NULL)
        {
            printf("Ws chn%u link error\n", u8WsChn);
            exit(-1);
        }
        SLIST_INIT(pstWsAttr->linkHead);

#if (CARDV_VDEC_ENABLE)
        sprintf(key, "Cam%d:Ws%dUseVdec", u8CamId, u8WsId);
        pstWsAttr->bUseVdec = iniparser_getint(pstDict, key, 0);
        if (pstWsAttr->bUseVdec)
        {
            sprintf(key, "Cam%d:Ws%dVdecChn", u8CamId, u8WsId);
            pstWsAttr->u32VdecChn = iniparser_getint(pstDict, key, -1);
            if (pstWsAttr->u32VdecChn == 0 || pstWsAttr->u32VdecChn > CARDV_MAX_VDEC_CHN_NUM)
            {
                printf("Ws chn%u vdec chn error\n", u8WsChn);
                exit(-1);
            }
        }
#endif
    }
}
#endif

void cardv_init_vifParam(dictionary *pstDict, MI_U8 u8CamId)
{
    char                  key[64];
    MI_SNR_PADID          u32SnrPad       = 0;
    CarDV_Sensor_Attr_t * pstSensorAttr   = NULL;
    CarDV_VifDevAttr_t *  pstVifDevAttr   = NULL;
    CarDV_VifGroupAttr_t *pstVifGroupAttr = NULL;
    CarDV_VifPortAttr_t * pstVifPort      = NULL;
    MI_U8                 u8VifGroupId    = 0;
    MI_U32                u32VifDev       = 0;
    MI_U8                 vifDevPerGroup  = 0;

    sprintf(key, "Cam%d:SensorPad", u8CamId);
    u32SnrPad = iniparser_getint(pstDict, key, CARDV_MAX_SENSOR_NUM);
    if (u32SnrPad >= CARDV_MAX_SENSOR_NUM)
    {
        printf("[Cam%u] not use sensor\n", u8CamId);
        return;
    }

    pstSensorAttr = &gstSensorAttr[u32SnrPad];

    sprintf(key, "Cam%d:ResIndex", u8CamId);
    pstSensorAttr->u8ResIndex = iniparser_getint(pstDict, key, 255);
    sprintf(key, "Cam%d:PlaneMode", u8CamId);
    pstSensorAttr->bPlaneMode = iniparser_getint(pstDict, key, pstSensorAttr->bPlaneMode);
    sprintf(key, "Cam%d:RequestW", u8CamId);
    pstSensorAttr->u16RequestWidth = iniparser_getint(pstDict, key, g_recordWidth[u32SnrPad]);
    sprintf(key, "Cam%d:RequestH", u8CamId);
    pstSensorAttr->u16RequestHeight = iniparser_getint(pstDict, key, g_recordHeight[u32SnrPad]);
    sprintf(key, "Cam%d:RequestFps", u8CamId);
    pstSensorAttr->u32RequestFps = iniparser_getint(pstDict, key, 0);
    pstSensorAttr->bUsed         = 1;

    sprintf(key, "Cam%d:VifDev", u8CamId);
    u32VifDev = iniparser_getint(pstDict, key, -1);
    if (u32VifDev >= CARDV_MAX_VIF_DEV_NUM)
    {
        printf("VIF u32VifDev %d err > max %d \n", u32VifDev, CARDV_MAX_VIF_DEV_NUM);
        exit(-1);
    }

    u8VifGroupId   = u32VifDev / CARDV_MAX_VIF_DEV_PERGROUP;
    vifDevPerGroup = u32VifDev % CARDV_MAX_VIF_DEV_PERGROUP;
    if (u8VifGroupId >= CARDV_MAX_VIF_GROUP_NUM)
    {
        printf("VIF u8VifGroupId %d err > max %d \n", u8VifGroupId, CARDV_MAX_VIF_GROUP_NUM);
        exit(-1);
    }

    pstVifGroupAttr = &gstVifModule.stVifGroupAttr[u8VifGroupId];
    if (pstVifGroupAttr->bUsed == FALSE)
    {
        pstVifGroupAttr->bUsed = TRUE;
        sprintf(key, "Cam%d:HDR", u8CamId);
        pstVifGroupAttr->eHDRType = (MI_VIF_HDRType_e)iniparser_getint(pstDict, key, E_MI_VIF_HDR_TYPE_OFF);
        pstVifGroupAttr->stGroupExtAttr.eSensorPadID = (MI_VIF_SNRPad_e)u32SnrPad;
        sprintf(key, "Cam%d:WorkMode", u8CamId);
        pstVifGroupAttr->eWorkMode = (MI_VIF_WorkMode_e)iniparser_getint(pstDict, key, E_MI_VIF_WORK_MODE_1MULTIPLEX);
    }

    pstVifGroupAttr->stGroupExtAttr.u32PlaneID[vifDevPerGroup] = vifDevPerGroup;
    if (pstSensorAttr->bPlaneMode)
    {
        pstVifGroupAttr->bNeedSetGroupExtAttr = TRUE;
    }

    pstVifDevAttr = &pstVifGroupAttr->stVifDevAttr[vifDevPerGroup];
    if (pstVifDevAttr->bUsed == FALSE)
    {
        pstVifDevAttr->bUsed   = TRUE;
        pstVifDevAttr->u8CamId = u8CamId;
        sprintf(key, "Cam%d:Field", u8CamId);
        pstVifDevAttr->eField = (MI_SYS_FieldType_e)iniparser_getint(pstDict, key, E_MI_SYS_FIELDTYPE_NONE);
        sprintf(key, "Cam%d:EnH2T1PMode", u8CamId);
        pstVifDevAttr->bEnH2T1PMode = iniparser_getint(pstDict, key, 0);

        // Only use VIF output port 0 now.
        pstVifPort        = &pstVifDevAttr->stVifOutPortAttr[0];
        pstVifPort->bUsed = TRUE;
        sprintf(key, "Cam%d:VifCropX", u8CamId);
        pstVifPort->stCapRect.u16X = iniparser_getint(pstDict, key, 0);
        sprintf(key, "Cam%d:VifCropY", u8CamId);
        pstVifPort->stCapRect.u16Y = iniparser_getint(pstDict, key, 0);
        sprintf(key, "Cam%d:VifCropWidth", u8CamId);
        pstVifPort->stCapRect.u16Width = iniparser_getint(pstDict, key, 0);
        sprintf(key, "Cam%d:VifCropHeight", u8CamId);
        pstVifPort->stCapRect.u16Height = iniparser_getint(pstDict, key, 0);
        sprintf(key, "Cam%d:CompressMode", u8CamId);
        pstVifPort->eCompressMode = (MI_SYS_CompressMode_e)iniparser_getint(pstDict, key, E_MI_SYS_COMPRESS_MODE_NONE);

        pstVifPort->stDestSize.u16Width  = pstVifPort->stCapRect.u16Width;
        pstVifPort->stDestSize.u16Height = pstVifPort->stCapRect.u16Height;
        pstVifPort->ePixFormat           = (MI_SYS_PixelFormat_e)0xFFFF;

        // For Analog-decoder, Ex:AHD or TVI
        sprintf(key, "Cam%d:AnadecW", u8CamId);
        pstVifDevAttr->stAnadecSrcAttr.stRes.u16Width = iniparser_getint(pstDict, key, 1920);
        sprintf(key, "Cam%d:AnadecH", u8CamId);
        pstVifDevAttr->stAnadecSrcAttr.stRes.u16Height = iniparser_getint(pstDict, key, 1080);
        sprintf(key, "Cam%d:AnadecFps", u8CamId);
        pstVifDevAttr->stAnadecSrcAttr.u32Fps = iniparser_getint(pstDict, key, 25);
        sprintf(key, "Cam%d:AnadecMode", u8CamId);
        pstVifDevAttr->stAnadecSrcAttr.eTransferMode =
            (MI_SNR_Anadec_TransferMode_e)iniparser_getint(pstDict, key, E_MI_SNR_ANADEC_TRANSFERMODE_TVI);
    }

    if (E_MI_VIF_HDR_TYPE_VC == pstVifGroupAttr->eHDRType || E_MI_VIF_HDR_TYPE_DOL == pstVifGroupAttr->eHDRType)
    {
        pstSensorAttr->bPlaneMode             = TRUE;
        pstVifGroupAttr->bNeedSetGroupExtAttr = FALSE;
    }
}

void cardv_init_ispParam(dictionary *pstDict, MI_U8 u8CamId)
{
    char                    key[64];
    char *                  string           = NULL;
    MI_U32                  u32IspDevId      = 0;
    MI_U32                  u32IspChnId      = 0;
    CarDV_IspDevAttr_t *    pstIspDevAttr    = NULL;
    CarDV_IspChannelAttr_t *pstIspChnAttr    = NULL;
    CarDV_InPortAttr_t *    pstIspInPortAttr = NULL;

    sprintf(key, "Cam%d:IspDevId", u8CamId);
    u32IspDevId = iniparser_getint(pstDict, key, -1);
    if (u32IspDevId < CARDV_MAX_ISP_DEV_NUM)
    {
        pstIspDevAttr        = &gstIspModule.stIspDevAttr[u32IspDevId];
        pstIspDevAttr->bUsed = TRUE;

        sprintf(key, "Cam%d:IspChnId", u8CamId);
        u32IspChnId = iniparser_getint(pstDict, key, -1);
        if (u32IspChnId < CARDV_MAX_ISP_CHN_NUM)
        {
            pstIspChnAttr = &pstIspDevAttr->stIspChnlAttr[u32IspChnId];
            if (pstIspChnAttr->bUsed == FALSE)
            {
                pstIspChnAttr->u8CamId = u8CamId;
                pstIspChnAttr->u8ChnId = u32IspChnId;
                pstIspChnAttr->bUsed   = TRUE;

                pstIspInPortAttr                           = &pstIspChnAttr->stIspInPortAttr[0];
                pstIspInPortAttr->stInputCropWin.u16X      = 0;
                pstIspInPortAttr->stInputCropWin.u16Y      = 0;
                pstIspInPortAttr->stInputCropWin.u16Width  = 0;
                pstIspInPortAttr->stInputCropWin.u16Height = 0;

                sprintf(key, "Cam%d:IspbindType", u8CamId);
                pstIspInPortAttr->stBindParam.eBindType =
                    (MI_SYS_BindType_e)iniparser_getint(pstDict, key, E_MI_SYS_BIND_TYPE_FRAME_BASE);
                pstIspInPortAttr->stBindParam.stChnPort.eModId = E_MI_MODULE_ID_VIF;
                sprintf(key, "Cam%d:VifDev", u8CamId);
                pstIspInPortAttr->stBindParam.stChnPort.u32DevId  = iniparser_getint(pstDict, key, -1);
                pstIspInPortAttr->stBindParam.stChnPort.u32ChnId  = 0;
                pstIspInPortAttr->stBindParam.stChnPort.u32PortId = 0;
                pstIspInPortAttr->stBindParam.u32SrcFrmrate       = 30;
                pstIspInPortAttr->stBindParam.u32DstFrmrate       = 30;

                sprintf(key, "Cam%d:IspBin", u8CamId);
                string = iniparser_getstring(pstDict, key, (char *)CONFIG_PATH "/iqfile/isp_api.bin");
                strcpy(pstIspChnAttr->szApiBinFilePath, string);

                sprintf(key, "Cam%d:HDR", u8CamId);
                pstIspChnAttr->stIspChnParam.eHDRType =
                    (MI_ISP_HDRType_e)iniparser_getint(pstDict, key, E_MI_ISP_HDR_TYPE_OFF);
                sprintf(key, "Cam%d:NRLevel", u8CamId);
                pstIspChnAttr->stIspChnParam.e3DNRLevel =
                    (MI_ISP_3DNR_Level_e)iniparser_getint(pstDict, key, E_MI_ISP_3DNR_LEVEL_OFF);
                pstIspChnAttr->stIspChnParam.eRot    = E_MI_SYS_ROTATE_NONE;
                pstIspChnAttr->stIspChnParam.bMirror = FALSE;
                pstIspChnAttr->stIspChnParam.bFlip   = FALSE;
                sprintf(key, "Cam%d:SensorPad", u8CamId);
                pstIspChnAttr->eBindMiSnrPadId = iniparser_getint(pstDict, key, 0);

                sprintf(key, "Cam%d:IspOutPort0", u8CamId);
                pstIspChnAttr->stIspOutPortAttr[CARDV_ISP_OUTPORT_SCL_REALTIME].bUsed =
                    iniparser_getint(pstDict, key, 0);
                sprintf(key, "Cam%d:IspOutPort1", u8CamId);
                pstIspChnAttr->stIspOutPortAttr[CARDV_ISP_OUTPORT_BUFFER].bUsed = iniparser_getint(pstDict, key, 0);
                if (pstIspChnAttr->stIspOutPortAttr[CARDV_ISP_OUTPORT_BUFFER].bUsed)
                {
                    sprintf(key, "Cam%d:IspOutPort1userdepth", u8CamId);
                    pstIspChnAttr->stIspOutPortAttr[CARDV_ISP_OUTPORT_BUFFER].u16UserDepth =
                        iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Cam%d:IspOutPort1depth", u8CamId);
                    pstIspChnAttr->stIspOutPortAttr[CARDV_ISP_OUTPORT_BUFFER].u16Depth =
                        iniparser_getint(pstDict, key, 4);
                    sprintf(key, "Cam%d:IspOutPort1X", u8CamId);
                    pstIspChnAttr->stIspOutPortAttr[CARDV_ISP_OUTPORT_BUFFER].stOrigPortCrop.u16X =
                        iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Cam%d:IspOutPort1Y", u8CamId);
                    pstIspChnAttr->stIspOutPortAttr[CARDV_ISP_OUTPORT_BUFFER].stOrigPortCrop.u16Y =
                        iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Cam%d:IspOutPort1W", u8CamId);
                    pstIspChnAttr->stIspOutPortAttr[CARDV_ISP_OUTPORT_BUFFER].stOrigPortCrop.u16Width =
                        iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Cam%d:IspOutPort1H", u8CamId);
                    pstIspChnAttr->stIspOutPortAttr[CARDV_ISP_OUTPORT_BUFFER].stOrigPortCrop.u16Height =
                        iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Cam%d:IspOutPort1pixel", u8CamId);
                    pstIspChnAttr->stIspOutPortAttr[CARDV_ISP_OUTPORT_BUFFER].ePixelFormat =
                        (MI_SYS_PixelFormat_e)iniparser_getint(pstDict, key, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                }
                pstIspChnAttr->stIspOutPortAttr[CARDV_ISP_OUTPORT_IR].bUsed = 0;
            }
        }
    }
}

#if (CARDV_LDC_ENABLE)
void cardv_init_ldcParam(dictionary *pstDict, MI_U8 u8CamId)
{
    char                    key[64];
    char *                  string        = NULL;
    MI_U32                  u32LdcDevId   = 0;
    MI_U32                  u32LdcChnId   = 0;
    CarDV_LdcDevAttr_t *    pstLdcDevAttr = NULL;
    CarDV_LdcChannelAttr_t *pstLdcChnAttr = NULL;

    sprintf(key, "Cam%d:LdcDevId", u8CamId);
    u32LdcDevId = iniparser_getint(pstDict, key, -1);
    if (u32LdcDevId < CARDV_MAX_LDC_DEV_NUM)
    {
        pstLdcDevAttr        = &gstLdcModule.stLdcDevAttr[u32LdcDevId];
        pstLdcDevAttr->bUsed = TRUE;

        sprintf(key, "Cam%d:LdcChnId", u8CamId);
        u32LdcChnId = iniparser_getint(pstDict, key, -1);
        if (u32LdcChnId < CARDV_MAX_LDC_CHN_NUM)
        {
            pstLdcChnAttr = &pstLdcDevAttr->stLdcChnlAttr[u32LdcChnId];
            if (pstLdcChnAttr->bUsed == FALSE)
            {
                pstLdcChnAttr->u8ChnId = u32LdcChnId;
                pstLdcChnAttr->bUsed   = TRUE;

                sprintf(key, "Cam%d:LdcInBindMod", u8CamId);
                pstLdcChnAttr->stBindParam.stChnPort.eModId =
                    (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
                sprintf(key, "Cam%d:LdcInBindDev", u8CamId);
                pstLdcChnAttr->stBindParam.stChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
                sprintf(key, "Cam%d:LdcInBindChn", u8CamId);
                pstLdcChnAttr->stBindParam.stChnPort.u32ChnId = iniparser_getint(pstDict, key, -1);
                sprintf(key, "Cam%d:LdcInBindPort", u8CamId);
                pstLdcChnAttr->stBindParam.stChnPort.u32PortId = iniparser_getint(pstDict, key, -1);
                pstLdcChnAttr->stBindParam.u32SrcFrmrate       = 30;
                pstLdcChnAttr->stBindParam.u32DstFrmrate       = 30;
                pstLdcChnAttr->stBindParam.eBindType           = E_MI_SYS_BIND_TYPE_FRAME_BASE;

                sprintf(key, "Cam%d:LdcConfigPath", u8CamId);
                string = iniparser_getstring(pstDict, key, (char *)"NULL");
                if (access(string, F_OK))
                {
                    printf("LDC ConfigPath is Invalid [%s]\n", string);
                    exit(-1);
                }
                strcpy(pstLdcChnAttr->CfgFilePath, string);

                sprintf(key, "Cam%d:LdcWorkMode", u8CamId);
                pstLdcChnAttr->eWorkMode = (MI_LDC_WorkMode_e)iniparser_getint(pstDict, key, MI_LDC_WORKMODE_LDC);
                if (pstLdcChnAttr->eWorkMode == MI_LDC_WORKMODE_DIS_GYRO)
                {
                    sprintf(key, "Cam%d:LdcProjection3x3Matrix", u8CamId);
                    string = iniparser_getstring(pstDict, key, NULL);
                    if (string)
                    {
                        int          value = 0, index = 0;
                        const char **array = NULL;
                        array              = ParserStrToArray(string, &value);
                        if (array && value == LDC_MAXTRIX_NUM)
                        {
                            printf("\tMatrix3x3:");
                            pstLdcChnAttr->bUseProjection3x3Matrix = TRUE;
                            for (index = 0; index < value; index++)
                            {
                                pstLdcChnAttr->afProjection3x3Matrix[index] = strtof(array[index], NULL);
                                printf("%f ", pstLdcChnAttr->afProjection3x3Matrix[index]);
                            }
                            printf("\n");
                            free(array);
                        }
                    }

                    sprintf(key, "Cam%d:LdcUserSliceNum", u8CamId);
                    pstLdcChnAttr->u16UserSliceNum = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Cam%d:LdcFocalLength", u8CamId);
                    pstLdcChnAttr->u16FocalLength = iniparser_getint(pstDict, key, 0);
                }

                sprintf(key, "Cam%d:LdcSenSorCalibPath", u8CamId);
                string = iniparser_getstring(pstDict, key, (char *)"NULL");
                if (access(string, F_OK) == 0)
                {
                    pstLdcChnAttr->eMapMode = MI_LDC_MAPINFOTYPE_SENSORCALIB;
                    strcpy(pstLdcChnAttr->SensorCalibBinPath, string);
                    printf("\tMapMode:sensorcalib\n\tCalibPath:%s\n", pstLdcChnAttr->SensorCalibBinPath);
                }
                else
                {
                    pstLdcChnAttr->eMapMode = MI_LDC_MAPINFOTYPE_DISPMAP;
                    sprintf(key, "Cam%d:LdcXmap", u8CamId);
                    string = iniparser_getstring(pstDict, key, (char *)"NULL");
                    if (access(string, F_OK))
                    {
                        printf("MapX Path is not set\n");
                        exit(-1);
                    }
                    strcpy(pstLdcChnAttr->Xmap, string);

                    sprintf(key, "Cam%d:LdcYmap", u8CamId);
                    string = iniparser_getstring(pstDict, key, (char *)"NULL");
                    if (access(string, F_OK))
                    {
                        printf("MapY Path is not set\n");
                        exit(-1);
                    }
                    strcpy(pstLdcChnAttr->Ymap, string);
                    printf("\tMapMode:dispmap\n\tXmap:%s, Ymap:%s\n", pstLdcChnAttr->Xmap, pstLdcChnAttr->Ymap);
                }
            }
        }
    }
}
#endif

void cardv_init_sclParam(dictionary *pstDict, MI_U8 u8CamId, MI_BOOL bDispPipe)
{
    char   key[64];
    char   section[16]       = "Disp";
    MI_U32 u32SclModuleNum   = 0;
    MI_U32 u32SclModuleId    = 0;
    MI_U32 u32SclDevId       = 0;
    MI_U32 u32SclDevUseSclId = 0;

    if (!bDispPipe)
        sprintf(section, "Cam%d", u8CamId);

    sprintf(key, "%s:SclNum", section);
    u32SclModuleNum = iniparser_getint(pstDict, key, 0);
    for (u32SclModuleId = 0; u32SclModuleId < u32SclModuleNum; u32SclModuleId++)
    {
        CarDV_SclDevAttr_t *pstSclDevAttr = NULL;

        sprintf(key, "%s:Scl%dDev", section, u32SclModuleId);
        u32SclDevId = iniparser_getint(pstDict, key, 0);
        if (u32SclDevId >= CARDV_MAX_SCL_DEV_NUM)
        {
            printf("Scl Dev [%d] Err\n", u32SclDevId);
            exit(-1);
        }

        sprintf(key, "%s:Scl%dDevUseSclId", section, u32SclModuleId);
        u32SclDevUseSclId = iniparser_getint(pstDict, key, 0);

        if (u32SclDevUseSclId > 0)
        {
            MI_U32                  u32SclChnId      = 0;
            MI_U32                  u32SclId         = 0;
            MI_U8                   u8PortId         = 0;
            CarDV_SclChannelAttr_t *pstSclChnAttr    = NULL;
            CarDV_InPortAttr_t *    pstSclInPortAttr = NULL;

            pstSclDevAttr                  = &gstSclModule.stSclDevAttr[u32SclDevId];
            pstSclDevAttr->bUsed           = TRUE;
            pstSclDevAttr->u8DevId         = u32SclDevId;
            pstSclDevAttr->u32UseHwSclMask = u32SclDevUseSclId;

            if (pstSclDevAttr->bConfigPrivPool == FALSE)
            {
                sprintf(key, "%s:Scl%dCfgPool", section, u32SclModuleId);
                pstSclDevAttr->bConfigPrivPool = iniparser_getint(pstDict, key, FALSE);
                sprintf(key, "%s:Scl%dPoolW", section, u32SclModuleId);
                pstSclDevAttr->u16PrivPoolMaxWidth = iniparser_getint(pstDict, key, 0);
                sprintf(key, "%s:Scl%dPoolH", section, u32SclModuleId);
                pstSclDevAttr->u16PrivPoolMaxHeight = iniparser_getint(pstDict, key, 0);
                sprintf(key, "%s:Scl%dPoolLine", section, u32SclModuleId);
                pstSclDevAttr->u16PrivPoolRingLine = iniparser_getint(pstDict, key, 0);

                if (pstSclDevAttr->bConfigPrivPool == TRUE
                    && (pstSclDevAttr->u16PrivPoolMaxWidth == 0 || pstSclDevAttr->u16PrivPoolMaxHeight == 0
                        || pstSclDevAttr->u16PrivPoolRingLine == 0))
                {
                    printf("Scl Dev [%d] Priv Pool Err\n", u32SclDevId);
                    exit(-1);
                }
            }

            for (u32SclId = 0; u32SclId < CARDV_MAX_SCL_HWSCLID_NUM; u32SclId++)
            {
                if (u32SclDevUseSclId & (E_MI_SCL_HWSCL0 << u32SclId))
                {
                    pstSclDevAttr->u8HwSclPortId[u32SclId] = u8PortId;
                    u8PortId++;
                }
            }

            sprintf(key, "%s:Scl%dChn", section, u32SclModuleId);
            u32SclChnId = iniparser_getint(pstDict, key, 0);
            if (u32SclChnId >= CARDV_MAX_SCL_CHN_NUM)
            {
                printf("[%s]%d find sclchn fail max %d all use \n", __FUNCTION__, __LINE__, CARDV_MAX_SCL_CHN_NUM);
                exit(-1);
            }

            pstSclChnAttr = &pstSclDevAttr->stSclChnlAttr[u32SclChnId];
            if (pstSclChnAttr->bUsed == TRUE)
            {
                printf("ini key [%s] error\n", key);
                exit(-1);
            }

            pstSclChnAttr->bUsed   = TRUE;
            pstSclChnAttr->u8ChnId = u32SclChnId;
            pstSclInPortAttr       = &pstSclChnAttr->stSclInPortAttr[0];

            if (pstSclDevAttr->u8DevId == CARDV_SCL_DEV_FROM_RDMA0)
            {
                sprintf(key, "%s:Scl%dInBindMod", section, u32SclModuleId);
                pstSclInPortAttr->stBindParam.stChnPort.eModId =
                    (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
                sprintf(key, "%s:Scl%dInBindDev", section, u32SclModuleId);
                pstSclInPortAttr->stBindParam.stChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
                sprintf(key, "%s:Scl%dInBindChn", section, u32SclModuleId);
                pstSclInPortAttr->stBindParam.stChnPort.u32ChnId = iniparser_getint(pstDict, key, -1);
                sprintf(key, "%s:Scl%dInBindPort", section, u32SclModuleId);
                pstSclInPortAttr->stBindParam.stChnPort.u32PortId = iniparser_getint(pstDict, key, -1);
                pstSclInPortAttr->stBindParam.u32SrcFrmrate       = 30;
                pstSclInPortAttr->stBindParam.u32DstFrmrate       = 30;
                sprintf(key, "%s:Scl%dInBindType", section, u32SclModuleId);
                pstSclInPortAttr->stBindParam.eBindType =
                    (MI_SYS_BindType_e)iniparser_getint(pstDict, key, E_MI_SYS_BIND_TYPE_FRAME_BASE);
            }
            else if (pstSclDevAttr->u8DevId == CARDV_SCL_DEV_FROM_ISP_REALTIME)
            {
                pstSclInPortAttr->stBindParam.stChnPort.eModId = E_MI_MODULE_ID_ISP;
                sprintf(key, "Cam%d:IspDevId", u8CamId);
                pstSclInPortAttr->stBindParam.stChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
                sprintf(key, "Cam%d:IspChnId", u8CamId);
                pstSclInPortAttr->stBindParam.stChnPort.u32ChnId  = iniparser_getint(pstDict, key, -1);
                pstSclInPortAttr->stBindParam.stChnPort.u32PortId = 0;
                pstSclInPortAttr->stBindParam.u32SrcFrmrate       = 30;
                pstSclInPortAttr->stBindParam.u32DstFrmrate       = 30;
                pstSclInPortAttr->stBindParam.eBindType           = E_MI_SYS_BIND_TYPE_REALTIME;
            }
            else if (pstSclDevAttr->u8DevId == CARDV_SCL_DEV_FROM_YUV_REALTIME)
            {
                pstSclInPortAttr->stBindParam.stChnPort.eModId = E_MI_MODULE_ID_VIF;
                sprintf(key, "Cam%d:VifDev", u8CamId);
                pstSclInPortAttr->stBindParam.stChnPort.u32DevId  = iniparser_getint(pstDict, key, -1);
                pstSclInPortAttr->stBindParam.stChnPort.u32ChnId  = 0;
                pstSclInPortAttr->stBindParam.stChnPort.u32PortId = 0;
                pstSclInPortAttr->stBindParam.u32SrcFrmrate       = 30;
                pstSclInPortAttr->stBindParam.u32DstFrmrate       = 30;
                pstSclInPortAttr->stBindParam.eBindType           = E_MI_SYS_BIND_TYPE_REALTIME;
            }
            else
            {
                printf("[%s]%d scl source err %d \n", __FUNCTION__, __LINE__, pstSclDevAttr->u8DevId);
                exit(-1);
            }

            for (u32SclId = 0; u32SclId < CARDV_MAX_SCL_HWSCLID_NUM; u32SclId++)
            {
                if (u32SclDevUseSclId & 0x1 << u32SclId)
                {
                    MI_U32 bUse            = 0;
                    MI_U32 u32SclOutPortId = 0;
                    sprintf(key, "%s:Scl%dHWScl%d", section, u32SclModuleId, u32SclId);
                    bUse = iniparser_getint(pstDict, key, 0);
                    if (bUse == 1)
                    {
                        CarDV_PortAttr_t *pstSclOutPortAttr = NULL;
                        u32SclOutPortId                     = pstSclDevAttr->u8HwSclPortId[u32SclId];
                        if (u32SclOutPortId >= CARDV_MAX_SCL_OUTPORT_NUM)
                        {
                            printf("[%s]%d find scl outputportid fail max %d all use \n", __FUNCTION__, __LINE__,
                                   CARDV_MAX_ISP_CHN_NUM);
                            exit(-1);
                        }

                        pstSclOutPortAttr        = &pstSclChnAttr->stSclOutPortAttr[u32SclOutPortId];
                        pstSclOutPortAttr->bUsed = TRUE;
                        sprintf(key, "%s:Scl%dHWScl%dCropX", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->stOrigPortCrop.u16X = iniparser_getint(pstDict, key, 0);
                        sprintf(key, "%s:Scl%dHWScl%dCropY", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->stOrigPortCrop.u16Y = iniparser_getint(pstDict, key, 0);
                        sprintf(key, "%s:Scl%dHWScl%dCropW", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->stOrigPortCrop.u16Width = iniparser_getint(pstDict, key, 0);
                        sprintf(key, "%s:Scl%dHWScl%dCropH", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->stOrigPortCrop.u16Height = iniparser_getint(pstDict, key, 0);

                        sprintf(key, "%s:Scl%dHWScl%dMirror", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->bMirror = iniparser_getint(pstDict, key, 0);
                        sprintf(key, "%s:Scl%dHWScl%dFlip", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->bFlip = iniparser_getint(pstDict, key, 0);

                        sprintf(key, "%s:Scl%dHWScl%dW", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->stOrigPortSize.u16Width = iniparser_getint(pstDict, key, 0);
                        sprintf(key, "%s:Scl%dHWScl%dH", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->stOrigPortSize.u16Height = iniparser_getint(pstDict, key, 0);
                        sprintf(key, "%s:Scl%dHWScl%dPixel", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->ePixelFormat = (MI_SYS_PixelFormat_e)iniparser_getint(
                            pstDict, key, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);

                        sprintf(key, "%s:Scl%dHWScl%dCompressMode", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->eCompressMode =
                            (MI_SYS_CompressMode_e)iniparser_getint(pstDict, key, E_MI_SYS_COMPRESS_MODE_NONE);

                        sprintf(key, "%s:Scl%dHWScl%duserdepth", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->u16UserDepth = iniparser_getint(pstDict, key, 0);
                        sprintf(key, "%s:Scl%dHWScl%ddepth", section, u32SclModuleId, u32SclId);
                        pstSclOutPortAttr->u16Depth = iniparser_getint(pstDict, key, 4);
                    }
                }
            }
        }
    }
}

#if (CARDV_VDISP_ENABLE)
void cardv_init_vdispParam(dictionary *pstDict, MI_U8 u8CamId)
{
    char                      key[64];
    MI_VDISP_DEV              s32VdispDevId   = 0;
    MI_VDISP_CHN              s32VdispChnId   = 0;
    MI_BOOL                   bDevUse         = FALSE;
    MI_BOOL                   bChnUse         = FALSE;
    CarDV_VdispDevAttr_t *    pstVdispDevAttr = NULL;
    CarDV_VdispChannelAttr_t *psVdispChnAttr  = NULL;

    for (s32VdispDevId = 0; s32VdispDevId < VDISP_MAX_DEVICE_NUM; s32VdispDevId++)
    {
        sprintf(key, "Cam%d:VdispDev%d", u8CamId, s32VdispDevId);
        bDevUse = iniparser_getint(pstDict, key, 0);
        if (bDevUse)
        {
            pstVdispDevAttr = &gstVdispModule.stVdispDevAttr[s32VdispDevId];
            if (pstVdispDevAttr->bUsed == FALSE)
            {
                pstVdispDevAttr->bUsed = TRUE;
                sprintf(key, "Cam%d:VdispDev%dOutW", u8CamId, s32VdispDevId);
                pstVdispDevAttr->stVdispOutAttr[0].u32Width = iniparser_getint(pstDict, key, 0);
                sprintf(key, "Cam%d:VdispDev%dOutH", u8CamId, s32VdispDevId);
                pstVdispDevAttr->stVdispOutAttr[0].u32Height = iniparser_getint(pstDict, key, 0);
                sprintf(key, "Cam%d:VdispDev%dPixel", u8CamId, s32VdispDevId);
                pstVdispDevAttr->stVdispOutAttr[0].ePixelFormat =
                    (MI_SYS_PixelFormat_e)iniparser_getint(pstDict, key, E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420);
                pstVdispDevAttr->stVdispOutAttr[0].u32BgColor = YUYV_BLACK;
                pstVdispDevAttr->stVdispOutAttr[0].u64pts     = 0;
                pstVdispDevAttr->stVdispOutAttr[0].u32FrmRate = 0;

                sprintf(key, "Cam%d:VdispDev%duserdepth", u8CamId, s32VdispDevId);
                pstVdispDevAttr->u16UserDepth = iniparser_getint(pstDict, key, 0);
                sprintf(key, "Cam%d:VdispDev%ddepth", u8CamId, s32VdispDevId);
                pstVdispDevAttr->u16Depth = iniparser_getint(pstDict, key, 4);
            }

            for (s32VdispChnId = 0; s32VdispChnId < VDISP_MAX_CHN_NUM_PER_DEV + VDISP_OVERLAY_CHN_NUM; s32VdispChnId++)
            {
                psVdispChnAttr = &pstVdispDevAttr->stVdispChnlAttr[s32VdispChnId];
                if (psVdispChnAttr->bUsed == TRUE)
                    continue;

                sprintf(key, "Cam%d:VdispDev%dChn%d", u8CamId, s32VdispDevId, s32VdispChnId);
                bChnUse = iniparser_getint(pstDict, key, 0);
                if (bChnUse)
                {
                    psVdispChnAttr->bUsed = TRUE;
                    sprintf(key, "Cam%d:VdispDev%dChn%dInX", u8CamId, s32VdispDevId, s32VdispChnId);
                    psVdispChnAttr->stVdispInPortAttr[0].u32OutX = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Cam%d:VdispDev%dChn%dInY", u8CamId, s32VdispDevId, s32VdispChnId);
                    psVdispChnAttr->stVdispInPortAttr[0].u32OutY = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Cam%d:VdispDev%dChn%dInW", u8CamId, s32VdispDevId, s32VdispChnId);
                    psVdispChnAttr->stVdispInPortAttr[0].u32OutWidth = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Cam%d:VdispDev%dChn%dInH", u8CamId, s32VdispDevId, s32VdispChnId);
                    psVdispChnAttr->stVdispInPortAttr[0].u32OutHeight = iniparser_getint(pstDict, key, 0);

                    psVdispChnAttr->stVdispInPortAttr[0].s32IsFreeRun = 1;

                    sprintf(key, "Cam%d:VdispDev%dChn%dInMod", u8CamId, s32VdispDevId, s32VdispChnId);
                    psVdispChnAttr->stBindParam[0].stChnPort.eModId =
                        (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
                    sprintf(key, "Cam%d:VdispDev%dChn%dInDev", u8CamId, s32VdispDevId, s32VdispChnId);
                    psVdispChnAttr->stBindParam[0].stChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
                    sprintf(key, "Cam%d:VdispDev%dChn%dInChn", u8CamId, s32VdispDevId, s32VdispChnId);
                    psVdispChnAttr->stBindParam[0].stChnPort.u32ChnId = iniparser_getint(pstDict, key, -1);
                    sprintf(key, "Cam%d:VdispDev%dChn%dInPort", u8CamId, s32VdispDevId, s32VdispChnId);
                    psVdispChnAttr->stBindParam[0].stChnPort.u32PortId = iniparser_getint(pstDict, key, -1);

                    psVdispChnAttr->stBindParam[0].u32SrcFrmrate = 30;
                    psVdispChnAttr->stBindParam[0].u32DstFrmrate = 30;
                    psVdispChnAttr->stBindParam[0].eBindType     = E_MI_SYS_BIND_TYPE_FRAME_BASE;
                }
            }
        }
    }
}
#endif

void cardv_init_vencParam(dictionary *pstDict, MI_U8 u8CamId)
{
    char              key[64];
    MI_U32            u32VencModuleNum = 0;
    MI_U32            u32VencModuleId  = 0;
    MI_U32            u32VencChn       = 0;
    CarDV_VencAttr_t *pstVencAttr;
    MI_U32            u32SensorFps = 0;
    MI_U32            u32VencFps   = 0;

    sprintf(key, "Cam%d:RequestFps", u8CamId);
    u32SensorFps = iniparser_getint(pstDict, key, 0);
    if (u32SensorFps == 0)
    {
        u32SensorFps = 30;
        u32VencFps   = 30;
    }
    else if (u32SensorFps >= 1000)
    {
        u32VencFps   = u32SensorFps | ((1000 << 16) & 0xFFFF0000);
        u32SensorFps = u32SensorFps / 1000;
    }
    else
    {
        u32VencFps = u32SensorFps;
    }

    if (gstVencPrivPoolCfg.bConfigPrivPool == FALSE)
    {
        sprintf(key, "Cam%d:VencCfgPool", u8CamId);
        gstVencPrivPoolCfg.bConfigPrivPool = iniparser_getint(pstDict, key, FALSE);
        sprintf(key, "Cam%d:VencPoolW", u8CamId);
        gstVencPrivPoolCfg.u16PrivPoolMaxWidth = iniparser_getint(pstDict, key, 0);
        sprintf(key, "Cam%d:VencPoolH", u8CamId);
        gstVencPrivPoolCfg.u16PrivPoolMaxHeight = iniparser_getint(pstDict, key, 0);
        sprintf(key, "Cam%d:VencPoolLine", u8CamId);
        gstVencPrivPoolCfg.u16PrivPoolRingLine = iniparser_getint(pstDict, key, 0);

        if (gstVencPrivPoolCfg.bConfigPrivPool == TRUE
            && (gstVencPrivPoolCfg.u16PrivPoolMaxWidth == 0 || gstVencPrivPoolCfg.u16PrivPoolMaxHeight == 0
                || gstVencPrivPoolCfg.u16PrivPoolRingLine == 0))
        {
            printf("Venc Priv Pool Err\n");
            exit(-1);
        }
    }

    sprintf(key, "Cam%d:VencNum", u8CamId);
    u32VencModuleNum = iniparser_getint(pstDict, key, 0);
    for (u32VencModuleId = 0; u32VencModuleId < u32VencModuleNum; u32VencModuleId++)
    {
        sprintf(key, "Cam%d:Venc%dChn", u8CamId, u32VencModuleId);
        u32VencChn = iniparser_getint(pstDict, key, -1);
        if (u32VencChn >= MAX_VIDEO_NUMBER)
        {
            printf("ini key [%s] error\n", key);
            exit(-1);
        }

        pstVencAttr = &gstVencAttr[u32VencChn];
        if (pstVencAttr->bUsed == FALSE)
        {
            pstVencAttr->bUsed            = TRUE;
            pstVencAttr->u32VencFrameRate = u32VencFps;
            pstVencAttr->u8VBR            = 0;
            pstVencAttr->s32ChangePos     = 80;
            pstVencAttr->u32Gop           = u32SensorFps;
            pstVencAttr->u32MaxIQp        = 48;
            pstVencAttr->u32MinIQp        = 12;
            pstVencAttr->u32MaxPQp        = 48;
            pstVencAttr->u32MinPQp        = 12;
            pstVencAttr->u32Qfactor       = 50;
            pstVencAttr->u8CamId          = u8CamId;
            sprintf(key, "Cam%d:Venc%dType", u8CamId, u32VencModuleId);
            pstVencAttr->eVideoType = (VideoStreamType_e)iniparser_getint(pstDict, key, VIDEO_STREAM_RTSP);
            sprintf(key, "Cam%d:Venc%dCodec", u8CamId, u32VencModuleId);
            pstVencAttr->u8EncoderType = iniparser_getint(pstDict, key, 0);
            sprintf(key, "Cam%d:Venc%dW", u8CamId, u32VencModuleId);
            pstVencAttr->u32Width = iniparser_getint(pstDict, key, H26X_MAX_ENCODE_WIDTH);
            sprintf(key, "Cam%d:Venc%dH", u8CamId, u32VencModuleId);
            pstVencAttr->u32Height = iniparser_getint(pstDict, key, H26X_MAX_ENCODE_HEIGHT);
            sprintf(key, "Cam%d:Venc%dMaxW", u8CamId, u32VencModuleId);
            pstVencAttr->u32MaxWidth = iniparser_getint(pstDict, key, H26X_MAX_ENCODE_WIDTH);
            sprintf(key, "Cam%d:Venc%dMaxH", u8CamId, u32VencModuleId);
            pstVencAttr->u32MaxHeight = iniparser_getint(pstDict, key, H26X_MAX_ENCODE_HEIGHT);
            sprintf(key, "Cam%d:Venc%dBufSize", u8CamId, u32VencModuleId);
            pstVencAttr->u32BufSize = iniparser_getint(pstDict, key, 0);
            sprintf(key, "Cam%d:Venc%dBitRate", u8CamId, u32VencModuleId);
            pstVencAttr->u32BitRate        = iniparser_getint(pstDict, key, 1000000);
            pstVencAttr->u32VidCompBufSize = pstVencAttr->u32BitRate / 8 * COMPBUF2SEC;

            sprintf(key, "Cam%d:Venc%dInBindType", u8CamId, u32VencModuleId);
            pstVencAttr->stVencInBindParam.eBindType =
                (MI_SYS_BindType_e)iniparser_getint(pstDict, key, E_MI_SYS_BIND_TYPE_FRAME_BASE);
            sprintf(key, "Cam%d:Venc%dInBindMod", u8CamId, u32VencModuleId);
            pstVencAttr->stVencInBindParam.stChnPort.eModId =
                (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
            sprintf(key, "Cam%d:Venc%dInBindDev", u8CamId, u32VencModuleId);
            pstVencAttr->stVencInBindParam.stChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
            sprintf(key, "Cam%d:Venc%dInBindChn", u8CamId, u32VencModuleId);
            pstVencAttr->stVencInBindParam.stChnPort.u32ChnId = iniparser_getint(pstDict, key, -1);
            sprintf(key, "Cam%d:Venc%dInBindPort", u8CamId, u32VencModuleId);
            pstVencAttr->stVencInBindParam.stChnPort.u32PortId = iniparser_getint(pstDict, key, -1);
            sprintf(key, "Cam%d:Venc%dInBindParam", u8CamId, u32VencModuleId);
            pstVencAttr->stVencInBindParam.u32BindParam  = iniparser_getint(pstDict, key, 0);
            pstVencAttr->stVencInBindParam.u32SrcFrmrate = pstVencAttr->u32VencFrameRate;
            pstVencAttr->stVencInBindParam.u32DstFrmrate = pstVencAttr->u32VencFrameRate;
#if (CARDV_RTSP_INPUT_ENABLE || CARDV_WS_INPUT_ENABLE)
            sprintf(key, "Cam%d:Venc%dPassVenc", u8CamId, u32VencModuleId);
            pstVencAttr->bPassVenc = iniparser_getint(pstDict, key, 0);
            if (pstVencAttr->bPassVenc)
            {
                sprintf(key, "Cam%d:Venc%dExtStreamChn", u8CamId, u32VencModuleId);
                pstVencAttr->u8ExtStreamChn = iniparser_getint(pstDict, key, -1);
#if (CARDV_RTSP_INPUT_ENABLE)
                if (pstVencAttr->u8ExtStreamChn >= RTSP_MAX_STREAM_CNT)
#elif (CARDV_WS_INPUT_ENABLE)
                if (pstVencAttr->u8ExtStreamChn >= WS_MAX_STREAM_CNT)
#endif
                {
                    printf("ini key [%s] error\n", key);
                    exit(-1);
                }
            }
#endif

            if (pstVencAttr->eVideoType == VIDEO_STREAM_RECORD || pstVencAttr->eVideoType == VIDEO_STREAM_SUBREC
                || pstVencAttr->eVideoType == VIDEO_STREAM_SHARE)
            {
                pstVencAttr->u8MuxerNum = 0;
                for (MI_U8 muxerMapIdx = 0; muxerMapIdx < MUXER_NUM_SHARE_VENC; muxerMapIdx++)
                {
                    pstVencAttr->u8MuxerIdx[muxerMapIdx] = 0xFF;
                }
            }
            else if (pstVencAttr->eVideoType == VIDEO_STREAM_THUMB)
                pstVencAttr->u8VidThumbBufCnt = 6;
            else if (pstVencAttr->eVideoType == VIDEO_STREAM_RTSP)
                g_bRtspEnable = TRUE;
            else if (pstVencAttr->eVideoType == VIDEO_STREAM_UVC)
                g_openUVCDevice = TRUE;
        }
    }
}

void cardv_init_muxerParam(dictionary *pstDict, MI_U8 u8CamId)
{
    char               key[64];
    MI_U32             u32MuxerModuleNum = 0;
    MI_U32             u32MuxerModuleId  = 0;
    MI_U32             u32MuxerChn       = 0;
    CarDV_MuxerAttr_t *pstMuxerAttr;
    MI_U32             u32VencChn;
    CarDV_VencAttr_t * pstVencAttr;

    sprintf(key, "Cam%d:MuxNum", u8CamId);
    u32MuxerModuleNum = iniparser_getint(pstDict, key, 0);
    for (u32MuxerModuleId = 0; u32MuxerModuleId < u32MuxerModuleNum; u32MuxerModuleId++)
    {
        sprintf(key, "Cam%d:Mux%dChn", u8CamId, u32MuxerModuleId);
        u32MuxerChn = iniparser_getint(pstDict, key, -1);
        if (u32MuxerChn >= MAX_MUXER_NUMBER)
        {
            printf("ini key [%s] error\n", key);
            exit(-1);
        }

        pstMuxerAttr = &gstMuxerAttr[u32MuxerChn];
        if (pstMuxerAttr->bUsed == FALSE)
        {
            pstMuxerAttr->bUsed   = TRUE;
            pstMuxerAttr->u8CamId = u8CamId;
            sprintf(key, "Cam%d:Mux%dTrakNum", u8CamId, u32MuxerModuleId);
            pstMuxerAttr->u8VidTrackNum = iniparser_getint(pstDict, key, 0);
            for (MI_U8 i = 0; i < pstMuxerAttr->u8VidTrackNum; i++)
            {
                sprintf(key, "Cam%d:Mux%dTrak%dVenc", u8CamId, u32MuxerModuleId, i);
                u32VencChn = pstMuxerAttr->u8VencChn[i] = iniparser_getint(pstDict, key, -1);
                pstVencAttr                             = &gstVencAttr[u32VencChn];
                if (pstVencAttr->bUsed == FALSE
                    || (pstVencAttr->eVideoType != VIDEO_STREAM_RECORD && pstVencAttr->eVideoType != VIDEO_STREAM_SUBREC
                        && pstVencAttr->eVideoType != VIDEO_STREAM_SHARE)
                    || pstVencAttr->u8MuxerNum >= MUXER_NUM_SHARE_VENC
                    || pstVencAttr->u8MuxerIdx[pstVencAttr->u8MuxerNum] != 0xFF)
                {
                    printf("ini key [%s] conflict\n", key);
                    exit(-1);
                }

                pstVencAttr->u8MuxerIdx[pstVencAttr->u8MuxerNum] = u32MuxerChn;
                pstVencAttr->u8MuxerVidTrackId                   = i;
                pstVencAttr->u8MuxerNum++;
            }
            sprintf(key, "Cam%d:Mux%dType", u8CamId, u32MuxerModuleId);
            pstMuxerAttr->eMuxerType = (MuxerType_e)iniparser_getint(pstDict, key, MUXER_NORMAL);
            sprintf(key, "Cam%d:Mux%dFile", u8CamId, u32MuxerModuleId);
            pstMuxerAttr->eMuxerFile = (MuxerFile_e)iniparser_getint(pstDict, key, MUXER_MAIN_FLIE);
            if (pstMuxerAttr->eMuxerFile == MUXER_SUB_FLIE)
            {
                int DB;
                switch (pstMuxerAttr->eMuxerType)
                {
                case MUXER_NORMAL:
                    DB = DB_NORMAL;
                    break;
                case MUXER_EMERG:
                    DB = DB_EVENT;
                    break;
                case MUXER_SHARE:
                    DB = DB_SHARE;
                    break;
                default:
                    DB = DB_NORMAL;
                    break;
                }

                if (DCF_IsSubFileEnable(DB, pstMuxerAttr->u8CamId) == FALSE)
                {
                    printf("DCF not support, check ini\n");
                    exit(-1);
                }
            }
            pstMuxerAttr->u64LimitTime =
                pstMuxerAttr->eMuxerType == MUXER_NORMAL ? g_u64NormalRecTime : g_u64ShareEmergRecTime;
#if (CARDV_GPS_ENABLE)
            pstMuxerAttr->bGpsTrack = TRUE;
#endif
            pstMuxerAttr->bGsnrTrack = TRUE;
            sprintf(key, "Cam%d:Mux%dThumb", u8CamId, u32MuxerModuleId);
            pstMuxerAttr->bThumbTrack = iniparser_getint(pstDict, key, 0);
        }
    }
}

#if (CARDV_JPD_ENABLE)
void cardv_init_jpdParam(dictionary *pstDict, MI_U8 u8CamId)
{
    char                key[64];
    char                szFilePath[] = "./image";
    char *              pszFilePath;
    MI_U32              u32JpdChnId   = 0;
    CarDV_JpdChnAttr_t *pstJpdChnAttr = NULL;

    sprintf(key, "Cam%d:JpdChn", u8CamId);
    u32JpdChnId = iniparser_getint(pstDict, key, -1);
    if (u32JpdChnId < CARDV_MAX_JPD_CHN_NUM)
    {
        pstJpdChnAttr                   = &gstJpdModule.stJpdChnAttr[u32JpdChnId];
        pstJpdChnAttr->bUsed            = TRUE;
        pstJpdChnAttr->u32ChnId         = u32JpdChnId;
        pstJpdChnAttr->u32MaxW          = 8192;
        pstJpdChnAttr->u32MaxH          = 8192;
        pstJpdChnAttr->u32StreamBufSize = 4 * 1024 * 1024;
        sprintf(key, "Cam%d:JpdPath", u8CamId);
        pszFilePath = iniparser_getstring(pstDict, key, szFilePath);
        strcpy(pstJpdChnAttr->szFilePath, pszFilePath);
        sprintf(key, "Cam%d:JpdFps", u8CamId);
        pstJpdChnAttr->u32Fps = iniparser_getint(pstDict, key, 25);
    }
}
#endif

#if (CARDV_VDEC_ENABLE)
void cardv_init_vdecParam(dictionary *pstDict, MI_U8 u8CamId)
{
    char                 key[64];
    char                 szFileName[] = "./video/test.h264";
    char *               pszFileName;
    MI_U32               u32VdecChnId   = 0;
    CarDV_VdecChnAttr_t *pstVdecChnAttr = NULL;

    sprintf(key, "Cam%d:VdecChn", u8CamId);
    u32VdecChnId = iniparser_getint(pstDict, key, -1);
    if (u32VdecChnId < CARDV_MAX_VDEC_CHN_NUM)
    {
        pstVdecChnAttr                   = &gstVdecModule.stVdecChnAttr[u32VdecChnId];
        pstVdecChnAttr->bUsed            = TRUE;
        pstVdecChnAttr->u32ChnId         = u32VdecChnId;
        pstVdecChnAttr->u32MaxW          = 3840;
        pstVdecChnAttr->u32MaxH          = 2160;
        pstVdecChnAttr->u32StreamBufSize = 1024 * 1024;
        sprintf(key, "Cam%d:VdecType", u8CamId);
        pstVdecChnAttr->eCodecType = (MI_VDEC_CodecType_e)iniparser_getint(pstDict, key, E_MI_VDEC_CODEC_TYPE_H264);
        sprintf(key, "Cam%d:VdecFileName", u8CamId);
        pszFileName = iniparser_getstring(pstDict, key, szFileName);
        strcpy(pstVdecChnAttr->szFileName, pszFileName);
        sprintf(key, "Cam%d:VdecFps", u8CamId);
        pstVdecChnAttr->u32Fps = iniparser_getint(pstDict, key, 25);
    }
}
#endif

void cardv_init_osdParam(dictionary *pstDict)
{
    char                 key[64];
    MI_U32               u32AttachId = 0;
    CarDV_OsdBufferAttr *pstBufferAttr;
    CarDV_OsdAttachAttr *pstOsdAttachAttr;

    pstBufferAttr                = &gstOsdAttr.stBufferAttr[0];
    pstBufferAttr->u32Width      = 640;
    pstBufferAttr->u32Height     = 120;
    pstBufferAttr->eRgnFormat    = E_MI_RGN_PIXEL_FORMAT_I4;
    pstBufferAttr->eFontSize     = FONT_SIZE_24;
    pstBufferAttr->stPointTime.x = 0;
    pstBufferAttr->stPointTime.y = 48;
    pstBufferAttr->stPointGps.x  = 0;
    pstBufferAttr->stPointGps.y  = 48 + pstBufferAttr->eFontSize + 2;

    pstBufferAttr                = &gstOsdAttr.stBufferAttr[1];
    pstBufferAttr->u32Width      = 960;
    pstBufferAttr->u32Height     = 180;
    pstBufferAttr->eRgnFormat    = E_MI_RGN_PIXEL_FORMAT_I4;
    pstBufferAttr->eFontSize     = FONT_SIZE_32;
    pstBufferAttr->stPointTime.x = 0;
    pstBufferAttr->stPointTime.y = 48;
    pstBufferAttr->stPointGps.x  = 0;
    pstBufferAttr->stPointGps.y  = 48 + pstBufferAttr->eFontSize + 2;

    pstBufferAttr                = &gstOsdAttr.stBufferAttr[2];
    pstBufferAttr->u32Width      = 1280;
    pstBufferAttr->u32Height     = 240;
    pstBufferAttr->eRgnFormat    = E_MI_RGN_PIXEL_FORMAT_I4;
    pstBufferAttr->eFontSize     = FONT_SIZE_48;
    pstBufferAttr->stPointTime.x = 0;
    pstBufferAttr->stPointTime.y = 48;
    pstBufferAttr->stPointGps.x  = 0;
    pstBufferAttr->stPointGps.y  = 48 + pstBufferAttr->eFontSize + 2;

    for (u32AttachId = 0; u32AttachId < CARDV_OSD_MAX_ATTACH_NUM; u32AttachId++)
    {
        pstOsdAttachAttr = &gstOsdAttr.stAttachAttr[u32AttachId];
        if (pstOsdAttachAttr->bUsed == FALSE)
        {
            sprintf(key, "OSD:Attach%dMod", u32AttachId);
            pstOsdAttachAttr->stRgnChnPort.eModId = (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
            sprintf(key, "OSD:Attach%dDev", u32AttachId);
            pstOsdAttachAttr->stRgnChnPort.s32DevId = iniparser_getint(pstDict, key, -1);
            sprintf(key, "OSD:Attach%dChn", u32AttachId);
            pstOsdAttachAttr->stRgnChnPort.s32ChnId = iniparser_getint(pstDict, key, -1);
            sprintf(key, "OSD:Attach%dPort", u32AttachId);
            pstOsdAttachAttr->stRgnChnPort.s32PortId = iniparser_getint(pstDict, key, -1);
            if (E_MI_MODULE_ID_MAX != pstOsdAttachAttr->stRgnChnPort.eModId)
            {
                pstOsdAttachAttr->bUsed = TRUE;
                g_displayOsd            = TRUE;
            }
        }
    }
}
#if (CARDV_DISPLAY_ENABLE)
void cardv_init_dispParam(dictionary *pstDict)
{
    char                  key[64];
    MI_U32                u32DispDev      = 0;
    MI_U32                u32DispLayer    = 0;
    MI_U32                u32DispPort     = 0;
    MI_BOOL               bDevUse         = FALSE;
    CarDV_DispModAttr_t * pstDispModAttr  = &gstDispModule;
    MI_U8                 u8BindInfo      = 0;
    CarDV_DispBindAttr_t *pstDispBindAttr = &gstDispBindAttr;
    char *                pstring         = NULL;

    cardv_init_sclParam(pstDict, -1, TRUE);

    for (u32DispDev = 0; u32DispDev < CARDV_MAX_DISP_DEV_NUM; u32DispDev++)
    {
        sprintf(key, "Disp:Dev%d", u32DispDev);
        bDevUse = iniparser_getint(pstDict, key, 0);
        if (bDevUse && FALSE == pstDispModAttr->stDispDevAttr[u32DispDev].bUsed)
        {
            pstDispModAttr->stDispDevAttr[u32DispDev].bUsed = TRUE;

            sprintf(key, "Disp:Dev%dIntfType", u32DispDev);
            pstring                                             = iniparser_getstring(pstDict, key, NULL);
            pstDispModAttr->stDispDevAttr[u32DispDev].eIntfType = CarDV_DispModuleParserIntfType(pstring);
            if (E_MI_DISP_INTF_MAX == pstDispModAttr->stDispDevAttr[u32DispDev].eIntfType)
            {
                printf("disp intf err\n");
                exit(-1);
            }

            sprintf(key, "Disp:Dev%dIntfSync", u32DispDev);
            pstring                                             = iniparser_getstring(pstDict, key, NULL);
            pstDispModAttr->stDispDevAttr[u32DispDev].eIntfSync = CarDV_DispModuleParserIntfSync(pstring);
            if (E_MI_DISP_OUTPUT_MAX == pstDispModAttr->stDispDevAttr[u32DispDev].eIntfSync)
            {
                printf("disp timing err\n");
                exit(-1);
            }

            CarDV_DispLayerAttr_t *pstDispLayerAttr = NULL;
            for (u32DispLayer = 0; u32DispLayer < CARDV_MAX_DISP_LAYER_NUM; u32DispLayer++)
            {
                MI_BOOL bLayerUsed = FALSE;
                sprintf(key, "Disp:Dev%dLayer%d", u32DispDev, u32DispLayer);
                bLayerUsed = iniparser_getint(pstDict, key, 0);
                if (bLayerUsed == FALSE)
                    continue;

                pstDispLayerAttr = &pstDispModAttr->stDispDevAttr[u32DispDev].stDispLayerAttr[u32DispLayer];
                if (TRUE == pstDispLayerAttr->bUsed)
                {
                    continue;
                }

                pstDispLayerAttr->bUsed      = TRUE;
                pstDispLayerAttr->s32LayerId = u32DispDev == 0 ? u32DispLayer : u32DispLayer + 2;
                sprintf(key, "Disp:Dev%dLayer%dWinW", u32DispDev, u32DispLayer);
                pstDispLayerAttr->stDispLayerWin.u16Width = iniparser_getint(pstDict, key, 0);
                sprintf(key, "Disp:Dev%dLayer%dWinH", u32DispDev, u32DispLayer);
                pstDispLayerAttr->stDispLayerWin.u16Height = iniparser_getint(pstDict, key, 0);
                sprintf(key, "Disp:Dev%dLayer%dWinX", u32DispDev, u32DispLayer);
                pstDispLayerAttr->stDispLayerWin.u16X = iniparser_getint(pstDict, key, 0);
                sprintf(key, "Disp:Dev%dLayer%dWinY", u32DispDev, u32DispLayer);
                pstDispLayerAttr->stDispLayerWin.u16Y = iniparser_getint(pstDict, key, 0);
                sprintf(key, "Disp:Dev%dLayer%dW", u32DispDev, u32DispLayer);
                pstDispLayerAttr->u16Width = iniparser_getint(pstDict, key, 0);
                sprintf(key, "Disp:Dev%dLayer%dH", u32DispDev, u32DispLayer);
                pstDispLayerAttr->u16Height = iniparser_getint(pstDict, key, 0);

                MI_U32 u32PortNum =
                    (0 == u32DispLayer) ? CARDV_MAX_DISP_LAYER0_PORT_NUM : CARDV_MAX_DISP_LAYER1_PORT_NUM;
                CarDV_DispPortAttr_t *pstDispPortAttr = NULL;

                for (u32DispPort = 0; u32DispPort < u32PortNum; u32DispPort++)
                {
                    MI_BOOL bPortUsed = FALSE;
                    sprintf(key, "Disp:Dev%dLayer%dPort%d", u32DispDev, u32DispLayer, u32DispPort);
                    bPortUsed = iniparser_getint(pstDict, key, 0);
                    if (bPortUsed == FALSE)
                        continue;

                    pstDispPortAttr = &pstDispLayerAttr->stDispPortAttr[u32DispPort];
                    if (TRUE == pstDispPortAttr->bUsed)
                    {
                        continue;
                    }

                    pstDispPortAttr->bUsed = TRUE;
                    sprintf(key, "Disp:Dev%dLayer%dPort%dX", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispPortWin.u16X = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Disp:Dev%dLayer%dPort%dY", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispPortWin.u16Y = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Disp:Dev%dLayer%dPort%dW", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispPortWin.u16Width = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Disp:Dev%dLayer%dPort%dH", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispPortWin.u16Height = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Disp:Dev%dLayer%dPort%dsrcW", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->u16SrcWidth = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Disp:Dev%dLayer%dPort%dsrcH", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->u16SrcHeight = iniparser_getint(pstDict, key, 0);

                    sprintf(key, "Disp:Dev%dLayer%dPort%dZoomX", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispZoomInWin.u16X = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Disp:Dev%dLayer%dPort%dZoomY", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispZoomInWin.u16Y = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Disp:Dev%dLayer%dPort%dZoomW", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispZoomInWin.u16Width = iniparser_getint(pstDict, key, 0);
                    sprintf(key, "Disp:Dev%dLayer%dPort%dZoomH", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispZoomInWin.u16Height = iniparser_getint(pstDict, key, 0);

                    sprintf(key, "Disp:Dev%dLayer%dPort%dInMod", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispInBindParam.stChnPort.eModId =
                        (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
                    sprintf(key, "Disp:Dev%dLayer%dPort%dInDev", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispInBindParam.stChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
                    sprintf(key, "Disp:Dev%dLayer%dPort%dInChn", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispInBindParam.stChnPort.u32ChnId = iniparser_getint(pstDict, key, -1);
                    sprintf(key, "Disp:Dev%dLayer%dPort%dInPort", u32DispDev, u32DispLayer, u32DispPort);
                    pstDispPortAttr->stDispInBindParam.stChnPort.u32PortId = iniparser_getint(pstDict, key, -1);
                    pstDispPortAttr->stDispInBindParam.u32SrcFrmrate       = 30;
                    pstDispPortAttr->stDispInBindParam.u32DstFrmrate       = 30;
                    pstDispPortAttr->stDispInBindParam.eBindType           = E_MI_SYS_BIND_TYPE_FRAME_BASE;
                }
            }
        }
    }

    // WBC
    bDevUse = iniparser_getint(pstDict, "Disp:Wbc", 0);
    if (bDevUse && FALSE == pstDispModAttr->stDispWbcAttr.bUsed)
    {
        pstDispModAttr->stDispWbcAttr.bUsed = TRUE;
        pstDispModAttr->stDispWbcAttr.eSourceType =
            (MI_DISP_WBC_SourceType_e)iniparser_getint(pstDict, "Disp:WbcSrcType", MI_DISP_WBC_SOURCE_DEV);
        pstDispModAttr->stDispWbcAttr.u32SourceId = iniparser_getint(pstDict, "Disp:WbcSrcId", 0);
        pstDispModAttr->stDispWbcAttr.u32Width    = iniparser_getint(pstDict, "Disp:WbcW", 720);
        pstDispModAttr->stDispWbcAttr.u32Height   = iniparser_getint(pstDict, "Disp:WbcH", 576);
    }

    for (u8BindInfo = 0; u8BindInfo < CARDV_MAX_DISP_BIND_INFO; u8BindInfo++)
    {
        sprintf(key, "Disp:Bind%dSrcMod", u8BindInfo);
        pstDispBindAttr->stBindInfo[u8BindInfo].stSrcChnPort.eModId =
            (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
        sprintf(key, "Disp:Bind%dSrcDev", u8BindInfo);
        pstDispBindAttr->stBindInfo[u8BindInfo].stSrcChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
        sprintf(key, "Disp:Bind%dSrcChn", u8BindInfo);
        pstDispBindAttr->stBindInfo[u8BindInfo].stSrcChnPort.u32ChnId = iniparser_getint(pstDict, key, -1);
        sprintf(key, "Disp:Bind%dSrcPort", u8BindInfo);
        pstDispBindAttr->stBindInfo[u8BindInfo].stSrcChnPort.u32PortId = iniparser_getint(pstDict, key, -1);
        sprintf(key, "Disp:Bind%dDstMod", u8BindInfo);
        pstDispBindAttr->stBindInfo[u8BindInfo].stDstChnPort.eModId =
            (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
        sprintf(key, "Disp:Bind%dDstDev", u8BindInfo);
        pstDispBindAttr->stBindInfo[u8BindInfo].stDstChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
        sprintf(key, "Disp:Bind%dDstChn", u8BindInfo);
        pstDispBindAttr->stBindInfo[u8BindInfo].stDstChnPort.u32ChnId = iniparser_getint(pstDict, key, -1);
        sprintf(key, "Disp:Bind%dDstPort", u8BindInfo);
        pstDispBindAttr->stBindInfo[u8BindInfo].stDstChnPort.u32PortId = iniparser_getint(pstDict, key, -1);

        pstDispBindAttr->stBindInfo[u8BindInfo].u32SrcFrmrate = 30;
        pstDispBindAttr->stBindInfo[u8BindInfo].u32DstFrmrate = 30;
        pstDispBindAttr->stBindInfo[u8BindInfo].eBindType     = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    }

    pstDispBindAttr->u32BindMapNum = iniparser_getint(pstDict, "Disp:BindMapNum", 0);
    for (u8BindInfo = 0; u8BindInfo < pstDispBindAttr->u32BindMapNum; u8BindInfo++)
    {
        sprintf(key, "Disp:BindMap%d", u8BindInfo);
        pstDispBindAttr->u32BindMap[u8BindInfo] = iniparser_getint(pstDict, key, 0);
    }

    pstDispBindAttr->u32BindPbMapNum = iniparser_getint(pstDict, "Disp:BindPBMapNum", 0);
    for (u8BindInfo = 0; u8BindInfo < CARDV_MAX_DISP_BIND_INFO; u8BindInfo++)
    {
        sprintf(key, "Disp:Bind%dPBSrcMod", u8BindInfo);
        pstDispBindAttr->stPlayBackBindInfo[u8BindInfo].stSrcChnPort.eModId =
            (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
        sprintf(key, "Disp:Bind%dPBSrcDev", u8BindInfo);
        pstDispBindAttr->stPlayBackBindInfo[u8BindInfo].stSrcChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
        sprintf(key, "Disp:Bind%dPBSrcChn", u8BindInfo);
        pstDispBindAttr->stPlayBackBindInfo[u8BindInfo].stSrcChnPort.u32ChnId = iniparser_getint(pstDict, key, -1);
        sprintf(key, "Disp:Bind%dPBSrcPort", u8BindInfo);
        pstDispBindAttr->stPlayBackBindInfo[u8BindInfo].stSrcChnPort.u32PortId = iniparser_getint(pstDict, key, -1);

        sprintf(key, "Disp:Bind%dPBDstMod", u8BindInfo);
        pstDispBindAttr->stPlayBackBindInfo[u8BindInfo].stDstChnPort.eModId =
            (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
        sprintf(key, "Disp:Bind%dPBDstDev", u8BindInfo);
        pstDispBindAttr->stPlayBackBindInfo[u8BindInfo].stDstChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
        sprintf(key, "Disp:Bind%dPBDstChn", u8BindInfo);
        pstDispBindAttr->stPlayBackBindInfo[u8BindInfo].stDstChnPort.u32ChnId = iniparser_getint(pstDict, key, -1);
        sprintf(key, "Disp:Bind%dPBDstPort", u8BindInfo);
        pstDispBindAttr->stPlayBackBindInfo[u8BindInfo].stDstChnPort.u32PortId = iniparser_getint(pstDict, key, -1);

        pstDispBindAttr->stPlayBackBindInfo[u8BindInfo].u32SrcFrmrate = 30;
        pstDispBindAttr->stPlayBackBindInfo[u8BindInfo].u32DstFrmrate = 30;
        pstDispBindAttr->stPlayBackBindInfo[u8BindInfo].eBindType     = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    }

    carInfo.stDispInfo.stPlayBackChnPort.eModId    = pstDispBindAttr->stPlayBackBindInfo[0].stSrcChnPort.eModId;
    carInfo.stDispInfo.stPlayBackChnPort.u32DevId  = pstDispBindAttr->stPlayBackBindInfo[0].stSrcChnPort.u32DevId;
    carInfo.stDispInfo.stPlayBackChnPort.u32ChnId  = pstDispBindAttr->stPlayBackBindInfo[0].stSrcChnPort.u32ChnId;
    carInfo.stDispInfo.stPlayBackChnPort.u32PortId = pstDispBindAttr->stPlayBackBindInfo[0].stSrcChnPort.u32PortId;
    carInfo.stDispInfo.u8SwitchNum                 = pstDispBindAttr->u32BindMapNum;
    IPC_CarInfo_Write_DispInfo(&carInfo.stDispInfo);
}
#endif
#if (CARDV_VDF_ENABLE)
void cardv_init_vdfParam(dictionary *pstDict)
{
    char                    key[64];
    MI_U32                  u32VdfChn    = 0;
    CarDV_VdfModAttr_t *    pstVdfModule = &gstVdfModule;
    CarDV_VdfChannelAttr_t *pstVdfChnAttr;

    for (u32VdfChn = 0; u32VdfChn < VDF_CHANNEL_NUM; u32VdfChn++)
    {
        pstVdfChnAttr = &pstVdfModule->stVdfChnAttr[u32VdfChn];
        sprintf(key, "VDF:Chn%d", u32VdfChn);
        pstVdfChnAttr->bUsed = iniparser_getint(pstDict, key, 0);
        if (pstVdfChnAttr->bUsed)
        {
            sprintf(key, "VDF:Chn%dsrcMod", u32VdfChn);
            pstVdfChnAttr->stBindParam.stChnPort.eModId =
                (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
            sprintf(key, "VDF:Chn%dsrcDev", u32VdfChn);
            pstVdfChnAttr->stBindParam.stChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
            sprintf(key, "VDF:Chn%dsrcChn", u32VdfChn);
            pstVdfChnAttr->stBindParam.stChnPort.u32ChnId = iniparser_getint(pstDict, key, -1);
            sprintf(key, "VDF:Chn%dsrcPort", u32VdfChn);
            pstVdfChnAttr->stBindParam.stChnPort.u32PortId = iniparser_getint(pstDict, key, -1);
            sprintf(key, "VDF:Chn%dsrcFps", u32VdfChn);
            pstVdfChnAttr->stBindParam.u32SrcFrmrate = iniparser_getint(pstDict, key, 30);
            sprintf(key, "VDF:Chn%ddstFps", u32VdfChn);
            pstVdfChnAttr->stBindParam.u32DstFrmrate = iniparser_getint(pstDict, key, 5);
            pstVdfChnAttr->stBindParam.eBindType     = E_MI_SYS_BIND_TYPE_FRAME_BASE;
            pstVdfChnAttr->u8ChnId                   = u32VdfChn;
            if (pstVdfChnAttr->stBindParam.stChnPort.eModId != E_MI_MODULE_ID_MAX)
            {
                pstVdfModule->bUsed = TRUE;
            }
        }
    }
}
#endif

#if (CARDV_ADAS_ENABLE)
void cardv_init_adasParam(dictionary *pstDict)
{
    char              key[64];
    MI_SYS_ChnPort_t *pstChnPort;

    pstChnPort = &gFrontAdasAttr.stSrcChnPort;

    sprintf(key, "ADAS:srcMod");
    pstChnPort->eModId = (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);

    sprintf(key, "ADAS:srcDev");
    pstChnPort->u32DevId = iniparser_getint(pstDict, key, -1);

    sprintf(key, "ADAS:srcChn");
    pstChnPort->u32ChnId = iniparser_getint(pstDict, key, -1);

    sprintf(key, "ADAS:srcPort");
    pstChnPort->u32PortId = iniparser_getint(pstDict, key, -1);

    pstChnPort = &gRearAdasAttr.stSrcChnPort;

    sprintf(key, "ADAS_REAR:srcMod");
    pstChnPort->eModId = (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);

    sprintf(key, "ADAS_REAR:srcDev");
    pstChnPort->u32DevId = iniparser_getint(pstDict, key, -1);

    sprintf(key, "ADAS_REAR:srcChn");
    pstChnPort->u32ChnId = iniparser_getint(pstDict, key, -1);

    sprintf(key, "ADAS_REAR:srcPort");
    pstChnPort->u32PortId = iniparser_getint(pstDict, key, -1);
}
#endif

#if (CARDV_LD_ENABLE)
void cardv_init_lightDetectParam(dictionary *pstDict)
{
    char key[64];
    sprintf(key, "LD:srcMod");
    gstLDSrcModule.eModId = (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
    sprintf(key, "LD:srcDev");
    gstLDSrcModule.u32DevId = iniparser_getint(pstDict, key, -1);
    sprintf(key, "LD:srcChn");
    gstLDSrcModule.u32ChnId = iniparser_getint(pstDict, key, -1);
    sprintf(key, "LD:srcPort");
    gstLDSrcModule.u32PortId = iniparser_getint(pstDict, key, -1);
}
#endif

#if (CARDV_MIPITX_ENABLE)
void cardv_init_mipitxParam(dictionary *pstDict)
{
    char                   key[64];
    CarDV_MipiTxChnAttr_t *pstMipiTxChnAttr = &gstMipiTxModule.stMipiTxChnAttr;

    sprintf(key, "MIPITX:use");
    pstMipiTxChnAttr->bUsed = iniparser_getint(pstDict, key, 0);
    sprintf(key, "MIPITX:W");
    pstMipiTxChnAttr->u32Width = iniparser_getint(pstDict, key, 0);
    sprintf(key, "MIPITX:H");
    pstMipiTxChnAttr->u32Height = iniparser_getint(pstDict, key, 0);
    sprintf(key, "MIPITX:srcMod");
    pstMipiTxChnAttr->stSrcChnPort.eModId = (MI_ModuleId_e)iniparser_getint(pstDict, key, E_MI_MODULE_ID_MAX);
    sprintf(key, "MIPITX:srcDev");
    pstMipiTxChnAttr->stSrcChnPort.u32DevId = iniparser_getint(pstDict, key, -1);
    sprintf(key, "MIPITX:srcChn");
    pstMipiTxChnAttr->stSrcChnPort.u32ChnId = iniparser_getint(pstDict, key, -1);
    sprintf(key, "MIPITX:srcPort");
    pstMipiTxChnAttr->stSrcChnPort.u32PortId = iniparser_getint(pstDict, key, -1);
}
#endif

void cardv_init_audioParam(void)
{
    memset(&g_audioInParam[0], 0x00, sizeof(CarDVAudioInParam));
    g_audioInParam[0].u32UserFrameDepth    = 18;
    g_audioInParam[0].u32BufQueueDepth     = 20;
    g_audioInParam[0].bAudioInMute         = g_bAudioInMute;
    g_audioInParam[0].stAudioAttr.enFormat = E_MI_AUDIO_FORMAT_PCM_S16_LE; // AAC only support AV_SAMPLE_FMT_S16 now.
    g_audioInParam[0].stAudioAttr.enSampleRate  = E_MI_AUDIO_SAMPLE_RATE_16000;
    g_audioInParam[0].stAudioAttr.enSoundMode   = E_MI_AUDIO_SOUND_MODE_MONO;
    g_audioInParam[0].stAudioAttr.u32PeriodSize = 1024;

    g_audioInParam[0].u32DevId = MI_AI_DEV_1;
    g_audioInParam[0].u8ChnId  = 0;
    g_audioInParam[0].enAiIf   = E_MI_AI_IF_ADC_AB; // 1:ADC_AB; 2:ADC_CD; 3:DMIC_A_01; 4:DMIC_A_23 default:0:NONE

    audioVqeVolumeTrans(g_audioInParam[0].u32DevId, &g_s8AudioInVolume);
    g_audioInParam[0].s8VolumeInDb    = g_s8AudioInVolume;
    g_audioInParam[0].u32CompBufSize  = 0x80000;
    g_audioInParam[0].eAudioInEncType = AUDIO_IN_AAC;
    // Note: mpegts support AAC/MP3. mp4 or mov support AAC/PCM/MP3.
    // MP3 & AAC need ffmpeg libavcodec support.
    if (g_audioInParam[0].eAudioInEncType == AUDIO_IN_AAC)
        g_audioInParam[0].stAudioAttr.u32PeriodSize = 1024;
    else if (g_audioInParam[0].eAudioInEncType == AUDIO_IN_MP3)
        g_audioInParam[0].stAudioAttr.u32PeriodSize = 576;

    // Audio Out
    memset(&g_audioOutParam[0], 0x00, sizeof(CarDVAudioOutParam));

    g_audioOutParam[0].u32DevId            = MI_AO_DEV_1;
    g_audioOutParam[0].enAoIfs             = E_MI_AO_IF_DAC_AB;
    g_audioOutParam[0].s8VolumeOutDb       = g_s8volume * (MAX_AO_VOLUME - MIN_AO_VOLUME) / 10 + MIN_AO_VOLUME;
    g_audioOutParam[0].eAudioOutGainFading = E_MI_AO_GAIN_FADING_64_SAMPLE;

    g_audioOutParam[0].stAoDevAttr.enFormat      = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    g_audioOutParam[0].stAoDevAttr.u32PeriodSize = 1024;
    g_audioOutParam[0].stAoDevAttr.enSoundMode   = E_MI_AUDIO_SOUND_MODE_MONO;
    g_audioOutParam[0].stAoDevAttr.enSampleRate  = E_MI_AUDIO_SAMPLE_RATE_48000;
    g_audioOutParam[0].stAoDevAttr.enChannelMode = E_MI_AO_CHANNEL_MODE_ONLY_LEFT;

#if (MAX_AUDIO_OUTPUT > 1)
    memset(&g_audioOutParam[1], 0x00, sizeof(CarDVAudioOutParam));
    g_audioOutParam[1].u32DevId = MI_AO_DEV_1;

    g_audioOutParam[1].s8VolumeOutDb       = g_s8volume * (MAX_AO_VOLUME - MIN_AO_VOLUME) / 10 + MIN_AO_VOLUME;
    g_audioOutParam[1].eAudioOutGainFading = E_MI_AO_GAIN_FADING_64_SAMPLE;

    g_audioOutParam[1].stAoDevAttr.u32PeriodSize = 1024;
    g_audioOutParam[1].stAoDevAttr.enFormat      = E_MI_AUDIO_FORMAT_PCM_S16_LE; // now only support bit_width_16

    g_audioOutParam[1].stAoDevAttr.enSoundMode  = E_MI_AUDIO_SOUND_MODE_MONO;
    g_audioOutParam[1].stAoDevAttr.enSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;

    // I2S setting
    g_audioOutParam[1].enAoIfs                 = E_MI_AO_IF_I2S_A;
    g_audioOutParam[1].stI2sConfig.enMode      = E_MI_AUDIO_I2S_MODE_I2S_MASTER;
    g_audioOutParam[1].stI2sConfig.bSyncClock  = FALSE;
    g_audioOutParam[1].stI2sConfig.enFormat    = E_MI_AUDIO_I2S_FMT_I2S_MSB;
    g_audioOutParam[1].stI2sConfig.enMclk      = E_MI_AUDIO_I2S_MCLK_12_288M;
    g_audioOutParam[1].stI2sConfig.u32TdmSlots = 2;
    g_audioOutParam[1].stI2sConfig.enBitWidth  = E_MI_AUDIO_BIT_WIDTH_16;
#endif
}

void cardv_parserIni(char *pIniPath)
{
    MI_U8       u8CamNum;
    MI_U8       u8CamID;
    dictionary *pstDict = iniparser_load(pIniPath);
    if (pstDict == NULL)
    {
        printf("ini NOT exist\n");
        exit(-1);
    }

    u8CamNum = iniparser_getint(pstDict, ":CamNum", 0);
    if (u8CamNum == 0)
    {
        printf("camera NOT exist\n");
        iniparser_freedict(pstDict);
        exit(-1);
    }

    if (u8CamNum > MAX_CAM_NUM)
    {
        printf("enlarge max cam num\n");
        exit(-1);
    }

    memset(&gstSensorAttr, 0, sizeof(CarDV_Sensor_Attr_t) * CARDV_MAX_SENSOR_NUM);
    memset(&gstIspModule, 0, sizeof(CarDV_IspModAttr_t));
#if (CARDV_LDC_ENABLE)
    memset(&gstLdcModule, 0, sizeof(CarDV_LdcModAttr_t));
#endif
    memset(&gstSclModule, 0, sizeof(CarDV_SclModAttr_t));
#if (CARDV_VDISP_ENABLE)
    memset(&gstVdispModule, 0, sizeof(CarDV_VdispModAttr_t));
#endif
    memset(&gstVencAttr, 0, sizeof(CarDV_VencAttr_t) * MAX_VIDEO_NUMBER);
    memset(&gstMuxerAttr, 0, sizeof(CarDV_MuxerAttr_t) * MAX_MUXER_NUMBER);
#if (CARDV_JPD_ENABLE)
    memset(&gstJpdModule, 0, sizeof(CarDV_JpdModAttr_t));
#endif
#if (CARDV_VDEC_ENABLE)
    memset(&gstVdecModule, 0, sizeof(CarDV_VdecModAttr_t));
#endif
    memset(&gstOsdAttr, 0, sizeof(CarDV_OsdAttr));
#if (CARDV_DISPLAY_ENABLE)
    memset(&gstDispModule, 0, sizeof(CarDV_DispModAttr_t));
#endif
#if (CARDV_MIPITX_ENABLE)
    memset(&gstMipiTxModule, 0, sizeof(CarDV_MipiTxModAttr_t));
#endif
#if (CARDV_VDF_ENABLE)
    memset(&gstVdfModule, 0, sizeof(CarDV_VdfModAttr_t));
#endif
#if (CARDV_ADAS_ENABLE)
    memset(&gFrontAdasAttr.stSrcChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&gRearAdasAttr.stSrcChnPort, 0, sizeof(MI_SYS_ChnPort_t));
#endif
#if (CARDV_LD_ENABLE)
    memset(&gstLDSrcModule, 0, sizeof(MI_SYS_ChnPort_t));
#endif

    for (u8CamID = 0; u8CamID < u8CamNum; u8CamID++)
    {
        g_camExisted[u8CamID] = TRUE;
#if (CARDV_RTSP_INPUT_ENABLE)
        cardv_init_inputRtspParam(pstDict, u8CamID);
#elif (CARDV_WS_INPUT_ENABLE)
        cardv_init_inputWsParam(pstDict, u8CamID);
#endif
        cardv_init_vifParam(pstDict, u8CamID);
        cardv_init_ispParam(pstDict, u8CamID);
#if (CARDV_LDC_ENABLE)
        cardv_init_ldcParam(pstDict, u8CamID);
#endif
        cardv_init_sclParam(pstDict, u8CamID, FALSE);
#if (CARDV_VDISP_ENABLE)
        cardv_init_vdispParam(pstDict, u8CamID);
#endif
        cardv_init_vencParam(pstDict, u8CamID);
        cardv_init_muxerParam(pstDict, u8CamID);
#if (CARDV_JPD_ENABLE)
        cardv_init_jpdParam(pstDict, u8CamID);
#endif
#if (CARDV_VDEC_ENABLE)
        cardv_init_vdecParam(pstDict, u8CamID);
#endif
    }

    cardv_init_osdParam(pstDict);
#if (CARDV_DISPLAY_ENABLE)
    cardv_init_dispParam(pstDict);
#endif
#if (CARDV_MIPITX_ENABLE)
    cardv_init_mipitxParam(pstDict);
#endif
#if (CARDV_VDF_ENABLE)
    cardv_init_vdfParam(pstDict);
#endif
#if (CARDV_ADAS_ENABLE)
    cardv_init_adasParam(pstDict);
#endif
#if (CARDV_LD_ENABLE)
    cardv_init_lightDetectParam(pstDict);
#endif
    iniparser_freedict(pstDict);
}

MI_S32 main(MI_S32 argc, char **argv)
{
    if (argc != 2)
    {
        printf("please run [cardv *.ini &]\n");
        exit(-1);
    }

#if (CARDV_RTSP_INPUT_ENABLE || CARDV_WS_INPUT_ENABLE)
    IpcServerStart();
#endif

    IPC_CarInfo_Open();
    IPC_CarInfo_Write(&carInfo);

    cardv_menu_updateSettingFromConfig();

    g_bPowerOnByGsensor = GsensorIsPowerOnByInt();
    if (g_bPowerOnByGsensor)
        g_u64NormalRecTime = 20 * 1000000;

    cardv_parserIni(argv[1]);
    cardv_init_audioParam();

    cardv_menu_updateVideoRecordSize(g_recordWidth[0], g_recordHeight[0]);

    // only the sensor frame rate effect
    for (int i = 0; i < MAX_VIDEO_NUMBER; i++)
    {
        if (gstVencAttr[i].bUsed == FALSE)
            continue;

        // set IPQPDelta
        if (gstVencAttr[i].u8EncoderType == VE_H264)
            gstVencAttr[i].s32IPQPDelta = 3;
        else if (gstVencAttr[i].u8EncoderType == VE_H265)
            gstVencAttr[i].s32IPQPDelta = 1;
    }

    if (cardv_message_queue_init())
    {
        printf("cardv initial message queue error!\n");
        exit(1);
    }

#if (CARDV_WATCHDOG_ENABLE)
    cardv_send_cmd_HP(CMD_SYSTEM_WATCHDOG_OPEN, NULL, 0);
#endif

    cardv_update_status(REC_STATUS, "0", 2);
    cardv_update_status(MMC_STATUS, "0", 2);

    // system core back trace
    cardv_send_cmd_HP(CMD_SYSTEM_CORE_BACKTRACE, NULL, 0);

    printf("\n");
    printf("    cardv app version: %s\n", CARDV_VER);

    // register exit function
    struct sigaction sigAction;
    sigAction.sa_handler = signalStop;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGHUP, &sigAction, NULL);  //-1
    sigaction(SIGINT, &sigAction, NULL);  //-2
    sigaction(SIGQUIT, &sigAction, NULL); //-3
    sigaction(SIGKILL, &sigAction, NULL); //-9
    sigaction(SIGTERM, &sigAction, NULL); //-15

    // create cardv_fifo file
    CREATE_FIFO(FIFO_NAME);
    CREATE_FIFO(FIFO_NAME_HIGH_PRIO);

    cardvResume(TRUE);

#if (CARDV_GPS_ENABLE)
    GpsStartThread();
#endif
    GsensorStartThread();

    cardvInputMessageLoop();

    // start exit cardv:
    sync();
    cardvExit();
    printf("cardv app exit\n");
    exit(0);
}
