/*
 * module_swspi.cpp- Sigmastar
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "module_common.h"
#include "module_gpio.h"

typedef struct
{
    MI_U8 u8CsGpioNum;
    MI_U8 u8SckGpioNum;
    MI_U8 u8SdiGpioNum;
} stSwSpi_t;

static void _toggle_SPI_CS(stSwSpi_t *pstSwSpi, MI_U8 value)
{
    cardv_gpio_set_val(pstSwSpi->u8CsGpioNum, value);
}

static void _toggle_SPI_SCL(stSwSpi_t *pstSwSpi, MI_U8 value)
{
    cardv_gpio_set_val(pstSwSpi->u8SckGpioNum, value);
}

static void _toggle_SPI_SDI(stSwSpi_t *pstSwSpi, MI_U8 value)
{
    cardv_gpio_set_val(pstSwSpi->u8SdiGpioNum, value);
}

void *cardv_swspi_open(MI_U8 u8CsGpioNum, MI_U8 u8SckGpioNum, MI_U8 u8SdiGpioNum)
{
    stSwSpi_t *pstSwSpi = (stSwSpi_t *)MALLOC(sizeof(stSwSpi_t));
    if (pstSwSpi)
    {
        pstSwSpi->u8CsGpioNum  = u8CsGpioNum;
        pstSwSpi->u8SckGpioNum = u8SckGpioNum;
        pstSwSpi->u8SdiGpioNum = u8SdiGpioNum;

        cardv_gpio_export(u8CsGpioNum);
        cardv_gpio_export(u8SckGpioNum);
        cardv_gpio_export(u8SdiGpioNum);

        cardv_gpio_set_direction(u8CsGpioNum, "out");
        cardv_gpio_set_direction(u8SckGpioNum, "out");
        cardv_gpio_set_direction(u8SdiGpioNum, "out");

        _toggle_SPI_CS(pstSwSpi, 1);
        _toggle_SPI_SCL(pstSwSpi, 1);
        _toggle_SPI_SDI(pstSwSpi, 1);

        return (void *)pstSwSpi;
    }

    return NULL;
}

void cardv_swspi_close(void *handler)
{
    stSwSpi_t *pstSwSpi = (stSwSpi_t *)handler;

    if (pstSwSpi)
    {
        cardv_gpio_unexport(pstSwSpi->u8CsGpioNum);
        cardv_gpio_unexport(pstSwSpi->u8SckGpioNum);
        cardv_gpio_unexport(pstSwSpi->u8SdiGpioNum);
        free(pstSwSpi);
    }
}

void cardv_swspi_write(void *handler, MI_U32 u32Val, MI_U32 u32Bits)
{
    stSwSpi_t *pstSwSpi = (stSwSpi_t *)handler;

    if (pstSwSpi)
    {
        MI_U32 i;
        MI_U32 MSB_First = 1 << (u32Bits - 1);
        _toggle_SPI_CS(pstSwSpi, 1);
        _toggle_SPI_SDI(pstSwSpi, 1);
        _toggle_SPI_SCL(pstSwSpi, 0);
        usleep(1);
        _toggle_SPI_CS(pstSwSpi, 0);
        for (i = 0; i < u32Bits; i++)
        {
            usleep(1);
            _toggle_SPI_SCL(pstSwSpi, 0);
            if (u32Val & MSB_First)
                _toggle_SPI_SDI(pstSwSpi, 1);
            else
                _toggle_SPI_SDI(pstSwSpi, 0);
            usleep(1);
            _toggle_SPI_SCL(pstSwSpi, 1);
            u32Val <<= 1;
        }
        usleep(1);
        _toggle_SPI_CS(pstSwSpi, 1);
        usleep(2);
    }
}