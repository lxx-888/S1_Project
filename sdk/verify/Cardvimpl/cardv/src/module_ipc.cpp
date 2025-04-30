/*
 * module_ipc.cpp- Sigmastar
 *
 * Copyright (C) 2020 Sigmastar Technology Corp.
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

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include "module_ipc.h"
#include "module_common.h"
#include "module_config.h"

#if (CARDV_RTSP_INPUT_ENABLE || CARDV_WS_INPUT_ENABLE)

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define IPC_MAGIC_NUM   (0x53435049)
#define IPC_SERVER_PATH "/tmp/ipc.server"

//==============================================================================
//
//                              STRUCT DEFINES
//
//==============================================================================

struct IpcParam_t
{
    MI_U32           u32MagicNum;
    MI_S32           s32SvrSock;
    MI_S32           s32ClientSock;
    CarDV_IpcParam_t stParam;
    TAILQ_ENTRY(IpcParam_t) tailq_entry;
};

typedef struct
{
    pthread_t tid;
    BOOL      bExit;
} CarDV_IpcServerAttr_t;

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

TAILQ_HEAD(tailq_head, IpcParam_t) g_ipcQ;
CarDV_IpcServerAttr_t g_stIpcServerAttr = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int IpcClientConnect(CarDV_IpcParam_t *pstParam)
{
    int                sd;
    int                ret;
    struct sockaddr_un client;
    IpcParam_t         stInternalParam;

    if (pstParam == NULL)
    {
        return -1;
    }

    sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sd < 0)
    {
        perror("Socket error ");
        return -2;
    }

    client.sun_family = AF_UNIX;
    snprintf(client.sun_path, sizeof(client.sun_path), "%s", IPC_SERVER_PATH);

    ret = connect(sd, (struct sockaddr *)&client, sizeof(client));
    if (ret < 0)
    {
        perror("Connect error: ");
        close(sd);
    }

    stInternalParam.u32MagicNum   = IPC_MAGIC_NUM;
    stInternalParam.s32ClientSock = sd;
    memcpy(&stInternalParam.stParam, pstParam, sizeof(CarDV_IpcParam_t));
    ret = write(sd, &stInternalParam, sizeof(IpcParam_t));
    if (ret != sizeof(IpcParam_t))
    {
        printf("Connect fail\n");
        close(sd);
        return -3;
    }

    return sd;
}

int IpcClientDisConnect(int fd)
{
    IpcParam_t *pstInternalParam;

    TAILQ_FOREACH(pstInternalParam, &g_ipcQ, tailq_entry)
    {
        if (pstInternalParam->s32ClientSock == fd)
        {
            if (pstInternalParam->stParam.pSvrDisConnect)
            {
                pstInternalParam->stParam.pSvrDisConnect(pstInternalParam->stParam.pArg,
                                                         pstInternalParam->stParam.u32ArgLen);
            }
            close(fd);
            close(pstInternalParam->s32SvrSock);
            TAILQ_REMOVE(&g_ipcQ, pstInternalParam, tailq_entry);
            FREEIF(pstInternalParam);
            return 0;
        }
    }

    return -1;
}

void *IpcServerTask(void *argv)
{
    int                ret       = 0;
    socklen_t          len       = sizeof(struct sockaddr_un);
    int                listen_sd = 0;
    int                client_sd = 0;
    int                flags     = 0; // set non blocking
    fd_set             rfd;           // for listen sd
    fd_set             fds;
    IpcParam_t *       pstInternalParam;
    struct timeval     timeout;
    struct sockaddr_un server;
    struct sockaddr_un client;

    TAILQ_INIT(&g_ipcQ);

#if (CARDV_REARSTREAM_TO_RTSP_ENABLE)
    MI_U8                   u8CamId;
    MI_U8                   u8StreamId;
    CarDV_CameraNode_t *    pstNode;
    CarDV_StreamLinkNode_t *node;
#endif

    listen_sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_sd < 0)
    {
        perror("socket error");
        goto out;
    }

    memset(&server, 0, sizeof(server));
    server.sun_family = AF_UNIX;
    snprintf(server.sun_path, sizeof(server.sun_path), "%s", IPC_SERVER_PATH);
    unlink(IPC_SERVER_PATH);

    flags = fcntl(listen_sd, F_GETFL, 0);
    fcntl(listen_sd, F_SETFL, flags | O_NONBLOCK);

    if (bind(listen_sd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind error");
        goto out;
    }

    if (listen(listen_sd, 20) < 0)
    {
        perror("listen error");
        goto out;
    }

    while (!g_stIpcServerAttr.bExit)
    {
        FD_ZERO(&rfd);
        FD_SET(listen_sd, &rfd);

        timeout.tv_sec  = 0;
        timeout.tv_usec = 100000;

        ret = select(listen_sd + 1, &rfd, NULL, NULL, &timeout);
        if (ret == 0)
        {
            continue;
        }
        else if (ret < 0)
        {
            printf("Select failed\n");
            continue;
        }

        if (FD_ISSET(listen_sd, &rfd))
        {
            memset(&client, 0, sizeof(struct sockaddr_un));
            client_sd = accept(listen_sd, (struct sockaddr *)&client, &len);

            if (client_sd < 0)
            {
                perror("accept error");
                usleep(10000);
                continue;
            }

            FD_ZERO(&fds);
            FD_SET(client_sd, &fds);

            timeout.tv_sec  = 1;
            timeout.tv_usec = 0;

            ret = select(client_sd + 1, &fds, NULL, NULL, &timeout);

            if (ret == 0)
            {
                printf("Read IPC request timeout\n");
                continue;
            }
            else if (ret < 0)
            {
                printf("Select failed\n");
                continue;
            }

            pstInternalParam = (IpcParam_t *)MALLOC(sizeof(IpcParam_t));
            if (pstInternalParam == NULL)
            {
                printf("ALLOC invalid\n");
                continue;
            }

            ret = read(client_sd, pstInternalParam, sizeof(IpcParam_t));
            if (ret != sizeof(IpcParam_t) || pstInternalParam->u32MagicNum != IPC_MAGIC_NUM)
            {
                printf("Input data invalid\n");
                close(client_sd);
                FREEIF(pstInternalParam);
                continue;
            }

            pstInternalParam->s32SvrSock = client_sd;
            TAILQ_INSERT_TAIL(&g_ipcQ, pstInternalParam, tailq_entry);

            if (pstInternalParam->stParam.pSvrConnect)
            {
                fcntl(client_sd, F_SETFL, O_NONBLOCK);
                pstInternalParam->stParam.pSvrConnect(client_sd, pstInternalParam->stParam.pArg,
                                                      pstInternalParam->stParam.u32ArgLen);
            }
        }
    }

out:
    pthread_exit(NULL);
    return 0;
}

void IpcServerStart(void)
{
    pthread_create(&g_stIpcServerAttr.tid, NULL, IpcServerTask, NULL);
    pthread_setname_np(g_stIpcServerAttr.tid, "IpcSvr");
}

void IpcServerStop(void)
{
    g_stIpcServerAttr.bExit = TRUE;
    pthread_join(g_stIpcServerAttr.tid, NULL);
    g_stIpcServerAttr.tid = 0;
}
#endif