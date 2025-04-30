/*************************************************
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 * All rights reserved.
 *
 **************************************************
 * File name:  module_storage.c
 * Author:     jeff.li@sigmastar.com.cn
 * Version:    Initial Draft
 * Date:       2019/3/6
 * Description: storage module source file
 *
 *
 *
 * History:
 *
 *    1. Date  :        2019/3/6
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
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/statfs.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "module_common.h"
#include "module_system.h"
#include "module_storage.h"
#include "DCF.h"
#include "IPC_msg.h"
#include "IPC_cardvInfo.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define PARTTITION_CHECK "/proc/partitions"

#define UEVENT_BUFFER_SIZE 1024
#define MAX_LINE_LEN       256

#define SD_DEV_NAME_POS        5
#define SD_DEV_NAME_LEN        16
#define SD_DEV_NAME_IS_INVALID (g_devName[SD_DEV_NAME_POS] == '\0')
#define SD_DEV_NAME_SET_INVALID            \
    {                                      \
        g_devName[SD_DEV_NAME_POS] = '\0'; \
    }

#define MOUNT_OPTION "umask=177"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

pthread_t g_storageDetectThread = 0;
BOOL      g_storageDetect_exit  = TRUE;
int       g_hotplug_sock        = -1;

static char          g_devName[32] = "/dev/";
static char          g_line[MAX_LINE_LEN];
static char          g_nextLine[MAX_LINE_LEN];
static FileSytemType g_eCurrentFS = FS_TYPE_NUM;

extern CarDV_Info carInfo;

extern "C" {
extern int mkfs_fat(const char *device_name, const char *volume, int debug_level, struct timeval *pstTimeOut);
extern int set_volume_fat(const char *device_name, const char *volume, struct timeval *pstTimeOut);
extern int mkfs_exfat(const char *device_name, const char *volume);
extern int mkfs_fat_mbr(const char *device_name);
}

//==============================================================================
//
//                              LOCAL FUNCTIONS
//
//==============================================================================

static char *fReadLine(FILE *stream)
{
    int count = 0;
    while (!feof(stream) && (count < MAX_LINE_LEN) && ((g_line[count++] = getc(stream)) != '\n'))
        ;

    if (!count)
        return NULL;

    g_line[count - 1] = '\0';
    return g_line;
}

static char *fReadNextLine(FILE *stream)
{
    int count = 0;
    while (!feof(stream) && (count < MAX_LINE_LEN) && ((g_nextLine[count++] = getc(stream)) != '\n'))
        ;

    if (!count)
        return NULL;

    g_nextLine[count - 1] = '\0';
    return g_nextLine;
}

static int DetectStorageDev(void)
{
    FILE *pFile      = NULL;
    char *pCurLine   = NULL;
    char *pBlock     = NULL;
    char *pPartition = NULL;

    SD_DEV_NAME_SET_INVALID

    pFile = fopen(PARTTITION_CHECK, "r");
    if (pFile)
    {
        while ((pCurLine = fReadLine(pFile)) != NULL)
        {
            pBlock = strstr(pCurLine, "mmcblk");
            if (pBlock)
            {
                break;
            }
        }

        if (pBlock)
        {
            while ((pCurLine = fReadNextLine(pFile)) != NULL)
            {
                pPartition = strstr(pCurLine, pBlock);
                if (pPartition)
                {
                    memcpy(&g_devName[SD_DEV_NAME_POS], pPartition, SD_DEV_NAME_LEN);
                    break;
                }
            }

            if (!pPartition)
            {
                memcpy(&g_devName[SD_DEV_NAME_POS], pBlock, SD_DEV_NAME_LEN);
            }
        }

        fclose(pFile);
    }

    return 0;
}

static int init_hotplug_sock(void)
{
    struct sockaddr_nl snl;
    const int          buffersize = 16 * 1024;
    struct timeval     timeout    = {1, 0};
    int                retval;
    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family    = AF_NETLINK;
    snl.nl_pid       = getpid();
    snl.nl_groups    = 1;
    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplug_sock == -1)
    {
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }
    /* set receive buffersize */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
    /* set receive timeout */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
    retval = bind(hotplug_sock, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
    if (retval < 0)
    {
        printf("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return -1;
    }
    return hotplug_sock;
}

static const char *mount_filesystem_type(FileSytemType eFsType)
{
    if (eFsType == FS_TYPE_FAT32)
    {
        return "vfat";
    }
#if (CARDV_EXFAT_ENABLE)
    else if (eFsType == FS_TYPE_EXFAT)
    {
        return "exfat";
    }
#endif
    return NULL;
}

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

unsigned long long StorageGetTotalSize(void)
{
    return carInfo.stSdInfo.u64TotalSize;
}

unsigned long long StorageGetOtherTotalSize(void)
{
    return carInfo.stSdInfo.u64OtherSpace;
}

unsigned long long StorageGetReservedTotalSize(void)
{
    return carInfo.stSdInfo.u64ReservedSpace;
}

BOOL StorageIsMounted(void)
{
    return carInfo.stSdInfo.bStorageMount;
}

int StorageSetVolume(const char *volume)
{
    int ret = -1;

    if (SD_DEV_NAME_IS_INVALID)
        return ret;

    ret = StorageUnMount(FALSE);
    if (ret != 0)
        return ret;

    if (g_eCurrentFS == FS_TYPE_FAT32)
    {
        struct timeval stTimeOut;
        stTimeOut.tv_sec  = 30;
        stTimeOut.tv_usec = 0;
        set_volume_fat(g_devName, volume, &stTimeOut);
    }
#if (CARDV_EXFAT_ENABLE)
    else if (g_eCurrentFS == FS_TYPE_EXFAT)
    {
        printf("UnSupport exFAT\n");
    }
#endif

    ret = StorageMount(FALSE);
    return ret;
}

int StorageFormat(FileSytemType eFsType)
{
    int ret   = -1;
    int count = 0;

    if (SD_DEV_NAME_IS_INVALID)
        return ret;

    ret = StorageUnMount(TRUE);
    if (ret != 0)
        return ret;

    if (eFsType == FS_TYPE_FAT32)
    {
        struct timeval stTimeOut;
        stTimeOut.tv_sec  = 30;
        stTimeOut.tv_usec = 0;
        while (count < 3)
        {
            ret = mkfs_fat(g_devName, "SSTAR", 0, &stTimeOut);
            // modify MBR before StorageMount
            mkfs_fat_mbr(g_devName);
            if (ret >= 0)
                break;
            usleep(100);
            count++;
        }
    }
#if (CARDV_EXFAT_ENABLE)
    else if (eFsType == FS_TYPE_EXFAT)
    {
        ret = mkfs_exfat(g_devName, "SSTAR");
    }
#endif

    if (ret < 0)
    {
        printf("format fail\n");
        IPC_MsgToUI_SendMsg(IPC_MSG_UI_FORMAT_SD_FAILED, NULL, 0);
        return ret;
    }

    g_eCurrentFS = eFsType;

    ret = StorageMount(TRUE);
    if (ret < 0)
    {
        printf("format1 fail\n");
        IPC_MsgToUI_SendMsg(IPC_MSG_UI_FORMAT_SD_FAILED, NULL, 0);
        return ret;
    }
    else
    {
        printf("format done\n");
        IPC_MsgToUI_SendMsg(IPC_MSG_UI_FORMAT_SD_OK, NULL, 0);
    }

    return ret;
}

int StorageMount(BOOL bFormat)
{
    int                ret        = 0;
    unsigned long      mountflags = MS_NOATIME | MS_NODIRATIME;
    struct statfs      diskInfo;
    unsigned long long blockSize     = 0;
    unsigned long long availableSize = 0;
    unsigned long long usedSize      = 0;

    if (carInfo.stSdInfo.bStorageMount == TRUE)
        return 0;

    if (SD_DEV_NAME_IS_INVALID)
        return -1;

    ret = access(g_devName, F_OK);
    while (ret != 0 && bFormat == FALSE)
    {
        usleep(10000);
        ret = access(g_devName, F_OK);
    }

    IPC_MsgToUI_SendMsg(IPC_MSG_UI_SD_PRE_MOUNT, NULL, 0);

    for (int i = 0; i < FS_TYPE_NUM; i++)
    {
        if (!bFormat)
            g_eCurrentFS = (FileSytemType)i;

        ret = mount(g_devName, SD_ROOT, mount_filesystem_type(g_eCurrentFS), mountflags, MOUNT_OPTION);
        if (ret == 0 || bFormat)
            break;
    }
    if (ret != 0)
    {
        perror("mount error");
        if (ret == -EPERM)
        {
            MI_S8 audioparam[64];
            audioparam[0] = 1;
            memcpy(&audioparam[1], WAV_PATH_CARD_FORMAT, sizeof(WAV_PATH_CARD_FORMAT));
            cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_CARD_FORMAT) + 1);
            IPC_MsgToUI_SendMsg(IPC_MSG_UI_SD_NEED_FORMAT, NULL, 0);
        }
        return ret;
    }
    else
    {
        printf("mount ok\n");
    }

    ret = statfs(SD_ROOT, &diskInfo);
    if (ret != 0)
    {
        perror("statfs fail");
        goto L_STAT_ERR;
    }
    else
    {
        printf("statfs ok\n");
    }

    blockSize                     = diskInfo.f_bsize;
    carInfo.stSdInfo.u64TotalSize = blockSize * diskInfo.f_blocks;
    if (bFormat)
    {
        ret = DCF_Format(carInfo.stSdInfo.u64TotalSize);
        if (ret < 0)
            goto L_STAT_ERR;

        sync();
    }

    ret = DCF_Mount();
    if (ret < 0)
    {
        MI_S8 audioparam[64];
        audioparam[0] = 1;
        memcpy(&audioparam[1], WAV_PATH_CARD_FORMAT, sizeof(WAV_PATH_CARD_FORMAT));
        cardv_send_cmd(CMD_AUDIO_OUT_PLAY, (MI_S8 *)audioparam, sizeof(WAV_PATH_CARD_FORMAT) + 1);
        IPC_MsgToUI_SendMsg(IPC_MSG_UI_SD_NEED_FORMAT, NULL, 0);
        goto L_STAT_ERR;
    }

    availableSize = blockSize * diskInfo.f_bavail;
    usedSize      = carInfo.stSdInfo.u64TotalSize - availableSize;
    if (usedSize > DCF_GetUsedSize())
        carInfo.stSdInfo.u64OtherSpace = usedSize - DCF_GetUsedSize();
    else
        carInfo.stSdInfo.u64OtherSpace = 0;

    carInfo.stSdInfo.u64ReservedSpace = carInfo.stSdInfo.u64TotalSize - carInfo.stSdInfo.u64OtherSpace;
    carInfo.stSdInfo.u64FreeSize      = carInfo.stSdInfo.u64ReservedSpace - DCF_GetUsedSize();
    printf("total size [%llu]\n", carInfo.stSdInfo.u64TotalSize);
    printf("other used size [%llu]\n", carInfo.stSdInfo.u64OtherSpace);
    printf("reserved for DCF size [%llu]\n", carInfo.stSdInfo.u64ReservedSpace);

    carInfo.stSdInfo.bStorageMount = TRUE;
    IPC_CarInfo_Write_SdInfo(&carInfo.stSdInfo);
    carInfo.stCapInfo.u32FileCnt = DCF_GetDBFileCntByDB(DB_PHOTO);
    IPC_CarInfo_Write_CapInfo(&carInfo.stCapInfo);
    cardv_update_status(MMC_STATUS, "1", 2);
    if (!bFormat)
        IPC_MsgToUI_SendMsg(IPC_MSG_UI_SD_MOUNT, NULL, 0);

    return ret;

L_STAT_ERR:
    ret = umount2(SD_ROOT, MNT_DETACH);
    if (ret != 0)
    {
        perror("unmount error");
        return ret;
    }
    else
    {
        printf("unmount ok\n");
        return -1;
    }
}

int StorageUnMount(BOOL bFormat)
{
    int ret = 0;

    if (carInfo.stSdInfo.bStorageMount == FALSE)
        return 0;

    ret = umount2(SD_ROOT, MNT_DETACH);
    if (ret != 0)
    {
        perror("unmount error");
        return ret;
    }
    else
    {
        printf("unmount ok\n");
    }

    carInfo.stSdInfo.bStorageMount = FALSE;
    cardv_update_status(MMC_STATUS, "0", 2);
    IPC_CarInfo_Write_SdInfo(&carInfo.stSdInfo);
    if (!bFormat)
        IPC_MsgToUI_SendMsg(IPC_MSG_UI_SD_UNMOUNT, NULL, 0);

    DCF_UnMount();

    return ret;
}

int StorageReMount(void)
{
    int           ret        = -1;
    unsigned long mountflags = MS_NOATIME | MS_NODIRATIME | MS_REMOUNT;

    if (SD_DEV_NAME_IS_INVALID)
        return ret;

    if (g_eCurrentFS == FS_TYPE_NUM)
        return ret;

    ret = mount(g_devName, SD_ROOT, mount_filesystem_type(g_eCurrentFS), mountflags, MOUNT_OPTION);
    if (ret != 0)
    {
        perror("remount error");
    }
    else
    {
        printf("remount ok\n");
    }
    return ret;
}

void *StorageDetectTask(void *argv)
{
    int ret        = 0;
    g_hotplug_sock = init_hotplug_sock();

    CARDV_THREAD();

    if (g_hotplug_sock == -1)
    {
        pthread_exit(NULL);
    }

    DetectStorageDev();
    if (!SD_DEV_NAME_IS_INVALID)
    {
        ret = access(g_devName, F_OK);
        if (ret == 0)
        {
            printf("[%s] Exist\n", g_devName);
            carInfo.stSdInfo.bStorageInsert = TRUE;
            IPC_CarInfo_Write_SdInfo(&carInfo.stSdInfo);
            ret = StorageMount(FALSE);
            if (ret == 0)
            {
                IPC_CarInfo_Read_UIModeInfo(&carInfo.stUIModeInfo);
                if (carInfo.stUIModeInfo.u8Mode == UI_VIDEO_MODE)
                    cardv_send_to_fifo(CARDV_CMD_START_REC, sizeof(CARDV_CMD_START_REC));
            }
        }
    }

    while (g_storageDetect_exit == FALSE)
    {
        char buf[UEVENT_BUFFER_SIZE] = {
            0,
        };
        char action[16] = {
            0,
        };

        ret = recv(g_hotplug_sock, &buf, sizeof(buf), 0);

        if (ret == -1)
            continue;

        sscanf(buf, "%[^'@']", action);

        if (!StorageIsMounted() && strncmp(action, "add", sizeof(action)) == 0)
        {
            DetectStorageDev();
            if (!SD_DEV_NAME_IS_INVALID)
            {
                if (strstr(buf, &g_devName[SD_DEV_NAME_POS]))
                {
                    carInfo.stSdInfo.bStorageInsert = TRUE;
                    IPC_CarInfo_Write_SdInfo(&carInfo.stSdInfo);
                    ret = StorageMount(FALSE);
                    if (ret == 0)
                    {
                        IPC_CarInfo_Read_UIModeInfo(&carInfo.stUIModeInfo);
                        if (carInfo.stUIModeInfo.u8Mode == UI_VIDEO_MODE)
                            cardv_send_to_fifo(CARDV_CMD_START_REC, sizeof(CARDV_CMD_START_REC));
                    }
                }
            }
        }
        else if (StorageIsMounted() && strncmp(action, "remove", sizeof(action)) == 0)
        {
            if (!SD_DEV_NAME_IS_INVALID)
            {
                if (strstr(buf, &g_devName[SD_DEV_NAME_POS]))
                {
                    cardv_send_to_fifo(CARDV_CMD_STOP_REC, sizeof(CARDV_CMD_STOP_REC));
                    carInfo.stSdInfo.bStorageInsert = FALSE;
                    IPC_CarInfo_Write_SdInfo(&carInfo.stSdInfo);
                    StorageUnMount(FALSE);
                    SD_DEV_NAME_SET_INVALID
                }
            }
        }
    }

    close(g_hotplug_sock);
    StorageUnMount(FALSE);
    printf("storage detect thread exit\n");
    pthread_exit(NULL);
}

void StorageStartDetectThread(void)
{
    printf("start storage detect thread\n");

    if (TRUE != g_storageDetect_exit)
    {
        CARDV_ERR("%s alread start\n", __func__);
        return;
    }

    g_storageDetect_exit = FALSE;
    int ret              = 0;

    ret = pthread_create(&g_storageDetectThread, NULL, StorageDetectTask, NULL);
    if (0 == ret)
    {
        pthread_setname_np(g_storageDetectThread, "storage_task");
    }
    else
    {
        CARDV_ERR("%s pthread_create failed\n", __func__);
    }
}

void StorageStopDetectThread(void)
{
    g_storageDetect_exit = TRUE;
    pthread_join(g_storageDetectThread, NULL);
    return;
}
