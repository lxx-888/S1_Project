/*************************************************
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 * All rights reserved.
 *
 **************************************************
 * File name:  module_mux.c
 * Author:     jeff.li@sigmastar.com.cn
 * Version:    Initial Draft
 * Date:       2019/1/25
 * Description: muxer module source file
 *
 *
 *
 * History:
 *
 *    1. Date  :        2019/1/25
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
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>

#include "module_common.h"
#include "module_gps.h"
#include "module_gsensor.h"
#include "module_mux.h"
#include "module_system.h"
#include "module_storage.h"
#include "module_video.h"
#include "mid_common.h"
#include "DCF.h"
#include "IPC_cardvInfo.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

CarDV_MuxerAttr_t gstMuxerAttr[MAX_MUXER_NUMBER]     = {0};
MI_Muxer *        g_muxerArray[MAX_MUXER_NUMBER]     = {NULL};
int               g_muxerNormalNum                   = 0;
int               g_muxerEmergNum                    = 0;
int               g_muxerShareNum                    = 0;
static int        g_muxerFileFullCnt[MUXER_TYPE_NUM] = {0};

//==============================================================================
//
//                              EXTERN GLOBAL VARIABLES
//
//==============================================================================

extern BOOL       g_bVideoStart;
extern CarDV_Info carInfo;
extern BOOL       g_camExisted[];
extern BOOL       g_bPowerOnByGsensor;
extern BOOL       g_bRecMdtTrigger;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void MuxerErrHandler(MI_U8 u8CamID, MuxerType_e eMuxerType, MuxerFile_e eMuxerFile)
{
    cardv_send_to_fifo(CARDV_CMD_STOP_REC, sizeof(CARDV_CMD_STOP_REC));
}

void MuxerOpenFileErrHandler(MI_U8 u8CamID, MuxerType_e eMuxerType, MuxerFile_e eMuxerFile)
{
    StorageReMount();
}

void MuxerPostFileHandler(MuxerType_e eMuxerType, MuxerFile_e eMuxerFile, int camId, char *fileName)
{
    /* Update file size */
    int DB;
    switch (eMuxerType)
    {
    case MUXER_NORMAL:
        if (g_bPowerOnByGsensor || g_bRecMdtTrigger)
            DB = DB_PARKING;
        else
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

    if (eMuxerFile == MUXER_MAIN_FLIE)
        DCF_UpdateFileSizeByName(DB, camId, fileName);
    else
        DCF_UpdateSubFileSizeByName(DB, camId, fileName);
}

void MuxerProcessFileFullHandler(MuxerType_e eMuxerType)
{
    g_muxerFileFullCnt[eMuxerType]++;

    if (eMuxerType == MUXER_NORMAL && g_muxerFileFullCnt[eMuxerType] == g_muxerNormalNum)
    {
        if (g_bPowerOnByGsensor)
        {
            g_muxerFileFullCnt[eMuxerType] = 0;
            sync();
            system("poweroff");
        }
        else if (g_bRecMdtTrigger)
        {
            g_bRecMdtTrigger               = FALSE;
            g_muxerFileFullCnt[eMuxerType] = 0;
            carInfo.stRecInfo.bMuxingMD    = FALSE;
        }
        else
        {
            // Only normal files in DB [NORMAL] need seamless recording.
            printf("\nSeamless\n");
            cardv_send_to_fifo(CARDV_CMD_SEAMLESS, sizeof(CARDV_CMD_SEAMLESS));
            g_muxerFileFullCnt[eMuxerType] = 0;
        }
    }

    if (eMuxerType == MUXER_EMERG && g_muxerFileFullCnt[eMuxerType] == g_muxerEmergNum)
    {
        g_muxerFileFullCnt[eMuxerType] = 0;
        carInfo.stRecInfo.bMuxingEmerg = FALSE;
        IPC_CarInfo_Write_RecInfo(&carInfo.stRecInfo);
        cardv_update_status(REC_STATUS, "1", 2);
        cardv_send_to_fifo(CARDV_CMD_SEAMLESS, sizeof(CARDV_CMD_SEAMLESS));
    }

    if (eMuxerType == MUXER_SHARE && g_muxerFileFullCnt[eMuxerType] == g_muxerShareNum)
    {
        g_muxerFileFullCnt[eMuxerType] = 0;
        carInfo.stRecInfo.bMuxingShare = FALSE;
        IPC_CarInfo_Write_RecInfo(&carInfo.stRecInfo);
    }
}

const char *MuxerGetMp4TrakName(MuxerType_e eMuxerType, MuxerFile_e eMuxerFile, MuxerMp4TrakType eMuxerTrakType)
{
    switch (eMuxerTrakType)
    {
    case MUXER_VIDEO:
        return "SS VIDEO";
    case MUXER_AUDIO:
        return "SS AUDIO";
    case MUXER_THUMB:
        return "SS THUMB";
    case MUXER_GPS:
        return "SS GPS";
    case MUXER_GSENSOR:
        return "SS GSNR";
    }

    return "SS";
}

int MuxerOpen(CarDV_MuxerAttr_t *muxerParam)
{
    for (int i = 0; i < MAX_MUXER_NUMBER; i++)
    {
        if (muxerParam[i].bUsed == FALSE)
        {
            g_muxerArray[i] = NULL;
            continue;
        }

        if (g_muxerArray[i] == NULL)
        {
            g_muxerArray[i] = MI_Muxer::createNew(muxerParam[i].u8CamId, muxerParam[i].u8VidTrackNum);
            if (g_muxerArray[i])
            {
                g_muxerArray[i]->SetMuxerType(muxerParam[i].eMuxerType);
                g_muxerArray[i]->SetMuxerFile(muxerParam[i].eMuxerFile);
                g_muxerArray[i]->SetGpsTrack(muxerParam[i].bGpsTrack);
                g_muxerArray[i]->SetGsnrTrack(muxerParam[i].bGsnrTrack);
                g_muxerArray[i]->SetThumbnailTrack(muxerParam[i].bThumbTrack);
                g_muxerArray[i]->SetThumbStreamId(VideoMapToIndex(muxerParam[i].u8CamId, VIDEO_STREAM_THUMB));
                g_muxerArray[i]->SetFrameRate(MUXER_FPS_NUM, MUXER_FPS_DEN, FALSE, MUXER_SLOW_MOTION_NONE);
                for (int j = 0; j < muxerParam[i].u8VidTrackNum; j++)
                    g_muxerArray[i]->SetStreamId(j, muxerParam[i].u8VencChn[j]);
            }
        }

        MuxerSetLimitTime(muxerParam[i].eMuxerType, muxerParam[i].u64LimitTime);
    }

    MuxerSetErrCallback(MuxerErrHandler);
    MuxerSetOpenFileErrCallback(MuxerOpenFileErrHandler);
    MuxerSetPostFileCallback(MuxerPostFileHandler);
    MuxerSetProcessFileFullCallback(MuxerProcessFileFullHandler);
    MuxerSetMp4TrakNameCallback(MuxerGetMp4TrakName);
    MuxerStartThread();

    return 0;
}

int MuxerClose(void)
{
    MuxerStopThread();

    for (int i = 0; i < MAX_MUXER_NUMBER; i++)
    {
        if (g_muxerArray[i])
        {
            delete g_muxerArray[i];
            g_muxerArray[i] = NULL;
        }
    }

    g_muxerNormalNum = 0;
    g_muxerEmergNum  = 0;
    g_muxerShareNum  = 0;

    return 0;
}

MI_S32 mux_process_cmd(CarDVCmdId id, MI_S8 *param, MI_S32 paramLen)
{
    switch (id)
    {
    case CMD_MUX_OPEN:
        MuxerOpen(gstMuxerAttr);
        break;

    case CMD_MUX_CLOSE:
        MuxerClose();
        break;

    case CMD_MUX_PRE_START:
        g_bVideoStart    = TRUE;
        g_muxerNormalNum = 0;
        g_muxerEmergNum  = 0;
        g_muxerShareNum  = 0;
        for (int i = 0; i < MAX_MUXER_NUMBER; i++)
        {
            if (g_muxerArray[i] == NULL)
                continue;

            if (g_camExisted[g_muxerArray[i]->GetMuxerCamID()])
                g_muxerArray[i]->MuxerEnable(TRUE);
            else
                g_muxerArray[i]->MuxerEnable(FALSE);

            if (g_muxerArray[i]->IsMuxerEnable() && g_muxerArray[i]->GetMuxerType() == MUXER_NORMAL)
                g_muxerNormalNum++;

            if (g_muxerArray[i]->IsMuxerEnable() && g_muxerArray[i]->GetMuxerType() == MUXER_EMERG)
                g_muxerEmergNum++;

            if (g_muxerArray[i]->IsMuxerEnable() && g_muxerArray[i]->GetMuxerType() == MUXER_SHARE)
                g_muxerShareNum++;
        }

        for (int i = 0; i < MAX_CAM_NUM; i++)
        {
            if (g_camExisted[i])
            {
                MuxerSetCheckTimeFullRefCamId(i);
                break;
            }
        }

#if (CARDV_GPS_ENABLE)
        GpsStartRec(TRUE);
#endif
        GsensorStartRec(TRUE);
        MuxerSendPreStart();
        break;

    case CMD_MUX_START: {
        MI_S8       muxerParam = param[0];
        MuxerType_e eMuxerType;
        time_t      timep;
        char        fileName[64];
        int         DB;
        int         ret = -1;

        eMuxerType = (MuxerType_e)muxerParam;
        switch (eMuxerType)
        {
        case MUXER_NORMAL:
            if (g_bPowerOnByGsensor || g_bRecMdtTrigger)
                DB = DB_PARKING;
            else
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

        time(&timep);
        for (int i = 0; i < MAX_MUXER_NUMBER; i++)
        {
            if (g_muxerArray[i] == NULL)
                continue;

            if (g_muxerArray[i]->IsMuxerEnable() && g_muxerArray[i]->GetMuxerType() == eMuxerType)
            {
                if (g_muxerArray[i]->GetMuxerFile() == MUXER_MAIN_FLIE)
                {
                    do
                    {
                        ret = DCF_CreateFileName(DB, g_muxerArray[i]->GetMuxerCamID(), fileName, timep,
                                                 CARDV_VIDEO_SUFFIX);
                        if (ret == -1)
                            StorageReMount();
                    } while (ret == -1);

                    if (ret == -2)
                        return 0;

                    g_muxerArray[i]->SetFileNameInfo(fileName);
                    DCF_AddFile(DB, g_muxerArray[i]->GetMuxerCamID(), fileName);
                }
                else
                {
                    do
                    {
                        ret = DCF_CreateSubFileName(DB, g_muxerArray[i]->GetMuxerCamID(), fileName, timep);
                        if (ret == -1)
                            StorageReMount();
                    } while (/* ret == -1 */ 0);

                    if (ret == -2)
                        return 0;

                    g_muxerArray[i]->SetFileNameInfo(fileName);
                }

                if (DCF_IsDBFormatFree(DB))
                {
                    MI_U64 u64LimitSize = DCF_GetDBFormatFreeSize(DB, g_muxerArray[i]->GetMuxerCamID()) * 9 / 10;
                    g_muxerArray[i]->SetFileSizeLimit(u64LimitSize);
                }
            }
        }

        MuxerSendStart(eMuxerType);

        // Update status.
        switch (eMuxerType)
        {
        case MUXER_NORMAL:
            if (g_bRecMdtTrigger)
                carInfo.stRecInfo.bMuxingMD = TRUE;
            else
                carInfo.stRecInfo.bMuxing = TRUE;
            cardv_update_status(REC_STATUS, "1", 2);
            break;
        case MUXER_EMERG:
            MuxerSendStop(MUXER_NORMAL, TRUE);
            carInfo.stRecInfo.bMuxing      = FALSE;
            carInfo.stRecInfo.bMuxingMD    = FALSE;
            carInfo.stRecInfo.bMuxingEmerg = TRUE;
            cardv_update_status(REC_STATUS, "2", 2);
            break;
        case MUXER_SHARE:
            carInfo.stRecInfo.bMuxingShare = TRUE;
            break;
        default:
            break;
        }
        IPC_CarInfo_Write_RecInfo(&carInfo.stRecInfo);
    }
    break;

    case CMD_MUX_STOP: {
        MuxerSendStop(MUXER_NORMAL, FALSE);
        MuxerSendStop(MUXER_EMERG, FALSE);
        MuxerSendStop(MUXER_SHARE, FALSE);
        // Trigger Post File
        MuxerSendMgr();
        // Wait for muxer stop.
        for (int i = 0; i < MAX_MUXER_NUMBER; i++)
        {
            if (g_muxerArray[i] == NULL)
                continue;
        L_RECHECK:
            if (g_muxerArray[i]->IsMuxerEnable() && g_muxerArray[i]->GetMuxerStatus() != MUXER_TX_STAT_IDLE)
            {
                usleep(10000);
                goto L_RECHECK;
            }
        }
#if (CARDV_GPS_ENABLE)
        GpsStartRec(FALSE);
#endif
        GsensorStartRec(FALSE);
        g_bRecMdtTrigger = FALSE;
    }
    break;

    case CMD_MUX_STOP_KEEP_ENCODING:
        MuxerSendStop(MUXER_NORMAL, TRUE);
        MuxerSendStop(MUXER_EMERG, TRUE);
        MuxerSendStop(MUXER_SHARE, TRUE);
        MuxerSendMgr(); // Trigger Post File
        g_bRecMdtTrigger = FALSE;
        break;

    case CMD_MUX_PAUSE:
        MuxerSendPause();
#if (CARDV_GPS_ENABLE)
        GpsStartRec(FALSE);
#endif
        GsensorStartRec(FALSE);
        break;

    case CMD_MUX_RESUME:
#if (CARDV_GPS_ENABLE)
        GpsStartRec(TRUE);
#endif
        GsensorStartRec(TRUE);
        MuxerSendResume();
        break;

    case CMD_MUX_SET_FRAMERATE: {
        MI_S32                muxParam[5];
        MuxerType_e           eMuxerType;
        U16                   u16FpsNum = 0, u16FpsDen = 0;
        BOOL                  bTimeLapse  = 0;
        MuxerSlowMotionStatus eSlowMotion = MUXER_SLOW_MOTION_NONE;

        memcpy(muxParam, param, sizeof(muxParam));
        eMuxerType  = (MuxerType_e)muxParam[0];
        u16FpsNum   = muxParam[1];
        u16FpsDen   = muxParam[2];
        bTimeLapse  = muxParam[3];
        eSlowMotion = (MuxerSlowMotionStatus)muxParam[4];

        for (int i = 0; i < MAX_MUXER_NUMBER; i++)
        {
            if (g_muxerArray[i] == NULL)
                continue;
            if (g_muxerArray[i]->GetMuxerType() == eMuxerType)
                g_muxerArray[i]->SetFrameRate(u16FpsNum, u16FpsDen, bTimeLapse, eSlowMotion);
        }
    }
    break;

    case CMD_MUX_SET_REC_LIMIT_TIME: {
        MI_U64 muxParam[2];
        memcpy(muxParam, param, sizeof(muxParam));
        MuxerSetLimitTime((MuxerType_e)muxParam[0], muxParam[1]);
    }
    break;

    case CMD_MUX_SET_PRE_REC_TIME: {
        MI_U64 muxParam[2];
        memcpy(muxParam, param, sizeof(muxParam));
        MuxerSetPreRecordTime((MuxerType_e)muxParam[0], muxParam[1]);
    }
    break;

    default:
        break;
    }

    return 0;
}