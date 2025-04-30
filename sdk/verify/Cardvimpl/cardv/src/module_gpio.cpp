/*************************************************
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 * All rights reserved.
 *
 **************************************************
 * File name:  module_gpio.c
 * Author:     jeff.li@sigmastar.com.cn
 * Version:    Initial Draft
 * Date:       2019/8/23
 * Description: gpio module source file
 *
 *
 *
 * History:
 *
 *    1. Date  :        2019/8/23
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

#include "module_common.h"

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int cardv_gpio_export(int gpioNum)
{
    int  fd     = -1;
    char buf[4] = {
        0,
    };

    sprintf(buf, "%d", gpioNum);
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0)
    {
        printf("fail to export gpio[%d]\n", gpioNum);
        return -1;
    }

    write(fd, buf, sizeof(buf));
    close(fd);
    return 0;
}

int cardv_gpio_unexport(int gpioNum)
{
    int  fd     = -1;
    char buf[4] = {
        0,
    };

    sprintf(buf, "%d", gpioNum);
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0)
    {
        printf("no export gpio[%d]\n", gpioNum);
        return -1;
    }

    write(fd, buf, sizeof(buf));
    close(fd);
    return 0;
}

int cardv_gpio_set_direction(int gpioNum, const char* direct)
{
    int  fd       = -1;
    char path[32] = {
        0,
    };

    sprintf(path, "/sys/class/gpio/gpio%d/direction", gpioNum);
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        printf("fail direction[%s] gpio[%d]\n", direct, gpioNum);
        return -1;
    }

    write(fd, direct, sizeof(direct));
    close(fd);
    return 0;
}

int cardv_gpio_get_val(int gpioNum)
{
    FILE* file    = NULL;
    char  buf[10] = {
        0,
    };
    char path[32] = {
        0,
    };
    int value = 0;

    sprintf(path, "/sys/class/gpio/gpio%d/value", gpioNum);

    file = fopen(path, "rb");
    if (file == NULL)
    {
        printf("fail to open gpio[%d]\n", gpioNum);
        return -1;
    }

    fseek(file, 0, SEEK_END);

    int len = ftell(file);

    rewind(file);
    fread(buf, 1, len, file);
    fclose(file);

    value = atoi(buf);
    // printf("Get gpio[%d] value[%d]\n", gpioNum, value);
    return value;
}

int cardv_gpio_set_val(int gpioNum, int value)
{
    int  fd     = -1;
    char buf[4] = {
        0,
    };
    char path[32] = {
        0,
    };

    sprintf(buf, "%d", value);
    sprintf(path, "/sys/class/gpio/gpio%d/value", gpioNum);

    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        printf("fail to open gpio[%d]\n", gpioNum);
        return -1;
    }

    write(fd, buf, 4);
    close(fd);
    // printf("Set gpio[%d] value[%d]\n", gpioNum, value);
    return 0;
}