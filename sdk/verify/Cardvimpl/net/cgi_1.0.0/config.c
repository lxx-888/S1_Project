/*
 *	/cgi-bin/Config.cgi?- Sigmastar
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include "nvconf.h"
#include "cgi_util.h"

CMDNODE *cmdhead = NULL, *cmdcurrent = NULL;

extern int     directory(char* myaction, char* myproperty, char* myformat, char* mycount, char* myfrom);
extern int64_t GetFileDuration(const char* pathname);

void add_cmd_list(char* strcgikey, char* strcginame, int cgitype, Callback cb, CMDNODE* cur)
{
    cmdcurrent = addCmdNode(strcgikey, strcginame, cgitype, cb, cur);
    if (cmdhead == NULL)
        cmdhead = cmdcurrent;
}

typedef struct _CGI_PARAM
{
    char action[64];
    char property[64];
    char value[64];
    char format[64];
    char count[64];
    char from[64];
    char flag[64];      // addition
    char key_value[64]; // addition
} CGI_PARAM, *P_CGI_PARAM;

#define CgiMax 4
CGI_PARAM CgiParamA[CgiMax];
int       CgiNum = 0;

void cb_action(char* result)
{
    int i = 0;
    for (i = 0; i < CgiMax; i++)
        strcpy(CgiParamA[i].action, result);
}

void cb_property(char* result)
{
    static int i = 0;
    strcpy(CgiParamA[i++].property, result);
    CgiNum = i;
}

void cb_value(char* result)
{
    static int i = 0;
    strcpy(CgiParamA[i++].value, result);
}

void cb_format(char* result)
{
    static int i = 0;
    strcpy(CgiParamA[i++].format, result);
}

void cb_count(char* result)
{
    static int i = 0;
    strcpy(CgiParamA[i++].count, result);
}

void cb_from(char* result)
{
    static int i = 0;
    strcpy(CgiParamA[i++].from, result);
}

void Cgi_CheckValue(int i)
{
    char cmd[64] = {0};
    int  cnt = 0, k = 0, j = 0;
    memcpy(cmd, CgiParamA[i].value, sizeof(CgiParamA[i].value));

    for (cnt = 0; k < 64; cnt++)
    {
        if (cmd[j] == 0 && ((j > 2 && k < 63) && (cmd[j - 1] != '$')) && (cmd[j - 2] != '$'))
        {
            // fill tail, fill 2bytes
            CgiParamA[i].value[k++] = '$';
            CgiParamA[i].value[k++] = '$';
            break;
        }
        else if (cmd[j] == 0 && ((j > 2 && k < 63) && (cmd[j - 1] == '$')) && (cmd[j - 2] != '$'))
        {
            // fill tail, fill 1byte
            CgiParamA[i].value[k++] = '$';
            break;
        }
        else if (cmd[j] == 0)
        {
            break;
        }

        CgiParamA[i].value[k++] = cmd[j];
        if ((cmd[j] == '$') && (cmd[j + 1] != '$') && ((j > 1 && k < 64) && (cmd[j - 1] != '$')))
        {
            // 0x24='$'
            CgiParamA[i].value[k++] = cmd[j];
        }
        j++;
    }
}

int ExeCgiCmd()
{
    int i = 0;
#if 0
    for (i = 0; i < CgiNum; i++)
    {
        fprintf(stderr, "%s: CgiParamA[%d].action=%s\n\r",   __FUNCTION__, i, CgiParamA[i].action);
        fprintf(stderr, "%s: CgiParamA[%d].property=%s\n\r", __FUNCTION__, i, CgiParamA[i].property);
        fprintf(stderr, "%s: CgiParamA[%d].value=%s\n\r",    __FUNCTION__, i, CgiParamA[i].value);
        fprintf(stderr, "%s: CgiParamA[%d].format=%s\n\r",   __FUNCTION__, i, CgiParamA[i].format);
        fprintf(stderr, "%s: CgiParamA[%d].count=%s\n\r",    __FUNCTION__, i, CgiParamA[i].count);
        fprintf(stderr, "%s: CgiParamA[%d].from=%s\n\r\n\r", __FUNCTION__, i, CgiParamA[i].from);
    }
#endif

    for (i = 0; i < CgiNum; i++)
    {
        if (((strcmp(CgiParamA[i].property, "AWB") == 0) || (strcmp(CgiParamA[i].property, "EV") == 0)
             || (strcmp(CgiParamA[i].property, "Exposure") == 0) || (strcmp(CgiParamA[i].property, "Flicker") == 0)
             || (strcmp(CgiParamA[i].property, "ImageRes") == 0) || (strcmp(CgiParamA[i].property, "Imageres") == 0)
             || (strcmp(CgiParamA[i].property, "MTD") == 0) || (strcmp(CgiParamA[i].property, "VideoRes") == 0)
             || (strcmp(CgiParamA[i].property, "Videores") == 0)
             || (strcmp(CgiParamA[i].property, "VideoClipTime") == 0)
             || (strcmp(CgiParamA[i].property, "LoopingVideo") == 0)
             || (strcmp(CgiParamA[i].property, "SoundRecord") == 0)
             || (strcmp(CgiParamA[i].property, "MovieAudio") == 0) || (strcmp(CgiParamA[i].property, "RecStamp") == 0)
             || (strcmp(CgiParamA[i].property, "SpeedUint") == 0)
             || (strcmp(CgiParamA[i].property, "SpeedCamAlert") == 0)
             || (strcmp(CgiParamA[i].property, "SpeedLimitAlert") == 0)
             || (strcmp(CgiParamA[i].property, "TimeZone") == 0) || (strcmp(CgiParamA[i].property, "SyncTime") == 0)
             || (strcmp(CgiParamA[i].property, "PosSetting_Add") == 0)
             || (strcmp(CgiParamA[i].property, "PosSetting_DelLast") == 0)
             || (strcmp(CgiParamA[i].property, "PosSetting_DelAll") == 0)
             || (strcmp(CgiParamA[i].property, "FactoryReset") == 0)
             || (strcmp(CgiParamA[i].property, "ParkingMonitor") == 0)
             || (strcmp(CgiParamA[i].property, "VoiceSwitch") == 0) || (strcmp(CgiParamA[i].property, "GSensor") == 0)
             || (strcmp(CgiParamA[i].property, "Camera.Preview.*") == 0)
             || (strcmp(CgiParamA[i].property, "Camera.Menu.*") == 0)
             || (strcmp(CgiParamA[i].property, "Camera.Menu.CardInfo.*") == 0)
             || (strcmp(CgiParamA[i].property, "Camera.Menu.SDInfo") == 0) // Camera.Menu.CardInfo.*
             )
            || (strcmp(CgiParamA[i].property, "streamer") == 0) || (strcmp(CgiParamA[i].property, "forceiframe") == 0)
            || (strcmp(CgiParamA[i].property, "setbitrate") == 0)
            || (strcmp(CgiParamA[i].property, "setframerate") == 0)
            || (strcmp(CgiParamA[i].property, "setvideogop") == 0)
            || (strcmp(CgiParamA[i].property, "reset_to_default") == 0)
            || (strcmp(CgiParamA[i].property, "reboot") == 0) || (strcmp(CgiParamA[i].property, "streamerstatus") == 0)
            || (strcmp(CgiParamA[i].property, "file_list") == 0)
            || (strcmp(CgiParamA[i].property, "devinfo.fwver") == 0)
            || (strcmp(CgiParamA[i].property, "devinfo.macaddr") == 0)
            || (strcmp(CgiParamA[i].property, "devinfo.linuxkernelver") == 0) || // IPCAM specified above
            (strcmp(CgiParamA[i].property, "thumb.jpeg") == 0) || (strcmp(CgiParamA[i].property, "duration") == 0)
            || (strcmp(CgiParamA[i].property, "Camera.System.Power") == 0) || // DoorBell cam specified above
            (strcmp(CgiParamA[i].property, "Camera.Preview.MJPEG.status.*") == 0)
            || (strcmp(CgiParamA[i].property, "Camera.Preview.MJPEG.TimeStamp") == 0)
            || (strcmp(CgiParamA[i].property, "Camera.Preview.MJPEG.TimeStamp.*") == 0)
            || (strcmp(CgiParamA[i].property, "Net.WIFI_AP.SSID") == 0)
            || (strcmp(CgiParamA[i].property, "Net.WIFI_AP.CryptoKey") == 0)
            || (strcmp(CgiParamA[i].property, "Net.WIFI_STA.AP.2.SSID") == 0)
            || (strcmp(CgiParamA[i].property, "Net.WIFI_STA.AP.2.CryptoKey") == 0)
            || (strcmp(CgiParamA[i].property, "Video") == 0) || (strncmp(CgiParamA[i].property, "$mnt$mmc$", 9) == 0)
            || (strcmp(CgiParamA[i].property, "TimeSettings") == 0) || (strcmp(CgiParamA[i].property, "SD0") == 0)
            || (strcmp(CgiParamA[i].property, "Camera.Menu.RearStarus") == 0)
            || (strcmp(CgiParamA[i].property, "Net") == 0) || // Car cam specified above
            (strcmp(CgiParamA[i].property, "Setting") == 0))
        {
            char cmd[256];
#if 0
            strcpy(CgiParamA[i].flag, "reserve function");
            fprintf(stderr, "%s: CgiParamA[%d].flag=%s\n\r",   __FUNCTION__, i, CgiParamA[i].flag);
#endif
            if (strcmp(CgiParamA[i].property, "TimeSettings") == 0)
                Cgi_CheckValue(i);

            if (strcmp(CgiParamA[i].action, "set") == 0)
            {
                sprintf(cmd, "./CGI_PROCESS.sh %s %s %s 1>&2", CgiParamA[i].action, CgiParamA[i].property,
                        CgiParamA[i].value);

                int status = system(cmd);
                if ((status >= 0) && (WIFEXITED(status)) && (WEXITSTATUS(status) == 0))
                {
                    fprintf(stdout, "0\n");
                    fprintf(stdout, "OK\n");
                    fprintf(stderr, "0\n");
                    fprintf(stderr, "OK\n");
                }
                else
                {
                    fprintf(stderr, "%s: cmd=%s (error: %s)\n\r", __FUNCTION__, cmd, strerror(errno));
                    fprintf(stdout, "709\n");
                    fprintf(stdout, "???\n");
                    fprintf(stderr, "709\n");
                    fprintf(stderr, "???\n");
                }

                if ((strcmp(CgiParamA[i].property, "AWB") == 0) || (strcmp(CgiParamA[i].property, "EV") == 0)
                    || (strcmp(CgiParamA[i].property, "Exposure") == 0)
                    || (strcmp(CgiParamA[i].property, "Flicker") == 0)
                    || (strcmp(CgiParamA[i].property, "ImageRes") == 0)
                    || (strcmp(CgiParamA[i].property, "Imageres") == 0) || (strcmp(CgiParamA[i].property, "MTD") == 0)
                    || (strcmp(CgiParamA[i].property, "VideoRes") == 0)
                    || (strcmp(CgiParamA[i].property, "Videores") == 0)
                    || (strcmp(CgiParamA[i].property, "VideoClipTime") == 0)
                    || (strcmp(CgiParamA[i].property, "LoopingVideo") == 0)
                    || (strcmp(CgiParamA[i].property, "SoundRecord") == 0)
                    || (strcmp(CgiParamA[i].property, "RecStamp") == 0)
                    || (strcmp(CgiParamA[i].property, "SpeedUint") == 0)
                    || (strcmp(CgiParamA[i].property, "SpeedCamAlert") == 0)
                    || (strcmp(CgiParamA[i].property, "SpeedLimitAlert") == 0)
                    || (strcmp(CgiParamA[i].property, "TimeZone") == 0)
                    || (strcmp(CgiParamA[i].property, "SyncTime") == 0)
                    || (strcmp(CgiParamA[i].property, "GSensor") == 0) || // GSensor
                    (strcmp(CgiParamA[i].property, "PosSetting_Add") == 0)
                    || (strcmp(CgiParamA[i].property, "PosSetting_DelLast") == 0)
                    || (strcmp(CgiParamA[i].property, "PosSetting_DelAll") == 0) ||
                    //(strcmp(CgiParamA[i].property, "FactoryReset")==0)||
                    (strcmp(CgiParamA[i].property, "VoiceSwitch") == 0)
                    || (strcmp(CgiParamA[i].property, "ParkingMonitor") == 0))
                {
                    if (strcmp(CgiParamA[i].property, "Exposure") == 0)
                    {
                        strcpy(CgiParamA[i].property, "EV");
                    }
                    if (strcmp(CgiParamA[i].property, "Imageres") == 0)
                    {
                        strcpy(CgiParamA[i].property, "ImageRes");
                    }
                    if (strcmp(CgiParamA[i].property, "Videores") == 0)
                    {
                        strcpy(CgiParamA[i].property, "VideoRes");
                    }

                    strcpy(CgiParamA[i].key_value, "Camera.Menu.");
                    strcat(CgiParamA[i].key_value, CgiParamA[i].property);
                    strcat(CgiParamA[i].key_value, "=");
                    strcat(CgiParamA[i].key_value, CgiParamA[i].value);
                    fprintf(stderr, "%s: key_value[%d]= %s\n\r", __FUNCTION__, i, CgiParamA[i].key_value);
                    SetProcess(CgiParamA[i].key_value);
                }

                if (strcmp(CgiParamA[i].property, "MovieAudio") == 0)
                {
                    strcpy(CgiParamA[i].key_value, "Camera.Preview.MJPEG.status.");
                    strcat(CgiParamA[i].key_value, CgiParamA[i].property);
                    strcat(CgiParamA[i].key_value, "=");
                    strcat(CgiParamA[i].key_value, CgiParamA[i].value);
                    fprintf(stderr, "%s: key_value[%d]= %s\n\r", __FUNCTION__, i, CgiParamA[i].key_value);
                    SetProcess(CgiParamA[i].key_value);
                }
            }
            else if (strcmp(CgiParamA[i].action, "get") == 0)
            {
                char    tmpbuf[200] = {0};
                int64_t duration    = 0;
                sprintf(cmd, "./CGI_PROCESS.sh %s %s", CgiParamA[i].action, CgiParamA[i].property);
                // fprintf(stderr, "%s: cmd=%s (popen())\n\r",  __FUNCTION__,cmd);

                if (strcmp(CgiParamA[i].property, "duration") == 0)
                {
                    sprintf(tmpbuf, "%s", CgiParamA[i].value);
                    duration = GetFileDuration(tmpbuf);
                    fprintf(stdout, "%llu\n", duration);
                }
                else
                {
                    FILE* fp = popen(cmd, "r");
                    if (!fp)
                    {
                        fprintf(stderr, "%s: popen, errno(%d), %s\n\r", __FUNCTION__, errno, strerror(errno));
                        return -1;
                    }

                    fprintf(stdout, "0\n");
                    fprintf(stdout, "OK\n");
                    fprintf(stderr, "0\n");
                    fprintf(stderr, "OK\n");

                    while (fgets(tmpbuf, sizeof(tmpbuf), fp) != NULL)
                    {
                        fprintf(stdout, "%s", tmpbuf);
                        fprintf(stderr, "%s", tmpbuf);
                    }

                    pclose(fp);
                }
                GetProcess(CgiParamA[i].property);
            }
            else if (strcmp(CgiParamA[i].action, "del") == 0)
            {
                if (strncmp(CgiParamA[i].property, "$mnt$mmc$", 9) == 0)
                {
                    char* substitute;
                    while (1)
                    {
                        substitute = strchr(CgiParamA[i].property, '$');
                        if (substitute != NULL)
                        {
                            fprintf(stderr, "%s: substitute=%c\n\r", __FUNCTION__, *substitute);
                            *substitute = '/';
                            fprintf(stderr, "%s: substitute=%c\n\r", __FUNCTION__, *substitute);
                        }
                        else
                        {
                            fprintf(stderr, "%s: break\n\r", __FUNCTION__);
                            break;
                        }
                    }
                }

                sprintf(cmd, "./CGI_PROCESS.sh %s %s 1>&2", CgiParamA[i].action, CgiParamA[i].property);
                system(cmd);

                fprintf(stdout, "0\n");
                fprintf(stdout, "OK\n");
                fprintf(stderr, "0\n");
                fprintf(stderr, "OK\n");
            }

            if (strcmp(CgiParamA[i].action, "set") == 0 && strcmp(CgiParamA[i].property, "65") == 0)
            {
                struct timespec gtime;
                if (clock_gettime(CLOCK_REALTIME, &gtime) == -1)
                {
                    fprintf(stderr, "%s: clock_gettime error\n\r", __FUNCTION__);
                }
                else
                {
                    fprintf(stderr, "%s: gtime.tv_sec=%ld\n\r", __FUNCTION__, gtime.tv_sec);
                    fprintf(stderr, "%s: gtime.tv_nsec=%ld\n\r", __FUNCTION__, gtime.tv_nsec);
                }

                struct timespec stime;
                char*           tmpbuf = CgiParamA[i].value;
                char*           tmpstr;
                char*           delim = ".";
                tmpstr                = (char*)strtok(tmpbuf, delim);
                fprintf(stderr, "%s: tmpstr=%s\n\r", __FUNCTION__, tmpstr);
                stime.tv_sec = atoi(tmpstr);
                fprintf(stderr, "%s: stime.tv_sec=%ld\n\r", __FUNCTION__, stime.tv_sec);

                while ((tmpstr = (char*)strtok(NULL, delim)))
                {
                    fprintf(stderr, "%s: tmpstr=%s\n\r", __FUNCTION__, tmpstr);
                    stime.tv_nsec = atoi(tmpstr);
                    fprintf(stderr, "%s: stime.tv_nsec=%ld\n\r", __FUNCTION__, stime.tv_nsec);
                }

                if (clock_settime(CLOCK_REALTIME, &stime) == -1)
                {
                    fprintf(stderr, "%s: clock_settime error\n\r", __FUNCTION__);
                }

                fprintf(stdout, "0\n");
                fprintf(stdout, "OK\n");
                fprintf(stderr, "0\n");
                fprintf(stderr, "OK\n");
            }
        }
        else if (strcmp(CgiParamA[i].action, "dir") == 0 || strcmp(CgiParamA[i].action, "reardir") == 0)
        {
#if 0
            strcpy(CgiParamA[i].flag, "directory process");
            fprintf(stderr, "%s: CgiParamA[%d].flag=%s\n\r",   __FUNCTION__, i, CgiParamA[i].flag);
#endif
            if (strcmp(CgiParamA[i].property, "DCIM") == 0)
                strcpy(CgiParamA[i].property, "Normal");
            if (strcasecmp(CgiParamA[i].format, "jpeg") == 0)
                strcpy(CgiParamA[i].format, "JPG");

            directory(CgiParamA[i].action, CgiParamA[i].property, CgiParamA[i].format, CgiParamA[i].count,
                      CgiParamA[i].from);
        }
        else
        {
            if (strcmp(CgiParamA[i].action, "get") == 0)
            {
                GetProcess(CgiParamA[i].property);
            }
            else if ((strcmp(CgiParamA[i].action, "set") == 0) && (strcmp(CgiParamA[i].property, "Playback") == 0)
                     && (strcmp(CgiParamA[i].value, "heartbeat") == 0))
            {
                // ExeCgiCmd: key_value[0]= Playback=heartbeat
                // don`t call SetProcess.
                fprintf(stdout, "0\n");
                fprintf(stdout, "OK\n");
                fprintf(stderr, "0\n");
                fprintf(stderr, "OK\n");
            }
            else if (strcmp(CgiParamA[i].action, "set") == 0)
            {
                strcpy(CgiParamA[i].key_value, CgiParamA[i].property);
                strcat(CgiParamA[i].key_value, "=");
                strcat(CgiParamA[i].key_value, CgiParamA[i].value);
                fprintf(stderr, "%s: key_value[%d]= %s\n\r", __FUNCTION__, i, CgiParamA[i].key_value);
                SetProcess(CgiParamA[i].key_value);
            }
            else if (strcmp(CgiParamA[i].action, "del") == 0)
            {
                DelProcess(CgiParamA[i].property);
            }
            else
            {
                fprintf(stderr, "CgiParam error, %s %s %s\n", CgiParamA[i].action, CgiParamA[i].property,
                        CgiParamA[i].value);

                fprintf(stdout, "709\n");
                fprintf(stdout, "???\n");
                fprintf(stderr, "709\n");
                fprintf(stderr, "???\n");
            }
        }
    }
    // fprintf(stderr, "%s: exit\n\r",   __FUNCTION__);
    return 0;
}

int main(void)
{
    cmdhead = cmdcurrent;
    add_cmd_list("action", "action_application_test", EXECMD, (Callback)cb_action, cmdcurrent);
    add_cmd_list("property", "property_application_test", EXECMD, (Callback)cb_property, cmdcurrent);
    add_cmd_list("value", "value_application_test", EXECMD, (Callback)cb_value, cmdcurrent);
    add_cmd_list("format", "format_application_test", EXECMD, (Callback)cb_format, cmdcurrent);
    add_cmd_list("count", "count_application_test", EXECMD, (Callback)cb_count, cmdcurrent);
    add_cmd_list("from", "from_application_test", EXECMD, (Callback)cb_from, cmdcurrent);

    if (semaphore_open() < 0)
    {
        fprintf(stderr, "CGI %s: more than two CMD.\n\r", __FUNCTION__);
        return -1;
    }

    ProcessCgiCmd(cmdhead, "cmd", "../system.html");

    if (strcmp(CgiParamA[0].property, "thumb.jpeg") == 0)
    {
        // fprintf(stderr, "response image\n");
        PrintHttpHdr_image();
    }
    else
    {
        // fprintf(stderr, "CGI: response text\n");
        PrintHttpHdr_text();
    }
    ExeCgiCmd();

    semaphore_close();
    return 0;
}
