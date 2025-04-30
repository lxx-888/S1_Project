/*
 * ut_adclp.c- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <drv_adclp.h>

void sample_warn(int num)
{
    printf("adclp data exceeding the threshold\n");
}

int main(int argc, char **argv)
{
    int                i;
    int                fd;
    char               cmd;
    int                flags;
    unsigned short     value;
    unsigned int       channel;
    unsigned int       chan_num;
    unsigned char      vdd_type;
    char               path[64];
    struct adclp_bound adclp_bd;

    if (argc < 2)
    {
        printf("format: ut_adclp once [channel] <upper> <lower>\n");
        printf("format: ut_adclp vdd  [channel] [type]\n");
        printf("format: ut_adclp scan [chan num]\n");
        return -1;
    }

    if (!strcmp(argv[1], "once"))
    {
        if (argc == 3)
        {
            channel = atoi(argv[2]);
        }
        else if (argc == 5)
        {
            channel              = atoi(argv[2]);
            adclp_bd.upper_bound = atoi(argv[3]);
            adclp_bd.lower_bound = atoi(argv[4]);
        }
        else
        {
            printf("format: ut_adclp once [channel] <upper> <lower>\n");
            return -1;
        }

        snprintf(path, sizeof(path), "/dev/adclp%u", channel);
        fd = open((const char *)(char *)path, O_RDWR);
        if (fd < 0)
        {
            printf("open device fail\n");
            return -1;
        }

        if (argc == 4)
        {
            ioctl(fd, IOCTL_ADCLP_SET_BOUND, &adclp_bd);
        }

        signal(SIGIO, sample_warn);

        fcntl(fd, F_SETOWN, getpid());
        flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags | FASYNC);

        while (1)
        {
            cmd = getchar();
            if (cmd == 'q' || cmd == 'Q')
            {
                break;
            }

            ioctl(fd, IOCTL_ADCLP_READ_VALUE, &value);
            printf("adclp%u data[%hu]\n", channel, value);
        }

        close(fd);
    }
    else if (!strcmp(argv[1], "vdd"))
    {
        if (argc == 4)
        {
            channel  = atoi(argv[2]);
            vdd_type = atoi(argv[3]);
        }
        else
        {
            printf("format: ut_adclp vdd [channel] [type]\n");
            return -1;
        }

        snprintf(path, sizeof(path), "/dev/adclp%u", channel);
        fd = open((const char *)(char *)path, O_RDWR);
        if (fd < 0)
        {
            printf("open device fail\n");
            return -1;
        }

        ioctl(fd, IOCTL_ADCLP_VDD_TYPE, &vdd_type);
        close(fd);
    }
    else if (!strcmp(argv[1], "scan"))
    {
        if (argc == 3)
        {
            chan_num = atoi(argv[2]);
        }
        else
        {
            printf("format: ut_adclp scan [chan num]\n");
            return -1;
        }

        for (i = 0; i < chan_num; i++)
        {
            snprintf(path, sizeof(path), "/dev/adclp%u", i);
            fd = open((const char *)(char *)path, O_RDWR);
            if (fd < 0)
            {
                printf("open device fail\n");
                continue;
            }

            ioctl(fd, IOCTL_ADCLP_READ_VALUE, &value);
            printf("adclp%u data[%hu]\n", i, value);
            close(fd);
        }
    }
    else
    {
        printf("format: ut_adclp once [channel] <upper> <lower>\n");
        printf("format: ut_adclp vdd  [channel] [type]\n");
        printf("format: ut_adclp scan [chan num]\n");
        return -1;
    }

    return 0;
}
