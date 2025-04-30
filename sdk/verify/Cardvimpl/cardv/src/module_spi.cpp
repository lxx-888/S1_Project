/*************************************************
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 * All rights reserved.
 *
 **************************************************
 * File name:  module_spi.c
 * Author:     jiahong.xiong@sigmastar.com.cn
 * Version:    Initial Draft
 * Date:       2020/9/1
 * Description: spi module source file
 *
 *
 *
 * History:
 *
 *    1. Date  :        2020/9/1
 *       Author:        jiahong.xiong@sigmastar.com.cn
 *       Modification:  Created file
 *
 **************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "module_common.h"
#include "module_spi.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

static MI_U8  mode;      /* SPI通信使用全双工，设置CPOL＝0，CPHA＝0。 */
static MI_U8  bits  = 9; /* 9ｂiｔｓ读写，MSB first。*/
static MI_U32 speed = 500000;
static MI_U16 delay = 0;

void cardv_spi_transfer(int fd)
{
    int   ret;
    MI_U8 tx[]               = {// 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                  // 0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
                  // 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                  // 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                  // 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                  // 0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xAD,
                  // 0xF0, 0x0D,
                  0xA5, 0xF0, 0x85};
    MI_U8 rx[ARRAY_SIZE(tx)] = {
        0,
    };
    struct spi_ioc_transfer tr = {
        .tx_buf        = (unsigned long)tx,
        .rx_buf        = (unsigned long)rx,
        .len           = ARRAY_SIZE(tx),
        .speed_hz      = speed,
        .delay_usecs   = delay,
        .bits_per_word = bits,
    };
    DBG_ENTRY("cardv_spi_transfer start\n");

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0)
        DBG_ERR("can't transfer spi message");

    /*
    {
        MI_U16 i;
        for (i = 0; i < ARRAY_SIZE(tx); i++) {
        if (!(i % 8))
            DBG_INFO("\n");
            DBG_INFO("%.2X ", rx[i]);
        }
        DBG_INFO("\n");
    }

    */
    DBG_ENTRY("cardv_spi_transfer end\n");
}

void cardv_spi_send_data(int fd, MI_U16 data, MI_U16 size)
{
    int    ret;
    MI_U16 tx[64]             = {0};
    MI_U16 rx[ARRAY_SIZE(tx)] = {0};

    tx[0] = data;

    struct spi_ioc_transfer tr = {
        .tx_buf        = (unsigned long)tx,
        .rx_buf        = (unsigned long)rx,
        .len           = (MI_U32)(size * 2),
        .speed_hz      = speed,
        .delay_usecs   = delay,
        .bits_per_word = bits,
    };
    DBG_ENTRY("cardv_spi_send_data start\n");

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0)
        DBG_ERR("can't send spi message");

    /*
    {
        MI_U16 i;
        for (i = 0; i < ARRAY_SIZE(tx); i++) {
            DBG_INFO("%.2X ", rx[i]);
        }
        DBG_INFO("\n ");
    }*/

    DBG_ENTRY("cardv_spi_send_data end\n");
}

void cardv_spi_read_data(int fd, MI_U8 size)
{
    int    ret;
    MI_U16 tx[10];
    MI_U16 rx[ARRAY_SIZE(tx)] = {0};

    tx[0] = 0x199;

    struct spi_ioc_transfer tr = {
        .tx_buf        = (unsigned long)NULL,
        .rx_buf        = (unsigned long)rx,
        .len           = (MI_U32)(size * 2),
        .speed_hz      = speed,
        .delay_usecs   = delay,
        .bits_per_word = bits,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0)
        DBG_ERR("can't send spi message");

    /*
    {
        MI_U16 i;
        for (i = 0; i < size; i++) {
            DBG_INFO("%.2X ", rx[i]);
        }
        DBG_INFO("\n ");
    }*/
}

/**
 * spi open device
 */
int cardv_spi_open(const char *device, MI_U8 u8mode, MI_U8 u8bits, MI_U32 u32speed, MI_U16 u16delay)
{
    int fd;
    int ret = 0;

    bits  = u8bits;
    mode  = u8mode;
    speed = u32speed;
    delay = u16delay;

    DBG_DEBUG("cardv_spi_open %s\n", device);

    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        DBG_FATAL("can't open device\n");
        return fd;
    }

    // spi mode
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
    {
        DBG_ERR("can't set spi mode\n");
        goto EXIT;
    }

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
    {
        DBG_ERR("can't set spi mode\n");
        goto EXIT;
    }

    // bits per word
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
    {
        DBG_ERR("can't set bits per word\n");
        goto EXIT;
    }

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
    {
        DBG_ERR("can't set bits per word\n");
        goto EXIT;
    }

    // max speed hz
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
    {
        DBG_ERR("can't set max speed hz\n");
        goto EXIT;
    }

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
    {
        DBG_ERR("can't set max speed hz\n");
        goto EXIT;
    }

    DBG_INFO("spi mode: %d\n", mode);
    DBG_INFO("bits per word: %d\n", bits);
    DBG_INFO("max speed: %d KHz (%d MHz)\n", speed / 1000, speed / 1000 / 1000);

EXIT:
    if (ret < 0)
    {
        close(fd);
        return ret;
    }
    else
    {
        DBG_DEBUG("spi:%s open success\n", device);
        return fd;
    }
}

/**
 * close spi device
 */
void cardv_spi_close(int fd)
{
    if (fd < 0)
    {
        DBG_ERR("error close spi device\n");
        return;
    }

    close(fd);
    return;
}

#if 0
// spi device lockback test,need connect MOSI to MISO
int cardv_spi_lockback_test(void)
{
    int       ret, i;
    const int BufSize = 16;
    MI_U32    tx[BufSize], rx[BufSize];

    bzero(rx, sizeof(rx));
    for (i    = 0; i < BufSize; i++)
        tx[i] = i;

    DBG_INFO("nSPI - LookBack Mode Test...\n");
    ret = cardv_spi_transfer(tx, rx, BufSize);
    if (ret > 1)
    {
        ret = memcmp(tx, rx, BufSize);
        if (ret != 0)
        {
            DBG_ERR("LookBack Mode Test error\n");
        }
        else
            DBG_DEBUG("SPI - LookBack Mode OK\n");
    }

    return ret;
}
#endif