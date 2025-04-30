/*
 * module_panel.cpp - Sigmastar
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

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================
#include "module_config.h"
#if (CARDV_PANEL_ENABLE)

#include "mi_disp.h"
#include "mid_common.h"
#include "module_panel.h"
#include "module_gpio.h"
#include "module_swspi.h"

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

static MI_PANEL_IntfType_e _TransDispToPanel(MI_DISP_Interface_e eDispIf)
{
    MI_PANEL_IntfType_e ePanelIf;
    ePanelIf = eDispIf == E_MI_DISP_INTF_LCD          ? E_MI_PNL_INTF_TTL
               : eDispIf == E_MI_DISP_INTF_TTL        ? E_MI_PNL_INTF_TTL
               : eDispIf == E_MI_DISP_INTF_TTL_SPI_IF ? E_MI_PNL_INTF_TTL_SPI_IF
               : eDispIf == E_MI_DISP_INTF_MIPIDSI    ? E_MI_PNL_INTF_MIPI_DSI
               : eDispIf == E_MI_DISP_INTF_MCU_NOFLM  ? E_MI_PNL_INTF_MCU_TYPE
               : eDispIf == E_MI_DISP_INTF_MCU        ? E_MI_PNL_INTF_MCU_TYPE
               : eDispIf == E_MI_DISP_INTF_SRGB       ? E_MI_PNL_INTF_SRGB
                                                      : E_MI_PNL_INTF_MAX;
    return ePanelIf;
}

static void _ST7789V_SRGB_Panel_SPI_Init(void)
{
#define RST_GPIO_NUM (78)
#define CS_GPIO_NUM  (28)
#define CLK_GPIO_NUM (29)
#define SDI_GPIO_NUM (33)

    void *pvSwSpi;
    system("/customer/riu_w 103c 74 200");

    cardv_gpio_export(RST_GPIO_NUM);
    cardv_gpio_set_direction(RST_GPIO_NUM, "out");

    cardv_gpio_set_val(RST_GPIO_NUM, 0);
    usleep(100 * 1000);
    cardv_gpio_set_val(RST_GPIO_NUM, 1);

    pvSwSpi = cardv_swspi_open(CS_GPIO_NUM, CLK_GPIO_NUM, SDI_GPIO_NUM);

    cardv_swspi_write(pvSwSpi, 0x0011, 9);
    usleep(120 * 1000);

    cardv_swspi_write(pvSwSpi, 0x0036, 9);
    cardv_swspi_write(pvSwSpi, 0x0100, 9);

    cardv_swspi_write(pvSwSpi, 0x003A, 9);
    cardv_swspi_write(pvSwSpi, 0x0105, 9);

    cardv_swspi_write(pvSwSpi, 0x00B0, 9);
    cardv_swspi_write(pvSwSpi, 0x0111, 9);
    cardv_swspi_write(pvSwSpi, 0x0104, 9);

    cardv_swspi_write(pvSwSpi, 0x00B1, 9);
    cardv_swspi_write(pvSwSpi, 0x0140, 9);
    cardv_swspi_write(pvSwSpi, 0x0104, 9);
    cardv_swspi_write(pvSwSpi, 0x010A, 9);

    cardv_swspi_write(pvSwSpi, 0x00B2, 9);
    cardv_swspi_write(pvSwSpi, 0x010C, 9);
    cardv_swspi_write(pvSwSpi, 0x010C, 9);
    cardv_swspi_write(pvSwSpi, 0x0100, 9);
    cardv_swspi_write(pvSwSpi, 0x0133, 9);
    cardv_swspi_write(pvSwSpi, 0x0133, 9);

    cardv_swspi_write(pvSwSpi, 0x00B7, 9);
    cardv_swspi_write(pvSwSpi, 0x0175, 9);

    cardv_swspi_write(pvSwSpi, 0x00BB, 9);
    cardv_swspi_write(pvSwSpi, 0x011D, 9);

    cardv_swspi_write(pvSwSpi, 0x00C0, 9);
    cardv_swspi_write(pvSwSpi, 0x012C, 9);

    cardv_swspi_write(pvSwSpi, 0x00C2, 9);
    cardv_swspi_write(pvSwSpi, 0x0101, 9);

    cardv_swspi_write(pvSwSpi, 0x00C3, 9);
    cardv_swspi_write(pvSwSpi, 0x0113, 9);

    cardv_swspi_write(pvSwSpi, 0x00C4, 9);
    cardv_swspi_write(pvSwSpi, 0x0120, 9);

    cardv_swspi_write(pvSwSpi, 0x00C6, 9);
    cardv_swspi_write(pvSwSpi, 0x010F, 9);

    cardv_swspi_write(pvSwSpi, 0x00D0, 9);
    cardv_swspi_write(pvSwSpi, 0x01A4, 9);
    cardv_swspi_write(pvSwSpi, 0x01A1, 9);

    cardv_swspi_write(pvSwSpi, 0x00D6, 9);
    cardv_swspi_write(pvSwSpi, 0x01A1, 9);

    cardv_swspi_write(pvSwSpi, 0x00E0, 9);
    cardv_swspi_write(pvSwSpi, 0x01F0, 9);
    cardv_swspi_write(pvSwSpi, 0x0105, 9);
    cardv_swspi_write(pvSwSpi, 0x0110, 9);
    cardv_swspi_write(pvSwSpi, 0x0113, 9);
    cardv_swspi_write(pvSwSpi, 0x0114, 9);
    cardv_swspi_write(pvSwSpi, 0x012D, 9);
    cardv_swspi_write(pvSwSpi, 0x0138, 9);
    cardv_swspi_write(pvSwSpi, 0x0144, 9);
    cardv_swspi_write(pvSwSpi, 0x0148, 9);
    cardv_swspi_write(pvSwSpi, 0x010B, 9);
    cardv_swspi_write(pvSwSpi, 0x0114, 9);
    cardv_swspi_write(pvSwSpi, 0x0111, 9);
    cardv_swspi_write(pvSwSpi, 0x0119, 9);
    cardv_swspi_write(pvSwSpi, 0x011C, 9);

    cardv_swspi_write(pvSwSpi, 0x00E1, 9);
    cardv_swspi_write(pvSwSpi, 0x01F0, 9);
    cardv_swspi_write(pvSwSpi, 0x0103, 9);
    cardv_swspi_write(pvSwSpi, 0x010B, 9);
    cardv_swspi_write(pvSwSpi, 0x010B, 9);
    cardv_swspi_write(pvSwSpi, 0x010B, 9);
    cardv_swspi_write(pvSwSpi, 0x0126, 9);
    cardv_swspi_write(pvSwSpi, 0x0138, 9);
    cardv_swspi_write(pvSwSpi, 0x0154, 9);
    cardv_swspi_write(pvSwSpi, 0x0148, 9);
    cardv_swspi_write(pvSwSpi, 0x013D, 9);
    cardv_swspi_write(pvSwSpi, 0x0119, 9);
    cardv_swspi_write(pvSwSpi, 0x0119, 9);
    cardv_swspi_write(pvSwSpi, 0x011B, 9);
    cardv_swspi_write(pvSwSpi, 0x011E, 9);

    cardv_swspi_write(pvSwSpi, 0x0029, 9);
    cardv_swspi_write(pvSwSpi, 0x002C, 9);

    cardv_swspi_close(pvSwSpi);
}

MI_S32 CarDV_PanelModuleInit(MI_DISP_Interface_e eDispIf)
{
    MI_PANEL_InitParam_t stInitParam;

    stInitParam.eIntfType = _TransDispToPanel(eDispIf);
    if (stInitParam.eIntfType == E_MI_PNL_INTF_SRGB)
    {
        _ST7789V_SRGB_Panel_SPI_Init();
    }

    CARDVCHECKRESULT_OS(MI_PANEL_InitDev(&stInitParam));
    return 0;
}

MI_S32 CarDV_PanelModuleUnInit(MI_DISP_Interface_e eDispIf)
{
    MI_PANEL_InitParam_t stInitParam;
    stInitParam.eIntfType = _TransDispToPanel(eDispIf);
    CARDVCHECKRESULT_OS(MI_PANEL_DeInitDev(&stInitParam));
    return 0;
}
#endif