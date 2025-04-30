/*
 *	cgi-bin/dir- Sigmastar
 *
 * Copyright (C) 2019 Sigmastar Technology Corp.
 *
 * Author: XXXX <XXXX@sigmastar.com.cn>
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
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <sys/file.h>
#include <unistd.h>
#include "DCF.h"

void         OutputXMLformat(char* name, char* format, unsigned int size, char* attr, char* time);
unsigned int fsize(char* filename);
char*        getFileCreationTime(char* filePath);
char*        getFileAttribute(char* filePath);
char*        GetFileType(char* pathname);
void         GetFileInfo(char* pathname, char* fileType);
void         DirOpen(char* format, char* property, unsigned int count, unsigned int from);
unsigned int amount = 0;

// int main()
int directory(char* myaction, char* myproperty, char* myformat, char* mycount, char* myfrom)
{
    if (strcmp(myaction, "dir") == 0)
    {
        char FullPath[50] = {0};
        printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
        printf("<%s>\n", myproperty);
        sprintf(FullPath, "/mnt/mmc/%s/%s", myproperty, CAM0_FOLDER);
        DirOpen(myformat, FullPath, atoi(mycount), atoi(myfrom));
        printf("  <amount>%d</amount>\n", amount);
        printf("</%s>\n", myproperty);
        fprintf(stderr, "%s action: %s\n\r", __FUNCTION__, myaction);
        fprintf(stderr, "%s fullproperty: %s\n\r", __FUNCTION__, FullPath);
        fprintf(stderr, "%s format: %s\n\r", __FUNCTION__, myformat);
        fprintf(stderr, "%s count: %s\n\r", __FUNCTION__, mycount);
        fprintf(stderr, "%s from: %s\n\r", __FUNCTION__, myfrom);
        fprintf(stderr, "%s: amount=%d\n\r", __FUNCTION__, amount);
    }
    else if (strcmp(myaction, "reardir") == 0)
    {
        char FullPath[50] = {0};
        printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
        printf("<%s>\n", myproperty);
        sprintf(FullPath, "/mnt/mmc/%s/%s", myproperty, CAM1_FOLDER);
        DirOpen(myformat, FullPath, atoi(mycount), atoi(myfrom));
        printf("  <amount>%d</amount>\n", amount);
        printf("</%s>\n", myproperty);
        fprintf(stderr, "%s action: %s\n\r", __FUNCTION__, myaction);
        fprintf(stderr, "%s fullproperty: %s\n\r", __FUNCTION__, FullPath);
        fprintf(stderr, "%s format: %s\n\r", __FUNCTION__, myformat);
        fprintf(stderr, "%s count: %s\n\r", __FUNCTION__, mycount);
        fprintf(stderr, "%s from: %s\n\r", __FUNCTION__, myfrom);
        fprintf(stderr, "%s: amount=%d\n\r", __FUNCTION__, amount);
    }
    return 0;
}

void OutputXMLformat(char* name, char* format, unsigned int size, char* attr, char* time)
{
    printf("   <file>\n");
    printf("      <name>%s</name>\n", name);
    printf("      <format>%s</format>\n", format);
    printf("      <size>%u</size>\n", size);
    printf("      <attr>%s</attr>\n", attr);
    printf("      <time>%s</time>\n", time);
    printf("   </file>\n");
    ++amount;
}

unsigned int fsize(char* filename)
{
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

char* getFileCreationTime(char* filePath)
{
    struct stat attrib;
    stat(filePath, &attrib);
    char* date = malloc(30);
    strftime(date, 30, "20%y-%m-%d %H:%M:%S", localtime(&(attrib.st_mtime)));
    return date;
}

char* getFileAttribute(char* filePath)
{
    struct stat  fileStat;
    unsigned int len  = 0;
    char*        attr = malloc(3);
    stat(filePath, &fileStat);
    len += snprintf(attr, 3, "%s", ((fileStat.st_mode & S_IRUSR) ? "R" : ""));
    len += snprintf(attr + len, 3 - len, "%s", ((fileStat.st_mode & S_IWUSR) ? "W" : ""));
    return attr;
}

char* GetFileType(char* pathname)
{
    char* delim = ".";
    char* pch   = NULL;
    pch         = strtok(pathname, delim);
    while (pch != NULL)
    {
        pch = strtok(NULL, delim);
        if (strcasecmp(pch, "JPG") == 0)
            strcpy(pch, "jpeg");
        return pch;
    }
    return pch;
}

int64_t GetFileDuration(const char* src_filename)
{
    char cmd[128];
    char dst_filename[64] = {
        0,
    };
    int64_t duration = 0;
    int     fd       = 0;
    int     ret      = 0;
    int     count    = 200;

    sprintf(cmd, "echo tt %s > /tmp/cardv_fifo", src_filename);
    system(cmd);
    sprintf(dst_filename, "%s%s", "/tmp", strrchr(src_filename, '/'));
    if (strtok(dst_filename, "."))
    {
        strcat(dst_filename, ".tt");
        ret = access(dst_filename, F_OK);
        while (ret != 0 && count-- > 0)
        {
            usleep(10000);
            ret = access(dst_filename, F_OK);
        }

        fd = open(dst_filename, O_RDONLY);
        if (fd >= 0)
        {
            read(fd, &duration, sizeof(duration));
            close(fd);
            remove(dst_filename);
            return duration;
        }
    }

    return 0;
}

void GetFileInfo(char* pathname, char* fileType)
{
    unsigned int fileSize;
    char*        attr;
    char*        time;
    fileSize = fsize(pathname);
    time     = getFileCreationTime(pathname);
    attr     = getFileAttribute(pathname);
    OutputXMLformat(pathname, fileType, fileSize, attr, time);
    free(time);
    free(attr);
}

char* GetRecordingFile(char* pathname)
{
    char        cmd[64];
    static char RecordingFile[64];
    memset(cmd, 0, sizeof(cmd));
    memset(RecordingFile, 0, sizeof(RecordingFile));

    sprintf(cmd, "cat %s", pathname);
    fprintf(stderr, "%s: cmd=%s (popen())\n\r", __FUNCTION__, cmd);
    FILE* fp = popen(cmd, "r");
    if (!fp)
    {
        fprintf(stderr, "%s: popen, errno(%d), %s\n\r", __FUNCTION__, errno, strerror(errno));
        return NULL;
    }
    else
    {
        while (fgets(RecordingFile, sizeof(RecordingFile), fp) != NULL)
        {
            fprintf(stderr, "%s: %s\n\r", __FUNCTION__, RecordingFile);
        }
        pclose(fp);
    }
    return RecordingFile;
}

int SkipRecordingFile(char* RecordingFile, char* filename, char* fileType)
{
    char FILENAME[64];
    memset(FILENAME, 0, sizeof(FILENAME));

    if (strcasecmp(fileType, "MOV") == 0 || strcasecmp(fileType, "TS") == 0)
    {
        strcpy(FILENAME, filename);
        strcat(FILENAME, ".");
        strcat(FILENAME, fileType);
        strcat(FILENAME, "\n");

        // fprintf(stderr,"%s:  %s\n\r",   __FUNCTION__, RecordingFile);
        // fprintf(stderr,"%s:  %s\n\r",   __FUNCTION__, FILENAME);

        if (strcmp(RecordingFile, FILENAME) == 0)
        {
            fprintf(stderr, "%s: Ignore Output XML\n\r", __FUNCTION__);
            fprintf(stderr, "%s: %s\n\r", __FUNCTION__, FILENAME);
            return 1;
        }
    }
    return 0;
}

void DirOpen(char* format, char* property, unsigned int count, unsigned int from)
{
    // int len;
    // struct dirent *pDirent;
    // DIR *pDir;
    char* fileType;
    char* filename;
    char  pathname[100];
    // struct stat st;
    unsigned int id = 0;
    // char* RecordingFile=GetRecordingFile("/tmp/rec_filename");

    struct dirent** entry_list;
    int             fcount = 0;
    int             i      = 0;

    fcount = scandir(property, &entry_list, 0, alphasort);
    if (fcount < 0)
    {
        fprintf(stderr, "Cannot open directory '%s'\n", property);
        return;
    }
    i = fcount;
    while (i > 0)
    {
        struct dirent* entry;
        entry    = entry_list[i - 1];
        filename = entry->d_name;
        if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0 || strstr(filename, FORMATFREE_PREFIX) != NULL)
        {
            i--;
            continue;
        }
        sprintf(pathname, "%s/%s", property, filename);
        fileType = GetFileType(filename);
        // fprintf(stderr,"pathname[%d]: '%s'\n", i,pathname);
        /*skip the recording file of mp4 in the filelists*/
        // if(SkipRecordingFile(RecordingFile,filename,fileType))
        //    continue;
        if (strcmp(format, "all") == 0)
        {
            if (id >= from && id < (count + from))
            {
                GetFileInfo(pathname, fileType);
            }
            id++;
        }
        else if (strcasecmp(format, fileType) == 0)
        {
            if (id >= from && id < (count + from))
            {
                GetFileInfo(pathname, fileType);
            }
            id++;
        }

        free(entry);
        i--;
    }
    free(entry_list);

    return;
}
