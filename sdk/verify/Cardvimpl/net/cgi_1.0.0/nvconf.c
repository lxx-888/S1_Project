/*
 *	nvconf- Sigmastar
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
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>  //OPEN ,O_WRONLY
#include <unistd.h> //WRITE, CLOSE
#include <string.h>

#include "nvconf.h"

void usage(char *argv)
{
    fprintf(stderr, "Usage: %s [command] [TYPE] [NAME] [VALUE]\n", argv);
    fprintf(stderr,
            "command:\n"
            "\tset <TYPE> <NAME> <VALUE>\tSet parameter to NAME\n"
            "\tget <TYPE> <NAME>\t\tGet parameter by NAME\n"
            "\tadd <TYPE> <NAME> <VALUE>\t\tAdd parameter by NAME\n"
            "\tdel <TYPE> <NAME>\t\tDelete parameter by NAME\n"
            "\tshow <TYPE>\t\t\tShow all parameters\n"
            "\tupdate <TYPE>\t\t\tupdate files crc\n");
    fprintf(stderr, "TYPE:\t0: CGI, 1: NET\n");
    // exit(1);
}

int main(int argc, char **argv)
{
    ENTRY  entry;
    PENTRY pentry;
    memset(entry.pkey, '\0', sizeof(entry.pkey));
    memset(entry.pval, '\0', sizeof(entry.pval));
    pentry = &entry;
    if (semaphore_open() < 0)
        return -1;

    if (argc > 5 || argc < 2)
    {
        usage(argv[0]);
        goto EXIT;
    }
    else if (!strcmp(argv[1], "set"))
    {
        if (argc != 5)
        {
            usage(argv[0]);
            goto EXIT;
        }
        else
        {
            if (!strcmp(argv[2], "0"))
            {
                if (load_nvconf(NVCONF_CGIPATH) != 0)
                    goto EXIT;
            }
            else
            {
                if (load_nvconf(NVCONF_NETPATH) != 0)
                    goto EXIT;
            }
            memcpy(entry.pkey, argv[3], strlen(argv[3]));
            memcpy(entry.pval, argv[4], strlen(argv[4]));
            nvconf_set(pentry);
            if (!strcmp(argv[2], "0"))
            {
                nvconf_commit(NVCONF_CGIPATH);
            }
            else
                nvconf_commit(NVCONF_NETPATH);
        }
    }
    else if (!strcmp(argv[1], "get"))
    {
        if (argc != 4)
        {
            usage(argv[0]);
            goto EXIT;
        }
        else
        {
            if (!strcmp(argv[2], "0"))
            {
                if (load_nvconf(NVCONF_CGIPATH) != 0)
                    goto EXIT;
            }
            else
            {
                if (load_nvconf(NVCONF_NETPATH) != 0)
                    goto EXIT;
            }
            memcpy(entry.pkey, argv[3], strlen(argv[3]));
            nvconf_get(entry.pkey);
        }
    }
    else if (!strcmp(argv[1], "add"))
    {
        if (argc != 5)
        {
            usage(argv[0]);
            goto EXIT;
        }
        else
        {
            if (!strcmp(argv[2], "0"))
            {
                if (load_nvconf(NVCONF_CGIPATH) != 0)
                    goto EXIT;
            }
            else
            {
                if (load_nvconf(NVCONF_NETPATH) != 0)
                    goto EXIT;
            }
            memcpy(entry.pkey, argv[3], strlen(argv[3]));
            memcpy(entry.pval, argv[4], strlen(argv[4]));
            nvconf_add_entry(&entry, 0);
            if (!strcmp(argv[2], "0"))
            {
                nvconf_commit(NVCONF_CGIPATH);
            }
            else
                nvconf_commit(NVCONF_NETPATH);
        }
    }
    else if (!strcmp(argv[1], "del"))
    {
        if (argc != 5)
        {
            usage(argv[0]);
            goto EXIT;
        }
        else
        {
            if (!strcmp(argv[2], "0"))
            {
                if (load_nvconf(NVCONF_CGIPATH) != 0)
                    goto EXIT;
            }
            else
            {
                if (load_nvconf(NVCONF_NETPATH) != 0)
                    goto EXIT;
            }
            memcpy(entry.pkey, argv[3], strlen(argv[3]));
            nvconf_del_entry(entry.pkey);
            if (!strcmp(argv[2], "0"))
            {
                nvconf_commit(NVCONF_CGIPATH);
            }
            else
                nvconf_commit(NVCONF_NETPATH);
        }
    }
    else if (!strcmp(argv[1], "show"))
    {
        if (argc != 3)
        {
            usage(argv[0]);
            goto EXIT;
        }
        else
        {
            if (!strcmp(argv[2], "0"))
            {
                if (load_nvconf(NVCONF_CGIPATH) != 0)
                    goto EXIT;
            }
            else
            {
                if (load_nvconf(NVCONF_NETPATH) != 0)
                    goto EXIT;
            }
            nvconf_all();
        }
    }
    else if (!strcmp(argv[1], "update"))
    {
        if (argc != 3)
        {
            usage(argv[0]);
            goto EXIT;
        }
        else
        {
            if (!strcmp(argv[2], "0"))
            {
                printf("update %s\n", NVCONF_CGIPATH);
                nvconf_update(NVCONF_CGIPATH);
            }
            else
            {
                printf("update %s\n", NVCONF_NETPATH);
                nvconf_update(NVCONF_NETPATH);
            }
            system("sync");
        }
    }
    else
    {
        usage(argv[0]);
        goto EXIT;
    }

EXIT:
    semaphore_close();
    return 0;
}
