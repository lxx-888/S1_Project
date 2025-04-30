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

#include <assert.h>
#include <elf.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "mi_common_datatype.h"
#include "st_common_ai_glasses_uart.h"
#include "ssapp_glasses_factory.h"
#include "ssapp_glasses_worker.h"
#include "ssapp_glasses_tasklist.h"
#include "ssapp_glasses_misc.h"
#include "ssapp_glasses_signal.h"
#include "ssapp_glasses_debugmode.h"
#include "ssapp_glasses_indemsgtask.h"
#include "cam_os_wrapper.h"

extern SSAPP_GLASSES_FACTORY_DebugParam_t g_stDebugParam;

BOOL g_bWakeUpByResume = FALSE;

pthread_t g_pHeartBeatToMcuWorkThreadHandle;
pthread_t g_pGlassesFactoryWorkDoneWakeUpMainHandle;

CamOsTsem_t      gp_worklistSemlock            = {0};
CamOsMutex_t     gp_uartMsgMutex               = {0};
CamOsCondition_t gp_pendingWorkThreadWaitqueue = {0};
CamOsCondition_t gp_doingWorkWaitqueue         = {0};

MI_S32 SSAPP_GLASSES_FACTORY_ReleaseWorkSpaceAndNotifyMcu(SSAPP_GLASSES_FACTORY_WorkSpace_t *pstFactoryWorkSpace)
{
    if (!pstFactoryWorkSpace)
    {
        printf("pstFactoryWorkSpace NULL!\n");
        return -1;
    }
    CamOsTsemDown(&gp_worklistSemlock);
    SSAPP_GLASSES_TASKLIST_DelList(pstFactoryWorkSpace);
    CamOsTsemUp(&gp_worklistSemlock);

    // if hijack by stop task, should send new taskid back to mcu
    // will send done to mcu
    printf("bNoReplayMcuDone:%d task:%d taskid:%d\n", pstFactoryWorkSpace->bNoReplayMcuDone,
           pstFactoryWorkSpace->stProtcolTask.eTaskCmd, pstFactoryWorkSpace->stProtcolTask.u16UserDefine & 0xFF);
    if (!pstFactoryWorkSpace->bNoReplayMcuDone)
    {
        printf("replay task done to mcu taskid:%x\n", pstFactoryWorkSpace->stProtcolTask.u16UserDefine);
        SSAPP_GLASSES_FACTORY_SendCmdToMcuUart(E_CMD_TYPE_ACK, E_TASK_DONE, FALSE, E_STATE_IDEL,
                                               pstFactoryWorkSpace->stProtcolTask.u16UserDefine);
    }
    CamOsMemRelease(pstFactoryWorkSpace);
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_FACTORY_WorkDoneWakeUpMain(void)
{
    SSAPP_GLASSES_FACTORY_WorkSpace_t *pstFactoryWorkSpace = NULL;
    CamOsTsemDown(&gp_worklistSemlock);
    pstFactoryWorkSpace = SSAPP_GLASSES_TASKLIST_GetDoingWork();
    CamOsTsemUp(&gp_worklistSemlock);
    if (!pstFactoryWorkSpace)
    {
        printf("cat't get a doing work\n");
        return -1;
    }
    pstFactoryWorkSpace->bWorkHadDone = TRUE;

    CamOsConditionWakeUpAll(&gp_doingWorkWaitqueue);
    return MI_SUCCESS;
}

/* WakeUp main work thread can send E_TASK_REQUEST cmd to mcu */
/* if st_Doing_WorkList empty When app resume by system */
void SSAPP_GLASSES_FACTORY_WakeUpMainByResume(void)
{
    g_bWakeUpByResume = TRUE;

    CamOsConditionWakeUpAll(&gp_doingWorkWaitqueue);
    CamOsConditionWakeUpAll(&gp_pendingWorkThreadWaitqueue);
}

void *SSAPP_GLASSES_FACTORY_MainPipeWorkThread(void *args)
{
#ifdef SS_SENSOR_DYNAMIC_POWER_ONOFF
    BOOL bJustResume = FALSE;
#endif
    UNUSED(args);
    while (!SSAPP_GLASSES_SIGNAL_ThreadCanExit())
    {
    BEGIN:
        if (!SSAPP_GLASSES_TASKLIST_PendingWorkListIsEmpty() && !SSAPP_GLASSES_SIGNAL_ThreadCanExit())
        {
            SSAPP_GLASSES_FACTORY_WorkSpace_t *pstFactoryWorkSpace = NULL;
            MI_VIRT                            begintime           = 0;

            CamOsTsemDown(&gp_worklistSemlock);
            pstFactoryWorkSpace = SSAPP_GLASSES_TASKLIST_GetPendingWork();
            assert(pstFactoryWorkSpace != NULL);
            SSAPP_GLASSES_TASKLIST_AddToDoingWorkList(pstFactoryWorkSpace);
            CamOsTsemUp(&gp_worklistSemlock);
            if (!pstFactoryWorkSpace)
            {
                printf("pstFactoryWorkSpace should not be NULL!\n");
                continue;
            }
#ifdef SS_SENSOR_DYNAMIC_POWER_ONOFF
            if (bJustResume)
            {
                if (pstFactoryWorkSpace->stProtcolTask.eTaskCmd == E_TASK_PHOTO
                    || pstFactoryWorkSpace->stProtcolTask.eTaskCmd == E_TASK_START_REC)
                {
                    pstFactoryWorkSpace->bResumeFromStr = bJustResume;
                    bJustResume                         = FALSE;
                }
            }
#endif
            if (!pstFactoryWorkSpace->workerFun)
            {
                printf("workerFun should not be NULL!\n");
                return (void *)-1;
            }
            begintime = SSAPP_GLASSES_MISC_GetPts();
            pstFactoryWorkSpace->workerFun(pstFactoryWorkSpace);
#ifdef SS_SENSOR_DYNAMIC_POWER_ONOFF
            if (pstFactoryWorkSpace->bHadSuspend)
            {
                bJustResume = TRUE;
            }
#endif
            printf("task:%d workerFun cost %ldms\n", pstFactoryWorkSpace->stProtcolTask.eTaskCmd,
                   SSAPP_GLASSES_MISC_GetPts() - begintime);
            if (pstFactoryWorkSpace->bNeedWaitDone)
            {
                printf("[%ld]gp_doingWorkWaitqueue begin\n", SSAPP_GLASSES_MISC_GetPts());
                if (g_stDebugParam.bWaitDoingTimeout)
                {
                    // wait work done with timeout
                    CamOsConditionTimedWait(&gp_doingWorkWaitqueue,
                                            pstFactoryWorkSpace->bWorkHadDone || g_bWakeUpByResume, 1000000UL);
                }
                else
                {
                    CamOsConditionWait(&gp_doingWorkWaitqueue, pstFactoryWorkSpace->bWorkHadDone || g_bWakeUpByResume);
                }
                printf("[%ld]gp_doingWorkWaitqueue end\n", SSAPP_GLASSES_MISC_GetPts());
            }
            if (pstFactoryWorkSpace->workerCleanFun)
            {
                begintime = SSAPP_GLASSES_MISC_GetPts();
                pstFactoryWorkSpace->workerCleanFun(pstFactoryWorkSpace);
                printf("task:%d workerCleanFun cost %ldms\n", pstFactoryWorkSpace->stProtcolTask.eTaskCmd,
                       SSAPP_GLASSES_MISC_GetPts() - begintime);
            }

            // release pstFactoryWorkSpace
            SSAPP_GLASSES_FACTORY_ReleaseWorkSpaceAndNotifyMcu(pstFactoryWorkSpace);
            goto BEGIN;
        }
        else
        {
            // send request task
            printf("we need request task again!\n");
            SSAPP_GLASSES_FACTORY_SendCmdToMcuUart(E_CMD_TYPE_REQ, E_TASK_REQUEST, FALSE, E_STATE_IDEL, 0xff);
        }

        printf("[%ld]gp_pendingWorkThreadWaitqueue begin\n", SSAPP_GLASSES_MISC_GetPts());
        CamOsConditionTimedWait(&gp_pendingWorkThreadWaitqueue,
                                !SSAPP_GLASSES_TASKLIST_PendingWorkListIsEmpty() || g_bWakeUpByResume, 100000UL);
        printf("[%ld]gp_pendingWorkThreadWaitqueue end. isempty:%d wakebyresume:%d\n", SSAPP_GLASSES_MISC_GetPts(),
               SSAPP_GLASSES_TASKLIST_PendingWorkListIsEmpty(), g_bWakeUpByResume);
        g_bWakeUpByResume = FALSE;
    }
    printf("%s exit\n", __FUNCTION__);
    return NULL;
}

MI_S32 SSAPP_GLASSES_FACTORY_ProtSend(char *cmdMsg, MI_U32 length)
{
    MI_S32 ret = MI_SUCCESS;

#if DEBUG_LOG_LEVEL >= DEBUG_LOG_INFO
    CamOsMutexLock(&gp_uartMsgMutex);
    printf("[%ld]send cmdMsg:\n", SSAPP_GLASSES_MISC_GetPts());
    for (int cnt = 0; cnt < length; cnt++)
    {
        printf("%x ", cmdMsg[cnt]);
    }
    printf("\n");
    CamOsMutexUnlock(&gp_uartMsgMutex);
#endif
    if (g_stDebugParam.bDebugMode && cmdMsg[5] == E_TASK_REQUEST && !SSAPP_GLASSES_DEBUGMODE_IsExist())
    {
        // add debug task
        ret = SSAPP_GLASSES_DEBUGMODE_GetDebugTaskInjectRealWork();
    }
    else
    {
        if (ST_Common_AiGlasses_Uart_Write(cmdMsg, length) != length)
        {
            ret = -1;
        }
    }
    return ret;
}

MI_S32 SSAPP_GLASSES_FACTORY_SendCmdToMcuUart(SS_CMD_TYPE_e eCmdType, SS_TASK_e eTaskCmd, BOOL isReject,
                                              SS_STATE_e eSocState, MI_U16 u16UserDefine)
{
    MI_S32             ret           = MI_SUCCESS;
    ST_Protrcol_Task_t stProtcolTask = {0};

    stProtcolTask.eCmdType      = eCmdType;
    stProtcolTask.eTaskCmd      = eTaskCmd;
    stProtcolTask.isReject      = isReject;
    stProtcolTask.eSocState     = eSocState;
    stProtcolTask.u16UserDefine = u16UserDefine;

    ret = ST_Common_AiGlasses_Prot_Make_Cmd_To_Send(&stProtcolTask, (void *)SSAPP_GLASSES_FACTORY_ProtSend);
    if (ret != MI_SUCCESS)
    {
        printf("make cmd failed ret:%d\r\n", ret);
    }
    return ret;
}

void *SSAPP_GLASSES_FACTORY_HeartBeatToMcuWorkThread(void *args)
{
    SSAPP_GLASSES_FACTORY_WorkSpace_t *pstDoingWork = NULL;
    SS_STATE_e                         eSocState    = E_STATE_IDEL;

    UNUSED(args);
    while (!SSAPP_GLASSES_SIGNAL_ThreadCanExit())
    {
        sleep(2);
        // maybe need 100ms dealy to send heartbeat
        pstDoingWork = SSAPP_GLASSES_TASKLIST_GetDoingWork();
        if (pstDoingWork)
        {
            if (pstDoingWork->stProtcolTask.eTaskCmd == E_TASK_PHOTO)
            {
                eSocState = E_STATE_CAP_PIC;
            }
            else if (pstDoingWork->stProtcolTask.eTaskCmd == E_TASK_START_REC)
            {
                eSocState = E_STATE_REC;
            }
            else if (pstDoingWork->stProtcolTask.eTaskCmd == E_TASK_TRANS)
            {
                eSocState = E_STATE_TRANS;
            }
        }
        SSAPP_GLASSES_FACTORY_SendCmdToMcuUart(E_CMD_TYPE_ACK, E_TASK_HEARTBEAT, FALSE, eSocState, 0xff);
    }
    return NULL;
}

MI_S32 SSAPP_GLASSES_FACTORY_Init(void)
{
    CamOsMutexInit(&gp_uartMsgMutex);
    SSAPP_GLASSES_TASKLIST_Init();
    CamOsTsemInit(&gp_worklistSemlock, 1);
    CamOsConditionInit(&gp_pendingWorkThreadWaitqueue);
    CamOsConditionInit(&gp_doingWorkWaitqueue);

    SSAPP_GLASSES_WORKER_Init();
    pthread_create(&g_pGlassesFactoryWorkDoneWakeUpMainHandle, NULL, SSAPP_GLASSES_FACTORY_MainPipeWorkThread, NULL);
    if (!g_stDebugParam.bDisableHeartBeat)
    {
        pthread_create(&g_pHeartBeatToMcuWorkThreadHandle, NULL, SSAPP_GLASSES_FACTORY_HeartBeatToMcuWorkThread, NULL);
    }

    SSAPP_GLASSES_DEBUGMODE_Init();

    // IndefLenMsg test
    {
        SS_INDEMSG_t stIndeMsg = {0};

        stIndeMsg.eIndeMsgType = E_INDEF_TYPE_SIGMASTAR_HELLO;
        sprintf(stIndeMsg.stIndefLenMsg.sigmaStar, "Leading AI Everywhere");
        SSAPP_GLASSES_INDEMSGTASK_SendMsg(&stIndeMsg);
    }

    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_FACTORY_DeInit(void)
{
    printf("factory deinit\n");
    pthread_join(g_pGlassesFactoryWorkDoneWakeUpMainHandle, NULL);
    printf("factory main exit\n");
    if (!g_stDebugParam.bDisableHeartBeat)
    {
        pthread_join(g_pHeartBeatToMcuWorkThreadHandle, NULL);
        printf("factory heartbeat exit\n");
    }
    CamOsTsemDeinit(&gp_worklistSemlock);
    CamOsConditionDeinit(&gp_pendingWorkThreadWaitqueue);
    CamOsConditionDeinit(&gp_doingWorkWaitqueue);

    SSAPP_GLASSES_DEBUGMODE_DeInit();
    SSAPP_GLASSES_WORKER_DeInit();
    CamOsMutexDestroy(&gp_uartMsgMutex);
    printf("factory exit\n");
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_FACTORY_TaskManagerAssignWorker(SSAPP_GLASSES_FACTORY_WorkSpace_t *pstFactoryWorkSpace)
{
    SSAPP_GLASSES_FACTORY_WorkSpace_t *pstDoingWork = NULL;
    BOOL                               bStopCmd     = FALSE;

    if (!pstFactoryWorkSpace)
    {
        printf("pstFactoryWorkSpace is NULL\n");
        return -1;
    }

    if (pstFactoryWorkSpace->stProtcolTask.eTaskCmd == E_TASK_NONE)
    {
        pstFactoryWorkSpace->workerFun = SSAPP_GLASSES_WORKER_DealTaskNone;
    }
    else if (pstFactoryWorkSpace->stProtcolTask.eTaskCmd == E_TASK_PHOTO)
    {
        pstFactoryWorkSpace->workerFun = SSAPP_GLASSES_WORKER_DealTaskPhoto;
    }
    else if (pstFactoryWorkSpace->stProtcolTask.eTaskCmd == E_TASK_START_REC)
    {
        pstFactoryWorkSpace->workerFun = SSAPP_GLASSES_WORKER_DealTaskStartRec;
    }
    else if (pstFactoryWorkSpace->stProtcolTask.eTaskCmd == E_TASK_STOP_REC)
    {
        bStopCmd = TRUE;
        CamOsTsemDown(&gp_worklistSemlock);
        pstDoingWork = SSAPP_GLASSES_TASKLIST_GetDoingWork();
        CamOsTsemUp(&gp_worklistSemlock);
        if (pstDoingWork && pstDoingWork->stProtcolTask.eTaskCmd == E_TASK_START_REC)
        {
            // we will hijack the pstDoingWork and replace it't taskId
            pstDoingWork->stProtcolTask.u16UserDefine = pstFactoryWorkSpace->stProtcolTask.u16UserDefine;
        }
        else
        {
            if (!pstDoingWork)
            {
                printf("pstDoingWork is NULL");
                bStopCmd                       = FALSE;
                pstFactoryWorkSpace->workerFun = SSAPP_GLASSES_WORKER_NullStopTask;
            }
            else
            {
                printf("pstDoingWork eTaskCmd:%d", pstDoingWork->stProtcolTask.eTaskCmd);
            }
            printf(" why the E_TASK_STOP_REC cmd come?\n");
        }
    }
    else if (pstFactoryWorkSpace->stProtcolTask.eTaskCmd == E_TASK_TRANS)
    {
        pstFactoryWorkSpace->workerFun = SSAPP_GLASSES_WORKER_DealTaskStartTrans;
    }
    else if (pstFactoryWorkSpace->stProtcolTask.eTaskCmd == E_TASK_STOP_TRANS)
    {
        bStopCmd = TRUE;
        CamOsTsemDown(&gp_worklistSemlock);
        pstDoingWork = SSAPP_GLASSES_TASKLIST_GetDoingWork();
        CamOsTsemUp(&gp_worklistSemlock);
        if (pstDoingWork && pstDoingWork->stProtcolTask.eTaskCmd == E_TASK_TRANS)
        {
            // we will hijack the pstDoingWork and replace it't taskId
            pstDoingWork->stProtcolTask.u16UserDefine = pstFactoryWorkSpace->stProtcolTask.u16UserDefine;
        }
        else
        {
            if (!pstDoingWork)
            {
                printf("pstDoingWork is NULL");
                bStopCmd                       = FALSE;
                pstFactoryWorkSpace->workerFun = SSAPP_GLASSES_WORKER_NullStopTask;
            }
            else
            {
                printf("pstDoingWork eTaskCmd:%d\n", pstDoingWork->stProtcolTask.eTaskCmd);
            }
            printf(" why the E_TASK_STOP_TRANS cmd come?\n");
        }
    }

    if (bStopCmd)
    {
        if (pstDoingWork)
        {
            pstDoingWork->bWorkHadDone = TRUE;
            CamOsConditionWakeUpAll(&gp_doingWorkWaitqueue);
        }
        CamOsMemRelease(pstFactoryWorkSpace);
    }
    else
    {
        CamOsTsemDown(&gp_worklistSemlock);
        SSAPP_GLASSES_TASKLIST_AddToPendingWorkList(pstFactoryWorkSpace);
        CamOsTsemUp(&gp_worklistSemlock);

        // wakeup worker thread
        CamOsConditionWakeUpAll(&gp_pendingWorkThreadWaitqueue);
    }
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_FACTORY_HandleTask(ST_Protrcol_Task_t *pstProtcolTask)
{
    SSAPP_GLASSES_FACTORY_WorkSpace_t *pstFactoryWorkSpace = NULL;
    if (!pstProtcolTask)
    {
        printf("pstProtcolTask is NULL!\n");
        return -1;
    }
    if (pstProtcolTask->pstProtMsg == NULL)
    {
        pstFactoryWorkSpace = CamOsMemAlloc(sizeof(SSAPP_GLASSES_FACTORY_WorkSpace_t));
        if (!pstFactoryWorkSpace)
        {
            printf("alloc pstFactoryWorkSpace failed\n");
            return -1;
        }
        memset(pstFactoryWorkSpace, 0x0, sizeof(SSAPP_GLASSES_FACTORY_WorkSpace_t));
        memcpy(&pstFactoryWorkSpace->stProtcolTask, pstProtcolTask, sizeof(ST_Protrcol_Task_t));

        return SSAPP_GLASSES_FACTORY_TaskManagerAssignWorker(pstFactoryWorkSpace);
    }
    if (pstProtcolTask->pstProtMsg->isIndeLengMsg)
    {
        return SSAPP_GLASSES_INDEMSGTASK_HandleMsg(pstProtcolTask->pstProtMsg);
    }
    printf("handle userTask IndeLengMsg err!\n");
    return -1;
}
