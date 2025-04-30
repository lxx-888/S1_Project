/*
 * Carimpl.cpp
 *
 * This file use to exchange data between CARDVUI with CARDVIMPL.
 *
 *  Created on: 2019-10-10
 *      Author: lei.qin
 */

#include "entry/EasyUIContext.h"
#include "uart/UartContext.h"
#include "manager/ConfigManager.h"
#include "utils/TimeHelper.h"
#include <cstring>
#include "utils/Log.h"
#include <stdlib.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include "fcntl.h"
#include "carimpl.h"
#include "demux.h"
#include <libexif/exif-loader.h>
#include "mi_fb.h"
#ifdef SUPPORT_CARLIFE
#include "LyLink.h"
#endif
/*===========================================================================
 * Macro define
 *===========================================================================*/

#define DCF_FROM_TAIL (1)

/*===========================================================================
 * Global variable
 *===========================================================================*/

CarDV_Info carimpl = {0};
bool bDcfMount = FALSE;

MI_U8 gu8LastUIMode = UI_VIDEO_MODE;

static long	gulVideoPowerOffRtcTime = 0; //Unit:seconds
static long	gulAutoPowerOffRtcTime   = 0;
static long	gulLCDPowerSaveRtcTime = 0;
static char gCmdBuf[256] = {0};
static Mutex sLock;
static WMSG_INFO m_WMSGInfo = WMSG_NONE;

static bool carimpl_ExecCmdHandler(CARIMPL_CMD_TYPE eCmdType, const char *szCmd, const char *szCmdParam);

bool Carimpl_SyncAllSetting(void) {
    //carimpl maybe need to maintain all of user`s setting.

    // Following code NOT used now, TBD
    // memcpy(&carimpl.stMenuInfo, MenuSettingConfig(), sizeof(MenuInfo));
    // IPC_CarInfo_Write_MenuInfo(&carimpl.stMenuInfo);
    if(MenuSettingConfig()->uiResetSetting){
        MenuSettingInit();
        MenuSettingConfig()->uiResetSetting = 0;
    }
    return true;
}

bool Carimpl_Reset(void)
{
    carimpl.stBrowserInfo.u8CamId = 0;     //fron cam
    carimpl.stBrowserInfo.u8DBId = 0;      //normal folder
    printf("%s %d\n", __FUNCTION__, __LINE__);
    for (int i = 0; i < DB_NUM; i++)
        for (int j = 0; j < CAM_NUM; j++)
            carimpl.stBrowserInfo.u32CurFileIdx[i][j] = 0;
    return true;
}

void Carimpl_Send2Fifo(const void *cmd, size_t size)
{
    int pipe_fd_w = -1;
    pipe_fd_w = open(FIFO_NAME, O_WRONLY);
    if (pipe_fd_w >= 0) {
        write(pipe_fd_w, cmd, size);
        close(pipe_fd_w);
    }
    else {
        printf("Error: failed to open %s\n", FIFO_NAME);
    }
}

/**********************************************************/
/********************* browser ****************************/
/**********************************************************/

int Carimpl_DcfMount(void)
{
    int ret = 0;

    DCF_Lock();
    if (bDcfMount)
        goto EXIT;

    ret = DCF_Mount();
    if (ret == 0)
        bDcfMount = TRUE;

EXIT:
    DCF_UnLock();
    return ret;
}

int Carimpl_DcfUnmount(void)
{
    int ret = 0;

    DCF_Lock();
    if (bDcfMount == FALSE)
        goto EXIT;

    bDcfMount = FALSE;
    DCF_UnMount();
EXIT:
    DCF_UnLock();
    return ret;
}

bool Carimpl_IsSubFileEnable(void)
{
    return DCF_IsSubFileEnable(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId);
}

unsigned int Carimpl_GetTotalFiles(unsigned char u8DB, unsigned char u8CamId)
{
    return DCF_GetDBFileCnt(u8DB, u8CamId);
}

unsigned int Carimpl_GetTotalFiles(void)
{
    return DCF_GetDBFileCnt(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId);
}

unsigned int Carimpl_GetCurFileIdx(unsigned char u8DB, unsigned char u8CamId)
{
    return carimpl.stBrowserInfo.u32CurFileIdx[u8DB][u8CamId];
}

unsigned int Carimpl_GetCurFileIdx(void)
{
    return carimpl.stBrowserInfo.u32CurFileIdx[carimpl.stBrowserInfo.u8DBId][carimpl.stBrowserInfo.u8CamId];
}

bool Carimpl_SetCurFileIdx(unsigned char u8DB, unsigned char u8CamId, unsigned int u32FileIdx)
{
    // Idx from tail or head
    carimpl.stBrowserInfo.u32CurFileIdx[u8DB][u8CamId] = u32FileIdx;
    IPC_CarInfo_Write_BrowserInfo(&carimpl.stBrowserInfo);
    return true;
}

bool Carimpl_SetCurFileIdx(unsigned int u32FileIdx)
{
    // Idx from tail or head
    carimpl.stBrowserInfo.u32CurFileIdx[carimpl.stBrowserInfo.u8DBId][carimpl.stBrowserInfo.u8CamId] = u32FileIdx;
    IPC_CarInfo_Write_BrowserInfo(&carimpl.stBrowserInfo);
    return true;
}

bool Carimpl_SetCurFolder(unsigned char u8DB, unsigned char u8CamId)
{
    if (u8DB >= DB_NUM || u8CamId >= CAM_NUM) {
        printf("%s err %d\n",__FUNCTION__, __LINE__);
        return FALSE;
    }

    carimpl.stBrowserInfo.u8DBId = u8DB;
    carimpl.stBrowserInfo.u8CamId = u8CamId;
    IPC_CarInfo_Write_BrowserInfo(&carimpl.stBrowserInfo);
    return TRUE;
}

unsigned char Carimpl_GetCurDB(void)
{
    return carimpl.stBrowserInfo.u8DBId;
}

unsigned char Carimpl_GetCurCamId(void)
{
    return carimpl.stBrowserInfo.u8CamId;
}

char* Carimpl_GetCurFileName(unsigned int u32FileIdx)
{
    #if (DCF_FROM_TAIL)
    return DCF_GetFileNameFromTailByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #else
    return DCF_GetFileNameFromHeadByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #endif
}

char* Carimpl_GetCurSubFileName(unsigned int u32FileIdx)
{
    #if (DCF_FROM_TAIL)
    return DCF_GetSubFileNameFromTailByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #else
    return DCF_GetSubFileNameFromHeadByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #endif
}

char* Carimpl_GetPairFileName(char* pszFileName, unsigned char u8CamId)
{
    char* strFile = NULL;
    if (DCF_IsSubFileEnable(carimpl.stBrowserInfo.u8DBId, u8CamId)){
        strFile = DCF_GetPairSubFileName(pszFileName, u8CamId);
        if(strFile)
            return strFile;
    }
    return DCF_GetPairFileName(pszFileName, u8CamId);
}

int Carimpl_DeleteFile(unsigned int u32FileIdx)
{
    #if (DCF_FROM_TAIL)
    return DCF_DeleteFileFromTailByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #else
    return DCF_DeleteFileFromHeadByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #endif
}

int Carimpl_ProtectFile(unsigned int u32FileIdx)
{
    #if (DCF_FROM_TAIL)
    return DCF_ProtectFileFromTailByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #else
    return DCF_ProtectFileFromHeadByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #endif
}

int Carimpl_UnProtectFile(unsigned int u32FileIdx)
{
    #if (DCF_FROM_TAIL)
    return DCF_UnProtectFileFromTailByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #else
    return DCF_UnProtectFileFromHeadByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #endif
}

bool Carimpl_IsProtectFile(unsigned int u32FileIdx)
{
    #if (DCF_FROM_TAIL)
    return DCF_IsProtectFileFromTailByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #else
    return DCF_IsProtectFileFromHeadByIdx(carimpl.stBrowserInfo.u8DBId, carimpl.stBrowserInfo.u8CamId, u32FileIdx);
    #endif
}

void carimpl_SyncDCF(void)
{
    carimpl_ExecCmdHandler(FIFO_CMD_TYPE, CARDV_CMD_SYNC_DCF, "");
}

int Carimpl_GetFileThumbnail(char *pFileName, char *pThumbFileName)
{
    return demux_thumbnail(pFileName, pThumbFileName);
}

int Carimpl_GetJpegFileThumbnail(char *pFileName, char *pThumbFileName)
{
    int ret = 0;
    int fd = 0;
    ExifLoader *loader;

    ret = access(pThumbFileName, F_OK);
    if (ret == 0) {
        unlink(pThumbFileName);
    }

    /* Create an ExifLoader object to manage the EXIF loading process */
    loader = exif_loader_new();
    if (loader) {
        ExifData *exif;

        /* Load the EXIF data from the image file */
        exif_loader_write_file(loader, pFileName);

        /* Get a pointer to the EXIF data */
        exif = exif_loader_get_data(loader);

        /* The loader is no longer needed--free it */
        exif_loader_unref(loader);
        loader = NULL;
        if (exif) {
            /* Make sure the image had a thumbnail before trying to write it */
            if (exif->data && exif->size > 4 &&
                exif->data[0] == 0xFF && exif->data[1] == 0xD8 &&
                exif->data[exif->size-2] == 0xFF && exif->data[exif->size-1] == 0xD9) {
                fd = open(pThumbFileName, O_WRONLY | O_CREAT, 0666);
                if (fd >= 0) {
                    write(fd, exif->data, exif->size);
                    close(fd);
                }
            }
            /* Free the EXIF data */
            exif_data_unref(exif);
        }
    }
    return ret;
}

int Carimpl_GetJpegFileResolution(const char *szFileName, unsigned int *pu32Width, unsigned int *pu32Height)
{
#define MAKEUS(a, b)    ((unsigned short) (((unsigned short)(a)) << 8 | ((unsigned short)(b))))

#define M_DATA  0x00
#define M_SOF0  0xc0
#define M_DHT   0xc4
#define M_SOI   0xd8
#define M_EOI   0xd9
#define M_SOS   0xda
#define M_DQT   0xdb
#define M_DNL   0xdc
#define M_DRI   0xdd
#define M_APP0  0xe0
#define M_APPF  0xef
#define M_COM   0xfe

    int Finished = 0;
    unsigned char id, u8High, u8Low;
    FILE *pFile = NULL;

    *pu32Width = 0;
    *pu32Height = 0;

    pFile = fopen(szFileName, "rb");
    if (pFile == NULL)
        return -1;

    while (!Finished) {
        if (!fread(&id, sizeof(char), 1, pFile) ||
            id != 0xff ||
            !fread(&id, sizeof(char), 1, pFile)) {
            Finished = -2;
            break;
        }

        if (id >= M_APP0 && id <= M_APPF) {
            fread(&u8High, sizeof(char), 1, pFile);
            fread(&u8Low, sizeof(char), 1, pFile);
            fseek(pFile, (long)(MAKEUS(u8High, u8Low) - 2), SEEK_CUR);
            continue;
        }

        switch (id) {
        case M_SOI:
            break;

        case M_COM:
        case M_DQT:
        case M_DHT:
        case M_DNL:
        case M_DRI:
            fread(&u8High, sizeof(char), 1, pFile);
            fread(&u8Low, sizeof(char), 1, pFile);
            fseek(pFile, (long)(MAKEUS(u8High, u8Low) - 2), SEEK_CUR);
            break;

        case M_SOF0:
            fseek(pFile, 3L, SEEK_CUR);
            fread(&u8High, sizeof(char), 1, pFile);
            fread(&u8Low, sizeof(char), 1, pFile);
            *pu32Height = (unsigned int)MAKEUS(u8High, u8Low);
            fread(&u8High, sizeof(char), 1, pFile);
            fread(&u8Low, sizeof(char), 1, pFile);
            *pu32Width = (unsigned int)MAKEUS(u8High, u8Low);
            fclose(pFile);
            return 0;

        case M_SOS:
        case M_EOI:
        case M_DATA:
            Finished = -1;
            break;

        default:
            fread(&u8High, sizeof(char), 1, pFile);
            fread(&u8Low, sizeof(char), 1, pFile);
            printf("unknown id [%x] length [%hd]\n", id, MAKEUS(u8High, u8Low));
            if (fseek(pFile, (long)(MAKEUS(u8High, u8Low) - 2), SEEK_CUR) != 0)
                Finished = -2;
            break;
        }
    }

    if (Finished == -1)
        printf("can't find SOF0\n");
    else if (Finished == -2)
        printf("jpeg format error\n");
    fclose(pFile);
    return -1;
}

int64_t Carimpl_GetFileDuration(char *pFileName)
{
    int64_t duration = demux_duration(pFileName);
    duration = duration / 1000000;
    return duration;
}

/**********************************************************/
/********************* record *****************************/
/**********************************************************/

void carimpl_VideoH26xInit(bool bInit)
{
    carimpl_ExecCmdHandler(FIFO_CMD_TYPE, CARDV_CMD_VIDEO_H26X_FLOW, bInit ? "1" : "0");
}

void carimpl_VideoJpegInit(bool bInit)
{
    carimpl_ExecCmdHandler(FIFO_CMD_TYPE, CARDV_CMD_VIDEO_JPEG_FLOW, bInit ? "1" : "0");
}

void carimpl_VideoAllInit(bool bInit)
{
    carimpl_ExecCmdHandler(FIFO_CMD_TYPE, CARDV_CMD_VIDEO_H26X_JPEG_FLOW, bInit ? "1" : "0");
}

void carimpl_VideoSensorInit(bool bInit)
{
    carimpl_ExecCmdHandler(FIFO_CMD_TYPE, CARDV_CMD_VIDEO_SENSOR_FLOW, bInit ? "1" : "0");
}

/**********************************************************/
/********************* playback ***************************/
/**********************************************************/


/**********************************************************/
/********************** setting ***************************/
/**********************************************************/

static bool carimpl_ExecCmdHandler(CARIMPL_CMD_TYPE eCmdType, const char *szCmd, const char *szCmdParam)
{
    char cmdstr[256] = {0};
    if (eCmdType == CGI_CMD_TYPE)
    {
        int  cmdstatus = 0;
        sprintf(cmdstr, "%s %s %s", CGI_PROCESS_PATH, szCmd, szCmdParam);
        if (strstr(szCmd, "set"))
        {
            cmdstatus = system(cmdstr);
            if (cmdstatus < 0) {
                printf("[%s]fail\n", cmdstr);
                return false;
            }
        }
        else if (strstr(szCmd, "get"))
        {
            memset(gCmdBuf, 0, sizeof(gCmdBuf));
            FILE *fp = popen(cmdstr, "r");
            if (!fp)
            {
                printf("CGI_CMD_TYPE: popen errno\n");
                return false;
            }
            while (fgets(gCmdBuf, sizeof(gCmdBuf), fp) != NULL)
            {
                //printf("CGI_CMD_TYPE: %s\n",gCmdBuf);
            }
            pclose(fp);
        }
    }
    else if (eCmdType == FIFO_CMD_TYPE)
    {
        sprintf(cmdstr, "%s %s\r\n", szCmd, szCmdParam);
        printf("%s", cmdstr);
        Carimpl_Send2Fifo(cmdstr, strlen(cmdstr));
    }
    else if (eCmdType == SCRIPT_CMD_TYPE)
    {
        int  cmdstatus = 0;
        sprintf(cmdstr, "%s %s", szCmd, szCmdParam);
        cmdstatus = system(cmdstr);
        if (cmdstatus < 0) {
            printf("[%s]fail\n", cmdstr);
            return false;
        }
    }
    return true;
}

char* carimpl_GetSettingCmdBuf(void)
{
    //Cmdbuf format key=value
    char *bufval = NULL;
    bufval = strchr(gCmdBuf, '=');
    if (bufval != NULL)
        return bufval+1;
    else
        return gCmdBuf;
}

void carimpl_VideoFunc_SetResolution(void)
{
    if (carimpl.stUIModeInfo.u8Mode == UI_MENU_MODE &&
        gu8LastUIMode != UI_VIDEO_MODE)
        return;

    switch (MenuSettingConfig()->uiMOVSize)
    {
    #if (MENU_MOVIE_SIZE_4K_25P_EN)
    case MOVIE_SIZE_4K_25P:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "2160P25fps");
        break;
    #endif
    #if (MENU_MOVIE_SIZE_1440_30P_EN)
    case MOVIE_SIZE_1440_30P:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "1440P30fps");
        break;
    #endif
    #if (MENU_MOVIE_SIZE_SHD_30P_EN)
    case MOVIE_SIZE_SHD_30P:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "1296P30fps");
        break;
    #endif
    #if (MENU_MOVIE_SIZE_SHD_25P_EN)
    case MOVIE_SIZE_SHD_25P:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "1296P25fps");
        break;
    #endif
    #if (MENU_MOVIE_SIZE_1080_60P_EN)
    case MOVIE_SIZE_1080_60P:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "1080P60fps");
        break;
    #endif
    #if (MENU_MOVIE_SIZE_1080P_EN)
    case MOVIE_SIZE_1080P:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "1080P30fps");
        break;
    #endif
    #if (MENU_MOVIE_SIZE_1080P_30_HDR_EN)
    case MOVIE_SIZE_1080_30P_HDR:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "1080P30fpsHDR");
        break;
    #endif
    #if (MENU_MOVIE_SIZE_900P_30P_EN)
    case MOVIE_SIZE_900P_30P:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "900P30fps");
        break;
    #endif
    #if (MENU_MOVIE_SIZE_960P_30P_EN)
    case MOVIE_SIZE_960P_30P:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "960P30fps");
        break;
    #endif
    #if (MENU_MOVIE_SIZE_720P_EN)
    case MOVIE_SIZE_720P:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "720P30fps");
        break;
    #endif
    #if (MENU_MOVIE_SIZE_720_60P_EN)
    case MOVIE_SIZE_720_60P:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "720P60fps");
        break;
    #endif
    #if (MENU_MOVIE_SIZE_VGA30P_EN)
    case MOVIE_SIZE_VGA_30P:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoRes", "VGA");
        break;
    #endif
    default:
        printf("Unsupported resolution/frame rate!\n");
        break;
    }
}

void carimpl_VideoFunc_SetQuality(void)
{
    switch (MenuSettingConfig()->uiMOVQuality)
    {
    #if (MENU_STILL_QUALITY_SUPER_FINE_EN || MENU_MOVIE_QUALITY_SUPER_FINE_EN)
        case QUALITY_SUPER_FINE:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoQuality", "SUPER_FINE");
        break;
    #endif
    #if (MENU_STILL_QUALITY_FINE_EN || MENU_MOVIE_QUALITY_FINE_EN)
    case QUALITY_FINE:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoQuality", "FINE");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_VideoFunc_SetMicSensitivity(void)
{
    switch (MenuSettingConfig()->uiMicSensitivity)
    {
    #if (MENU_MOVIE_MIC_SEN_STANDARD_EN)
        case MIC_SEN_STANDARD:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set MicSensitivity", "STANDARD");
        break;
    #endif
    #if (MENU_MOVIE_MIC_SEN_LOW_EN)
        case MIC_SEN_LOW:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set MicSensitivity", "LOW");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_VideoFunc_SetPreRecord(void)
{
    switch (MenuSettingConfig()->uiMOVPreRecord)
    {
    #if (MENU_MOVIE_PRE_RECORD_ON_EN)
    case MOVIE_PRE_RECORD_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoPreRecord", "ON");
        break;
    #endif
    #if (MENU_MOVIE_PRE_RECORD_OFF_EN)
    case MOVIE_PRE_RECORD_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoPreRecord", "OFF");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_VideoFunc_SetClipTime(void)
{
    switch (MenuSettingConfig()->uiMOVClipTime)
    {
    #if (MENU_MOVIE_CLIP_TIME_OFF_EN)
    case MOVIE_CLIP_TIME_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LoopingVideo", "OFF");
        break;
    #endif
    #if (MENU_MOVIE_CLIP_TIME_1MIN_EN)
    case MOVIE_CLIP_TIME_1MIN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LoopingVideo", "1MIN");
        break;
    #endif
    #if (MENU_MOVIE_CLIP_TIME_2MIN_EN)
    case MOVIE_CLIP_TIME_2MIN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LoopingVideo", "2MIN");
        break;
    #endif
    #if (MENU_MOVIE_CLIP_TIME_3MIN_EN)
    case MOVIE_CLIP_TIME_3MIN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LoopingVideo", "3MIN");
        break;
    #endif
    #if (MENU_MOVIE_CLIP_TIME_5MIN_EN)
    case MOVIE_CLIP_TIME_5MIN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LoopingVideo", "5MIN");
        break;
    #endif
    #if (MENU_MOVIE_CLIP_TIME_10MIN_EN)
    case MOVIE_CLIP_TIME_10MIN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LoopingVideo", "10MIN");
        break;
    #endif
    #if (MENU_MOVIE_CLIP_TIME_30MIN_EN)
    case MOVIE_CLIP_TIME_30MIN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LoopingVideo", "30MIN");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_VideoFunc_SetPowerOffTime(void)
{
    switch (MenuSettingConfig()->uiMOVPowerOffTime)
    {
    #if (MENU_MOVIE_POWER_OFF_DELAY_0SEC_EN)
    case MOVIE_POWEROFF_TIME_0MIN:
        gulVideoPowerOffRtcTime = 0;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "0MIN");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_5SEC_EN)
    case MOVIE_POWEROFF_TIME_5SEC:
        gulVideoPowerOffRtcTime = 5;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "5SEC");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_10SEC_EN)
    case MOVIE_POWEROFF_TIME_10SEC:
        gulVideoPowerOffRtcTime = 10;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "10SEC");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_15SEC_EN)
    case MOVIE_POWEROFF_TIME_15SEC:
        gulVideoPowerOffRtcTime = 15;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "15SEC");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_30SEC_EN)
    case MOVIE_POWEROFF_TIME_30SEC:
        gulVideoPowerOffRtcTime = 30;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "30SEC");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_1MIN_EN)
    case MOVIE_POWEROFF_TIME_1MIN:
        gulVideoPowerOffRtcTime = 60;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "1MIN");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_2MIN_EN)
    case MOVIE_POWEROFF_TIME_2MIN:
        gulVideoPowerOffRtcTime = 120;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "2MIN");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_3MIN_EN)
    case MOVIE_POWEROFF_TIME_3MIN:
        gulVideoPowerOffRtcTime = 180;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "3MIN");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_5MIN_EN)
    case MOVIE_POWEROFF_TIME_5MIN:
        gulVideoPowerOffRtcTime = 300;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "5MIN");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_10MIN_EN)
    case MOVIE_POWEROFF_TIME_10MIN:
        gulVideoPowerOffRtcTime = 600;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "10MIN");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_15MIN_EN)
    case MOVIE_POWEROFF_TIME_15MIN:
        gulVideoPowerOffRtcTime = 900;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "15MIN");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_30MIN_EN)
    case MOVIE_POWEROFF_TIME_30MIN:
        gulVideoPowerOffRtcTime = 1800;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "30MIN");
        break;
    #endif
    #if (MENU_MOVIE_POWER_OFF_DELAY_60MIN_EN)
    case MOVIE_POWEROFF_TIME_60MIN:
        gulVideoPowerOffRtcTime = 3600;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set VideoOffTime", "60MIN");
        break;
    #endif
    default:
        break;
    }
    long ulRtcCurrentTime = TimeHelper::getCurrentTime();
    gulVideoPowerOffRtcTime += ulRtcCurrentTime;
}

void carimpl_VideoFunc_SetSoundRecord(void)
{
    switch (MenuSettingConfig()->uiMOVSoundRecord)
    {
    #if (MENU_MOVIE_SOUND_RECORD_ON_EN)
    case MOVIE_SOUND_RECORD_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set SoundRecord", "ON");
        break;
    #endif
    #if (MENU_MOVIE_SOUND_RECORD_OFF_EN)
    case MOVIE_SOUND_RECORD_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set SoundRecord", "OFF");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_VideoFunc_SetRecordVolume(void)
{
    char value[256] = {0};
    sprintf(value, "%d", MenuSettingConfig()->uiMOVRecordVolume);
    carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set RecordVolume", value);
}

void carimpl_VideoFunc_SetVMDRecordTime(void)
{
    switch (MenuSettingConfig()->uiVMDRecTime)
    {
    #if (MENU_MOVIE_VMD_REC_TIME_5SEC_EN)
    case VMD_REC_TIME_5SEC:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set MotionVideoTime", "5");
        break;
    #endif
    #if (MENU_MOVIE_VMD_REC_TIME_10SEC_EN)
    case VMD_REC_TIME_10SEC:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set MotionVideoTime", "10");
        break;
    #endif
    #if (MENU_MOVIE_VMD_REC_TIME_30SEC_EN)
    case VMD_REC_TIME_30SEC:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set MotionVideoTime", "30");
        break;
    #endif
    #if (MENU_MOVIE_VMD_REC_TIME_1MIN_EN)
    case VMD_REC_TIME_1MIN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set MotionVideoTime", "60");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_VideoFunc_SetAutoReord(void)
{
    switch (MenuSettingConfig()->uiAutoRec)
    {
    case AUTO_REC_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AutoRec", "ON");
        break;
    case AUTO_REC_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AutoRec", "OFF");
        break;
    default:
        break;
    }
}

void carimpl_VideoFunc_SetTimelapse(void)
{
    switch (MenuSettingConfig()->uiTimeLapseTime)
    {
    #if (MENU_VIDEO_TIMELAPSE_MODE_OFF_EN)
    case MOVIE_VR_TIMELAPSE_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Timelapse", "OFF");
        break;
    #endif
    #if (MENU_VIDEO_TIMELAPSE_MODE_1SEC_EN)
    case MOVIE_VR_TIMELAPSE_1SEC:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Timelapse", "1SEC");
        break;
    #endif
    #if (MENU_VIDEO_TIMELAPSE_MODE_5SEC_EN)
    case MOVIE_VR_TIMELAPSE_5SEC:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Timelapse", "5SEC");
        break;
    #endif
    #if (MENU_VIDEO_TIMELAPSE_MODE_10SEC_EN)
    case MOVIE_VR_TIMELAPSE_10SEC:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Timelapse", "10SEC");
        break;
    #endif
    #if (MENU_VIDEO_TIMELAPSE_MODE_30SEC_EN)
    case MOVIE_VR_TIMELAPSE_30SEC:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Timelapse", "30SEC");
        break;
    #endif
    #if (MENU_VIDEO_TIMELAPSE_MODE_60SEC_EN)
    case MOVIE_VR_TIMELAPSE_60SEC:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Timelapse", "60SEC");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_VideoFunc_SetSlowMotion(void)
{
    switch (MenuSettingConfig()->uiSlowMotion)
    {
    case SLOWMOTION_X1:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set SlowMotion", "X1");
        break;
    case SLOWMOTION_X2:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set SlowMotion", "X2");
        break;
    case SLOWMOTION_X4:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set SlowMotion", "X4");
        break;
    case SLOWMOTION_X8:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set SlowMotion", "X8");
        break;
    default:
        break;
    }
}

void carimpl_VideoFunc_SetHDRMode(void)
{
    switch (MenuSettingConfig()->uiHDR)
    {
    case HDR_EN_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set HDR", "ON");
        break;
    case HDR_EN_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set HDR", "OFF");
        break;
    default:
        break;
    }
}

void carimpl_VideoFunc_SetWNRMode(void)
{
    switch (MenuSettingConfig()->uiWNR)
    {
    case WNR_EN_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set WNR", "ON");
        break;
    case WNR_EN_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set WNR", "OFF");
        break;
    default:
        break;
    }
}

void carimpl_VideoFunc_SetNightMode(void)
{
    switch (MenuSettingConfig()->uiNightMode)
    {
    #if (MENU_MOVIE_NIGHT_MODE_ON_EN)
    case MOVIE_NIGHT_MODE_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set NightMode", "ON");
        break;
    #endif
    #if (MENU_MOVIE_NIGHT_MODE_OFF_EN)
    case MOVIE_NIGHT_MODE_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set NightMode", "OFF");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_VideoFunc_SetParkingMode(void)
{
    switch (MenuSettingConfig()->uiParkingMode)
    {
    case MOVIE_PARKING_MODE_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ParkingMonitor", "ENABLE");
        break;
    case MOVIE_PARKING_MODE_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ParkingMonitor", "DISABLE");
        break;
    default:
        break;
    }
}

void carimpl_VideoFunc_SetLDWSMode(void)
{
    switch (MenuSettingConfig()->uiLDWS)
    {
    case LDWS_EN_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LDWS", "ON");
        break;
    case LDWS_EN_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LDWS", "OFF");
        break;
    default:
        break;
    }
}

void carimpl_VideoFunc_SetFCWSMode(void)
{
    switch (MenuSettingConfig()->uiFCWS)
    {
    case FCWS_EN_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set FCWS", "ON");
        break;
    case FCWS_EN_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set FCWS", "OFF");
        break;
    default:
        break;
    }
}

void carimpl_VideoFunc_SetSAGMode(void)
{
    switch (MenuSettingConfig()->uiSAG)
    {
    case SAG_EN_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set SAG", "ON");
        break;
    case SAG_EN_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set SAG", "OFF");
        break;
    default:
        break;
    }
}

void carimpl_VideoFunc_StartRecording(bool bStart)
{
    #if ACT_SAME_AS_CGI_CMD
    carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Video", "record");
    #else
    if (bStart)
    carimpl_ExecCmdHandler(FIFO_CMD_TYPE, CARDV_CMD_START_REC, "");
    else
    carimpl_ExecCmdHandler(FIFO_CMD_TYPE, CARDV_CMD_STOP_REC, "");
    #endif
}

void carimpl_VideoFunc_StartEmergRecording(void)
{
    carimpl_ExecCmdHandler(FIFO_CMD_TYPE, CARDV_CMD_EMERG_REC, "");
    carimpl_show_wmsg(true, WMSG_LOCK_CURRENT_FILE); // TBD : warning message
}

void carimpl_VideoFunc_Capture(void)
{
    #if ACT_SAME_AS_CGI_CMD
    carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Video", "capture");
    #else
    carimpl_ExecCmdHandler(FIFO_CMD_TYPE, CARDV_CMD_CAPTURE, "");
    #endif
}

void carimpl_IQFunc_SetContrastAdjust(void)
{
    char value[256] = {0};
    sprintf(value, "%d", MenuSettingConfig()->uiContrast);
    carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Contrast", value);
}

void carimpl_IQFunc_SetSaturationAdjust(void)
{
    char value[256] = {0};
    sprintf(value, "%d", MenuSettingConfig()->uiSaturation);
    carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Saturation", value);
}

void carimpl_IQFunc_SetSharpnessAdjust(void)
{
    char value[256] = {0};
    sprintf(value, "%d", MenuSettingConfig()->uiSharpness);
    carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Sharpness", value);
}

void carimpl_IQFunc_SetGammaAdjust(void)
{
    char value[256] = {0};
    sprintf(value, "%d", MenuSettingConfig()->uiGamma);
    carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Gamma", value);
}

void carimpl_IQFunc_SetScene(void)
{
    switch (MenuSettingConfig()->uiScene)
    {
    #if (MENU_MANUAL_SCENE_AUTO_EN)
    case SCENE_AUTO:
        break;
    #endif
    #if (MENU_MANUAL_SCENE_SPORT_EN)
    case SCENE_SPORT:
        break;
    #endif
    #if (MENU_MANUAL_SCENE_PORTRAIT_EN)
    case SCENE_PORTRAIT:
        break;
    #endif
    #if (MENU_MANUAL_SCENE_LANDSCAPE_EN)
    case SCENE_LANDSCAPE:
        break;
    #endif
    #if (MENU_MANUAL_SCENE_TWILIGHT_PORTRAIT_EN)
    case SCENE_TWILIGHT_PORTRAIT:
        break;
    #endif
    #if (MENU_MANUAL_SCENE_TWILIGHT_EN)
    case SCENE_TWILIGHT:
        break;
    #endif
    #if (MENU_MANUAL_SCENE_SNOW_EN)
    case SCENE_SNOW:
        break;
    #endif
    #if (MENU_MANUAL_SCENE_BEACH_EN)
    case SCENE_BEACH:
        break;
    #endif
    #if (MENU_MANUAL_SCENE_FIREWORKS_EN)
    case SCENE_FIREWORKS:
        break;
    #endif
    default:
        break;
    }
}

void carimpl_IQFunc_SetEV(void)
{
    switch (MenuSettingConfig()->uiEV)
    {
    #if (MENU_MANUAL_EV_N20_EN)
    case EVVALUE_N20:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVN200");
        break;
    #endif
    #if (MENU_MANUAL_EV_N17_EN)
    case EVVALUE_N17:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVN167");
        break;
    #endif
    #if (MENU_MANUAL_EV_N15_EN)
    case EVVALUE_N15:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVN155");
        break;
    #endif
    #if (MENU_MANUAL_EV_N13_EN)
    case EVVALUE_N13:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVN133");
        break;
    #endif
    #if (MENU_MANUAL_EV_N10_EN)
    case EVVALUE_N10:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVN100");
        break;
    #endif
    #if (MENU_MANUAL_EV_N07_EN)
    case EVVALUE_N07:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVN67");
        break;
    #endif
    #if (MENU_MANUAL_EV_N05_EN)
    case EVVALUE_N05:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVN55");
        break;
    #endif
    #if (MENU_MANUAL_EV_N03_EN)
    case EVVALUE_N03:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVN33");
        break;
    #endif
    //#if (MENU_MANUAL_EV_00_EN)    // always exist
    case EVVALUE_00:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EV0");
        break;
    //#endif
    #if (MENU_MANUAL_EV_P03_EN)
    case EVVALUE_P03:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVP33");
        break;
    #endif
    #if (MENU_MANUAL_EV_P05_EN)
    case EVVALUE_P05:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVP55");
        break;
    #endif
    #if (MENU_MANUAL_EV_P07_EN)
    case EVVALUE_P07:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVP67");
        break;
    #endif
    #if (MENU_MANUAL_EV_P10_EN)
    case EVVALUE_P10:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVP100");
        break;
    #endif
    #if (MENU_MANUAL_EV_P13_EN)
    case EVVALUE_P13:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVP133");
        break;
    #endif
    #if (MENU_MANUAL_EV_P15_EN)
    case EVVALUE_P15:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVP155");
        break;
    #endif
    #if (MENU_MANUAL_EV_P17_EN)
    case EVVALUE_P17:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVP167");
        break;
    #endif
    #if (MENU_MANUAL_EV_P20_EN)
    case EVVALUE_P20 :
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set EV", "EVP200");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_IQFunc_SetISO(void)
{
    switch (MenuSettingConfig()->uiISO)
    {
    #if (MENU_MANUAL_ISO_AUTO_EN)
    case ISO_AUTO:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ISO", "ISO_AUTO");
        break;
    #endif
    //  ISO_80,
    #if (MENU_MANUAL_ISO_100_EN)
    case ISO_100:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ISO", "ISO_100");
        break;
    #endif
    #if (MENU_MANUAL_ISO_200_EN)
    case ISO_200:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ISO", "ISO_200");
        break;
    #endif
    #if (MENU_MANUAL_ISO_400_EN)
    case ISO_400:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ISO", "ISO_400");
        break;
    #endif
    #if (MENU_MANUAL_ISO_800_EN)
    case ISO_800:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ISO", "ISO_800");
        break;
    #endif
    #if (MENU_MANUAL_ISO_1200_EN)
    case ISO_1200:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ISO", "ISO_1200");
        break;
    #endif
    #if (MENU_MANUAL_ISO_1600_EN)
    case ISO_1600:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ISO", "ISO_1600");
        break;
    #endif
    #if (MENU_MANUAL_ISO_3200_EN)
    case ISO_3200:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ISO", "ISO_3200");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_IQFunc_SetWB(void)
{
    switch (MenuSettingConfig()->uiWB)
    {
    #if (MENU_MANUAL_WB_AUTO_EN)
    case WB_AUTO:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AWB", "Auto");
        break;
    #endif
    #if (MENU_MANUAL_WB_DAYLIGHT_EN)
    case WB_DAYLIGHT:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AWB", "Daylight");
        break;
    #endif
    #if (MENU_MANUAL_WB_CLOUDY_EN)
    case WB_CLOUDY:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AWB", "Cloudy");
        break;
    #endif
    #if (MENU_MANUAL_WB_FLUORESCENT1_EN)
    case WB_FLUORESCENT1:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AWB", "Fluorescent1");
        break;
    #endif
    #if (MENU_MANUAL_WB_FLUORESCENT2_EN)
    case WB_FLUORESCENT2:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AWB", "Fluorescent2");
        break;
    #endif
    #if (MENU_MANUAL_WB_FLUORESCENT3_EN)
    case WB_FLUORESCENT3:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AWB", "Fluorescent3");
        break;
    #endif
    #if (MENU_MANUAL_WB_INCANDESCENT_EN)
    case WB_INCANDESCENT:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AWB", "Incandescent");
        break;
    #endif
    #if (MENU_MANUAL_WB_FLASH_EN)
    case WB_FLASH:
        break;
    #endif
    #if (MENU_MANUAL_WB_ONEPUSH_EN)
    case WB_ONEPUSH:
        break;
    #endif
    #if (MENU_MANUAL_WB_ONE_PUSH_SET_EN)
    case WB_ONE_PUSH_SET,
    #endif
    #if (MENU_MANUAL_WB_UNDERWATER)
    case WB_UNDERWATER:
        break;
    #endif
    default:
        break;
    }
}

void carimpl_IQFunc_SetColor(void)
{
    switch (MenuSettingConfig()->uiColor)
    {
    #if (MENU_MANUAL_COLOR_NATURAL_EN)
    case COLOR_NATURAL:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Saturation", "60");
        break;
    #endif
    #if (MENU_MANUAL_COLOR_VIVID_EN)
    case COLOR_VIVID:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Saturation", "100");
        break;
    #endif
    #if (MENU_MANUAL_COLOR_PALE_EN)
    case COLOR_PALE:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Saturation", "20");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_IQFunc_SetEffect(void)
{
    switch (MenuSettingConfig()->uiEffect)
    {
    #if (MENU_MANUAL_EFFECT_NORMAL_EN)
    case EFFECT_NORMAL:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Effect", "noraml");
        break;
    #endif
    #if (MENU_MANUAL_EFFECT_SEPIA_EN)
    case EFFECT_SEPIA:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Effect", "sepia");
        break;
    #endif
    #if (MENU_MANUAL_EFFECT_BLACK_WHITE_EN)
    case EFFECT_BLACK_WHITE:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Effect", "blackwhite");
        break;
    #endif
    #if (MENU_MANUAL_EFFECT_EMBOSS_EN)
    case EFFECT_EMBOSS:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Effect", "emboss");
        break;
    #endif
    #if (MENU_MANUAL_EFFECT_NEGATIVE_EN)
    case EFFECT_NEGATIVE:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Effect", "negative");
        break;
    #endif
    #if (MENU_MANUAL_EFFECT_SKETCH_EN)
    case EFFECT_SKETCH:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Effect", "sketch");
        break;
    #endif
    #if (MENU_MANUAL_EFFECT_OIL_EN)
    case EFFECT_OIL:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Effect", "oli");
        break;
    #endif
    #if (MENU_MANUAL_EFFECT_CRAYON_EN)
    case EFFECT_CRAYON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Effect", "crayon");
        break;
    #endif
    #if (MENU_MANUAL_EFFECT_BEAUTY_EN)
    case EFFECT_BEAUTY:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Effect", "beauty");
        break;
    #endif
    default:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Effect", "noraml");
        break;
    }
}

void carimpl_IQFunc_SetFlicker(void)
{
    switch (MenuSettingConfig()->uiFlickerHz)
    {
    #if (MENU_GENERAL_FLICKER_50HZ_EN)
    case FLICKER_50HZ:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Flicker", "50HZ");
        break;
    #endif
    #if (MENU_GENERAL_FLICKER_60HZ_EN)
    case FLICKER_60HZ:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Flicker", "60HZ");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_CaptureFunc_SetResolution(void)
{
    if (carimpl.stUIModeInfo.u8Mode == UI_MENU_MODE &&
        gu8LastUIMode != UI_CAPTURE_MODE)
        return;

    switch (MenuSettingConfig()->uiIMGSize)
    {
#if (MENU_STILL_SIZE_30M_EN)
    case IMAGE_SIZE_30M:
        printf("unsupport!");
        break;
#endif
#if (MENU_STILL_SIZE_14M_EN)
    case IMAGE_SIZE_14M:
        printf("unsupport!");
        break;
#endif
#if (MENU_STILL_SIZE_12M_EN)
    case IMAGE_SIZE_12M:
        printf("unsupport!");
        break;
#endif
#if (MENU_STILL_SIZE_8M_EN)
    case IMAGE_SIZE_8M:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ImageRes", "8M");
        break;
#endif
#if (MENU_STILL_SIZE_6M_WIDE_EN)
    case IMAGE_SIZE_6M_WIDE:
        printf("unsupport!");
        break;
#endif
#if (MENU_STILL_SIZE_6M_EN)
    case IMAGE_SIZE_6M:
        printf("unsupport!");
        break;
#endif
#if (MENU_STILL_SIZE_5M_EN)
    case IMAGE_SIZE_5M:
        printf("unsupport!");
        break;
#endif
#if (MENU_STILL_SIZE_3M_EN)
    case IMAGE_SIZE_3M:      ///< 2304x1296
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ImageRes", "3M");
        break;
#endif
#if (MENU_STILL_SIZE_2M_WIDE_EN)
    case IMAGE_SIZE_2M:      ///< 1920x1080
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ImageRes", "2M");
        break;
#endif
#if (MENU_STILL_SIZE_1d2M_EN)
    case IMAGE_SIZE_1_2M:    ///< 1280x960
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set ImageRes", "1D2M");
        break;
#endif
#if (MENU_STILL_SIZE_VGA_EN)
    case IMAGE_SIZE_VGA:
        printf("unsupport!");
        break;
#endif
    default:
        break;
    }
}

void carimpl_CaptureFunc_SetBurstShot(void)
{
    switch (MenuSettingConfig()->uiBurstShot)
    {
    #if (MENU_STILL_BURST_SHOT_OFF_EN)
    case BURST_SHOT_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set StillBurstShot", "OFF");
        break;
    #endif
    #if (MENU_STILL_BURST_SHOT_LO_EN)
    case BURST_SHOT_LO:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set StillBurstShot", "LO");
        break;
    #endif
    #if (MENU_STILL_BURST_SHOT_MID_EN)
    case BURST_SHOT_MID:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set StillBurstShot", "MID");
        break;
    #endif
    #if (MENU_STILL_BURST_SHOT_HI_EN)
    case BURST_SHOT_HI:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set StillBurstShot", "HI");
        break;
    #endif
    default:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set StillBurstShot", "OFF");
        break;
    }
}

void carimpl_PlaybackFunc_setVolume(void)
{
    switch (MenuSettingConfig()->uiVolume)
    {
    #if (MENU_PLAYBACK_VOLUME_LV0_EN)
    case VOLUME_00:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PlaybackVolume", "00");
        break;
    #endif
    #if (MENU_PLAYBACK_VOLUME_LV1_EN)
    case VOLUME_01:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PlaybackVolume", "01");
        break;
    #endif
    #if (MENU_PLAYBACK_VOLUME_LV2_EN)
    case VOLUME_02:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PlaybackVolume", "02");
        break;
    #endif
    #if (MENU_PLAYBACK_VOLUME_LV3_EN)
    case VOLUME_03:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PlaybackVolume", "03");
        break;
    #endif
    #if (MENU_PLAYBACK_VOLUME_LV4_EN)
    case VOLUME_04:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PlaybackVolume", "04");
        break;
    #endif
    #if (MENU_PLAYBACK_VOLUME_LV5_EN)
    case VOLUME_05:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PlaybackVolume", "05");
        break;
    #endif
    #if (MENU_PLAYBACK_VOLUME_LV6_EN)
    case VOLUME_06:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PlaybackVolume", "06");
        break;
    #endif
    #if (MENU_PLAYBACK_VOLUME_LV7_EN)
    case VOLUME_07:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PlaybackVolume", "07");
        break;
    #endif
    #if (MENU_PLAYBACK_VOLUME_LV8_EN)
    case VOLUME_08:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PlaybackVolume", "08");
        break;
    #endif
    #if (MENU_PLAYBACK_VOLUME_LV9_EN)
    case VOLUME_09:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PlaybackVolume", "09");
        break;
    #endif
    #if (MENU_PLAYBACK_VOLUME_LV10_EN)
    case VOLUME_10:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PlaybackVolume", "10");
        break;
    #endif
    default:
        break;
    }
}

extern int MenuItemGetPlaybackVideoCamId(void);
void carimpl_PlaybackFunc_setVideoType(void)
{
    int iType = MenuItemGetPlaybackVideoCamId();
    switch (MenuSettingConfig()->uiPlaybackVideoType)
    {
    #if (MENU_PLAYBACK_VIDEO_TYPE_NORMAL_EN)
    case 0:
        Carimpl_SetCurFolder(DB_NORMAL, iType);
        break;
    #endif
    #if (MENU_PLAYBACK_VIDEO_TYPE_PARKING_EN)
    case 1:
        Carimpl_SetCurFolder(DB_PARKING, iType);
        break;
    #endif
    #if (MENU_PLAYBACK_VIDEO_TYPE_EMERGENCY_EN)
    case 2:
        Carimpl_SetCurFolder(DB_EVENT, iType);
        break;
    #endif
    case 3:
        Carimpl_SetCurFolder(DB_SHARE, iType);
        break;
    default:
        Carimpl_SetCurFolder(DB_NORMAL, iType);
        break;
    }
    // Carimpl_SetCurFileIdx(0);
}

void carimpl_PlaybackFunc_setImageType(void)
{
    int iType = MenuItemGetPlaybackVideoCamId();
    Carimpl_SetCurFolder(DB_PHOTO, iType);
    // Carimpl_SetCurFileIdx(0);
}

void carimpl_PlaybackFunc_DeleteOne(void)
{
    int MaxDcfObj, CurObjIdx;

    if (!IMPL_SD_ISMOUNT) {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        return;
    }
    MaxDcfObj = Carimpl_GetTotalFiles();
    if (MaxDcfObj == 0) {
        carimpl_show_wmsg(true, WMSG_NO_FILE_IN_BROWSER);
        return;
    }
    CurObjIdx = Carimpl_GetCurFileIdx();
    if (Carimpl_DeleteFile(CurObjIdx) == 0)
        carimpl_show_wmsg(true, WMSG_DELETE_FILE_OK);
    else
        carimpl_show_wmsg(true, WMSG_CANNOT_DELETE);

    carimpl.stCapInfo.u32FileCnt = DCF_GetDBFileCntByDB(DB_PHOTO);
    IPC_CarInfo_Write_CapInfo(&carimpl.stCapInfo);
}

void carimpl_PlaybackFunc_DeleteAll_Video(void)
{
    int MaxDcfObj, CurObjIdx, DeleteCnt = 0;
    int usedIdx = 0;
    int i;
    int iType = MenuItemGetPlaybackVideoCamId();
    bool bDBSwitch = false;

    if (!IMPL_SD_ISMOUNT) {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        return;
    }
    if (Carimpl_GetCurDB() == DB_PHOTO)
        bDBSwitch = true;
    if (bDBSwitch)
        carimpl_PlaybackFunc_setVideoType();
    MaxDcfObj = Carimpl_GetTotalFiles();

    if (MaxDcfObj == 0) {
        carimpl_show_wmsg(true, WMSG_NO_FILE_IN_BROWSER);
        if (bDBSwitch)
            Carimpl_SetCurFolder(DB_PHOTO, iType);
        return;
    }
    usedIdx = Carimpl_GetCurFileIdx();
    Carimpl_SetCurFileIdx(0);
    for (i = 0; i < MaxDcfObj; i++) {
        CurObjIdx = Carimpl_GetCurFileIdx();
        if (Carimpl_DeleteFile(CurObjIdx) != 0) {
            CurObjIdx++;
            Carimpl_SetCurFileIdx(CurObjIdx);
        } else {
            DeleteCnt++;
        }
    }

    if (DeleteCnt)
        carimpl_show_wmsg(true, WMSG_DELETE_FILE_OK);
    else {
        carimpl_show_wmsg(true, WMSG_CANNOT_DELETE);
        Carimpl_SetCurFileIdx(usedIdx);
    }
    if (bDBSwitch)
        Carimpl_SetCurFolder(DB_PHOTO, iType);
    return;
}

void carimpl_PlaybackFunc_DeleteAll_Image(void)
{
    int MaxDcfObj, CurObjIdx, DeleteCnt = 0;
    int usedIdx = 0;
    int i;
    int iType = MenuItemGetPlaybackVideoCamId();
    bool bDBSwitch = false;

    if (!IMPL_SD_ISMOUNT) {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        return;
    }
    if (Carimpl_GetCurDB() != DB_PHOTO)
        bDBSwitch = true;
    if (bDBSwitch)
        Carimpl_SetCurFolder(DB_PHOTO, iType);
    MaxDcfObj = Carimpl_GetTotalFiles();
    if (MaxDcfObj == 0) {
        carimpl_show_wmsg(true, WMSG_NO_FILE_IN_BROWSER);
        if (bDBSwitch)
            carimpl_PlaybackFunc_setVideoType();
        return;
    }
    usedIdx = Carimpl_GetCurFileIdx();
    Carimpl_SetCurFileIdx(0);
    for (i = 0; i < MaxDcfObj; i++) {
        CurObjIdx = Carimpl_GetCurFileIdx();
        if (Carimpl_DeleteFile(CurObjIdx) != 0) {
            CurObjIdx++;
            Carimpl_SetCurFileIdx(CurObjIdx);
        } else {
            DeleteCnt++;
        }
    }

    if (DeleteCnt)
        carimpl_show_wmsg(true, WMSG_DELETE_FILE_OK);
    else {
        carimpl_show_wmsg(true, WMSG_CANNOT_DELETE);
        Carimpl_SetCurFileIdx(usedIdx);
    }
    carimpl.stCapInfo.u32FileCnt = DCF_GetDBFileCntByDB(DB_PHOTO);
    IPC_CarInfo_Write_CapInfo(&carimpl.stCapInfo);
    if (bDBSwitch)
        carimpl_PlaybackFunc_setVideoType();
    return;
}

void carimpl_PlaybackFunc_ProtectOne(void)
{
    int MaxDcfObj, CurObjIdx;

    if (!IMPL_SD_ISMOUNT) {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        return;
    }
    MaxDcfObj = Carimpl_GetTotalFiles();
    if (MaxDcfObj == 0) {
        carimpl_show_wmsg(true, WMSG_NO_FILE_IN_BROWSER);
        return;
    }
    CurObjIdx = Carimpl_GetCurFileIdx();
    if (Carimpl_ProtectFile(CurObjIdx) == 0)
        carimpl_show_wmsg(true, WMSG_PROTECT_FILE_OK);
    else
        carimpl_show_wmsg(true, WMSG_INVALID_OPERATION);
}

void carimpl_PlaybackFunc_ProtectAll_Video(void)
{
    int MaxDcfObj;
    int i;
    int iType = MenuItemGetPlaybackVideoCamId();
    bool bDBSwitch = false;

    if (!IMPL_SD_ISMOUNT) {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        return;
    }
    if (Carimpl_GetCurDB() == DB_PHOTO)
        bDBSwitch = true;
    if (bDBSwitch)
        carimpl_PlaybackFunc_setVideoType();
    MaxDcfObj = Carimpl_GetTotalFiles();
    if (MaxDcfObj == 0) {
        carimpl_show_wmsg(true, WMSG_NO_FILE_IN_BROWSER);
        if (bDBSwitch)
            Carimpl_SetCurFolder(DB_PHOTO, iType);
        return;
    }
    for (i = 0; i < MaxDcfObj; i++)
    {
        if (Carimpl_ProtectFile(i) != 0)
        {
            carimpl_show_wmsg(true, WMSG_INVALID_OPERATION);
            continue;
        }
        //DrawProgressBar
    }
    carimpl_show_wmsg(true, WMSG_PROTECT_FILE_OK);
    if (bDBSwitch)
        Carimpl_SetCurFolder(DB_PHOTO, iType);
}

void carimpl_PlaybackFunc_ProtectAll_Image(void)
{
    int MaxDcfObj;
    int i;
    int iType = MenuItemGetPlaybackVideoCamId();
    bool bDBSwitch = false;

    if (!IMPL_SD_ISMOUNT) {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        return;
    }
    if (Carimpl_GetCurDB() != DB_PHOTO)
        bDBSwitch = true;
    if (bDBSwitch)
        Carimpl_SetCurFolder(DB_PHOTO, iType);
    MaxDcfObj = Carimpl_GetTotalFiles();
    if (MaxDcfObj == 0) {
        carimpl_show_wmsg(true, WMSG_NO_FILE_IN_BROWSER);
        if (bDBSwitch)
            carimpl_PlaybackFunc_setVideoType();
        return;
    }
    for (i = 0; i < MaxDcfObj; i++)
    {
        if (Carimpl_ProtectFile(i) != 0)
        {
            carimpl_show_wmsg(true, WMSG_INVALID_OPERATION);
            continue;
        }
        //DrawProgressBar
    }
    carimpl_show_wmsg(true, WMSG_PROTECT_FILE_OK);
    if (bDBSwitch)
        carimpl_PlaybackFunc_setVideoType();
}

void carimpl_PlaybackFunc_UnProtectOne(void)
{
    int MaxDcfObj, CurObjIdx;

    if (!IMPL_SD_ISMOUNT) {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        return;
    }
    MaxDcfObj = Carimpl_GetTotalFiles();
    if (MaxDcfObj == 0) {
        carimpl_show_wmsg(true, WMSG_NO_FILE_IN_BROWSER);
        return;
    }
    CurObjIdx = Carimpl_GetCurFileIdx();
    if (Carimpl_UnProtectFile(CurObjIdx) != 0)
        carimpl_show_wmsg(true, WMSG_INVALID_OPERATION);
}

void carimpl_PlaybackFunc_UnProtectAll_Video(void)
{
    int MaxDcfObj;
    int i;
    int iType = MenuItemGetPlaybackVideoCamId();
    bool bDBSwitch = false;

    if (!IMPL_SD_ISMOUNT) {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        return;
    }
    if (Carimpl_GetCurDB() == DB_PHOTO)
        bDBSwitch = true;
    if (bDBSwitch)
        carimpl_PlaybackFunc_setVideoType();
    MaxDcfObj = Carimpl_GetTotalFiles();
    if (MaxDcfObj == 0) {
        carimpl_show_wmsg(true, WMSG_NO_FILE_IN_BROWSER);
        if (bDBSwitch)
            Carimpl_SetCurFolder(DB_PHOTO, iType);
        return;
    }
    for (i = 0; i < MaxDcfObj; i++)
    {
        if (Carimpl_UnProtectFile(i) != 0)
        {
            carimpl_show_wmsg(true, WMSG_INVALID_OPERATION);
            continue;
        }
        //DrawProgressBar
    }
    if (bDBSwitch)
        Carimpl_SetCurFolder(DB_PHOTO, iType);
}

void carimpl_PlaybackFunc_UnProtectAll_Image(void)
{
    int MaxDcfObj;
    int i;
    int iType = MenuItemGetPlaybackVideoCamId();
    bool bDBSwitch = false;

    if (!IMPL_SD_ISMOUNT) {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        return;
    }
    if (Carimpl_GetCurDB() != DB_PHOTO)
        bDBSwitch = true;
    if (bDBSwitch)
        Carimpl_SetCurFolder(DB_PHOTO, iType);
    MaxDcfObj = Carimpl_GetTotalFiles();
    if (MaxDcfObj == 0) {
        carimpl_show_wmsg(true, WMSG_NO_FILE_IN_BROWSER);
        if (bDBSwitch)
            carimpl_PlaybackFunc_setVideoType();
        return;
    }
    for (i = 0; i < MaxDcfObj; i++)
    {
        if (Carimpl_UnProtectFile(i) != 0)
        {
            carimpl_show_wmsg(true, WMSG_INVALID_OPERATION);
            continue;
        }
        //DrawProgressBar
    }
    if (bDBSwitch)
        carimpl_PlaybackFunc_setVideoType();
}

void carimpl_MediaToolFunc_FormatSDCard(void)
{
    if (!IMPL_SD_ISINSERT) {
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        return;
    }

    if (gu8LastUIMode == UI_PLAYBACK_MODE) {
        carimpl_show_wmsg(true, WMSG_INVALID_OPERATION);
        return;
    }

    if (gu8LastUIMode == UI_BROWSER_MODE)
        Carimpl_DcfUnmount();

    carimpl_show_wmsg(true, WMSG_FORMAT_SD_PROCESSING);
    carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set SD0", "");
}

void carimpl_GeneralFunc_SetBeep(void)
{
    switch (MenuSettingConfig()->uiBeep)
    {
    #if (MENU_GENERAL_BEEP_ON_EN)
    case BEEP_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Beep", "ON");
        break;
    #endif
    #if (MENU_GENERAL_BEEP_OFF_EN)
    case BEEP_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Beep", "OFF");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_GeneralFunc_SetLcdBrightness(void)
{
    char value[256] = {0};
    sprintf(value, "%d", MenuSettingConfig()->uiLCDBrightness);
    carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LCDBrightness", value);
}


void carimpl_GeneralFunc_SetAutoPowerOff(void)
{
    switch (MenuSettingConfig()->uiAutoPowerOff)
    {
    #if (MENU_GENERAL_AUTO_POWEROFF_NEVER_EN)
    case AUTO_POWER_OFF_NEVER:
        gulAutoPowerOffRtcTime = 0;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AutoPowerOff", "NEVER");
        break;
    #endif
    #if (MENU_GENERAL_AUTO_POWEROFF_15SEC_EN)
    case AUTO_POWER_OFF_15SEC:
        gulAutoPowerOffRtcTime = 15;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AutoPowerOff", "15SEC");
        break;
    #endif
    #if (MENU_GENERAL_AUTO_POWEROFF_30SEC_EN)
    case AUTO_POWER_OFF_30SEC:
        gulAutoPowerOffRtcTime = 30;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AutoPowerOff", "30SEC");
        break;
    #endif
    #if (MENU_GENERAL_AUTO_POWEROFF_1MIN_EN)
    case AUTO_POWER_OFF_1MIN:
        gulAutoPowerOffRtcTime = 60;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AutoPowerOff", "1MIN");
        break;
    #endif
    #if (MENU_GENERAL_AUTO_POWEROFF_2MIN_EN)
    case AUTO_POWER_OFF_2MINS:
        gulAutoPowerOffRtcTime = 120;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AutoPowerOff", "2MINS");
        break;
    #endif
    #if (MENU_GENERAL_AUTO_POWEROFF_3MIN_EN)
    case AUTO_POWER_OFF_3MINS:
        gulAutoPowerOffRtcTime = 180;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AutoPowerOff", "3MINS");
        break;
    #endif
    #if (MENU_GENERAL_AUTO_POWEROFF_5MIN_EN)
    case AUTO_POWER_OFF_5MINS:
        gulAutoPowerOffRtcTime = 300;
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set AutoPowerOff", "5MINS");
        break;
    #endif
    default:
        break;
    }
    long ulRtcCurrentTime = TimeHelper::getCurrentTime();
    gulAutoPowerOffRtcTime += ulRtcCurrentTime;
    if (0 == gulAutoPowerOffRtcTime) {
        // overflow
        gulAutoPowerOffRtcTime++;
    }
}

void carimpl_GeneralFunc_SetDatetimeFormat(void)
{
    switch (MenuSettingConfig()->uiDateTimeFormat)
    {
    #if (MENU_GENERAL_DATE_FORMAT_NONE_EN)
    case DATETIME_SETUP_NONE:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set DateTimeFormat", "NONE");
        break;
    #endif
    #if (MENU_GENERAL_DATE_FORMAT_YMD_EN)
    case DATETIME_SETUP_YMD:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set DateTimeFormat", "YMD");
        break;
    #endif
    #if (MENU_GENERAL_DATE_FORMAT_MDY_EN)
    case DATETIME_SETUP_MDY:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set DateTimeFormat", "MDY");
        break;
    #endif
    #if (MENU_GENERAL_DATE_FORMAT_DMY_EN)
    case DATETIME_SETUP_DMY:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set DateTimeFormat", "DMY");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_GeneralFunc_SetDatelogoStamp(void)
{
    switch (MenuSettingConfig()->uiDateLogoStamp)
    {
    #if (MENU_GENERAL_DATE_LOGO_STAMP_DATE_LOGO_EN)
    case DATELOGO_STAMP:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set DateLogoStamp", "DATELOGO");
        break;
    #endif
    #if (MENU_GENERAL_DATE_LOGO_STAMP_DATE_EN)
    case DATE_STAMP:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set DateLogoStamp", "DATE");
        break;
    #endif
    #if (MENU_GENERAL_DATE_LOGO_STAMP_LOGO_EN)
    case LOGO_STAMP:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set DateLogoStamp", "LOGO");
        break;
    #endif
    #if (MENU_GENERAL_DATE_LOGO_STAMP_OFF_EN)
    case OFF_STAMP:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set DateLogoStamp", "OFF");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_GeneralFunc_SetGpsStamp(void)
{
    switch (MenuSettingConfig()->uiGPSStamp)
    {
    #if (MENU_GENERAL_GPS_STAMP_ON_EN)
    case GPS_STAMP_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set GpsStamp", "ON");
        break;
    #endif
    #if (MENU_GENERAL_GPS_STAMP_OFF_EN)
    case GPS_STAMP_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set GpsStamp", "OFF");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_GeneralFunc_SetSpeedStamp(void)
{
    switch (MenuSettingConfig()->uiSpeedStamp)
    {
    #if (MENU_GENERAL_SPEED_STAMP_ON_EN)
    case SPEED_STAMP_ON:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set SpeedStamp", "ON");
        break;
    #endif
    #if (MENU_GENERAL_SPEED_STAMP_OFF_EN)
    case SPEED_STAMP_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set SpeedStamp", "OFF");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_GeneralFunc_SetLanguage(void)
{
    switch (MenuSettingConfig()->uiLanguage)
    {
    #if (MENU_GENERAL_LANGUAGE_ENGLISH_EN)
    case LANGUAGE_ENGLISH:
        EASYUICONTEXT->updateLocalesCode("en_US");
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "English");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_SPANISH_EN)
    case LANGUAGE_SPANISH:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Spanish");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_PORTUGUESE_EN)
    case LANGUAGE_PORTUGUESE:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Portuguese");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_RUSSIAN_EN)
    case LANGUAGE_RUSSIAN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Russian");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_SCHINESE_EN)
    case LANGUAGE_SCHINESE:
        EASYUICONTEXT->updateLocalesCode("zh_CN");
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Simplified Chinese");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_TCHINESE_EN)
    case LANGUAGE_TCHINESE:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Traditional Chinese");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_GERMAN_EN)
    case LANGUAGE_GERMAN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "German");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_ITALIAN_EN)
    case LANGUAGE_ITALIAN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Italian");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_LATVIAN_EN)
    case LANGUAGE_LATVIAN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Latvian");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_POLISH_EN)
    case LANGUAGE_POLISH:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Polish");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_ROMANIAN_EN)
    case LANGUAGE_ROMANIAN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Romanian");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_SLOVAK_EN)
    case LANGUAGE_SLOVAK:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Slovak");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_UKRANINIAN_EN)
    case LANGUAGE_UKRANINIAN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "UKRomanian");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_FRENCH_EN)
    case LANGUAGE_FRENCH:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "French");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_JAPANESE_EN)
    case LANGUAGE_JAPANESE:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Japanese");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_KOREAN_EN)
    case LANGUAGE_KOREAN:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Korean");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_CZECH_EN)
    case LANGUAGE_CZECH:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set Language", "Czech");
        break;
    #endif
    default:
        break;
    }
}

void carimpl_GeneralFunc_UpdateUILocalesCode(void)
{
    switch (MenuSettingConfig()->uiLanguage)
    {
    #if (MENU_GENERAL_LANGUAGE_ENGLISH_EN)
    case LANGUAGE_ENGLISH:
        EASYUICONTEXT->updateLocalesCode("en_US");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_SPANISH_EN)
    case LANGUAGE_SPANISH:
        EASYUICONTEXT->updateLocalesCode("es_ES");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_PORTUGUESE_EN)
    case LANGUAGE_PORTUGUESE:
        EASYUICONTEXT->updateLocalesCode("pt_PT");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_RUSSIAN_EN)
    case LANGUAGE_RUSSIAN:
        EASYUICONTEXT->updateLocalesCode("ru_RU");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_SCHINESE_EN)
    case LANGUAGE_SCHINESE:
        EASYUICONTEXT->updateLocalesCode("zh_CN");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_TCHINESE_EN)
    case LANGUAGE_TCHINESE:
        EASYUICONTEXT->updateLocalesCode("zh_TW");  // Traditional Chinese,TW
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_GERMAN_EN)
    case LANGUAGE_GERMAN:
        EASYUICONTEXT->updateLocalesCode("de_DE");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_ITALIAN_EN)
    case LANGUAGE_ITALIAN:
        EASYUICONTEXT->updateLocalesCode("it_IT");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_LATVIAN_EN)
    case LANGUAGE_LATVIAN:
        EASYUICONTEXT->updateLocalesCode("lv_LV");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_POLISH_EN)
    case LANGUAGE_POLISH:
        EASYUICONTEXT->updateLocalesCode("pl_PL");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_ROMANIAN_EN)
    case LANGUAGE_ROMANIAN:
        EASYUICONTEXT->updateLocalesCode("ro_RO");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_SLOVAK_EN)
    case LANGUAGE_SLOVAK:
        EASYUICONTEXT->updateLocalesCode("sk_SK");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_UKRANINIAN_EN)
    case LANGUAGE_UKRANINIAN:
        printf("err Has NO UKRomanian!! \n");
        EASYUICONTEXT->updateLocalesCode("en_US");  // set to default language
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_FRENCH_EN)
    case LANGUAGE_FRENCH:
        EASYUICONTEXT->updateLocalesCode("fr_FR");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_JAPANESE_EN)
    case LANGUAGE_JAPANESE:
        EASYUICONTEXT->updateLocalesCode("ja_JP");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_KOREAN_EN)
    case LANGUAGE_KOREAN:
        EASYUICONTEXT->updateLocalesCode("ko_KR");
        break;
    #endif
    #if (MENU_GENERAL_LANGUAGE_CZECH_EN)
    case LANGUAGE_CZECH:
        EASYUICONTEXT->updateLocalesCode("cs_CZ");
        break;
    #endif
    default:
        EASYUICONTEXT->updateLocalesCode("en_US");  // set to default language
        break;
    }
}

void carimpl_GeneralFunc_SetResetSetting(void)
{
    if (MenuSettingConfig()->uiResetSetting != 0)
    {
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set FactoryReset", "0");
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set RebootSystem", "0");
        //MenuSettingConfig()->uiResetSetting = 0;
    }
}

void carimpl_GeneralFunc_SetUsbFunction(void)
{
    switch (MenuSettingConfig()->uiUSBFunction)
    {
        #if (MENU_GENERAL_USB_FUNCTION_MSDC_EN)
        case MENU_SETTING_USB_MSDC:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set UsbFunction", "MSDC");
            break;
        #endif
        #if (MENU_GENERAL_USB_FUNCTION_PCAM_EN)
        case MENU_SETTING_USB_PCAM:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set UsbFunction", "PCAM");
            break;
        #endif
        default:
            break;
    }
}

void carimpl_GeneralFunc_SetLcdPowerSave(void)
{
    switch (MenuSettingConfig()->uiLCDPowerSave)
    {
        #if (MENU_GENERAL_LCD_POWER_SAVE_OFF_EN)
        case LCD_POWER_SAVE_OFF:
            gulLCDPowerSaveRtcTime = 0;
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LcdPowerSave", "OFF");
            break;
        #endif
        #if (MENU_GENERAL_LCD_POWER_SAVE_10SEC_EN)
        case LCD_POWER_SAVE_10SEC:
            gulLCDPowerSaveRtcTime = 10;
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LcdPowerSave", "10SEC");
            break;
        #endif
        #if (MENU_GENERAL_LCD_POWER_SAVE_30SEC_EN)
        case LCD_POWER_SAVE_30SEC:
            gulLCDPowerSaveRtcTime = 30;
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LcdPowerSave", "30SEC");
            break;
        #endif
        #if (MENU_GENERAL_LCD_POWER_SAVE_1MIN_EN)
        case LCD_POWER_SAVE_1MIN:
            gulLCDPowerSaveRtcTime = 60;
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LcdPowerSave", "1MIN");
            break;
        #endif
        #if (MENU_GENERAL_LCD_POWER_SAVE_3MIN_EN)
        case LCD_POWER_SAVE_3MIN:
            gulLCDPowerSaveRtcTime = 180;
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set LcdPowerSave", "3MIN");
            break;
        #endif
        default:
            break;
    }
    long ulRtcCurrentTime = TimeHelper::getCurrentTime();
    gulLCDPowerSaveRtcTime += ulRtcCurrentTime;
}

void carimpl_GeneralFunc_setGSensorSensitivity(void)
{
    switch (MenuSettingConfig()->uiGsensorSensitivity)
    {
        #if (MENU_GENERAL_GSENSOR_OFF_EN)
        case GSENSOR_SENSITIVITY_OFF:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set GSensor", "OFF");
            break;
        #endif
        #if (MENU_GENERAL_GSENSOR_LEVEL0_EN)
        case GSENSOR_SENSITIVITY_L0:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set GSensor", "LEVEL0");
            break;
        #endif
        #if (MENU_GENERAL_GSENSOR_LEVEL1_EN)
        case GSENSOR_SENSITIVITY_L1:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set GSensor", "LEVEL1");
            break;
        #endif
        #if (MENU_GENERAL_GSENSOR_LEVEL2_EN)
        case GSENSOR_SENSITIVITY_L2:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set GSensor", "LEVEL2");
            break;
        #endif
        #if (MENU_GENERAL_GSENSOR_LEVEL3_EN)
        case GSENSOR_SENSITIVITY_L3:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set GSensor", "LEVEL3");
            break;
        #endif
        #if (MENU_GENERAL_GSENSOR_LEVEL4_EN)
        case GSENSOR_SENSITIVITY_L4:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set GSensor", "LEVEL4");
            break;
        #endif
    default:
        break;
    }
}

void carimpl_GeneralFunc_SetPowerOnGSensorSensitivity(void)
{
    printf("%s,%d\n",__func__, MenuSettingConfig()->uiPowerOnGsensorSensitivity);
    switch (MenuSettingConfig()->uiPowerOnGsensorSensitivity)
    {
    #if (MENU_GENERAL_POWERON_GSENSOR_OFF_EN)
    case GSENSOR_POWERON_SENSITIVITY_OFF:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PowerOnGSensor", "OFF");
        break;
    #endif
    #if (MENU_GENERAL_POWER_ON_GSENSOR_LEVEL0_EN)
    case GSENSOR_POWERON_SENSITIVITY_L0:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PowerOnGSensor", "LEVEL0");
        break;
    #endif
    #if (MENU_GENERAL_POWER_ON_GSENSOR_LEVEL1_EN)
    case GSENSOR_POWERON_SENSITIVITY_L1:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PowerOnGSensor", "LEVEL1");
        break;
    #endif
    #if (MENU_GENERAL_POWER_ON_GSENSOR_LEVEL2_EN)
    case GSENSOR_POWERON_SENSITIVITY_L2:
        carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set PowerOnGSensor", "LEVEL2");
        break;
    #endif
        default:
            break;
    }
}

void carimpl_SysFunc_SetPowerOnGSensorSensitivity(void)
{

#if (MENU_GENERAL_POWERON_GSENSOR_OFF_EN)
    if(GSENSOR_POWERON_SENSITIVITY_OFF == MenuSettingConfig()->uiPowerOnGsensorSensitivity)
        return;
#endif

    //Threshold of active interrupt=active_th*K(mg)
    //K = 3.91(2g range),
    //K = 7.81(4g range),
    //K = 15.625(8g range),
    //K = 31.25mg(16g range)
    switch (MenuSettingConfig()->uiPowerOnGsensorSensitivity)
    {
#if (MENU_GENERAL_POWER_ON_GSENSOR_LEVEL0_EN)
    case GSENSOR_POWERON_SENSITIVITY_L0:
        system("echo 0x70 > /sys/devices/virtual/input/input0/slope_th");
    break;
#endif
#if (MENU_GENERAL_POWER_ON_GSENSOR_LEVEL1_EN)
    case GSENSOR_POWERON_SENSITIVITY_L1:
        system("echo 0xa0 > /sys/devices/virtual/input/input0/slope_th");
    break;
#endif
#if (MENU_GENERAL_POWER_ON_GSENSOR_LEVEL2_EN)
    case GSENSOR_POWERON_SENSITIVITY_L2:
        system("echo 0xc0 > /sys/devices/virtual/input/input0/slope_th");
    break;
#endif
    default:
    break;
    }
    system("echo 1 > /sys/devices/virtual/input/input0/int2_enable");
    usleep(10000);
}

void carimpl_GeneralFunc_SetMotionDetect(void)
{
    switch (MenuSettingConfig()->uiMotionDtcSensitivity)
    {
        #if (MENU_GENERAL_MOTION_DTC_OFF_EN)
        case MOTION_DTC_SENSITIVITY_OFF:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set MotionDetect", "OFF");
            break;
        #endif
        #if (MENU_GENERAL_MOTION_DTC_LOW_EN)
        case MOTION_DTC_SENSITIVITY_LOW:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set MotionDetect", "LOW");
            break;
        #endif
        #if (MENU_GENERAL_MOTION_DTC_MID_EN)
        case MOTION_DTC_SENSITIVITY_MID:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set MotionDetect", "MID");
            break;
        #endif
        #if (MENU_GENERAL_MOTION_DTC_HIGH_EN)
        case MOTION_DTC_SENSITIVITY_HIGH:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set MotionDetect", "HIGH");
            break;
        #endif
        default:
            break;
    }
}

void carimpl_GeneralFunc_SetTimeZone(void)
{
    char strTZ[12] = {0};
    //sync cardv tz
    switch (MenuSettingConfig()->uiTimeZone)
    {
        case TIMEZONE_GMT_M_12:  //00:0x00
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_12");
            break;
        case TIMEZONE_GMT_M_11:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_11");
            break;
        case TIMEZONE_GMT_M_10:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_10");
            break;
        case TIMEZONE_GMT_M_9:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_9");
            break;
        case TIMEZONE_GMT_M_8:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_8");
            break;
        case TIMEZONE_GMT_M_7:   //05:0x05
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_7");
            break;
        case TIMEZONE_GMT_M_6:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_6");
            break;
        case TIMEZONE_GMT_M_5:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_5");
            break;
        case TIMEZONE_GMT_M_4:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_4");
            break;
        case TIMEZONE_GMT_M_3_30:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_3_30");
            break;
        case TIMEZONE_GMT_M_3:   //10:0x0A
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_3");
            break;
        case TIMEZONE_GMT_M_2:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_2");
            break;
        case TIMEZONE_GMT_M_1:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_M_1");
            break;
        case TIMEZONE_GMT:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT00");
            break;
        case TIMEZONE_GMT_P_1:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_1");
            break;
        case TIMEZONE_GMT_P_2:   //15:0x0F
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_2");
            break;
        case TIMEZONE_GMT_P_3:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_3");
            break;
        case TIMEZONE_GMT_P_3_30:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_3_30");
            break;
        case TIMEZONE_GMT_P_4:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_4");
            break;
        case TIMEZONE_GMT_P_4_30:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_4_30");
            break;
        case TIMEZONE_GMT_P_5:   //20:0x14
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_5");
            break;
        case TIMEZONE_GMT_P_5_30:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_5_30");
            break;
        case TIMEZONE_GMT_P_5_45:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_5_45");
            break;
        case TIMEZONE_GMT_P_6:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_6");
            break;
        case TIMEZONE_GMT_P_6_30:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_6_30");
            break;
        case TIMEZONE_GMT_P_7:   //25:0x19
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_7");
            break;
        case TIMEZONE_GMT_P_8:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_8");
            break;
        case TIMEZONE_GMT_P_9:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_9");
            break;
        case TIMEZONE_GMT_P_9_30:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_9_30");
            break;
        case TIMEZONE_GMT_P_10:
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_10");
            break;
        case TIMEZONE_GMT_P_11:  //30:0x1E
            carimpl_ExecCmdHandler(CGI_CMD_TYPE, "set TimeZone", "GMT_P_11");
            break;
        default:
            break;
    }
    //sync ui tz
    Menu_General_EnGet((char*)COMMON_KEY_TIME_ZONE, strTZ, sizeof(strTZ));
    setenv("TZ", strTZ, 1);
    tzset();
}

void carimpl_GeneralFunc_SetWifiOnOff(void)
{
    switch (MenuSettingConfig()->uiWifi)
    {
        #if (MENU_GENERAL_WIFI_ON_EN)
        case WIFI_MODE_ON:
            if(carimpl_ExecCmdHandler(SCRIPT_CMD_TYPE, "net_toggle.sh", "")){
                MenuSettingConfig()->uiWifi = WIFI_MODE_OFF;
            }
            break;
        #endif
        #if (MENU_GENERAL_WIFI_OFF_EN)
        case WIFI_MODE_OFF:
            if(carimpl_ExecCmdHandler(SCRIPT_CMD_TYPE, "net_toggle.sh", "")){
                MenuSettingConfig()->uiWifi = WIFI_MODE_ON;
            }
            break;
        #endif
        default:
            break;
    }
}

#ifdef SUPPORT_CARLIFE
void carimpl_GeneralFunc_SetCarLifeOnOff(void)
{
    switch (MenuSettingConfig()->uiWifi)
    {
        #if (MENU_GENERAL_WIFI_ON_EN)
        case WIFI_MODE_ON:
            if(carimpl_ExecCmdHandler(SCRIPT_CMD_TYPE, "carlife.sh", "")){
                MenuSettingConfig()->uiWifi = WIFI_MODE_OFF;
            }
            break;
        #endif
        #if (MENU_GENERAL_WIFI_OFF_EN)
        case WIFI_MODE_OFF:
            if(carimpl_ExecCmdHandler(SCRIPT_CMD_TYPE, "carlife.sh", "")){
                MenuSettingConfig()->uiWifi = WIFI_MODE_ON;
            }
            break;
        #endif
        default:
            break;
    }
}
#endif

void carimpl_GeneralFunc_GetSoftwareVersion(void)
{
    carimpl_ExecCmdHandler(CGI_CMD_TYPE, "get FwVer", "");
}

//echo disp start CamIdx > /tmp/cardv_fifo
//echo disp stop CamIdx > /tmp/cardv_fifo
//echo disp switch CamIdx > /tmp/cardv_fifo
void carimpl_GeneralFunc_SetPreviewCamIdx(void)
{
    static unsigned char u8CamIdx = 0;
    char strParam[4] = {0};

    if (carimpl.stDispInfo.u8SwitchNum > 1) {
        u8CamIdx++;
        u8CamIdx = u8CamIdx % carimpl.stDispInfo.u8SwitchNum;
        sprintf(strParam, "%d", u8CamIdx);
        //printf("%s %s\n",__FUNCTION__,strParam);
        carimpl_ExecCmdHandler(FIFO_CMD_TYPE, "disp switch", strParam);
    }
}

void carimpl_MenuSetting_Init(void)
{
    carimpl_GeneralFunc_UpdateUILocalesCode();
}

static void carimpl_PowerSavingModeDetection(long ulRtcCurrentTime)
{
    //check record status and playback status first
    if (strcmp(EASYUICONTEXT->currentAppName(), "playerActivity"))
    {
        carimpl_GeneralFunc_SetAutoPowerOff();
        return;
    }
    if (gulAutoPowerOffRtcTime != 0)
    {
        int time_diff = (int)(ulRtcCurrentTime - gulAutoPowerOffRtcTime);
        // 21600000(ms): avoid overflow case, set system tolerance 6-HR
        // need modify it when UI setting except to 6-HR
        if ((time_diff >= 0) && (time_diff < 21600000))
        {
            printf("\r\n!!! Entering Power saving mode\r\n");
            gulAutoPowerOffRtcTime = 0; // Never enter again
            //force poweroff flow

        }
    }
}

static void carimpl_LCDPowerSaveDetection(long ulRtcCurrentTime)
{
    if (gulLCDPowerSaveRtcTime != 0)
    {
        if ((ulRtcCurrentTime - gulLCDPowerSaveRtcTime) >= 0)
        {
            gulLCDPowerSaveRtcTime = 0;
            //LCD backlight control

        }
    }
}

static void carimpl_VideoPowerOnOffDetection(long ulRtcCurrentTime)
{
    static bool bPoweroff = false;
    if (gulVideoPowerOffRtcTime == 0)
        return;
    if ((int)(ulRtcCurrentTime - gulVideoPowerOffRtcTime) > 0 && bPoweroff == false)
    {
        bPoweroff = true;
        printf("!!! Auto Power off %ld\r\n", gulVideoPowerOffRtcTime);
        //force poweroff flow
    }
}

void DeviceDetection(void)
{
    static int DetectCnt = 0;
    long ulRtcCurrentTime = 0;
    while (1)
    {
        DetectCnt++;
        if (DetectCnt == 20)
        {
            DetectCnt = 0;
            ulRtcCurrentTime = TimeHelper::getCurrentTime();
            carimpl_PowerSavingModeDetection(ulRtcCurrentTime);
            carimpl_LCDPowerSaveDetection(ulRtcCurrentTime);
            carimpl_VideoPowerOnOffDetection(ulRtcCurrentTime);
        }
    }
}
/**********************************************************/
/********************** wifi ***************************/
/**********************************************************/
NET_STATUS_TYPE check_wifi_status(void)
{
    NET_STATUS_TYPE eRet = NET_STATUS_INIT_FAIL;
    int net_fd = -1;
    char status[10] = {0};
    net_fd = open("/sys/class/net/wlan0/operstate", O_RDONLY);
    if (net_fd < 0) {
        eRet = NET_STATUS_INIT_FAIL;
    } else {
        if (read(net_fd, status, sizeof(status)) > 0) {
            if (NULL != strstr(status,"up")) {
                eRet = NET_STATUS_UP;
            } else if (NULL != strstr(status,"down")) {
                eRet = NET_STATUS_DOWN;
            } else {
                eRet = NET_STATUS_UNKNOW;
            }
        }
        close (net_fd);
    }

    if (eRet == NET_STATUS_UP) {
        char cmdstr[256] = {0};
        int SignalLevel = 0;
        //AP mode find deviceinfo
        sprintf(cmdstr, "iw dev wlan0 station dump | grep signal | /usr/bin/awk '{print $2}'");
        FILE *fp = popen(cmdstr, "r");
        if (!fp) {
            return eRet;
        }
        memset(gCmdBuf, 0, sizeof(gCmdBuf));
        while (fgets(gCmdBuf, sizeof(gCmdBuf), fp) != NULL) {
            //printf("DeviceInfo: %s\n", gCmdBuf);
        }
        pclose(fp);

        if (strlen(gCmdBuf) != 0) {
            //printf("check signal level dBm\n");
             SignalLevel = atoi(gCmdBuf);
            if (SignalLevel < -60 && SignalLevel > -90)
                eRet = NET_STATUS_WEAK;
            else
                eRet = NET_STATUS_READY;
        }
    }
    return eRet;
}
/**********************************************************/
/**********************others******************************/
/**********************************************************/
MI_S32 carimpl_refresh_display_yuv420p(MI_S32 s32DispWidth, MI_S32 s32DispHeight, void *pYData, void *pUData, void *pVData)
{
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_ChnPort_t pstSysChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;

    if (!s32DispWidth || !s32DispHeight)
        return 0;

    pstSysChnPort.eModId = carimpl.stDispInfo.stPlayBackChnPort.eModId;
    pstSysChnPort.u32DevId = carimpl.stDispInfo.stPlayBackChnPort.u32DevId;
    pstSysChnPort.u32ChnId = carimpl.stDispInfo.stPlayBackChnPort.u32ChnId;
    pstSysChnPort.u32PortId = carimpl.stDispInfo.stPlayBackChnPort.u32PortId;

    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));

    stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.u64TargetPts = 0;
    stBufConf.stFrameCfg.u16Width = s32DispWidth;
    stBufConf.stFrameCfg.u16Height = s32DispHeight;
    stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR;
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

    if (MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&pstSysChnPort, &stBufConf, &stBufInfo, &hHandle, -1)) {
        stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
        stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
        stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
        stBufInfo.bEndOfStream = FALSE;
        MI_SYS_GetCurPts(0, &stBufInfo.u64Pts);

        int bufsize, index;
        bufsize = s32DispWidth * s32DispHeight;

        if (pYData && pUData && pVData) {
            if (stBufInfo.stFrameData.u32Stride[0] == stBufInfo.stFrameData.u16Width) {
                memcpy(stBufInfo.stFrameData.pVirAddr[0], pYData, bufsize);
                memcpy(stBufInfo.stFrameData.pVirAddr[1], pUData, bufsize / 4);
                memcpy(stBufInfo.stFrameData.pVirAddr[2], pVData, bufsize / 4);
            } else {
                for (index = 0; index < stBufInfo.stFrameData.u16Height; index ++) {
                    memcpy((MI_U8 *)stBufInfo.stFrameData.pVirAddr[0] + index * stBufInfo.stFrameData.u32Stride[0],
                           (MI_U8 *)pYData + index * stBufInfo.stFrameData.u16Width,
                           stBufInfo.stFrameData.u16Width);
                }

                for (index = 0; index < stBufInfo.stFrameData.u16Height / 4; index ++) {
                    memcpy((MI_U8 *)stBufInfo.stFrameData.pVirAddr[1] + index * stBufInfo.stFrameData.u32Stride[1],
                           (MI_U8 *)pUData + index * stBufInfo.stFrameData.u16Width,
                           stBufInfo.stFrameData.u16Width);
                }

                for (index = 0; index < stBufInfo.stFrameData.u16Height / 4; index ++) {
                    memcpy((MI_U8 *)stBufInfo.stFrameData.pVirAddr[2] + index * stBufInfo.stFrameData.u32Stride[2],
                           (MI_U8 *)pVData + index * stBufInfo.stFrameData.u16Width,
                           stBufInfo.stFrameData.u16Width);
                }
            }
        } else {
            // Fill black screen
            if (stBufInfo.stFrameData.u32Stride[0] == stBufInfo.stFrameData.u16Width) {
                memset(stBufInfo.stFrameData.pVirAddr[0], 0x00, bufsize);
                memset(stBufInfo.stFrameData.pVirAddr[1], 0x80, bufsize / 4);
                memset(stBufInfo.stFrameData.pVirAddr[2], 0x80, bufsize / 4);
            } else {
                for (index = 0; index < stBufInfo.stFrameData.u16Height; index ++) {
                    memset((MI_U8 *)stBufInfo.stFrameData.pVirAddr[0] + index * stBufInfo.stFrameData.u32Stride[0],
                           0x00,
                           stBufInfo.stFrameData.u16Width);
                }

                for (index = 0; index < stBufInfo.stFrameData.u16Height / 4; index ++) {
                    memset((MI_U8 *)stBufInfo.stFrameData.pVirAddr[1] + index * stBufInfo.stFrameData.u32Stride[1],
                           0x80,
                           stBufInfo.stFrameData.u16Width);
                }

                for (index = 0; index < stBufInfo.stFrameData.u16Height / 4; index ++) {
                    memset((MI_U8 *)stBufInfo.stFrameData.pVirAddr[2] + index * stBufInfo.stFrameData.u32Stride[2],
                           0x80,
                           stBufInfo.stFrameData.u16Width);
                }
            }
        }

        MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, FALSE);
    }

    return 0;
}

MI_S32 carimpl_refresh_display_yuv422(MI_S32 s32DispWidth, MI_S32 s32DispHeight, void *pYData)
{
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_ChnPort_t pstSysChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;

    if (!s32DispWidth || !s32DispHeight)
        return 0;

    pstSysChnPort.eModId = carimpl.stDispInfo.stPlayBackChnPort.eModId;
    pstSysChnPort.u32DevId = carimpl.stDispInfo.stPlayBackChnPort.u32DevId;
    pstSysChnPort.u32ChnId = carimpl.stDispInfo.stPlayBackChnPort.u32ChnId;
    pstSysChnPort.u32PortId = carimpl.stDispInfo.stPlayBackChnPort.u32PortId;

    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));

    stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.u64TargetPts = 0;
    stBufConf.stFrameCfg.u16Width = s32DispWidth;
    stBufConf.stFrameCfg.u16Height = s32DispHeight;
    stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    if (MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&pstSysChnPort, &stBufConf, &stBufInfo, &hHandle, -1)) {
        stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
        stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
        stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
        stBufInfo.bEndOfStream = FALSE;
        MI_SYS_GetCurPts(0, &stBufInfo.u64Pts);

        int bufsize, index;
        bufsize = s32DispWidth * s32DispHeight * 2;

        if (pYData) {
            if (stBufInfo.stFrameData.u32Stride[0] == stBufInfo.stFrameData.u16Width * 2) {
                memcpy(stBufInfo.stFrameData.pVirAddr[0], pYData, bufsize);
            } else {
                for (index = 0; index < stBufInfo.stFrameData.u16Height; index ++) {
                    memcpy((MI_U8 *)stBufInfo.stFrameData.pVirAddr[0] + index * stBufInfo.stFrameData.u32Stride[0],
                           (MI_U8 *)pYData + index * stBufInfo.stFrameData.u16Width * 2,
                           stBufInfo.stFrameData.u16Width * 2);
                }
            }
        } else {
            // Fill black screen
            if (stBufInfo.stFrameData.u32Stride[0] == stBufInfo.stFrameData.u16Width * 2) {
                memset(stBufInfo.stFrameData.pVirAddr[0], 0x00, bufsize);
            } else {
                for (index = 0; index < stBufInfo.stFrameData.u16Height; index ++) {
                    memset((MI_U8 *)stBufInfo.stFrameData.pVirAddr[0] + index * stBufInfo.stFrameData.u32Stride[0],
                           0x00,
                           stBufInfo.stFrameData.u16Width * 2);
                }
            }
        }

        MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, FALSE);
    }

    return 0;
}
MI_S32 carimpl_refresh_display_yuv422p(MI_S32 s32DispWidth, MI_S32 s32DispHeight, void *pYData, void *pUData, void *pVData)
{
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_ChnPort_t pstSysChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;

    if (!s32DispWidth || !s32DispHeight)
        return 0;

    pstSysChnPort.eModId = carimpl.stDispInfo.stPlayBackChnPort.eModId;
    pstSysChnPort.u32DevId = carimpl.stDispInfo.stPlayBackChnPort.u32DevId;
    pstSysChnPort.u32ChnId = carimpl.stDispInfo.stPlayBackChnPort.u32ChnId;
    pstSysChnPort.u32PortId = carimpl.stDispInfo.stPlayBackChnPort.u32PortId;

    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));

    stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.u64TargetPts = 0;
    stBufConf.stFrameCfg.u16Width = s32DispWidth;
    stBufConf.stFrameCfg.u16Height = s32DispHeight;
    stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR;
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

    if (MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&pstSysChnPort, &stBufConf, &stBufInfo, &hHandle, -1)) {
        stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
        stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
        stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
        stBufInfo.bEndOfStream = FALSE;
        MI_SYS_GetCurPts(0, &stBufInfo.u64Pts);

        int bufsize, index;
        bufsize = s32DispWidth * s32DispHeight;

        if (pYData && pUData && pVData) {
            if (stBufInfo.stFrameData.u32Stride[0] == stBufInfo.stFrameData.u16Width) {
                memcpy(stBufInfo.stFrameData.pVirAddr[0], pYData, bufsize);
                memcpy(stBufInfo.stFrameData.pVirAddr[1], pUData, bufsize / 2);
                memcpy(stBufInfo.stFrameData.pVirAddr[2], pVData, bufsize / 2);
            } else {
                for (index = 0; index < stBufInfo.stFrameData.u16Height; index ++) {
                    memcpy((MI_U8 *)stBufInfo.stFrameData.pVirAddr[0] + index * stBufInfo.stFrameData.u32Stride[0],
                           (MI_U8 *)pYData + index * stBufInfo.stFrameData.u16Width,
                           stBufInfo.stFrameData.u16Width);
                }

                for (index = 0; index < stBufInfo.stFrameData.u16Height / 2; index ++) {
                    memcpy((MI_U8 *)stBufInfo.stFrameData.pVirAddr[1] + index * stBufInfo.stFrameData.u32Stride[1],
                           (MI_U8 *)pUData + index * stBufInfo.stFrameData.u16Width,
                           stBufInfo.stFrameData.u16Width);
                }

                for (index = 0; index < stBufInfo.stFrameData.u16Height / 2; index ++) {
                    memcpy((MI_U8 *)stBufInfo.stFrameData.pVirAddr[2] + index * stBufInfo.stFrameData.u32Stride[2],
                           (MI_U8 *)pVData + index * stBufInfo.stFrameData.u16Width,
                           stBufInfo.stFrameData.u16Width);
                }
            }
        } else {
            // Fill black screen
            if (stBufInfo.stFrameData.u32Stride[0] == stBufInfo.stFrameData.u16Width) {
                memset(stBufInfo.stFrameData.pVirAddr[0], 0x00, bufsize);
                memset(stBufInfo.stFrameData.pVirAddr[1], 0x80, bufsize / 2);
                memset(stBufInfo.stFrameData.pVirAddr[2], 0x80, bufsize / 2);
            } else {
                for (index = 0; index < stBufInfo.stFrameData.u16Height; index ++) {
                    memset((MI_U8 *)stBufInfo.stFrameData.pVirAddr[0] + index * stBufInfo.stFrameData.u32Stride[0],
                           0x00,
                           stBufInfo.stFrameData.u16Width);
                }

                for (index = 0; index < stBufInfo.stFrameData.u16Height / 2; index ++) {
                    memset((MI_U8 *)stBufInfo.stFrameData.pVirAddr[1] + index * stBufInfo.stFrameData.u32Stride[1],
                           0x80,
                           stBufInfo.stFrameData.u16Width);
                }

                for (index = 0; index < stBufInfo.stFrameData.u16Height / 2; index ++) {
                    memset((MI_U8 *)stBufInfo.stFrameData.pVirAddr[2] + index * stBufInfo.stFrameData.u32Stride[2],
                           0x80,
                           stBufInfo.stFrameData.u16Width);
                }
            }
        }

        MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, FALSE);
    }

    return 0;
}


void carimpl_set_colorkey(const char * dev, bool bEnable)
{
    MI_FB_ColorKey_t stColorKeyInfo;
    int fd = open(dev, O_RDWR);
    if (fd < 0) {
        printf("Error: failed to open FB\n");
        return;
    }

    if (ioctl(fd, FBIOGET_COLORKEY, &stColorKeyInfo) < 0) {
        printf("Error: failed to FBIOGET_COLORKEY\n");
        goto L_COLORKEY_EXIT;
    }

    stColorKeyInfo.bKeyEnable = bEnable;
    if (ioctl(fd, FBIOSET_COLORKEY, &stColorKeyInfo) < 0) {
        printf("Error: failed to FBIOSET_COLORKEY\n");
        goto L_COLORKEY_EXIT;
    }
L_COLORKEY_EXIT:
    close(fd);
}

void carimpl_set_alpha_blending(const char * dev, bool bEnable, MI_U8 u8GlobalAlpha)
{
    MI_FB_GlobalAlpha_t stAlphaInfo;
    int fd = open(dev, O_RDWR);
    if (fd < 0) {
        printf("Error: failed to open FB\n");
        return;
    }
    memset(&stAlphaInfo, 0, sizeof(MI_FB_GlobalAlpha_t));
    if (ioctl(fd, FBIOGET_GLOBAL_ALPHA, &stAlphaInfo) < 0) {
        printf("Error: failed to FBIOGET_GLOBAL_ALPHA\n");
        goto L_ALPHA_EXIT;
    }

    if (bEnable == false) {
        u8GlobalAlpha = 0xFF;
    }
#ifdef CHIP_IFORD
    stAlphaInfo.u8AlphaMode = E_MI_FB_ALPHA_CHANNEL;
#else
    stAlphaInfo.bAlphaEnable = true;
    stAlphaInfo.bAlphaChannel = false;
#endif
    stAlphaInfo.u8GlobalAlpha = u8GlobalAlpha; // constant alpha(0 ~ 0xFF)
    if (ioctl(fd, FBIOSET_GLOBAL_ALPHA, &stAlphaInfo) < 0) {
        printf("Error: failed to FBIOSET_GLOBAL_ALPHA\n");
        goto L_ALPHA_EXIT;
    }
L_ALPHA_EXIT:
    close(fd);
}

extern void StatusBar_Update(void);
void carimpl_show_wmsg(bool bEnable, WMSG_INFO WMSGInfo)
{
    Mutex::Autolock _l(sLock);
    if (bEnable && (WMSGInfo == WMSG_NONE))
        return;
    if (bEnable) {
        m_WMSGInfo = WMSGInfo;
        StatusBar_Update();
        if (EASYUICONTEXT->isStatusBarShow() == false) {
            EASYUICONTEXT->showStatusBar();
        }
    } else {
        m_WMSGInfo = WMSG_NONE;
        EASYUICONTEXT->hideStatusBar();
    }
}

WMSG_INFO carimpl_get_wmsginfo(void)
{
    return m_WMSGInfo;
}

void carimpl_Poweroff_handler(void)
{
    carimpl_show_wmsg(true, WMSG_SYSTEM_POWER_OFF);
    if (IMPL_REC_STATUS || IMPL_EMERG_REC_STATUS || IMPL_MD_REC_STATUS) {
        carimpl_VideoFunc_StartRecording(0);
    }
}

void carimpl_update_status(const char *status_name, const char *status, size_t size)
{
    int fd_wr = -1;
    fd_wr = open(status_name, O_RDWR | O_CREAT, 0666);
    if (fd_wr >= 0) {
        write(fd_wr, status, size);
        close(fd_wr);
    }
    else {
        printf("Error: failed to open %s\n", status_name);
    }
}

MI_S32 carimpl_process_msg(IPC_MSG_ID id, MI_S8 *param, MI_S32 paramLen)
{
    printf("[UI] msg [%d] name [%s]\n", id, EASYUICONTEXT->currentAppName());
    if (EASYUICONTEXT->currentAppName() == NULL)
        return 0;

    IPC_CarInfo_Read(&carimpl);

    switch (id) {
    case IPC_MSG_UI_SD_MOUNT:
        //carimpl_show_wmsg(true, WMSG_WAIT_INITIAL_DONE);
        if (strcmp(EASYUICONTEXT->currentAppName(), "VideoMenuActivity") == 0 &&
            (gu8LastUIMode == UI_BROWSER_MODE || gu8LastUIMode == UI_PLAYBACK_MODE)){
            Carimpl_DcfMount();
        }
        break;
    case IPC_MSG_UI_SD_PRE_MOUNT:
        carimpl_show_wmsg(true, WMSG_WAIT_INITIAL_DONE);
        break;
    case IPC_MSG_UI_SD_UNMOUNT:
        if (strcmp(EASYUICONTEXT->currentAppName(), "playerActivity") == 0) {
            gu8LastUIMode = UI_PLAYBACK_MODE;
            EASYUICONTEXT->goBack();
        }
        Carimpl_DcfUnmount();
        carimpl_show_wmsg(true, WMSG_PLUG_OUT_SD_CARD);
        break;
    case IPC_MSG_UI_SD_NEED_FORMAT:
        carimpl_show_wmsg(true, WMSG_FORMAT_SD_CARD);
        break;
    case IPC_MSG_UI_FORMAT_SD_OK:
        carimpl_show_wmsg(true, WMSG_FORMAT_SD_CARD_OK);
        break;
    case IPC_MSG_UI_FORMAT_SD_FAILED:
        carimpl_show_wmsg(true, WMSG_FORMAT_SD_CARD_FAIL);
        break;
    case IPC_MSG_UI_SPEECH_POWEROFF:
        carimpl_Poweroff_handler();
        break;
    case IPC_MSG_UI_SETTING_UPDATE:
        if(strcmp(EASYUICONTEXT->currentAppName(), "VideoMenuActivity") != 0){
            MenuSettingInit();//Maybe config bin has been modify by android APP.
        }
        break;
    case IPC_MSG_UI_SD_NOCARD:
        carimpl_show_wmsg(true, WMSG_NO_CARD);
        break;
    case IPC_MSG_UI_STOP:
        EASYUICONTEXT->reqExit();
        break;
    default:
        printf("%s: a msg from CardvImpl,unsupported!\n",__FUNCTION__);
        break;
    }

    return 0;
}

void carimpl_msg_handler(struct IPCMsgBuf *pMsgBuf)
{
    carimpl_process_msg(pMsgBuf->eIPCMsgId, (MI_S8 *)pMsgBuf->s8ParamBuf, pMsgBuf->u32ParamLen);
}
