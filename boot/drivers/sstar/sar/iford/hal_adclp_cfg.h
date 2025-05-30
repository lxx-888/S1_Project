/*
 * hal_adclp_cfg.h- Sigmastar
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

#ifndef _HAL_ADCLP_CFG_H_
#define _HAL_ADCLP_CFG_H_

#include <adclp_os.h>

#define HAL_ADCLP_SRCCLK 12000000
#define BASE_REG_RIU_PA  0x1F000000

struct hal_adclp_reg
{
    u64 bank_base;
    u8  reg_offset;
    u8  bit_shift;
    u32 bit_mask;
};

struct hal_adclp_clk
{
    u8  clk_mod;
    u32 clk_freq;
};

#ifndef CONFIG_SSTAR_CLK
struct hal_adclp_reg adclp_clk_reg = {
    .bank_base  = (BASE_REG_RIU_PA + (0xE << 9)),
    .reg_offset = 0x22,
    .bit_shift  = 5,
    .bit_mask   = 0xF0,
};

struct hal_adclp_clk adclp_clk_src = {
    .clk_mod  = 0,
    .clk_freq = 12000000,
};
#endif

#endif
