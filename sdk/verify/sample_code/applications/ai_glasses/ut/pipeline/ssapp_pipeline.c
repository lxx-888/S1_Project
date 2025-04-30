/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include "st_common_ai_glasses.h"
#include "ptree_api_vif.h"
#include "ptree_api_isp.h"
#include "ptree_api_file.h"
#include "ssos_time.h"

typedef enum
{
    E_SSAPP_PIPELINE_CAPTURE = 0,
    E_SSAPP_PIPELINE_RECORD,
    E_SSAPP_PIPELINE_MIX,
    E_SSAPP_PIPELINE_MIX_REVERSE
} SSAPP_PIPELINE_CaseType_t;

typedef struct
{
    SSAPP_PIPELINE_CaseType_t stCaseType;
    int                       testCnt;
    int                       recordDuration;
} SSAPP_PIPELINE_CaseParam_t;

static int    g_testExit         = 0;
unsigned char g_bOnlyLightSensor = 1;

static void _SSAPP_PIPELINE_HelpMessage(void)
{
    printf("Pipeline command usage:\n");
    printf("    'f': specify the json path to load.\n");
    printf("    'c': capturing. Take one phote\n");
    printf("    'r': start recording. The recording will end after the specifid time interval.\n");
    printf("    'v': run mix mode in reverse order, test the specifid times.\n");
    printf("    'd': set record duration. Only take effect when app run in mix mode and record mode.\n");
    printf("    'n': set test times. Only take effect when app run in mix mode.\n");
    printf("    'h': Help message.\n");
}

static inline unsigned long _SSAPP_PIPELINE_GetTimer(void)
{
    SSOS_TIME_Spec_t ts;
    SSOS_TIME_Get(&ts);
    return (ts.tvSec * 1000000) + (ts.tvNSec / 1000);
}

static inline int _SSAPP_PIPELINE_DeleteFile(const char *filePath)
{
    int ret = -1;
    if (remove(filePath) == 0)
    {
        ret = 0;
    }
    else if (errno == ENOENT)
    {
        printf("File '%s' does not exist, no need to delete.\n", filePath);
        ret = 0;
    }
    else
    {
        printf("errno: %d\n", errno);
    }

    return ret;
}

static inline int _SSAPP_PIPELINE_CheckFile(const char *filePath)
{
    struct stat fileStat;
    if (stat(filePath, &fileStat) == -1)
    {
        return -1;
    }
    return ((fileStat.st_size > 0) ? 0 : -1);
}

int SSAPP_PIPELINE_DoCapture(ST_Common_AiGlasses_Handle_t *pHandle)
{
    int ret = 0;
    ret     = _SSAPP_PIPELINE_DeleteFile("capture.jpeg");
    ret |= _SSAPP_PIPELINE_DeleteFile("capture_thumb.jpeg");

    if (!ret)
    {
        printf("start capture time: %luus\n", _SSAPP_PIPELINE_GetTimer());
        ret = ST_Common_AiGlasses_Capture(pHandle, "capture.jpeg", "capture_thumb.jpeg", 1, -1);
        ret |= _SSAPP_PIPELINE_CheckFile("capture.jpeg");
        ret |= _SSAPP_PIPELINE_CheckFile("capture_thumb.jpeg");
    }

    printf("Excute capture %s.\n", (ret ? "fail" : "success"));
    return ret;
}

int SSAPP_PIPELINE_DoRecord(ST_Common_AiGlasses_Handle_t *pHandle, int duration)
{
    int ret = 0;
    ret     = _SSAPP_PIPELINE_DeleteFile("record.h265");
    ret |= _SSAPP_PIPELINE_DeleteFile("record_thumb.jpeg");

    if (!ret)
    {
        printf("start record time: %luus\n", _SSAPP_PIPELINE_GetTimer());
        ret = ST_Common_AiGlasses_StartRecord(pHandle, "record.h265", "record_thumb.jpeg", -1);
        if (!ret)
        {
            printf("Start record success, it will exit after %ds ...\n", duration);
            sleep(duration);
            ret = ST_Common_AiGlasses_StopRecord(pHandle);
            ret |= _SSAPP_PIPELINE_CheckFile("record.h265");
            ret |= _SSAPP_PIPELINE_CheckFile("record_thumb.jpeg");
        }
    }

    printf("Excute record %s.\n", (ret ? "fail" : "success"));
    return ret;
}

int SSAPP_PIPELINE_DoMix(ST_Common_AiGlasses_Handle_t *pHandle, int runCnt, int duration)
{
    int ret = 0;
    while (runCnt > 0)
    {
        if (g_testExit)
        {
            break;
        }
        ret = SSAPP_PIPELINE_DoCapture(pHandle);
        if (ret)
        {
            printf("DoCapture fail\n");
            break;
        }
        ret = SSAPP_PIPELINE_DoRecord(pHandle, duration);
        if (ret)
        {
            printf("DoRecord fail\n");
            break;
        }
        runCnt--;
    }

    printf("Excute mix %s.\n", (ret ? "fail" : "success"));
    return ret;
}

int SSAPP_PIPELINE_DoReverseMix(ST_Common_AiGlasses_Handle_t *pHandle, int runCnt, int duration)
{
    int ret = 0;
    while (runCnt > 0)
    {
        if (g_testExit)
        {
            break;
        }
        ret = SSAPP_PIPELINE_DoRecord(pHandle, duration);
        if (ret)
        {
            printf("DoRecord fail\n");
            break;
        }
        ret = SSAPP_PIPELINE_DoCapture(pHandle);
        if (ret)
        {
            printf("DoCapture fail\n");
            break;
        }
        runCnt--;
    }

    printf("Excute mix %s.\n", (ret ? "fail" : "success"));
    return ret;
}

int SSAPP_PIPELINE_UtTest(char *jsonPath, SSAPP_PIPELINE_CaseParam_t *pstCaseParam)
{
    ST_Common_AiGlasses_Handle_t *pstHandle = NULL;
    int                           ret       = 0;

    ST_Common_AiGlasses_Init();
    pstHandle = ST_Common_AiGlasses_CreatePipeline(jsonPath);

    if (!pstHandle)
    {
        printf("pHandle: %p\n", pstHandle);
        goto EXIT;
    }

    switch (pstCaseParam->stCaseType)
    {
        case E_SSAPP_PIPELINE_CAPTURE:
            while (pstCaseParam->testCnt > 0)
            {
                if (g_testExit)
                {
                    break;
                }
                ret = SSAPP_PIPELINE_DoCapture(pstHandle);
                if (ret)
                {
                    printf("DoCapture fail\n");
                    break;
                }
                pstCaseParam->testCnt--;
            }
            break;
        case E_SSAPP_PIPELINE_RECORD:
            while (pstCaseParam->testCnt > 0)
            {
                if (g_testExit)
                {
                    break;
                }
                ret = SSAPP_PIPELINE_DoRecord(pstHandle, pstCaseParam->recordDuration);
                if (ret)
                {
                    printf("DoCapture fail\n");
                    break;
                }
                pstCaseParam->testCnt--;
            }
            break;
        case E_SSAPP_PIPELINE_MIX:
            ret = SSAPP_PIPELINE_DoMix(pstHandle, pstCaseParam->testCnt, pstCaseParam->recordDuration);
            break;
        case E_SSAPP_PIPELINE_MIX_REVERSE:
            ret = SSAPP_PIPELINE_DoReverseMix(pstHandle, pstCaseParam->testCnt, pstCaseParam->recordDuration);
            break;
        default:
            printf("impossible case\n");
            ret = -1;
            break;
    }

EXIT:
    ST_Common_AiGlasses_DestroyPipeline(pstHandle);
    ST_Common_AiGlasses_Deinit();

    return ret;
}

void SSAPP_PIPELINE_SignalHandler(int signal)
{
    if (signal == SIGINT)
    {
        printf("Received SIGINT signal.\n");
        g_testExit = 1;
    }
}

int main(int argc, char **argv)
{
    SSAPP_PIPELINE_CaseParam_t stCaseParam   = {E_SSAPP_PIPELINE_MIX, 3, 3};
    int                        ret           = 0;
    int                        option        = 0;
    char                       jsonPath[256] = {0};

    signal(SIGINT, SSAPP_PIPELINE_SignalHandler);
    strcpy(jsonPath, "/customer/sample_code/bin/resource/ai_glasses_aestable_whole.json");

    while ((option = getopt(argc, argv, "hrcvn:d:f:")) != -1)
    {
        switch (option)
        {
            case 'h':
                _SSAPP_PIPELINE_HelpMessage();
                break;
            case 'r':
                stCaseParam.stCaseType = E_SSAPP_PIPELINE_RECORD;
                break;
            case 'c':
                stCaseParam.stCaseType = E_SSAPP_PIPELINE_CAPTURE;
                break;
            case 'v':
                stCaseParam.stCaseType = E_SSAPP_PIPELINE_MIX_REVERSE;
                break;
            case 'n':
                stCaseParam.testCnt = strtol(optarg, NULL, 10);
                break;
            case 'd':
                stCaseParam.recordDuration = strtol(optarg, NULL, 10);
                break;
            case 'f':
                memset(jsonPath, 0, sizeof(jsonPath));
                strcpy(jsonPath, optarg);
                break;
            default:
                break;
        }
    }

    if (!strlen(jsonPath))
    {
        printf("Invalid json path!\n");
        return -1;
    }

    ret = SSAPP_PIPELINE_UtTest(jsonPath, &stCaseParam);

    return ret;
}
