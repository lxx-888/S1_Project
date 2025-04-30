/*
 * hal_sdmmc_regs.h- Sigmastar
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
/***************************************************************************************************************
 *
 * FileName hal_sdmmc_regs.h
 *     @author jeremy.wang (2015/08/12)
 * Desc:
 *     This file is the header file of hal_card_regs.c.
 *
 *     For Base RegisterSetting:
 *     (1) BASE Register Address
 *     (2) BASE Register Operation
 *     (3) BASE FCIE Reg Meaning Position (Including File)
 *
 *     P.S. If you want to use only IP for single card or dual cards,
 *          please modify FCIE1 and FCIE2 setting to the same reg position.
 *
 ***************************************************************************************************************/

#ifndef __HAL_SDMMC_REGS_H
#define __HAL_SDMMC_REGS_H

#include "hal_sdmmc_base.h"
#if (D_OS == D_OS__LINUX)
#include "drv_sdmmc_lnx.h"
#elif (D_OS == D_OS__RTK)
#include "drv_sdmmc_rtk.h"
#endif

//***********************************************************************************************************
// (1) BASE Register Address
//***********************************************************************************************************
#include "hal_sdmmc_platform_regs.h"

//***********************************************************************************************************
// (2) BASE Register Operation
//***********************************************************************************************************

//###########################################################################################################
#if (D_OS == D_OS__LINUX)
//###########################################################################################################
#include "../../../sstar/include/ms_platform.h" //IO_ADDRESS       // IO Mapping Address
#define IO_MAPADDR(Reg_Addr) IO_ADDRESS(Reg_Addr)
//#endif
//###########################################################################################################
#else
//###########################################################################################################
#define IO_MAPADDR(Reg_Addr) Reg_Addr
//###########################################################################################################
#endif

#define D_MIU_WIDTH             8 // Special MIU WIDTH for FCIE4
#define REG_OFFSET_BITS         2 // Register Offset Byte  (2= 4byte = 32bits)
#define GET_CARD_REG_ADDR(x, y) ((x) + ((y) << REG_OFFSET_BITS))

#define CARD_REG(Reg_Addr)    (*(volatile U16_T*)(IO_MAPADDR(Reg_Addr)))
#define CARD_REG_L8(Reg_Addr) (*(volatile U8_T*)(IO_MAPADDR(Reg_Addr)))
#define CARD_REG_H8(Reg_Addr) (*((volatile U8_T*)(IO_MAPADDR(Reg_Addr)) + 1))

/* write register by long */
#define sdmmc_writel(v, a) (*(volatile U32_T*)IO_MAPADDR(a) = (v))
#define sdmmc_readw(a)     (*(volatile U16_T*)IO_MAPADDR(a))

#define SDMMC_OUTREG32(x, y) sdmmc_writel((u32)(y), x)
#define SDMMC_INREG16(x)     sdmmc_readw(x)

/* Make sure riu bit operation enabled, or the effect of SETREG16_BIT_OP is the same as SETREG16 */
#define SDMMC_SETREG16_BIT_OP(x, y) SDMMC_OUTREG32(x, SDMMC_INREG16(x) | (y) | (U32_T)(y) << 16)
/* Make sure riu bit operation enabled, or the effect of CLRREG16_BIT_OP is the same as CLRREG16 */
#define SDMMC_CLRREG16_BIT_OP(x, y) SDMMC_OUTREG32(x, (SDMMC_INREG16(x) & ~(y)) | (U32_T)(y) << 16)

#define CARD_REG_W(Reg_Addr, Value)      CARD_REG(Reg_Addr) = (Value)
#define CARD_REG_R(Reg_Addr, Value)      Value = CARD_REG(Reg_Addr)
#define CARD_REG_SETBIT(Reg_Addr, Value) SDMMC_SETREG16_BIT_OP(Reg_Addr, Value)
#define CARD_REG_CLRBIT(Reg_Addr, Value) SDMMC_CLRREG16_BIT_OP(Reg_Addr, Value)

#define CARD_BANK(Bank_Addr) IO_MAPADDR(Bank_Addr)

void           Hal_CREG_SET_REG_BANK(IpOrder eIP, struct sstar_mmc_priv* p_mmc_priv);
volatile void* Hal_CREG_GET_REG_BANK(IpOrder eIP);
void           Hal_CREG_SET_PLL_REG_BANK(IpOrder eIP, struct sstar_mmc_priv* p_mmc_priv);
volatile void* Hal_CREG_GET_PLL_REG_BANK(IpOrder eIP);
void           Hal_CREG_SET_CIFD_REG_BANK(IpOrder eIP, struct sstar_mmc_priv* p_mmc_priv);
volatile void* Hal_CREG_GET_CIFD_REG_BANK(IpOrder eIP);
void           Hal_CREG_SET_PWR_SAVE_REG_BANK(IpOrder eIP, struct sstar_mmc_priv* p_mmc_priv);
volatile void* Hal_CREG_GET_PWR_SAVE_REG_BANK(IpOrder eIP);

#define GET_CARD_BANK     Hal_CREG_GET_REG_BANK
#define GET_PLL_BANK      Hal_CREG_GET_PLL_REG_BANK
#define GET_CIFD_BANK     Hal_CREG_GET_CIFD_REG_BANK
#define GET_PWR_SAVE_BANK Hal_CREG_GET_PWR_SAVE_REG_BANK

//***********************************************************************************************************
// (3) BASE FCIE Reg Meaning Position (Including File)
//***********************************************************************************************************

//###########################################################################################################
#include "hal_sdmmc_regs_v5.h"
//###########################################################################################################

#endif // End of __HAL_SDMMC_REGS_H
