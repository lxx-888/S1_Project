/*
 * speech.cpp - Sigmastar
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
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <errno.h>
#include "base_types.h"
#include "CSpotterSDKApi.h"
#include "CSpotterSDKApi_Const.h"

#include "mid_common.h"
#include "speech.h"
#include "linux_list.h"

#include "cmdqueue.h"
#include "module_common.h"
#include "module_menusetting.h"
#include "txz_engine.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

typedef HANDLE (*CSpotter_Handler_InitWithFiles)(char *lpchEngineLibFile, char *lpchCommandFile, char *lpchLicenseFile,
                                                 INT *lpnErr);
typedef INT (*CSpotter_Handler_Release)(HANDLE hCSpotter);
typedef INT (*CSpotter_Handler_Reset)(HANDLE hCSpotter);
typedef INT (*CSpotter_Handler_AddSample)(HANDLE hCSpotter, SHORT *lpsSample, INT nNumSamples);
typedef INT (*CSpotter_Handler_SaveCMSToFile)(HANDLE hCSpotter, char *lpchCMSFile);
typedef INT (*CSpotter_Handler_GetCommandNumber)(HANDLE hCSpotter);
typedef INT (*CSpotter_Handler_GetUTF8Result)(HANDLE hCSpotter, INT *lpnResultID, BYTE *lpbyUTF8Result);
typedef INT (*CSpotter_Handler_GetUTF8Command)(HANDLE hCSpotter, INT nID, BYTE *lpbyUTF8Buffer);
typedef INT (*CSpotter_Handler_SetAccelerationLevel)(HANDLE hCSpotter, INT nAccelerationLevel);

typedef cmd_int32_t (*pf_cmdGetName)(cmd_uint16_t index, const char **name);
typedef void (*pf_cmdCreate)(void **cmdInst);
typedef cmd_int32_t (*pf_cmdInit)(void *cmdInst);
typedef cmd_int32_t (*pf_cmdReset)(void *cmdInst);
typedef cmd_int32_t (*pf_cmdProcess)(void *cmdInst, const cmd_int16_t *audio, cmd_uint16_t samples,
                                     cmd_uint16_t *keyIndex, float *confidence);
typedef void (*pf_cmdDesrtoy)(void **cmdInst);
typedef cmd_int32_t (*pf_cmdGetConfig)(void *cmdInst, cmd_paramterId parmId, void *data);
typedef cmd_int32_t (*pf_cmdSetConfig)(void *cmdInst, cmd_paramterId parmId, const void *data);
typedef const char *(*pf_cmdGetKeywords)(void);
typedef const char *(*pf_cmdGetVersion)(void);
typedef cmd_int32_t (*pf_cmdCheckLicense)(void *channel, const char *token, const char *dir_txz_save,
                                          const char *dir_usb_push);
typedef int (*pf_cmdSetStorageFunc)(FuncLoad load_callback, FuncSave save_callback);
typedef int (*pf_cmdtxzUsbMountStatusFunc)(TXZ_USB_MOUNT_STATUS_FUNC func);
typedef int (*pf_cmdtxzStatusNotifyFunc)(TXZ_ACTIVATE_STATUS_NOTIFY_FUNC func);

typedef struct CSpotterLibInfo_s
{
    void *                                pCSpotterLibHandle;
    CSpotter_Handler_InitWithFiles        CSpotter_InitWithFiles;
    CSpotter_Handler_Release              CSpotter_Release;
    CSpotter_Handler_Reset                CSpotter_Reset;
    CSpotter_Handler_AddSample            CSpotter_AddSample;
    CSpotter_Handler_SaveCMSToFile        CSpotter_SaveCMSToFile;
    CSpotter_Handler_GetCommandNumber     CSpotter_GetCommandNumber;
    CSpotter_Handler_GetUTF8Result        CSpotter_GetUTF8Result;
    CSpotter_Handler_GetUTF8Command       CSpotter_GetUTF8Command;
    CSpotter_Handler_SetAccelerationLevel CSpotter_SetAccelerationLevel;
} CSpotterLibInfo_t;

typedef struct TongXingZheLibInfo_s
{
    void *                      pLibHandle;
    pf_cmdGetName               ptxzEngineGetName;
    pf_cmdCreate                ptxzEngineCreate;
    pf_cmdInit                  ptxzEngineCmdInit;
    pf_cmdReset                 ptxzEngineReset;
    pf_cmdProcess               ptxzEngineProcess;
    pf_cmdDesrtoy               ptxzEngineDesrtoy;
    pf_cmdGetConfig             pcmdGetConfig;
    pf_cmdSetConfig             ptxzEngineSetConfig;
    pf_cmdGetKeywords           pcmdGetKeywords;
    pf_cmdGetVersion            pcmdGetVersion;
    pf_cmdCheckLicense          ptxzEngineCheckLicense;
    pf_cmdSetStorageFunc        pSetStorageFunc;
    pf_cmdtxzUsbMountStatusFunc ptxzEnineSetUsbMountStatusFunc;
    pf_cmdtxzStatusNotifyFunc   ptxzEnineSetActivateStatusNotifyFunc;
} TongXingZheLibInfo_t;

typedef struct
{
    unsigned char *  pCmd;
    int              cmdLen;
    int              aliveTime; // millisecond
    struct timeval   timestamp;
    struct list_head stList;
} CarDV_Voice_Cmd_S;

typedef struct
{
    struct list_head stListHead;
    pthread_mutex_t  stMutex;
    HANDLE           hCSpotter;
    pthread_t        pt;
    MI_BOOL          bRunFlag;
    MI_U32           u32TotalFrames;
    MI_BOOL          bInit;
} CarDV_Voice_Mng_S;

typedef struct
{
    unsigned char *  pFrameData;
    int              frameLen;
    struct list_head stList;
} CarDV_Voice_Frame_S;

typedef struct
{
    struct list_head stListHead;
    pthread_mutex_t  stMutex;
} CarDV_Cmd_Mng_S;

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================
#if defined(SHOW_TO_UI_SEM)
MsgQueue g_toUI_msg_queue;
sem_t    g_toUI_sem;
#endif
#define SPEECH_LICENSE_DAT "/bootconfig/SpeechLicense.dat"
VoiceAnalyzeprocessCB  g_VoiceAnalyzeprocessCallback  = NULL;
ActivateStatusNotifyCB g_ActivateStatusNotifyCallback = NULL;

static CarDV_Voice_Mng_S g_stVoiceMng = {0};
static CarDV_Cmd_Mng_S   g_stCmdMng   = {0};

pthread_t g_txzCheckLicenseThread   = 0;
bool      CheckLicense              = 0;
MI_S8     g_SpeechLicenseToken[128] = "abea67cc1005f6695b67b171bfdf14e3eda9c838";

int32_t cmd_setpara();
//==============================================================================
//
//                              LOCAL FUNCTIONS
//
//==============================================================================

static void _InitFrameQueue(void)
{
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    // printf("%s++\n",__FUNCTION__);
    INIT_LIST_HEAD(&pstVoiceMng->stListHead);
    pthread_mutex_init(&pstVoiceMng->stMutex, NULL);
    pstVoiceMng->u32TotalFrames = 0;
    // printf("%s--\n",__FUNCTION__);
}

static void _InitCMDQueue(void)
{
    CarDV_Cmd_Mng_S *pstCmdMng = &g_stCmdMng;
    INIT_LIST_HEAD(&pstCmdMng->stListHead);
    pthread_mutex_init(&pstCmdMng->stMutex, NULL);
}

static CarDV_Voice_Frame_S *_PopFrameFromQueue(void)
{
    CarDV_Voice_Mng_S *  pstVoiceMng   = &g_stVoiceMng;
    CarDV_Voice_Frame_S *pstVoiceFrame = NULL;
    struct list_head *   pListPos      = NULL;

    pthread_mutex_lock(&pstVoiceMng->stMutex);
    if (list_empty(&pstVoiceMng->stListHead))
    {
        pthread_mutex_unlock(&pstVoiceMng->stMutex);
        return NULL;
    }

    pListPos = pstVoiceMng->stListHead.next;

    pstVoiceMng->stListHead.next = pListPos->next;
    pListPos->next->prev         = pListPos->prev;
    pthread_mutex_unlock(&pstVoiceMng->stMutex);

    pstVoiceFrame = list_entry(pListPos, CarDV_Voice_Frame_S, stList);
    return pstVoiceFrame;
}

static void _PutFrameToQueue(MI_AI_Data_t *pstAudioFrame)
{
    CarDV_Voice_Mng_S *  pstVoiceMng   = &g_stVoiceMng;
    struct list_head *   pListPos      = NULL;
    struct list_head *   pListPosN     = NULL;
    CarDV_Voice_Frame_S *pstVoiceFrame = NULL;
    int                  queueDepth    = 0;

    if (pstAudioFrame == NULL)
    {
        return;
    }

    // calc depth
    pthread_mutex_lock(&pstVoiceMng->stMutex);
    list_for_each_safe(pListPos, pListPosN, &pstVoiceMng->stListHead)
    {
        queueDepth++;
    }
    pthread_mutex_unlock(&pstVoiceMng->stMutex);

    // printf("%s %d, u32TotalFrames:%d, queueDepth:%d\n", __func__, __LINE__, pstVoiceMng->u32TotalFrames, queueDepth);

    // max depth check
    if (queueDepth >= MAX_FRAME_QUEUE_DEPTH)
    {
        CARDV_WARN("drop\n");
        // pop frame
        pstVoiceFrame = _PopFrameFromQueue();
        if (pstVoiceFrame != NULL)
        {
            if (pstVoiceFrame->pFrameData != NULL)
            {
                free(pstVoiceFrame->pFrameData);
                pstVoiceFrame->pFrameData = NULL;
            }

            free(pstVoiceFrame);
            pstVoiceFrame = NULL;
        }
    }

    pstVoiceFrame = (CarDV_Voice_Frame_S *)malloc(sizeof(CarDV_Voice_Frame_S));
    if (pstVoiceFrame == NULL)
    {
        CARDV_ERR("malloc error, not enough memory\n");
        goto END;
    }
    memset(pstVoiceFrame, 0, sizeof(CarDV_Voice_Frame_S));

    pstVoiceFrame->pFrameData = (unsigned char *)malloc(pstAudioFrame->u32Byte[0]);
    if (pstVoiceFrame->pFrameData == NULL)
    {
        CARDV_ERR("malloc error, not enough memory\n");
        goto END;
    }
    memset(pstVoiceFrame->pFrameData, 0, pstAudioFrame->u32Byte[0]);
    memcpy(pstVoiceFrame->pFrameData, pstAudioFrame->apvBuffer[0], pstAudioFrame->u32Byte[0]);
    pstVoiceFrame->frameLen = pstAudioFrame->u32Byte[0];

    // CARDV_DBG("pFrameData:%p, frameLen:%d\n", pstVoiceFrame->pFrameData, pstVoiceFrame->frameLen);

    pthread_mutex_lock(&pstVoiceMng->stMutex);
    list_add_tail(&pstVoiceFrame->stList, &pstVoiceMng->stListHead);
    pstVoiceMng->u32TotalFrames++;
    pthread_mutex_unlock(&pstVoiceMng->stMutex);
    /* coverity[leaked_storage] */
    return;
END:
    if (pstVoiceFrame->pFrameData != NULL)
    {
        free(pstVoiceFrame->pFrameData);
        pstVoiceFrame->pFrameData = NULL;
    }

    if (pstVoiceFrame)
    {
        free(pstVoiceFrame);
        pstVoiceFrame = NULL;
    }
}

#if 0
static void _ClearFrameQueue(void)
{
    CarDV_Voice_Mng_S *  pstVoiceMng   = &g_stVoiceMng;
    CarDV_Voice_Frame_S *pstVoiceFrame = NULL;
    struct list_head *   pHead         = &pstVoiceMng->stListHead;
    struct list_head *   pListPos      = NULL;
    struct list_head *   pListPosN     = NULL;

    pthread_mutex_lock(&pstVoiceMng->stMutex);
    list_for_each_safe(pListPos, pListPosN, pHead)
    {
        pstVoiceFrame = list_entry(pListPos, CarDV_Voice_Frame_S, stList);
        list_del(pListPos);

        if (pstVoiceFrame->pFrameData)
        {
            free(pstVoiceFrame->pFrameData);
        }

        free(pstVoiceFrame);
    }
    pthread_mutex_unlock(&pstVoiceMng->stMutex);
}
#endif

#if 0
static void _PutCMDToQueue(unsigned char *pCmd, int cmdLen)
{
    CarDV_Cmd_Mng_S *pstCmdMng = &g_stCmdMng;
    // struct list_head *pListPos = NULL;
    // struct list_head *pListPosN = NULL;
    CarDV_Voice_Cmd_S *pstVoiceCmd = NULL;

    if (pCmd == NULL || cmdLen <= 0)
    {
        return;
    }

    pstVoiceCmd = (CarDV_Voice_Cmd_S *)malloc(sizeof(CarDV_Voice_Cmd_S));
    if (pstVoiceCmd == NULL)
    {
        CARDV_ERR("malloc error, not enough memory\n");
        goto END;
    }
    memset(pstVoiceCmd, 0, sizeof(CarDV_Voice_Cmd_S));

    pstVoiceCmd->pCmd = (unsigned char *)malloc(cmdLen + 1);
    if (pstVoiceCmd->pCmd == NULL)
    {
        CARDV_ERR("malloc error, not enough memory\n");
        goto END;
    }

    printf("%s %d, %p, %p\n", __func__, __LINE__, pstVoiceCmd, pstVoiceCmd->pCmd);
    memset(pstVoiceCmd->pCmd, 0, cmdLen + 1);
    memcpy(pstVoiceCmd->pCmd, pCmd, cmdLen);
    pstVoiceCmd->cmdLen    = cmdLen + 1;
    pstVoiceCmd->aliveTime = 4000;
    gettimeofday(&pstVoiceCmd->timestamp, NULL);

    pthread_mutex_lock(&pstCmdMng->stMutex);
    list_add_tail(&pstVoiceCmd->stList, &pstCmdMng->stListHead);
    pthread_mutex_unlock(&pstCmdMng->stMutex);

    return;
END:
    if (pstVoiceCmd->pCmd != NULL)
    {
        free(pstVoiceCmd->pCmd);
        pstVoiceCmd->pCmd = NULL;
    }

    if (pstVoiceCmd)
    {
        free(pstVoiceCmd);
        pstVoiceCmd = NULL;
    }
}
#endif

static void _ClearCMDQueue(void)
{
    CarDV_Cmd_Mng_S *  pstCmdMng   = &g_stCmdMng;
    CarDV_Voice_Cmd_S *pstVoiceCmd = NULL;

    // CarDV_Voice_Frame_S *pstVoiceFrame = NULL;
    struct list_head *pHead     = &pstCmdMng->stListHead;
    struct list_head *pListPos  = NULL;
    struct list_head *pListPosN = NULL;

    pthread_mutex_lock(&pstCmdMng->stMutex);
    list_for_each_safe(pListPos, pListPosN, pHead)
    {
        pstVoiceCmd = list_entry(pListPos, CarDV_Voice_Cmd_S, stList);
        list_del(pListPos);

        if (pstVoiceCmd->pCmd)
        {
            free(pstVoiceCmd->pCmd);
        }

        free(pstVoiceCmd);
    }
    pthread_mutex_unlock(&pstCmdMng->stMutex);
}

//==============================================================================
//
//                              EXPORT FUNCTIONS
//
//==============================================================================
#if (1) //(CARDV_SPEECH_ENABLE)
#define NN                128
#define TAIL              1024
#define NUM_SMPL          1024
#define MACRO2STR1(macro) #macro
#define MACRO2STR(macro)  MACRO2STR1(macro)
static TongXingZheLibInfo_t SpeechLibInf;

#define GET_LIB_FUNC(name, pname)                                              \
    {                                                                          \
        char  str[]          = MACRO2STR(name);                                \
        char *tmp            = str;                                            \
        SpeechLibInf.p##name = (pname)dlsym(SpeechLibInf.pLibHandle, tmp);     \
        if (NULL == SpeechLibInf.p##name)                                      \
        {                                                                      \
            CARDV_ERR(" %s: dlsym %s failed, %s\n", __func__, tmp, dlerror()); \
            return -1;                                                         \
        }                                                                      \
    }

void Speech_VoiceAnalyzeprocessCallback(VoiceAnalyzeprocessCB pFunc)
{
    g_VoiceAnalyzeprocessCallback = pFunc;
}
void Speech_txzActivateStatusNotifyCallback(ActivateStatusNotifyCB pFunc)
{
    g_ActivateStatusNotifyCallback = pFunc;
}
static int SpeechLiceseDataLoad(void *pData, cmd_int32_t len)
{
    int  fd          = 0;
    char outBuf[512] = {
        0,
    };
    fd = open(SPEECH_LICENSE_DAT, O_RDONLY);
    if (fd >= 0)
    {
        read(fd, outBuf, sizeof(outBuf));
        close(fd);
    }

    return 0;
}
static int SpeechLiceseDataSave(void *pDta, cmd_int32_t len)
{
    int fd = 0;
    fd     = open(SPEECH_LICENSE_DAT, O_RDWR | O_CREAT, 0666);
    if (fd >= 0)
    {
        write(fd, pDta, len);
        close(fd);
    }
    return 0;
}

static int SpeechMMCMountStatus()
{
    int  fd;
    char buffer[8] = {
        0,
    };
    fd = open("/tmp/mmc_status", O_RDONLY);
    if (fd >= 0)
    {
        if (read(fd, buffer, sizeof(buffer)) == -1)
        {
            close(fd);
            return 0;
        }
        close(fd);
        if (buffer[0] == '1')
            return 0;
    }
    return 1;
}

static void *Speech_CheckLicense_Task(void *argv)
{
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    void *             cmdInst;
    cmd_int32_t        ret = 0;

    SpeechLibInf.ptxzEnineSetUsbMountStatusFunc(SpeechMMCMountStatus);
    if (g_ActivateStatusNotifyCallback)
        SpeechLibInf.ptxzEnineSetActivateStatusNotifyFunc(g_ActivateStatusNotifyCallback);
    SpeechLibInf.pSetStorageFunc(SpeechLiceseDataLoad, SpeechLiceseDataSave);
    if (0
        == SpeechLibInf.ptxzEngineCheckLicense((void *)"haizhen", (char *)g_SpeechLicenseToken, "/mnt/mmc/",
                                               "/mnt/mmc/"))
    {
        printf("Speech CheckLicense OK! \n");

        SpeechLibInf.ptxzEngineCreate(&cmdInst);
        if (NULL == cmdInst)
        {
            printf(" cmdcreate err \n");
            goto ret;
        }
        if (CMD_CODE_NORMAL != SpeechLibInf.ptxzEngineCmdInit(cmdInst))
        {
            printf("cmdinit err \n");
            goto ret;
        }

        pstVoiceMng->hCSpotter = cmdInst;
        cmd_setpara();
        printf("cmdinit ok \n");

        ret = SpeechLibInf.ptxzEngineReset(pstVoiceMng->hCSpotter);
        if (ret != CMD_CODE_NORMAL)
        {
            printf("cmdReset err \n ");
            goto ret;
        }
        CheckLicense = 1;
    }
    else
        printf("Speech CheckLicense fail! \n");

ret:
    pthread_exit(NULL);
}

// short audio_buf[160];//max 320 points
// void* cmdInst;
// int endflag=0;
// long cmd_time = 0;
// long cmd_start = 0;

int32_t cmd_setpara()
{
    /* CMD configure */
    cmd_uint16_t       cmd_shift_frames    = 1;    // 1 (0~10)
    cmd_uint16_t       cmd_smooth_frames   = 1;    // 1 (0~30)
    cmd_uint16_t       cmd_lock_frames     = 1;    // 1 (0~100)
    cmd_uint16_t       cmd_post_max_frames = 35;   // 35 (0~100)
    cmd_float32_t      cmd_threshold       = 0.50; // 0.70f (0.0f~1.0f)
    CarDV_Voice_Mng_S *pstVoiceMng         = &g_stVoiceMng;

    printf("cmd threshod = %f\n", cmd_threshold);

    if (CMD_CODE_NORMAL != SpeechLibInf.ptxzEngineSetConfig(pstVoiceMng->hCSpotter, cmd_shiftFrames, &cmd_shift_frames))
    {
        printf("ptxzEngineSetConfig cmd_shiftFrames error.\n");
        return -1;
    }
    if (CMD_CODE_NORMAL
        != SpeechLibInf.ptxzEngineSetConfig(pstVoiceMng->hCSpotter, cmd_smoothFrames, &cmd_smooth_frames))
    {
        printf("ptxzEngineSetConfig cmd_smoothFrames error.\n");
        return -1;
    }
    if (CMD_CODE_NORMAL != SpeechLibInf.ptxzEngineSetConfig(pstVoiceMng->hCSpotter, cmd_lockFrames, &cmd_lock_frames))
    {
        printf("ptxzEngineSetConfig cmd_lockFrames error.\n");
        return -1;
    }
    if (CMD_CODE_NORMAL
        != SpeechLibInf.ptxzEngineSetConfig(pstVoiceMng->hCSpotter, cmd_postMaxFrames, &cmd_post_max_frames))
    {
        printf("ptxzEngineSetConfig cmd_postMaxFrames error.\n");
        return -1;
    }
    if (CMD_CODE_NORMAL != SpeechLibInf.ptxzEngineSetConfig(pstVoiceMng->hCSpotter, cmd_thresHold, &cmd_threshold))
    {
        printf("ptxzEngineSetConfig cmd_threshold error.\n");
        return -1;
    }

    printf("cmdSetConfig ok\n");
    return 0;
}

int uv_cmd_init()
{
    // CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    // void* cmdInst;
    int ret = 0;

    memset(&SpeechLibInf, 0x00, sizeof(TongXingZheLibInfo_t));
    SpeechLibInf.pLibHandle = dlopen("/lib/libtxz_engine.so", RTLD_NOW);
    if (NULL == SpeechLibInf.pLibHandle)
    {
        CARDV_ERR("load libtxz_engine.so error(%d):%s\n", errno, /*strerror(errno)*/ dlerror());
        return -1;
    }

    GET_LIB_FUNC(txzEngineGetName, pf_cmdGetName);
    GET_LIB_FUNC(txzEngineCreate, pf_cmdCreate);
    GET_LIB_FUNC(txzEngineCmdInit, pf_cmdInit);
    GET_LIB_FUNC(txzEngineReset, pf_cmdReset);
    GET_LIB_FUNC(txzEngineProcess, pf_cmdProcess);
    GET_LIB_FUNC(txzEngineDesrtoy, pf_cmdDesrtoy);
    // GET_LIB_FUNC(cmdGetConfig,pf_cmdGetConfig);
    // GET_LIB_FUNC(cmdSetConfig,pf_cmdSetConfig);
    GET_LIB_FUNC(txzEngineSetConfig, pf_cmdSetConfig);
    // GET_LIB_FUNC(cmdGetKeywords,pf_cmdGetKeywords);
    // GET_LIB_FUNC(cmdGetVersion,pf_cmdGetVersion);
    GET_LIB_FUNC(txzEngineCheckLicense, pf_cmdCheckLicense);
    GET_LIB_FUNC(SetStorageFunc, pf_cmdSetStorageFunc);
    GET_LIB_FUNC(txzEnineSetUsbMountStatusFunc, pf_cmdtxzUsbMountStatusFunc);
    GET_LIB_FUNC(txzEnineSetActivateStatusNotifyFunc, pf_cmdtxzStatusNotifyFunc);

    ret = pthread_create(&g_txzCheckLicenseThread, NULL, Speech_CheckLicense_Task, NULL);
    if (0 == ret)
        pthread_setname_np(g_txzCheckLicenseThread, "Speech_CheckLicense_Task");
    else
        printf("%s create Speech_CheckLicense_Task failed\n", __func__);

    /*SpeechLibInf.ptxzEngineCreate(&cmdInst);
    if (NULL == cmdInst) {
        printf(" cmdcreate err \n");
        return -1;
    }
    if (CMD_CODE_NORMAL != SpeechLibInf.ptxzEngineCmdInit(cmdInst)) {
        printf("cmdinit err \n");
        return -1;
    }

    pstVoiceMng->hCSpotter = cmdInst;
    cmd_setpara();
    printf("cmdinit ok \n");
    */
    return 0;
}

int _Voice_parse_cmd(cmd_uint16_t cmdIndex)
{
    if (g_VoiceAnalyzeprocessCallback)
        g_VoiceAnalyzeprocessCallback(cmdIndex);
    return 0;
}

static void *_VoiceAnalyzeProc_(void *pdata)
{
    CarDV_Voice_Mng_S *  pstVoiceMng    = &g_stVoiceMng;
    CarDV_Voice_Frame_S *pstVoiceFrame  = NULL;
    MI_S32               s32Ret         = 0;
    cmd_uint16_t         cmdIndex       = 0;
    float                cmd_confidence = 0;
    const char *         commands       = NULL;
    unsigned short *     pu16Pcm        = NULL;

    CARDV_THREAD();

    // wait for txz check license
    while (!CheckLicense)
    {
        sleep(1);
    }

    pstVoiceMng->bRunFlag = TRUE;

#if (CSPOTTER_DATA_FROM_FILE == 1)
    char szPcmFile[64] = {
        0,
    };
    int           fdIn = -1;
    unsigned char pcmBuf[AUDIO_PT_NUMBER_FRAME * 2];
    int           readLen = 0;

    snprintf(szPcmFile, sizeof(szPcmFile) - 1, "audio_in.pcm");
    fdIn = open(szPcmFile, O_RDONLY);
    if (fdIn <= 0)
    {
        CARDV_WARN("create file %s error\n", szPcmFile);
    }
    else
    {
        CARDV_DBG("open %s success, fd:%d\n", szPcmFile, fdIn);
    }
#endif

#if CSPOTTER_DATA_TO_FILE
    char szFileName[64] = {
        0,
    };
    int fd = -1;
    snprintf(szFileName, sizeof(szFileName) - 1, "/mnt/nfs/voice.pcm");

    fd = open(szFileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd <= 0)
    {
        CARDV_WARN("create file %s error\n", szFileName);
    }
    else
    {
        CARDV_DBG("open %s success, fd:%d\n", szFileName, fd);
    }
#endif

    if (SpeechLibInf.ptxzEngineReset(pstVoiceMng->hCSpotter) != CMD_CODE_NORMAL)
    {
        CARDV_ERR("%s Fail to reset recognition )!\n", __func__);
        return NULL;
    }

    CARDV_DBG("pid=%ld\n", syscall(SYS_gettid));

    while (pstVoiceMng->bRunFlag)
    {
#if (CSPOTTER_DATA_FROM_FILE == 1)
        memset(pcmBuf, 0, NUM_SMPL * 2);
        readLen = read(fdIn, pcmBuf, NUM_SMPL * 2);

        printf("::%s %d, readLen:%d\n", __func__, __LINE__, readLen);

        if (readLen < NUM_SMPL * 2)
        {
            lseek(fdIn, 0, SEEK_SET);
            continue;
        }

        usleep(1000 * 10);
        s32Ret =
            SpeechLibInf.ptxzEngineProcess(pstVoiceMng->hCSpotter, pcmBuf, (readLen / 2), &cmdIndex, &cmd_confidence);

#else // CSPOTTER_DATA_FROM_FILE
        pstVoiceFrame = _PopFrameFromQueue();

        if (pstVoiceFrame == NULL)
        {
            usleep(1000 * 10);
            continue;
        }

#if CSPOTTER_DATA_TO_FILE
        s32Ret = write(fd, pstVoiceFrame->pFrameData, pstVoiceFrame->frameLen);
        if (s32Ret <= 0)
        {
            CARDV_ERR("fwrite failed, u32Byte = %d, s32Ret = %d\n", pstVoiceFrame->frameLen, s32Ret);
        }
#endif

        // samples: Number of samples in audio buffer, must be 160 now.
        if (0 != (pstVoiceFrame->frameLen % NUM_SMPL))
        {
            goto END_PROC;
        }
        pu16Pcm = (unsigned short *)pstVoiceFrame->pFrameData;

        do
        {
            // printf("pFrameData:%p, frameLen:%d\n", pu16Pcm, pstVoiceFrame->frameLen);
            s32Ret = SpeechLibInf.ptxzEngineProcess(pstVoiceMng->hCSpotter, (const cmd_int16_t *)pu16Pcm, NUM_SMPL,
                                                    &cmdIndex, &cmd_confidence);
            pstVoiceFrame->frameLen -= 2 * NUM_SMPL;
            pu16Pcm += NUM_SMPL;

            if (s32Ret == CMD_CODE_NORMAL || s32Ret == CMD_CODE_PROCESSEND)
            {
                if (cmdIndex > 0)
                {
                    SpeechLibInf.ptxzEngineGetName(cmdIndex, &commands);
                    printf(" cmdIndex=%d , command:<%s> ,confidence=%f\n", cmdIndex, commands, cmd_confidence);
                    SpeechLibInf.ptxzEngineReset(pstVoiceMng->hCSpotter);
                    // memcpy(pbyResult,commands,strlen(commands));
                    // CARDV_DBG("Result:%s, len:%d\n", pbyResult, strlen((const char*)pbyResult));
                    _Voice_parse_cmd(cmdIndex);
                }
            }
            else
            {
                printf("cmdProcess error, ret=%d:%s\n", s32Ret, strerror(errno));
                SpeechLibInf.ptxzEngineReset(pstVoiceMng->hCSpotter);
            }

        } while (pstVoiceFrame->frameLen > 0);

    END_PROC:
        if (pstVoiceFrame != NULL)
        {
            if (pstVoiceFrame->pFrameData != NULL)
            {
                free(pstVoiceFrame->pFrameData);
                pstVoiceFrame->pFrameData = NULL;
            }

            free(pstVoiceFrame);
            pstVoiceFrame = NULL;
        }

        // reduce CSpotter_AddSample core dump
        usleep(1000 * 10);

#endif // CSPOTTER_DATA_FROM_FILE
    }

#if CSPOTTER_DATA_TO_FILE
    if (fd > 0)
    {
        close(fd);
        fd = -1;
    }
#endif

#if (CSPOTTER_DATA_FROM_FILE == 1)
    if (fdIn > 0)
    {
        close(fdIn);
        fdIn = -1;
    }
#endif

    _ClearCMDQueue();

    return NULL;
}

MI_S32 Speech_VoiceAnalyzeInit()
{
    // cmd_int32_t ret = 0;
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    int                cmdret;

    cmdret = uv_cmd_init();
    if (cmdret != 0)
    {
        printf("uv_cmd_init err \n ");
        return -1;
    }

    // ret = SpeechLibInf.ptxzEngineReset(pstVoiceMng->hCSpotter);
    // if (ret !=  CMD_CODE_NORMAL){
    //    printf("cmdReset err \n ");
    //    return -1;
    //}

#if defined(SHOW_TO_UI_SEM)
    sem_init(&g_toUI_sem, 0, 0);
#endif
    _InitCMDQueue();
    _InitFrameQueue();

    pstVoiceMng->bInit = TRUE;
    return MI_SUCCESS;
}

MI_S32 Speech_VoiceAnalyzeDeInit()
{
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    if (TRUE == pstVoiceMng->bInit)
    {
        SpeechLibInf.ptxzEngineDesrtoy(&pstVoiceMng->hCSpotter);
    }
    return MI_SUCCESS;
}

MI_S32 Speech_VoiceAnalyzeStart()
{
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    int                ret         = 0;

    // pstVoiceMng->bRunFlag = TRUE;
    pthread_create(&pstVoiceMng->pt, NULL, _VoiceAnalyzeProc_, NULL);
    if (0 == ret)
    {
        pthread_setname_np(pstVoiceMng->pt, "speech");
    }
    else
    {
        CARDV_ERR("%s pthread_create failed\n", __func__);
    }

    return MI_SUCCESS;
}

MI_S32 Speech_VoiceAnalyzeStop()
{
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    if (TRUE == pstVoiceMng->bRunFlag)
    {
        pstVoiceMng->bRunFlag = FALSE;
        pthread_join(pstVoiceMng->pt, NULL);
    }

    return MI_SUCCESS;
}

MI_BOOL Speech_VoiceIsAnalyzeStart()
{
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    return pstVoiceMng->bRunFlag;
}

MI_S32 Speech_VoiceLearnStart()
{
    return 0;
}

MI_S32 Speech_VoiceLearnStop()
{
    return 0;
}

void Speech_VoiceSendAudioFrame(MI_AI_Data_t *pstAudioFrame)
{
    _PutFrameToQueue(pstAudioFrame);
}

#else
static CSpotterLibInfo_t stCSpotterLibInfo;
static unsigned char     pbyResult[256];

int _Voice_parse_msg(void)
{
    char *pResult = (char *)pbyResult;
    // char *str = strtok(pbyResult, ", ");
    // printf("Speech Result:%s, %d\n", pResult, strlen((const char*)pResult));

    if (strcmp(pResult, "Take a picture") == 0)
        cardv_send_to_fifo(CARDV_CMD_CAPTURE, sizeof(CARDV_CMD_CAPTURE));
    else
    {
        unsigned int i = 0, j = strlen(pResult);
        printf("TODO:Speech Result:%s, %d\n", pResult, strlen(pResult));

        for (i = 0; i < j; i++)
            printf("%02X ", *pResult++);
        printf("\n");
    }

    return 0;
}

static void *_VoiceAnalyzeProc_(void *pdata)
{
    CarDV_Voice_Mng_S *  pstVoiceMng   = &g_stVoiceMng;
    CarDV_Voice_Frame_S *pstVoiceFrame = NULL;
    MI_S32               s32Ret        = 0;
// unsigned long msg[4];
#if (CSPOTTER_DATA_FROM_FILE == 1)
    char szPcmFile[64] = {
        0,
    };
    int           fdIn = -1;
    unsigned char pcmBuf[AUDIO_PT_NUMBER_FRAME * 2];
    int           readLen = 0;

    snprintf(szPcmFile, sizeof(szPcmFile) - 1, "audio_in.pcm");
    fdIn = open(szPcmFile, O_RDONLY);
    if (fdIn <= 0)
    {
        CARDV_WARN("create file %s error\n", szPcmFile);
    }
    else
    {
        CARDV_DBG("open %s success, fd:%d\n", szPcmFile, fdIn);
    }
#endif

#if CSPOTTER_DATA_TO_FILE
    char szFileName[64] = {
        0,
    };
    int fd = -1;
    snprintf(szFileName, sizeof(szFileName) - 1, "/mnt/nfs/voice.pcm");

    fd = open(szFileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd <= 0)
    {
        CARDV_WARN("create file %s error\n", szFileName);
    }
    else
    {
        CARDV_DBG("open %s success, fd:%d\n", szFileName, fd);
    }
#endif

    if ((s32Ret = stCSpotterLibInfo.CSpotter_Reset(pstVoiceMng->hCSpotter)) != CSPOTTER_SUCCESS)
    {
        CARDV_ERR("CSpotter_Reset:: Fail to start recognition ( %d )!\n", s32Ret);
        return NULL;
    }

    CARDV_DBG("pid=%ld\n", syscall(SYS_gettid));

    while (pstVoiceMng->bRunFlag)
    {
#if (CSPOTTER_DATA_FROM_FILE == 1)
        memset(pcmBuf, 0, AUDIO_PT_NUMBER_FRAME * 2);
        readLen = read(fdIn, pcmBuf, AUDIO_PT_NUMBER_FRAME * 2);

        printf("%s %d, readLen:%d\n", __func__, __LINE__, readLen);

        if (readLen < AUDIO_PT_NUMBER_FRAME * 2)
        {
            lseek(fdIn, 0, SEEK_SET);
            continue;
        }

        usleep(1000 * 10);

        s32Ret = stCSpotterLibInfo.CSpotter_AddSample(pstVoiceMng->hCSpotter, (short *)pcmBuf, readLen / 2);

#if CSPOTTER_DATA_TO_FILE
        s32Ret = write(fd, pstVoiceFrame->pFrameData, pstVoiceFrame->frameLen);
        if (s32Ret <= 0)
        {
            CARDV_ERR("fwrite failed, u32Byte = %d, s32Ret = %d\n", pstVoiceFrame->frameLen, s32Ret);
        }
#endif

#else // CSPOTTER_DATA_FROM_FILE
        pstVoiceFrame = _PopFrameFromQueue();

        if (pstVoiceFrame == NULL)
        {
            usleep(1000 * 10);
            continue;
        }

#if CSPOTTER_DATA_TO_FILE
        s32Ret = write(fd, pstVoiceFrame->pFrameData, pstVoiceFrame->frameLen);
        if (s32Ret <= 0)
        {
            CARDV_ERR("fwrite failed, u32Byte = %d, s32Ret = %d\n", pstVoiceFrame->frameLen, s32Ret);
        }
#endif

        // reduce CSpotter_AddSample core dump
        usleep(1000 * 10);

        // CARDV_DBG("pFrameData:%p, frameLen:%d\n", pstVoiceFrame->pFrameData, pstVoiceFrame->frameLen);
        s32Ret = stCSpotterLibInfo.CSpotter_AddSample(pstVoiceMng->hCSpotter, (short *)pstVoiceFrame->pFrameData,
                                                      pstVoiceFrame->frameLen / 2);
        if (s32Ret == CSPOTTER_SUCCESS)
        {
            // printf("Get result!\n");
            s32Ret = stCSpotterLibInfo.CSpotter_GetUTF8Result(pstVoiceMng->hCSpotter, NULL, pbyResult);
            if (s32Ret >= 0)
            {
                CARDV_DBG("Result:%s, s32Ret:%d, %d\n", pbyResult, s32Ret, strlen((const char *)pbyResult));
                _Voice_parse_msg();
#if defined(SHOW_TO_UI_SEM)
                unsigned long msg[4];
                memset(msg, 0, 16);
                msg[0] = MSG_TYPE_SMARTMIC_MACTCH;
                msg[1] = 0;
                msg[2] = (unsigned long)pbyResult;
                msg[3] = strlen((const char *)pbyResult);
                g_toUI_msg_queue.send_message(MODULE_MSG, (void *)msg, sizeof(msg), &g_toUI_sem);
//_PutCMDToQueue(pbyResult, s32Ret);
#endif
#if 0
                char cmdGb2312[100] = "";
                utf8ToGb2312(cmdGb2312, 100, (char *)pbyResult, strlen((char *)pbyResult));
                printf("Result: %s \n", cmdGb2312);
                static int key = 0;
                if (key == 0)
                {
                    key = 1;
                    madp_osd_set_text(0, cmdGb2312);
                }
                else
                {
                    osd_text_update(0, cmdGb2312);
                }
#endif
            }

#if 0
            // Save CMS
            nRes = stCSpotterLibInfo.CSpotter_SaveCMSToFile(hCSpotter, p_Param->m_pchCMSFile);
            if (nRes != CSPOTTER_SUCCESS)
                printf("ThreadRecog():: Fail to save CMS to file!\n");
#endif
        }

        if (pstVoiceFrame != NULL)
        {
            if (pstVoiceFrame->pFrameData != NULL)
            {
                free(pstVoiceFrame->pFrameData);
                pstVoiceFrame->pFrameData = NULL;
            }

            free(pstVoiceFrame);
            pstVoiceFrame = NULL;
        }
#endif // CSPOTTER_DATA_FROM_FILE
    }

#if CSPOTTER_DATA_TO_FILE
    if (fd > 0)
    {
        close(fd);
        fd = -1;
    }
#endif

#if (CSPOTTER_DATA_FROM_FILE == 1)
    if (fdIn > 0)
    {
        close(fdIn);
        fdIn = -1;
    }
#endif

    _ClearCMDQueue();

    return NULL;
}

//==============================================================================
//
//                              EXPORT FUNCTIONS
//
//==============================================================================

MI_S32 Speech_VoiceAnalyzeInit()
{
    // unsigned long msg[4];
    HANDLE hCSpotter            = NULL;
    int    nErr                 = 0;
    int    nCmdNum              = 0;
    int    i                    = 0;
    int    ret                  = 0;
    char   szEngineLibFile[128] = {
        0,
    };
    char szCommandBinFile[128] = {
        0,
    };
    char szLicenseBinFile[128] = {
        0,
    };
    char szCmdBuf[128] = {
        0,
    };
    char szCmdBufTemp[128] = {
        0,
    };
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;

    memset(&stCSpotterLibInfo, 0x00, sizeof(CSpotterLibInfo_t));
    stCSpotterLibInfo.pCSpotterLibHandle = dlopen("libCSpotter.so", RTLD_NOW);
    if (NULL == stCSpotterLibInfo.pCSpotterLibHandle)
    {
        CARDV_ERR(" %s: load libCSpotter.so error \n", __func__);
        return -1;
    }
    stCSpotterLibInfo.CSpotter_InitWithFiles =
        (CSpotter_Handler_InitWithFiles)dlsym(stCSpotterLibInfo.pCSpotterLibHandle, "CSpotter_InitWithFiles");
    if (NULL == stCSpotterLibInfo.CSpotter_InitWithFiles)
    {
        CARDV_ERR(" %s: dlsym CSpotter_InitWithFiles failed, %s\n", __func__, dlerror());
        return -1;
    }
    stCSpotterLibInfo.CSpotter_Release =
        (CSpotter_Handler_Release)dlsym(stCSpotterLibInfo.pCSpotterLibHandle, "CSpotter_Release");
    if (NULL == stCSpotterLibInfo.CSpotter_Release)
    {
        CARDV_ERR(" %s: dlsym CSpotter_Release failed, %s\n", __func__, dlerror());
        return -1;
    }
    stCSpotterLibInfo.CSpotter_Reset =
        (CSpotter_Handler_Reset)dlsym(stCSpotterLibInfo.pCSpotterLibHandle, "CSpotter_Reset");
    if (NULL == stCSpotterLibInfo.CSpotter_Reset)
    {
        CARDV_ERR(" %s: dlsym CSpotter_Reset failed, %s\n", __func__, dlerror());
        return -1;
    }
    stCSpotterLibInfo.CSpotter_AddSample =
        (CSpotter_Handler_AddSample)dlsym(stCSpotterLibInfo.pCSpotterLibHandle, "CSpotter_AddSample");
    if (NULL == stCSpotterLibInfo.CSpotter_AddSample)
    {
        CARDV_ERR(" %s: dlsym CSpotter_AddSample failed, %s\n", __func__, dlerror());
        return -1;
    }
    stCSpotterLibInfo.CSpotter_SaveCMSToFile =
        (CSpotter_Handler_SaveCMSToFile)dlsym(stCSpotterLibInfo.pCSpotterLibHandle, "CSpotter_SaveCMSToFile");
    if (NULL == stCSpotterLibInfo.CSpotter_SaveCMSToFile)
    {
        CARDV_ERR(" %s: dlsym CSpotter_SaveCMSToFile failed, %s\n", __func__, dlerror());
        return -1;
    }
    stCSpotterLibInfo.CSpotter_GetCommandNumber =
        (CSpotter_Handler_GetCommandNumber)dlsym(stCSpotterLibInfo.pCSpotterLibHandle, "CSpotter_GetCommandNumber");
    if (NULL == stCSpotterLibInfo.CSpotter_GetCommandNumber)
    {
        CARDV_ERR(" %s: dlsym CSpotter_GetCommandNumber failed, %s\n", __func__, dlerror());
        return -1;
    }
    stCSpotterLibInfo.CSpotter_GetUTF8Result =
        (CSpotter_Handler_GetUTF8Result)dlsym(stCSpotterLibInfo.pCSpotterLibHandle, "CSpotter_GetUTF8Result");
    if (NULL == stCSpotterLibInfo.CSpotter_GetUTF8Result)
    {
        CARDV_ERR(" %s: dlsym CSpotter_GetUTF8Result failed, %s\n", __func__, dlerror());
        return -1;
    }
    stCSpotterLibInfo.CSpotter_GetUTF8Command =
        (CSpotter_Handler_GetUTF8Command)dlsym(stCSpotterLibInfo.pCSpotterLibHandle, "CSpotter_GetUTF8Command");
    if (NULL == stCSpotterLibInfo.CSpotter_GetUTF8Command)
    {
        CARDV_ERR(" %s: dlsym CSpotter_GetUTF8Command failed, %s\n", __func__, dlerror());
        return -1;
    }
    stCSpotterLibInfo.CSpotter_SetAccelerationLevel = (CSpotter_Handler_SetAccelerationLevel)dlsym(
        stCSpotterLibInfo.pCSpotterLibHandle, "CSpotter_SetAccelerationLevel");
    if (NULL == stCSpotterLibInfo.CSpotter_SetAccelerationLevel)
    {
        CARDV_ERR(" %s: dlsym CSpotter_SetAccelerationLevel failed, %s\n", __func__, dlerror());
        return -1;
    }

#if defined(SHOW_TO_UI_SEM)
    sem_init(&g_toUI_sem, 0, 0);
#endif
    _InitCMDQueue();
    _InitFrameQueue();

    snprintf(szEngineLibFile, sizeof(szEngineLibFile) - 1, "%s/libNINJA.so", CSPOTTER_LIB_PATH);
    snprintf(szCommandBinFile, sizeof(szCommandBinFile) - 1, "%s/RecogCmd.bin", CSPOTTER_DATA_PATH);
    snprintf(szLicenseBinFile, sizeof(szLicenseBinFile) - 1, "%s/CybLicense.bin", CSPOTTER_DATA_PATH);
    system("date -s \"2019-06-06 20:49\""); // system("date -s \"2017-12-06 20:49\"");
    printf("%s\n", szEngineLibFile);
    printf("%s\n", szCommandBinFile);
    printf("%s\n", szLicenseBinFile);
    hCSpotter = stCSpotterLibInfo.CSpotter_InitWithFiles(szEngineLibFile, szCommandBinFile, szLicenseBinFile, &nErr);
    if (hCSpotter == NULL)
    {
        CARDV_ERR("CSpotter_InitWithFiles fail, err:%d\n", nErr);
        return -1;
    }

    nCmdNum = stCSpotterLibInfo.CSpotter_GetCommandNumber(hCSpotter);
    for (i = 0; i < nCmdNum; i++)
    {
        memset(szCmdBuf, 0, sizeof(szCmdBuf));
        ret = stCSpotterLibInfo.CSpotter_GetUTF8Command(hCSpotter, i, (BYTE *)szCmdBuf);
        if (ret > 0)
        {
            // skip the same cmd
            if (strcmp(szCmdBuf, szCmdBufTemp) != 0)
            {
                printf("%s %d\n", szCmdBuf, nCmdNum);
                strcpy(szCmdBufTemp, szCmdBuf);
#if defined(SHOW_TO_UI_SEM)
                unsigned long msg[4];
                memset(msg, 0, 16);
                msg[0] = MSG_TYPE_SMARTMIC_START;
                msg[1] = i;
                msg[2] = (unsigned long)szCmdBuf;
                msg[3] = strlen(szCmdBuf);
                g_toUI_msg_queue.send_message(MODULE_MSG, (void *)msg, sizeof(msg), &g_toUI_sem);
                usleep(3 * 1000);
#endif
            }
        }
    }
    stCSpotterLibInfo.CSpotter_SetAccelerationLevel(hCSpotter, 2); // nAccelerationLevel=2

    pstVoiceMng->hCSpotter = hCSpotter;
    pstVoiceMng->bInit     = TRUE;

    return MI_SUCCESS;
}

MI_S32 Speech_VoiceAnalyzeDeInit()
{
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    if (TRUE == pstVoiceMng->bInit)
    {
        stCSpotterLibInfo.CSpotter_Release(pstVoiceMng->hCSpotter);
    }

    if (stCSpotterLibInfo.pCSpotterLibHandle)
    {
        dlclose(stCSpotterLibInfo.pCSpotterLibHandle);
        stCSpotterLibInfo.pCSpotterLibHandle = NULL;
    }

    return MI_SUCCESS;
}

MI_S32 Speech_VoiceAnalyzeStart()
{
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    int                ret         = 0;

    pstVoiceMng->bRunFlag = TRUE;
    pthread_create(&pstVoiceMng->pt, NULL, _VoiceAnalyzeProc_, NULL);
    if (0 == ret)
    {
        pthread_setname_np(pstVoiceMng->pt, "speech");
    }
    else
    {
        CARDV_ERR("%s pthread_create failed\n", __func__);
    }

    return MI_SUCCESS;
}

MI_S32 Speech_VoiceAnalyzeStop()
{
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    if (TRUE == pstVoiceMng->bRunFlag)
    {
        pstVoiceMng->bRunFlag = FALSE;
        pthread_join(pstVoiceMng->pt, NULL);
    }

    return MI_SUCCESS;
}

MI_BOOL Speech_VoiceIsAnalyzeStart()
{
    CarDV_Voice_Mng_S *pstVoiceMng = &g_stVoiceMng;
    return pstVoiceMng->bRunFlag;
}

MI_S32 Speech_VoiceLearnStart()
{
    return 0;
}

MI_S32 Speech_VoiceLearnStop()
{
    return 0;
}

void Speech_VoiceSendAudioFrame(MI_AUDIO_Frame_t *pstAudioFrame)
{
    _PutFrameToQueue(pstAudioFrame);
}

#if 0 // Not used code
CarDV_Voice_Cmd_S *Speech_VoiceGetCMD(void)
{
    CarDV_Cmd_Mng_S *pstCmdMng = &g_stCmdMng;

    CarDV_Voice_Cmd_S *pstVoiceCMDNode = NULL;
    CarDV_Voice_Cmd_S *pstVoiceCMD     = NULL;
    struct list_head * pListPos        = NULL;
    struct timeval     time_now;
    double             timestamp = 0;

    pthread_mutex_lock(&pstCmdMng->stMutex);
    if (list_empty(&pstCmdMng->stListHead))
    {
        pthread_mutex_unlock(&pstCmdMng->stMutex);
        return NULL;
    }

    pListPos        = pstCmdMng->stListHead.next;
    pstVoiceCMDNode = list_entry(pListPos, CarDV_Voice_Cmd_S, stList);

    pstVoiceCMD = (CarDV_Voice_Cmd_S *)malloc(sizeof(CarDV_Voice_Cmd_S));
    if (pstVoiceCMD == NULL)
    {
        CARDV_ERR("malloc error, not enough memory\n");
        pthread_mutex_unlock(&pstCmdMng->stMutex);
        return NULL;
    }

    pstVoiceCMD->pCmd = (unsigned char *)malloc(pstVoiceCMDNode->cmdLen);
    if (pstVoiceCMD->pCmd == NULL)
    {
        CARDV_ERR("malloc error, not enough memory\n");
        free(pstVoiceCMD);
        pthread_mutex_unlock(&pstCmdMng->stMutex);
        return NULL;
    }
    memset(pstVoiceCMD->pCmd, 0, pstVoiceCMDNode->cmdLen);

    memcpy(pstVoiceCMD->pCmd, pstVoiceCMDNode->pCmd, pstVoiceCMDNode->cmdLen);
    pstVoiceCMD->cmdLen    = pstVoiceCMDNode->cmdLen;
    pstVoiceCMD->aliveTime = pstVoiceCMDNode->aliveTime;
    pstVoiceCMD->timestamp = pstVoiceCMDNode->timestamp;
    pstVoiceCMD->stList    = pstVoiceCMDNode->stList;

    gettimeofday(&time_now, NULL);
    timestamp = (double)(time_now.tv_sec * 1000.0 + time_now.tv_usec / 1000)
                - (double)(pstVoiceCMDNode->timestamp.tv_sec * 1000.0 + pstVoiceCMDNode->timestamp.tv_usec / 1000);
    if (timestamp > pstVoiceCMDNode->aliveTime)
    {
        pstCmdMng->stListHead.next = pListPos->next;
        pListPos->next->prev       = pListPos->prev;

        printf("%s %d del node, %p, %p, cmd:%s\n", __func__, __LINE__, pstVoiceCMDNode, pstVoiceCMDNode->pCmd,
               pstVoiceCMDNode->pCmd);

        if (pstVoiceCMDNode->pCmd)
            free(pstVoiceCMDNode->pCmd);
        free(pstVoiceCMDNode);
    }
    pthread_mutex_unlock(&pstCmdMng->stMutex);

    return pstVoiceCMD;
}
#endif

#endif // libCSpotter
