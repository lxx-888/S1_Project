/*
 * module_jpd.cpp- Sigmastar
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

#include "mid_common.h"
#include "module_common.h"
#include "module_jpd.h"
#include <pthread.h>
#include <string>
#include <vector>
#include <dirent.h>

#if (CARDV_JPD_ENABLE)

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define CARDV_JPG_MAX_SIZE (3 * 512 * 1024)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

CarDV_JpdModAttr_t gstJpdModule = {0};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void getFiles(std::string path, std::vector<std::string> &files)
{
    DIR *          dir;
    struct dirent *ptr;

    std::string p;
    if ((dir = opendir(path.c_str())) == NULL)
    {
        printf("open [%s] fail\n", path.c_str());
        return;
    }
    while ((ptr = readdir(dir)) != NULL)
    {
        if (ptr->d_type & DT_DIR)
        {
            if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
                getFiles(p.assign(path).append("/").append(ptr->d_name), files);
        }
        else
        {
            std::string fname(ptr->d_name);
            std::string imgType = fname.substr(fname.rfind("."), fname.length());
            if (imgType == ".jpg" || imgType == ".jpeg" || imgType == ".JPG" || imgType == ".JPEG")
            {
                files.push_back(p.assign(path).append("/").append(ptr->d_name));
            }
        }
    }

    closedir(dir);
}

static void *_PutJpdInputDataThread(void *args)
{
    MI_S32                   s32Ret = MI_SUCCESS;
    MI_JPD_CHN               u32JpdChn;
    MI_JPD_DEV               u32JpdDevId       = CARDV_JPD_DEV_ID;
    MI_U8 *                  pu8Buf            = NULL;
    MI_U32                   u32FrameLen       = 0;
    MI_S32                   s32TimeOutMs      = 0;
    MI_U32                   u32RequiredLength = 0;
    MI_JPD_StreamBuf_t       stRetStreamBuf;
    CarDV_JpdChnAttr_t *     pstJpdChnAttr = (CarDV_JpdChnAttr_t *)args;
    int                      i = 0, fd = -1;
    std::vector<std::string> img_vec;

    u32JpdChn = pstJpdChnAttr->u32ChnId;
    pu8Buf    = (MI_U8 *)malloc(CARDV_JPG_MAX_SIZE);
    if (pu8Buf == NULL)
    {
        printf("alloc buffer fail\n");
        return 0;
    }

    getFiles(pstJpdChnAttr->szFilePath, img_vec);
    if (img_vec.size() == 0)
    {
        printf("NO jpg file\n");
        free(pu8Buf);
        return 0;
    }

    while (!pstJpdChnAttr->bThreadExit)
    {
        fd = open(img_vec[i].c_str(), O_RDONLY);
        if (fd < 0)
        {
            printf("open file %s fail\n", img_vec[i].c_str());
            return 0;
        }
        u32FrameLen = read(fd, pu8Buf, CARDV_JPG_MAX_SIZE);
        if (u32FrameLen <= 0)
        {
            printf("read file %s fail\n", img_vec[i].c_str());
            close(fd);
            free(pu8Buf);
            return 0;
        }
        close(fd);

        u32RequiredLength = u32FrameLen;
        memset(&stRetStreamBuf, 0x0, sizeof(MI_JPD_StreamBuf_t));
        s32TimeOutMs = 30; // timeout is 30ms
        s32Ret       = MI_JPD_GetStreamBuf(u32JpdDevId, u32JpdChn, u32RequiredLength, &stRetStreamBuf, s32TimeOutMs);
        if (MI_SUCCESS != s32Ret)
        {
            // printf("chn%u MI_JPD_GetStreamBuf failed, s32Ret: 0x%x.\n", u32JpdChn, s32Ret);
            continue;
        }

        memcpy(stRetStreamBuf.pu8HeadVirtAddr, pu8Buf, MIN(stRetStreamBuf.u32HeadLength, u32RequiredLength));

        if (stRetStreamBuf.u32HeadLength + stRetStreamBuf.u32TailLength < u32RequiredLength)
        {
            // something wrong happen
            printf("MI_JPD_GetStreamBuf return wrong value: HeadLen%u TailLen%u RequiredLength%u\n",
                   stRetStreamBuf.u32HeadLength, stRetStreamBuf.u32TailLength, u32RequiredLength);
            s32Ret = MI_JPD_DropStreamBuf(u32JpdDevId, u32JpdChn, &stRetStreamBuf);
            if (MI_SUCCESS != s32Ret)
            {
                printf("chn%u MI_JPD_DropStreamBuf failed, s32Ret: 0x%x.\n", u32JpdChn, s32Ret);
                continue;
            }
        }
        else if (stRetStreamBuf.u32TailLength > 0)
        {
            memcpy(stRetStreamBuf.pu8TailVirtAddr, pu8Buf + stRetStreamBuf.u32HeadLength,
                   MIN(stRetStreamBuf.u32TailLength, u32RequiredLength - stRetStreamBuf.u32HeadLength));
        }

        stRetStreamBuf.u32ContentLength = u32RequiredLength;
        s32Ret                          = MI_JPD_PutStreamBuf(u32JpdDevId, u32JpdChn, &stRetStreamBuf);
        if (MI_SUCCESS != s32Ret)
        {
            printf("chn%u MI_JPD_PutStreamBuf failed, s32Ret: 0x%x.\n", u32JpdChn, s32Ret);
            continue;
        }

        usleep(1000000 / pstJpdChnAttr->u32Fps);
        i++;
        i = i % img_vec.size();
    }

    free(pu8Buf);
    return NULL;
}

MI_S32 CarDV_JpdModuleInit(MI_JPD_CHN u32JpdChn)
{
    MI_JPD_DEV            u32JpdDevId = CARDV_JPD_DEV_ID;
    MI_JPD_ChnCreatConf_t stChnCreatConf;
    CarDV_JpdChnAttr_t *  pstJpdChnAttr = &gstJpdModule.stJpdChnAttr[u32JpdChn];
    char                  taskName[64];

    if (pstJpdChnAttr->bUsed && pstJpdChnAttr->bCreate == FALSE)
    {
        memset(&stChnCreatConf, 0x0, sizeof(MI_JPD_ChnCreatConf_t));
        stChnCreatConf.u32MaxPicWidth   = pstJpdChnAttr->u32MaxW;
        stChnCreatConf.u32MaxPicHeight  = pstJpdChnAttr->u32MaxH;
        stChnCreatConf.u32StreamBufSize = pstJpdChnAttr->u32StreamBufSize;
        CARDVCHECKRESULT_OS(MI_JPD_CreateChn(u32JpdDevId, u32JpdChn, &stChnCreatConf));
        CARDVCHECKRESULT_OS(MI_JPD_StartChn(u32JpdDevId, u32JpdChn));
        pstJpdChnAttr->bCreate = TRUE;

        pstJpdChnAttr->bThreadExit = FALSE;
        pthread_create(&pstJpdChnAttr->pPutDataThread, NULL, _PutJpdInputDataThread, (void *)pstJpdChnAttr);
        sprintf(taskName, "jpd%d_task", u32JpdChn);
        pthread_setname_np(pstJpdChnAttr->pPutDataThread, taskName);
    }

    return 0;
}

MI_S32 CarDV_JpdModuleUnInit(MI_JPD_CHN u32JpdChn)
{
    MI_JPD_DEV          u32JpdDevId   = CARDV_JPD_DEV_ID;
    CarDV_JpdChnAttr_t *pstJpdChnAttr = &gstJpdModule.stJpdChnAttr[u32JpdChn];

    if (pstJpdChnAttr->bUsed && pstJpdChnAttr->bCreate == TRUE)
    {
        pstJpdChnAttr->bThreadExit = TRUE;
        pthread_join(pstJpdChnAttr->pPutDataThread, NULL);

        CARDVCHECKRESULT(MI_JPD_StopChn(u32JpdDevId, u32JpdChn));
        CARDVCHECKRESULT(MI_JPD_DestroyChn(u32JpdDevId, u32JpdChn));
        pstJpdChnAttr->bCreate = FALSE;
    }

    return 0;
}
#endif