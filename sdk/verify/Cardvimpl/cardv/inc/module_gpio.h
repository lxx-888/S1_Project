/*
 * module_gpio.h- Sigmastar
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
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
#ifndef _MODULE_GPIO_H_
#define _MODULE_GPIO_H_

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define GPIO_DIR_OUT "out"
#define GPIO_DIR_IN  "in"

#define GPIO_HIGH (1)
#define GPIO_LOW  (0)

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

int cardv_gpio_export(int gpioNum);
int cardv_gpio_unexport(int gpioNum);
int cardv_gpio_set_direction(int gpioNum, const char* direct);
int cardv_gpio_get_val(int gpioNum);
int cardv_gpio_set_val(int gpioNum, int value);

#endif //#define _MODULE_GPIO_H_
