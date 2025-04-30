/*************************************************
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 * All rights reserved.
 *
 **************************************************
 * File name:  module_dcf.c
 * Author:     jeff.li@sigmastar.com.cn
 * Version:    Initial Draft
 * Date:       2019/2/26
 * Description: dcf module source file
 *
 *
 *
 * History:
 *
 *    1. Date  :        2019/2/26
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
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/queue.h>
#include <sys/statfs.h>
#include <pthread.h>
#include <dirent.h>
#include <fnmatch.h>

#include "module_common.h"
#include "module_storage.h"
#include "DCF.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define RETRY_CNT (5)

//==============================================================================
//
//                           DELETE FILE MECHANISM
//
//==============================================================================

BOOL DCF_CheckDeleteFilesForSpace(int DB)
{
    unsigned long long usedSizeKB[DB_NUM] = {0};
    unsigned long long usedTotalKB        = 0;
    unsigned long long totalSizeKB        = StorageGetReservedTotalSize() >> 10;
    int                ret                = 0, retry;
    int                idx, camId, pairCamId;
    int                fileCnt         = 0;
    char*              pszFileName     = NULL;
    char*              pszPairFileName = NULL;

    if (DCF_IsDBFormatFree(DB))
    {
        for (camId = 0; camId < CAM_NUM; camId++)
        {
            while (DCF_IsDBFormatFreeEmpty(DB, camId))
            {
                idx = 0;
                while (DCF_IsProtectFileFromHeadByIdx(DB, camId, idx))
                    idx++;

                if (idx == DCF_GetDBFileCnt(DB, camId))
                    return FALSE;

                retry = 0;
                do
                {
                    ret = DCF_DeleteFileFromHeadByIdx(DB, camId, idx);
                } while (ret < 0 && retry++ < RETRY_CNT && StorageReMount() == 0);
            }
        }
    }
    else
    {
        if (DB == DB_NORMAL)
        {
            usedTotalKB = DCF_GetUsedSize() >> 10;
            printf("[%s]\nTotal Size [%llu] MB\nAll Used Size [%llu] MB\n", DCF_GetDBFolderName(DB), totalSizeKB >> 10,
                   usedTotalKB >> 10);

            while (usedTotalKB * 100 / totalSizeKB > 90)
            {
                fileCnt = DCF_GetDBFileCntByDB(DB);
                for (camId = 0; camId < CAM_NUM; camId++)
                {
                    // No files.
                    if (0 == DCF_GetDBFileCnt(DB, camId))
                        continue;

                    idx = 0;
                    while (DCF_IsProtectFileFromHeadByIdx(DB, camId, idx))
                        idx++;

                    // All files are protected.
                    if (idx == DCF_GetDBFileCnt(DB, camId))
                        continue;

                    pszFileName = DCF_GetFileNameFromHeadByIdx(DB, camId, idx);
                    for (pairCamId = camId + 1; pairCamId < CAM_NUM; pairCamId++)
                    {
                        pszPairFileName = DCF_GetPairFileName(pszFileName, pairCamId);
                        if (pszPairFileName)
                        {
                            retry = 0;
                            do
                            {
                                ret = DCF_DeleteFileByFileName(pszPairFileName);
                            } while (ret < 0 && retry++ < RETRY_CNT && StorageReMount() == 0);
                        }
                    }

                    retry = 0;
                    do
                    {
                        ret = DCF_DeleteFileFromHeadByIdx(DB, camId, idx);
                    } while (ret < 0 && retry++ < RETRY_CNT && StorageReMount() == 0);
                    break;
                }

                if (fileCnt == DCF_GetDBFileCntByDB(DB))
                {
                    printf("[%s] Can't Delete Any Files\n", DCF_GetDBFolderName(DB));
                    return FALSE;
                }
                usedTotalKB = DCF_GetUsedSize() >> 10;
            }
        }
        else
        {
            usedSizeKB[DB] = DCF_GetUsedSizeByDB(DB) >> 10;
            printf("[%s]\nTotal Size [%llu] MB\nUsed Size [%llu] MB\n", DCF_GetDBFolderName(DB), totalSizeKB >> 10,
                   usedSizeKB[DB] >> 10);

            while (usedSizeKB[DB] * 100 / totalSizeKB > 30)
            {
                fileCnt = DCF_GetDBFileCntByDB(DB);
                for (camId = 0; camId < CAM_NUM; camId++)
                {
                    // No files.
                    if (0 == DCF_GetDBFileCnt(DB, camId))
                        continue;

                    idx = 0;
                    while (DCF_IsProtectFileFromHeadByIdx(DB, camId, idx))
                        idx++;

                    // All files are protected.
                    if (idx == DCF_GetDBFileCnt(DB, camId))
                        continue;

                    pszFileName = DCF_GetFileNameFromHeadByIdx(DB, camId, idx);
                    for (pairCamId = camId + 1; pairCamId < CAM_NUM; pairCamId++)
                    {
                        pszPairFileName = DCF_GetPairFileName(pszFileName, pairCamId);
                        if (pszPairFileName)
                        {
                            retry = 0;
                            do
                            {
                                ret = DCF_DeleteFileByFileName(pszPairFileName);
                            } while (ret < 0 && retry++ < RETRY_CNT && StorageReMount() == 0);
                        }
                    }

                    retry = 0;
                    do
                    {
                        ret = DCF_DeleteFileFromHeadByIdx(DB, camId, idx);
                    } while (ret < 0 && retry++ < RETRY_CNT && StorageReMount() == 0);
                    break;
                }

                if (fileCnt == DCF_GetDBFileCntByDB(DB))
                {
                    printf("[%s] Can't Delete Any Files\n", DCF_GetDBFolderName(DB));
                    return FALSE;
                }
                usedSizeKB[DB] = DCF_GetUsedSizeByDB(DB) >> 10;
            }
            return DCF_CheckDeleteFilesForSpace(DB_NORMAL);
        }
    }

    return TRUE;
}

void DCF_CheckDeleteFilesForNum(int DB, int nFileNumLimit)
{
    int ret = 0, retry;
    int idx;

    if (DCF_IsDBFormatFree(DB))
        return;

    for (int camId = 0; camId < CAM_NUM; camId++)
    {
        printf("DB [%d] Cam [%d] File Cnt [%d]\n", DB, camId, DCF_GetDBFileCnt(DB, camId));
        while (DCF_GetDBFileCnt(DB, camId) > nFileNumLimit)
        {
            idx = 0;
            while (DCF_IsProtectFileFromHeadByIdx(DB, camId, idx))
                idx++;

            if (idx == DCF_GetDBFileCnt(DB, camId))
                break;

            retry = 0;
            do
            {
                ret = DCF_DeleteFileFromHeadByIdx(DB, camId, idx);
            } while (ret < 0 && retry++ < RETRY_CNT && StorageReMount() == 0);
        }
    }
}

void DCF_CheckDeleteFilesForFormatFreeReserved(int DB, int nReservedCnt)
{
    int ret = 0, retry;
    int idx;

    if (!DCF_IsDBFormatFree(DB))
        return;

    for (int camId = 0; camId < CAM_NUM; camId++)
    {
        while (DCF_GetDBFormatFreeReservedCnt(DB, camId) < nReservedCnt)
        {
            idx = 0;
            while (DCF_IsProtectFileFromHeadByIdx(DB, camId, idx))
                idx++;

            if (idx == DCF_GetDBFileCnt(DB, camId))
                return;

            retry = 0;
            do
            {
                ret = DCF_DeleteFileFromHeadByIdx(DB, camId, idx);
            } while (ret < 0 && retry++ < RETRY_CNT && StorageReMount() == 0);
        }
    }
}