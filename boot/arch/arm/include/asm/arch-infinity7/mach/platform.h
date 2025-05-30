/*
* platform.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: nick.lin <nick.lin@sigmastar.com.tw>
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

#ifndef _ASM_ARCH_PLATFORM_H_
#define _ASM_ARCH_PLATFORM_H_

//------------------------------------------------------------------------------
//  Include Files
//------------------------------------------------------------------------------
#include "asm/arch/mach/sstar_types.h"
#include <asm/arch/mach/io.h>

//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------

// Register macros
#define REG(Reg_Addr)				(*(volatile U16*)(Reg_Addr))
#define GET_REG_ADDR(x, y)			((x) + ((y) << 2))
#define GET_BASE_ADDR_BY_BANK(x, y)	((x) + ((y) << 1))

//------------------------------------------------------------------------------
//
//  Macros:  INREGx/OUTREGx/SETREGx/CLRREGx
//
//  This macros encapsulates basic I/O operations.
//  Memory address space operation is used on all platforms.
//
#define INREG8(x)				sstar_readb(x)
#define OUTREG8(x, y)			sstar_writeb((u8)(y), x)
#define SETREG8(x, y)			OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)			OUTREG8(x, INREG8(x)&~(y))
#define INREGMSK8(x, y)			(INREG8(x) & (y))
#define OUTREGMSK8(x, y, z)		OUTREG8(x, ((INREG8(x)&~(z))|((y)&(z))))

#define INREG16(x)				sstar_readw(x)
#define OUTREG16(x, y)			sstar_writew((u16)(y), x)
#define SETREG16(x, y)			OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)			OUTREG16(x, INREG16(x)&~(y))
#define INREGMSK16(x, y)		(INREG16(x) & (y))
#define OUTREGMSK16(x, y, z)	OUTREG16(x, ((INREG16(x)&~(z))|((y)&(z))))

/* Make sure riu bit operation enabled, or the effect of SETREG16_BIT_OP is the same as SETREG16 */
#define SETREG16_BIT_OP(x, y)       OUTREG32(x, INREG16(x)|(y)|(u32)(y)<<16)
/* Make sure riu bit operation enabled, or the effect of CLRREG16_BIT_OP is the same as CLRREG16 */
#define CLRREG16_BIT_OP(x, y)       OUTREG32(x, (INREG16(x)&~(y))|(u32)(y)<<16)
/* Make sure riu bit operation enabled, or the effect of OUTREGMSK16_BIT_OP is the same as OUTREGMSK16 */
#define OUTREGMSK16_BIT_OP(x, y, z) OUTREG32(x, ((INREG16(x)&~(z))|((y)&(z)))|(u32)(z)<<16)

#define INREG32(x)				sstar_readl(x)
#define OUTREG32(x, y)			sstar_writel((u32)(y), x)
#define SETREG32(x, y)			OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)			OUTREG32(x, INREG32(x)&~(y))
#define INREGMSK32(x, y)		(INREG32(x) & (y))
#define OUTREGMSK32(x, y, z)	OUTREG32(x, ((INREG32(x)&~(z))|((y)&(z))))

#define XTAL_26000K	26000000
#define XTAL_24000K	24000000
#define XTAL_16369K	16369000
#define XTAL_16367K	16367000

//-----------------------------------------------------------------------------

// Chip reboot type
#define SSTAR_REBOOT_BY_WDT_RST	1
#define SSTAR_REBOOT_BY_SW_RST	2
#define SSTAR_REBOOT_BY_HW_RST	3

// Chip revisions
#define SSTAR_REVISION_U01		(0x0)
#define SSTAR_REVISION_U02		(0x1)
#define SSTAR_REVISION_U03		(0x2)


//------------------------------------------------------------------------------
//
//  Macros:  TYPE_CAST
//
//  This macros interprets the logic of type casting and shows the old type and
//  the new type of the casted variable.
//
#define TYPE_CAST(OldType, NewType, Var)    ((NewType)(Var))

//------------------------------------------------------------------------------
//
//  Macros:  SSTAR_ASSERT
//
//  This macro implements the assertion no matter in Debug or Release build.
//
#define SSTAR_ASSERT_PRINT(exp,file,line) printk(("\r\n*** ASSERTION FAILED in ") (file) ("(") (#line) ("):\r\n") (#exp) ("\r\n"))
#define SSTAR_ASSERT_AT(exp,file,line) (void)( (exp) || (SSTAR_ASSERT_PRINT(exp,file,line), 0 ) )
#define SSTAR_ASSERT(exp) SSTAR_ASSERT_AT(exp,__FILE__,__LINE__)

//------------------------------------------------------------------------------
//
//  Define:  SSTAR_BASE_REG_RIU_PA
//
//  Locates the RIU register base.
//
#define SSTAR_BASE_REG_RIU_PA		(0x1F000000)
#define SSTAR_BASE_REG_IMI_PA		(0xA0000000)
#define BASE_REG_PMPOR_PA			GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x0600)
#define BASE_REG_WDT_PA				GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x3000)
#define SSTAR_BASE_REG_UART0_PA		GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x110800)
#define SSTAR_BASE_REG_TIMER0_PA	GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x003020)
#define SSTAR_BASE_REG_TIMER2_PA	GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x003000)
#define SSTAR_BASE_REG_CHIPTOP_PA	GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x101E00)
#define REG_ADDR_BASE_WDT			GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x003000)
#define WDT_WDT_CLR					(0x00)
#define WDT_DUMMY_REG_1				(0x01)
#define WDT_RST_RSTLEN				(0x02)
#define WDT_INTR_PERIOD				(0x03)
#define WDT_MAX_PRD_L				(0x04)
#define WDT_MAX_PRD_H				(0x05)

#define REG_ADDR_BASE_AESDMA		GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x112200)
#define REG_ADDR_BASE_L3_BRIDGE		GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x102200)
#define REG_ADDR_BASE_PADTOP		GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x103C00)
#define SSTAR_BASE_REG_DID_KEY_PA	GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x003800)
#define REG_ADDR_BASE_PM_SLEEP		GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x000E00)
#define REG_ADDR_BASE_MIU			GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x101200)
#define REG_ADDR_BASE_PMPOR			GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x000600)
#define BASE_REG_OTP2_PA            GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x101F00)


#define SSTAR_SPI_ADDR				(0x14000000)
#define SSTAR_SPI_BOOT_ROM_SIZE		(0x00010000)
#define SSTAR_SPI_UBOOT_SIZE		(0x00080000)

#define BK_REG(reg)					((reg) << 2)
#define REG_ID_00					BK_REG(0x00)
#define REG_ID_01					BK_REG(0x01)
#define REG_ID_02					BK_REG(0x02)
#define REG_ID_03					BK_REG(0x03)
#define REG_ID_04					BK_REG(0x04)
#define REG_ID_05					BK_REG(0x05)
#define REG_ID_06					BK_REG(0x06)
#define REG_ID_07					BK_REG(0x07)
#define REG_ID_08					BK_REG(0x08)
#define REG_ID_09					BK_REG(0x09)
#define REG_ID_0A					BK_REG(0x0A)
#define REG_ID_0B					BK_REG(0x0B)
#define REG_ID_0C					BK_REG(0x0C)
#define REG_ID_0D					BK_REG(0x0D)
#define REG_ID_0E					BK_REG(0x0E)
#define REG_ID_0F					BK_REG(0x0F)

#define REG_ID_7C                   BK_REG(0x7C)
#define REG_ID_7D                   BK_REG(0x7D)
#define REG_ID_7E                   BK_REG(0x7E)
#define REG_ID_7F                   BK_REG(0x7F)

typedef enum
{
	DEVINFO_BOOT_TYPE_NONE=0x00,
	DEVINFO_BOOT_TYPE_NAND,
	DEVINFO_BOOT_TYPE_SPI,
	DEVINFO_BOOT_TYPE_EMMC,
	DEVINFO_BOOT_TYPE_SPINAND_EXT_ECC,
	DEVINFO_BOOT_TYPE_SPINAND_INT_ECC,
	DEVINFO_BOOT_TYPE_SDCARD,
} DEVINFO_BOOT_TYPE;

typedef enum
{
	/*
		DEVINFO_313E=0x00,
		DEVINFO_318,
	*/
	DEVINFO_NON,
}DEVINFO_CHIP_TYPE;

#define MIU0_START_ADDR			PHYS_SDRAM_1
#define REG_ADDR_STATUS			(SSTAR_BASE_REG_CHIPTOP_PA + (0x21 << 2)) //sync with IPL

/*==========================================================================
  MXPT 2015/10/14 updated
  ===========================================================================*/
#define IPL_CUST_OFFSET			0x10000 //ROM(16K) + IPL(48K)
#define IPL_CUST_MAX_SIZE		0xF000  //60K
#define IPL_CUST_LOAD_ADDRESS	(MIU0_START_ADDR + 0x03C00000)
#define IPL_CUST_BIT			0x1

#define KEY_CUST_OFFSET			0x1F000 //ROM(16K) + IPL(48K) + IPL_CUST(60K)
#define KEY_CUST_MAX_SIZE		0x1000  //4K
#define KEY_CUST_LOAD_ADDRESS	(MIU0_START_ADDR + 0x03FFF000)
#define KEY_CUST_BIT			0x2

/* DO NOT MODIFY MXPT_OFFSET */
#define MXPT_OFFSET				0x20000 //ROM(16K) + IPL(48K) + IPL_CUST(60K) + KEY_CUST(4K)
#define MXPT_MAX_SIZE			0x1000  //4K

#define ZBOOT_OFFSET			0x21000 //ROM(16K) + IPL(48K) + IPL_CUST(60K) + KEY_CUST(4K) + MXPT(4K)
#define ZBOOT_MAX_SIZE			0x10000 //64K
#define ZBOOT_MAX_DECOMP_SIZE	0x100000//1M
#define ZBOOT_LOAD_ADDRESS		(MIU0_START_ADDR + 0x03D00000)
#define ZBOOT_RUN_ADDRESS		(MIU0_START_ADDR + 0x03E00000)
#define ZBOOT_BIT				0x4

/* RESERVED to 0x40000 */

#define UBOOT_OFFSET			0x40000
#define UBOOT_MAX_SIZE			0x3F000 //252K
#define UBOOT_MAX_DECOMP_SIZE	0x100000//1M
#define UBOOT_BIT				0x8

#define UBOOT_ENV_OFFSET		0x7F000 //0x40000 + UBOOT(252K)
#define UBOOT_ENV_MAX_SIZE		0x1000  //4K

#define DTB_OFFSET				0x80000 //0x40000 + UBOOT(252K) + UBOOT_ENV(4K)
#define DTB_MAX_SIZE			0x10000 //64K
#define DTB_LOAD_ADDRESS		(MIU0_START_ADDR + 0x03FD0000)

#define KERNEL_OFFSET			0x90000  //0x40000 + UBOOT(124K) + UBOOT_ENV(4K) + DTB(64K)
#define KERNEL_MAX_SIZE			0x280000 //2.5M
#define KERNEL_MAX_DECOMP_SIZE	0x500000 //5M
#define KERNEL_LOAD_ADDRESS		(MIU0_START_ADDR + 0x03800000)
#define KERNEL_RUN_ADDRESS		(MIU0_START_ADDR + 0x00008000)
#define KERNEL_BIT				0x10


#endif /* _ASM_ARCH_PLATFORM_H_ */

