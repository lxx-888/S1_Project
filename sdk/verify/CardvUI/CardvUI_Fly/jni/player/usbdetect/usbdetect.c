/*
 * usb.c
 *
 *  Created on: 2019年8月26日
 *      Author: koda.xu
 */

#ifdef SUPPORT_PLAYER_MODULE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/bitypes.h>
#include "usbdetect.h"


#define USB_PARTTITION_CHECK                "/proc/partitions"
#define USB_MOUNT_CHECK                     "/proc/mounts"
#define USB_FAT32_FORMAT                    "vfat"
#define USB_NTFS_FORMAT                     "ntfs"
#define USB_MOUNT_DEFAULT_DIR               "/tmp/udisk"

#define MAX_LINE_LEN        512
#define MAX_DEV_NAMELEN     64
#define MAX_MOUNT_NAMELEN     256

static char g_line[MAX_LINE_LEN];
static char g_devName[MAX_DEV_NAMELEN];
static char g_mountName[MAX_MOUNT_NAMELEN];

static char *freadline(FILE *stream)
{
    int count = 0;

    while (!feof(stream) && (count < MAX_LINE_LEN) && ((g_line[count++] = getc(stream)) != '\n'));
    if (!count)
        return NULL;

    g_line[count - 1] = '\0';

    return g_line;
}

static int DetectUsbDev(void)
{
    FILE *pFile = fopen(USB_PARTTITION_CHECK, "r");
    char *pCurLine = NULL;
    char *pSeek = NULL;

    memset(g_devName, 0, sizeof(g_devName));

    if (pFile)
    {
        while((pCurLine = freadline(pFile)) != NULL)
        {
            pSeek = strstr(pCurLine, "mmcblk0p1");//mmcblk0p1
            if (pSeek)
            {
                printf("Fread line %s:%X,%X\n",pSeek,pSeek[2],pSeek[3]);
                if ((pSeek[7] >= 'a' && pSeek[7] <= 'z') && (pSeek[8] >= '1' && pSeek[8] <= '9'))
                {
                    memcpy(g_devName, pSeek, 9);
                    fclose(pFile);
                    pFile = NULL;
                    printf("g_devName:%s\n",g_devName);
                    return 0;
                }
            }
        }

        fclose(pFile);
        pFile = NULL;
    }

    printf("Can't find usb device\n");
    return -1;
}

static int CheckUsbDevMountStatus()
{
    FILE *pFile = fopen(USB_MOUNT_CHECK, "r");
    char *pCurLine = NULL;
    char *pSeek = NULL;

    memset(g_mountName, 0, sizeof(g_mountName));

    if (pFile)
    {
        while((pCurLine = freadline(pFile)) != NULL)
        {
            pSeek = strstr(pCurLine, g_devName);
            if (pSeek)
            {
                char *pMount = NULL;
                pSeek += strlen(g_devName);
                while(*(pSeek) == ' ')
                    pSeek++;

                if (pSeek)
                {
                    pMount = pSeek;
                    while (*pSeek != ' ')
                    {
                        pSeek++;
                    }

                    memcpy(g_mountName, pMount, (pSeek - pMount));
                    printf("/dev/%s has been mounted on %s\n", g_devName, g_mountName);
                    fclose(pFile);
                    pFile = NULL;

                    return 0;
                }
            }
        }

        fclose(pFile);
        pFile = NULL;
    }
    else
    {
        printf("open %s failed\n", USB_MOUNT_CHECK);
    }

    return -1;
}

static int AutoMountUsbDev()
{
    if (CheckUsbDevMountStatus())
    {
        char dirCmd[64] = {0};
        char mountCmd[128] = {0};
        int nRet = 0;

        sprintf(dirCmd, "mkdir -p %s", USB_MOUNT_DEFAULT_DIR);
        nRet = system(dirCmd);
        if (nRet)
        {
            printf("%s execute failed\n", dirCmd);
            return -1;
        }

        sprintf(mountCmd, "mount -t %s /dev/%s %s", USB_FAT32_FORMAT, g_devName, USB_MOUNT_DEFAULT_DIR);
        nRet = system(mountCmd);
        if (!nRet)
        {
            printf("mount /dev/%s on %s success, usb format is FAT32.\n", g_devName, USB_MOUNT_DEFAULT_DIR);
            memset(g_mountName, 0, sizeof(g_mountName));
            memcpy(g_mountName, USB_MOUNT_DEFAULT_DIR, strlen(USB_MOUNT_DEFAULT_DIR));
            return 0;
        }

        memset(mountCmd, 0, sizeof(mountCmd));
        sprintf(mountCmd, "mount -t %s /dev/%s %s", USB_NTFS_FORMAT, g_devName, USB_MOUNT_DEFAULT_DIR);
        nRet = system(mountCmd);
        if (!nRet)
        {
            printf("mount /dev/%s on %s success, usb format is NTFS.\n", g_devName, USB_MOUNT_DEFAULT_DIR);
            memset(g_mountName, 0, sizeof(g_mountName));
            memcpy(g_mountName, USB_MOUNT_DEFAULT_DIR, strlen(USB_MOUNT_DEFAULT_DIR));
            return 0;
        }

        printf("mount /dev/%s on %s failed\n", g_devName, USB_MOUNT_DEFAULT_DIR);
        return -1;
    }

    return 0;
}

static int AutoUmountUsbDev()
{
    if (!CheckUsbDevMountStatus())
    {
        char umountCmd[128] = {0};
        int nRet = 0;

        sprintf(umountCmd, "umount /dev/%s", g_devName);
        nRet = system(umountCmd);
        if (nRet)
        {
            printf("umount /dev/%s failed\n", g_devName);
            return -1;
        }
    }

    printf("umount /dev/%s success\n", g_devName);
    return 0;
}

int SSTAR_InitUsbDev(char *pDirName, int nLen)
{
    int s32Ret = -1;
    s32Ret = DetectUsbDev();
    if (s32Ret)
        goto exit;

    s32Ret = AutoMountUsbDev();
    if (s32Ret)
        goto exit;

    strncpy(pDirName, g_mountName, nLen);

exit:
    return s32Ret;
}

int SSTAR_DeinitUsbDev()
{
    return AutoUmountUsbDev();
}
#endif
