/*
 * hal_card_platform_regs.h- Sigmastar
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

#define A_RIU_BASE (0x1F000000)
// but this does send command error: EV_STS_MIE_TOUT
//---- sd & sdio ----
#define A_FCIE1_0_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0980) // SDIO0_0_BANK 1413h (SD on i6)
#define A_FCIE1_1_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0A00) // SDIO0_1_BANK 1414h
#define A_FCIE1_2_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0A80) // SDIO0_2_BANK 1415h

#define A_FCIE2_0_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0B00) // SDIO2_0_BANK 1416h
#define A_FCIE2_1_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0B80)
#define A_FCIE2_2_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C00)

#define A_FCIE3_0_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0800) // FCIE0_0_BANK 1410h (FCIE on m6p)
#define A_FCIE3_1_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0880) // FCIE0_1_BANK 1411h
#define A_FCIE3_2_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0900) // FCIE0_2_BANK 1412h

#define A_FCIE4_0_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0D00) // SDIO3_0_BANK 141Ah
#define A_FCIE4_1_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0D80)
#define A_FCIE4_2_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0E00)

#define A_PLL_1_0_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80) // PLL_1_0_BANK 1419h
#define A_PLL_1_1_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80) // not used
#define A_PLL_1_2_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80)
#define A_PLL_2_0_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80)
#define A_PLL_2_1_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80)
#define A_PLL_2_2_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80)

#define A_PLL_3_0_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80) // not used
#define A_PLL_3_1_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80)
#define A_PLL_3_2_BANK GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0C80)
