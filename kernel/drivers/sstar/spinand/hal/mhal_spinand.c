/*
 * mhal_spinand.c- Sigmastar
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
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include "mhal_spinand.h"
#include "reg_spinand.h"
#include "mdrv_spinand_command.h"
#include "mdrv_spinand_common.h"
#include <ms_platform.h>
#include <ms_msys.h>
//	#include "registers.h"
//	#include "registers.h"
//	#include "chip_int.h"
//	#include "chip_setup.h"
#define spi_nand_err(fmt, ...) printk(KERN_ERR "%s:error, " fmt "\n", __func__, ##__VA_ARGS__)
MS_U32 BASE_SPI_OFFSET = 0;

// extern struct device *spi_nand_dev;

//-------------------------------------------------------------------------------------------------
//  Macro definition
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
typedef struct
{
    U8  u8Clk;
    U16 eClkCkg;
} hal_clk_ckg_t;

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
#ifdef CONFIG_NAND_SINGLE_READ
SPINAND_MODE gNandReadMode = E_SPINAND_SINGLE_MODE;
#endif

#ifdef CONFIG_NAND_DUAL_READ
SPINAND_MODE gNandReadMode = E_SPINAND_DUAL_MODE;
#endif

#if defined(CONFIG_NAND_QUAL_READ) || defined(CONFIG_NAND_QUAL_WRITE)
SPINAND_MODE gNandReadMode = E_SPINAND_QUAD_MODE;
#endif

hal_fsp_t _hal_fsp = {
    .u32FspBaseAddr  = I3_RIU_PM_BASE + BK_FSP,
    .u32QspiBaseAddr = I3_RIU_PM_BASE + BK_QSPI,
    .u32PMBaseAddr   = I3_RIU_PM_BASE + BK_PMSLP,
    .u32CLK0BaseAddr = I3_RIU_BASE + BK_CLK0,
    .u32CHIPBaseAddr = I3_RIU_BASE + BK_CHIP,
    .u32RiuBaseAddr  = I3_RIU_PM_BASE,
    .u32BDMABaseAddr = I3_RIU_BASE + BK_BDMA,
};

//
//  Spi  Clk Table (List)
//
static hal_clk_ckg_t _hal_ckg_spi_pm[] = {{12, PM_SPI_CLK_XTALI}, {27, PM_SPI_CLK_27MHZ},  {36, PM_SPI_CLK_36MHZ},
                                          {43, PM_SPI_CLK_43MHZ}, {54, PM_SPI_CLK_54MHZ},  {72, PM_SPI_CLK_72MHZ},
                                          {86, PM_SPI_CLK_86MHZ}, {108, PM_SPI_CLK_108MHZ}};

static hal_clk_ckg_t _hal_ckg_spi_nonpm[] = {
    {12, CLK0_CKG_SPI_XTALI}, {27, CLK0_CKG_SPI_27MHZ}, {36, CLK0_CKG_SPI_36MHZ}, {43, CLK0_CKG_SPI_43MHZ},
    {54, CLK0_CKG_SPI_54MHZ}, {72, CLK0_CKG_SPI_72MHZ}, {86, CLK0_CKG_SPI_86MHZ}, {108, CLK0_CKG_SPI_108MHZ}};

U8 _u8SPINANDDbgLevel;

static struct bdma_alloc_dmem
{
    dma_addr_t  bdma_phy_addr;
    const char *DMEM_BDMA_INPUT;
    u8 *        bdma_vir_addr;
} ALLOC_DMEM = {0, "BDMA", 0};

static void *alloc_dmem(const char *name, unsigned int size, dma_addr_t *addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name, name, strlen(name) + 1);
    dmem.length = size;
    if (0 != msys_request_dmem(&dmem))
    {
        return NULL;
    }
    *addr = dmem.phys;
    return (void *)((uintptr_t)dmem.kvirt);
}
//
//	static void free_dmem(const char* name, unsigned int size, void *virt, dma_addr_t addr )
//	{
//		MSYS_DMEM_INFO dmem;
//		memcpy(dmem.name,name,strlen(name)+1);
//		dmem.length=size;
//		dmem.kvirt=(unsigned long long)((uintptr_t)virt);
//		dmem.phys=(unsigned long long)((uintptr_t)addr);
//		msys_release_dmem(&dmem);
//	}

//	static void _ms_bdma_mem_free(U32 u32DataSize)
//	{
//	    if(ALLOC_DMEM.bdma_vir_addr != 0){
//	       free_dmem(ALLOC_DMEM.DMEM_BDMA_INPUT, u32DataSize, ALLOC_DMEM.bdma_vir_addr, ALLOC_DMEM.bdma_phy_addr);
//	       ALLOC_DMEM.bdma_vir_addr = 0;
//	       }
//	}

BOOL _gbRIURead = FALSE;

static BOOL FSP_WRITE_BYTE(U32 u32RegAddr, U8 u8Val)
{
    if (!u32RegAddr)
    {
        printk(KERN_ERR "%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    ((volatile U8 *)(_hal_fsp.u32FspBaseAddr))[(u32RegAddr << 1) - (u32RegAddr & 1)] = u8Val;
    return TRUE;
}

static BOOL FSP_WRITE(U32 u32RegAddr, U16 u16Val)
{
    if (!u32RegAddr)
    {
        printk(KERN_ERR "%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    ((volatile U16 *)(_hal_fsp.u32FspBaseAddr))[u32RegAddr] = u16Val;
    return TRUE;
}

static U8 FSP_READ_BYTE(U32 u32RegAddr)
{
    return ((volatile U8 *)(_hal_fsp.u32FspBaseAddr))[(u32RegAddr << 1) - (u32RegAddr & 1)];
}

static U16 FSP_READ(U32 u32RegAddr)
{
    return ((volatile U16 *)(_hal_fsp.u32FspBaseAddr))[u32RegAddr];
}

static void _HAL_SPINAND_BDMA_INIT(U32 u32DataSize)
{
    if (!(ALLOC_DMEM.bdma_vir_addr = alloc_dmem(ALLOC_DMEM.DMEM_BDMA_INPUT, u32DataSize, &ALLOC_DMEM.bdma_phy_addr)))
    {
        printk("[input]unable to allocate aesdma memory\n");
    }

    memset(ALLOC_DMEM.bdma_vir_addr, 0, u32DataSize);
}

static BOOL _HAL_FSP_ChkWaitDone(void)
{
    // consider as it spend very long time to check if FSP done, so it may implment timeout method to improve
    U16 u16Try     = 0;
    U8  u8DoneFlag = 0;

    while (u16Try < CHK_NUM_WAITDONE)
    {
        u8DoneFlag = FSP_READ(REG_FSP_DONE);
        if ((u8DoneFlag & DONE_FSP) == DONE_FSP)
        {
            return TRUE;
        }

        if (++u16Try % 1000 == 0)
            cond_resched();

        udelay(1);
    }
    return FALSE;
}

static void _HAL_FSP_GetRData(U8 *pu8Data, U8 u8DataSize)
{
    U8 u8Index = 0;
    // printk(KERN_ERR"_HAL_FSP_GetRData %lx\r\n",(U32)u8DataSize);
    for (u8Index = 0; u8Index < u8DataSize; u8Index++)
    {
        pu8Data[u8Index] = FSP_READ_BYTE(REG_FSP_READ_BUFF + u8Index);
    }
}

static U32 _HAL_FSP_CHECK_SPINAND_DONE(U8 *pu8Status)
{
    U8  u8Data = SPI_NAND_STAT_OIP;
    U16 u16Try = 0;

    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_GF);
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, SPI_NAND_REG_STAT);
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x2);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x1);

    while (1) // while(u16Try < CHK_NUM_WAITDONE)
    {
        // Trigger FSP
        FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

        // Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE)
        {
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "CD Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
        }

        // Clear FSP done flag
        FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

        _HAL_FSP_GetRData(&u8Data, 1);
        if ((u8Data & SPI_NAND_STAT_OIP) == 0)
            break;

        if (++u16Try % 1000 == 0)
            cond_resched();

        udelay(1);
    }

    if (u16Try == CHK_NUM_WAITDONE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "CD Wait OIP Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    if (pu8Status)
        *pu8Status = u8Data;

    return ERR_SPINAND_SUCCESS;
}

static U32 _HAL_SPINAND_WRITE_ENABLE(void)
{
    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP Read Command
    //  FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_WREN);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x1);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 0);
    // Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "WE Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    return ERR_SPINAND_SUCCESS;
}

U32 HAL_SPINAND_RIU_READ(U16 u16Addr, U32 u32DataSize, U8 *u8pData)
{
    U32 u32Index      = 0;
    U8  u8Addr        = 0;
    U32 u32RealLength = 0;
    U8  u8WbufIndex   = 0;
    S8  s8Index       = 0;

    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP Read Command
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_RFC);
    u8WbufIndex++;

    // Set Write & Read Length
    FSP_WRITE(REG_FSP_WRITE_SIZE, 4); // add 4 because it is included of command and Address setting length
    if (u32DataSize > MAX_READ_BUF_CNT)
        FSP_WRITE(REG_FSP_READ_SIZE, MAX_READ_BUF_CNT);
    else
        FSP_WRITE(REG_FSP_READ_SIZE, u32DataSize);

    for (u32Index = 0; u32Index < u32DataSize; u32Index += MAX_READ_BUF_CNT)
    {
        for (s8Index = (SPI_NAND_PAGE_ADDR_LEN - 1); s8Index >= 0; s8Index--)
        {
            u8Addr = (u16Addr >> (8 * s8Index)) & 0xFF;
            FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u8Addr);
            u8WbufIndex++;
        }
        // set dummy byte
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), 0x00);
        // Trigger FSP
        FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

        // Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE)
        {
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "RIUR Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
        }
        // Get Read Data
        u32RealLength = u32DataSize - u32Index;
        if (u32RealLength >= MAX_READ_BUF_CNT)
            u32RealLength = MAX_READ_BUF_CNT;

        _HAL_FSP_GetRData((u8pData + u32Index), u32RealLength);

        // Clear FSP done flag
        FSP_WRITE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
        // update Read Start Address
        u16Addr += u32RealLength;
        u8WbufIndex -= SPI_NAND_PAGE_ADDR_LEN;
    }
    return ERR_SPINAND_SUCCESS;
}

static U32 _HAL_SPINAND_BDMA_READ(U16 u16Addr, U32 u32DataSize, U8 *u8pData)
{
    U16 u16data  = 0;
    U32 u32Timer = 0;
    U32 u32Ret   = ERR_SPINAND_TIMEOUT;

    // Set source and destination path
    BDMA_WRITE(0x00, 0x0000);

    //	    u32Addr1 = dma_map_single(spi_nand_dev, (void*)u8pData, u32DataSize, DMA_FROM_DEVICE);

    BDMA_WRITE(0x02, 0X4035); // 5:source device (spi)
                              // 3:source device data width (8 bytes)
                              // 0:destination device (channel0)
                              // 4:destination device data width (16 bytes)

    BDMA_WRITE(0x03, 0X0000); // 0: channel0 => MIU0

    // Set start address
    BDMA_WRITE(0x04, (u16Addr & 0x0000FFFF));
    BDMA_WRITE(0x05, (u16Addr >> 16));

    // Set end address
    BDMA_WRITE(0x06, (Chip_Phys_to_MIU(ALLOC_DMEM.bdma_phy_addr) & 0x0000FFFF));
    BDMA_WRITE(0x07, (Chip_Phys_to_MIU(ALLOC_DMEM.bdma_phy_addr) >> 16));

    // Set Size
    BDMA_WRITE(0x08, (u32DataSize & 0x0000FFFF));
    BDMA_WRITE(0x09, (u32DataSize >> 16));

    // Trigger
    BDMA_WRITE(0x00, 1);

    do
    {
        // check done
        u16data = BDMA_READ(0x01);
        if (u16data & 8)
        {
            // clear done
            BDMA_WRITE(0x01, 8);
            u32Ret = ERR_SPINAND_SUCCESS;
            break;
        }
        if (++u32Timer % 1000 == 0)
            cond_resched();

        udelay(1);
    } while (u32Timer < CHK_NUM_WAITDONE);

    //	    dma_unmap_single(spi_nand_dev, u32Addr1, u32DataSize, DMA_FROM_DEVICE);
    memcpy(u8pData, ALLOC_DMEM.bdma_vir_addr, u32DataSize);

    //	    _ms_bdma_mem_free(u32DataSize);

    return u32Ret;
}

void HAL_SPINAND_SetNormalMode(void)
{
    QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN | REG_SPI_DUMMY_CYC_SINGLE);
    QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_NORMAL_MODE);
}

void HAL_SPINAND_AssertNormalMode(void)
{
    U16  u16Reg;
    BOOL bBug = FALSE;

    u16Reg = QSPI_READ(REG_SPI_CKG_SPI);
    if ((u16Reg & REG_SPI_USER_DUMMY_EN) && ((u16Reg & 0xF) != REG_SPI_DUMMY_CYC_SINGLE))
    {
        printk("Invalid REG_SPI_CKG_SPI %X\n", u16Reg);
        bBug = TRUE;
    }
    u16Reg = QSPI_READ(REG_SPI_MODE_SEL);
    if (((u16Reg & 0xF) != REG_SPI_NORMAL_MODE) && ((u16Reg & 0xF) != REG_SPI_FAST_READ))
    {
        printk("Invalid REG_SPI_MODE_SEL %X\n", u16Reg);
        bBug = TRUE;
    }
    if (bBug)
    {
        dump_stack();
        HAL_SPINAND_SetNormalMode();
    }
}

void HAL_SPINAND_PreHandle(SPINAND_MODE eMode)
{
    U8  u8Status = 0;
    U16 u16Reg;

    HAL_SPINAND_SetNormalMode();

    u16Reg = QSPI_READ(REG_SPI_CKG_SPI);
    if ((u16Reg & 0xF) != REG_SPI_DUMMY_CYC_SINGLE)
    {
        printk("Invalid REG_SPI_CKG_SPI %X\n", u16Reg);
        BUG();
    }
    u16Reg = QSPI_READ(REG_SPI_MODE_SEL);
    if ((u16Reg & 0xF) != REG_SPI_NORMAL_MODE)
    {
        printk("Invalid REG_SPI_MODE_SEL %X\n", u16Reg);
        BUG();
    }
    // if (_gtSpinandInfo.au8_ID[0] == 0xC8)
    {
        switch (eMode)
        {
            case E_SPINAND_SINGLE_MODE:
            case E_SPINAND_FAST_MODE:
            case E_SPINAND_DUAL_MODE:
            case E_SPINAND_DUAL_MODE_IO: // GD support If QE is enabled, the quad IO operations can be executed.
                HAL_SPINAND_ReadStatusRegister(&u8Status, SPI_NAND_REG_FEAT); // output:u8Status is OTP data.
                if (u8Status & QUAD_ENABLE)
                {
                    u8Status &= ~(QUAD_ENABLE);
                    HAL_SPINAND_WriteStatusRegister(&u8Status, SPI_NAND_REG_FEAT);
                }
                break;
            case E_SPINAND_QUAD_MODE:
            case E_SPINAND_QUAD_MODE_IO: // GD support
                HAL_SPINAND_ReadStatusRegister(&u8Status, SPI_NAND_REG_FEAT);
                if ((u8Status & QUAD_ENABLE) == 0)
                {
                    u8Status |= QUAD_ENABLE;
                    HAL_SPINAND_WriteStatusRegister(&u8Status, SPI_NAND_REG_FEAT);
                }
                break;
        }
    }
}

U32 HAL_SPINAND_Init(void)
{
    U8 u8Status = 0;

    HAL_SPINAND_Chip_Config();

    // set pad mux for spinand
    //	    printk("MDrv_SPINAND_Init: Set pad mux\n");
    CHIP_WRITE(0x50, 0x000); // disable all pad in
    QSPI_WRITE(0x7A, 0x00);  // CS
    PM_WRITE(0x35, 0x08);
    if (BDMA_FLAG)
        _HAL_SPINAND_BDMA_INIT(2048 + 64);
    //	    printk("MDrv_SPINAND_Init: Set pad mux end");

    // chip section timeout
    QSPI_WRITE(0x66, 0x000F);
    QSPI_WRITE(0x67, 0x8000);

    // reset spinand
    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_RESET);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 1);
    FSP_WRITE(REG_FSP_READ_SIZE, 0);

    // Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "INI Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    // For first RESET condition after power up, tRST will be 1ms maximum
    return _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);
}

void HAL_SPINAND_Config(U32 u32PMRegBaseAddr, U32 u32NonPMRegBaseAddr)
{
    DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printk(KERN_INFO "%s(0x%08X, 0x%08X)\n", __FUNCTION__, (int)u32PMRegBaseAddr,
                                                (int)u32NonPMRegBaseAddr));
    _hal_fsp.u32FspBaseAddr  = u32PMRegBaseAddr + BK_FSP;
    _hal_fsp.u32PMBaseAddr   = u32PMRegBaseAddr + BK_PMSLP;
    _hal_fsp.u32QspiBaseAddr = u32PMRegBaseAddr + BK_QSPI;
    _hal_fsp.u32CLK0BaseAddr = u32NonPMRegBaseAddr + BK_CLK0; // BK_CLK0;
    _hal_fsp.u32BDMABaseAddr = u32NonPMRegBaseAddr + BK_BDMA;
    _hal_fsp.u32RiuBaseAddr  = u32PMRegBaseAddr;
}

BOOL HAL_SPINAND_PLANE_HANDLER(U32 u32Addr)
{
    if (PLANE && (((u32Addr / BLOCK_PAGE_SIZE) & 0x1) == 1)) // odd numbered blocks
    {
        u32Addr = QSPI_READ(REG_SPI_WRAP_VAL);
        u32Addr |= (1 << REG_SPI_WRAP_BIT_OFFSET);
        QSPI_WRITE(REG_SPI_WRAP_VAL, u32Addr);
    }
    else
    {
        u32Addr = QSPI_READ(REG_SPI_WRAP_VAL);
        u32Addr &= ~(1 << REG_SPI_WRAP_BIT_OFFSET);
        QSPI_WRITE(REG_SPI_WRAP_VAL, u32Addr);
    }
    return TRUE;
}

void HAL_SPINAND_DieSelect(U8 u8Die)
{
    if (u8Die != 0) // only 2 die
        u8Die = 1;

    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_DIESELECT);
    // Set Start Address
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, u8Die);
    // Set Write & Read Length
    FSP_WRITE(REG_FSP_WRITE_SIZE, 2);
    FSP_WRITE(REG_FSP_READ_SIZE, 0);

    // Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk("RID Wait FSP Done Time Out !!!!\r\n"));
    }

    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
}

U32 HAL_SPINAND_RFC(U32 u32Addr, U8 *pu8Data)
{
    U8 u8Addr      = 0;
    U8 u8WbufIndex = 0;
    S8 s8Index     = 0;

    // DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printk("HAL_SPINAND_RFC : u32Addr = %lx \r\n",u32Addr));
    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP | ENABLE_SEC_CMD));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP Read Command
    //  FIRSET COMMAND PRELOAD
    //    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_WREN);
    //    u8WbufIndex++;

    // SECOND COMMAND READ COMMAND + 3BYTE ADDRESS
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_PGRD);
    u8WbufIndex++;
    // set Read Start Address
    for (s8Index = (SPI_NAND_ADDR_LEN - 1); s8Index >= 0; s8Index--)
    {
        u8Addr = (u32Addr >> (8 * s8Index)) & 0xFF;
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u8Addr);
        u8WbufIndex++;
    }

    // THIRD COMMAND GET FATURE CHECK CAHCHE READY
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_GF);
    u8WbufIndex++;
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_REG_STAT); // read
    u8WbufIndex++;

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x024); // 17-bit block/page address

    FSP_WRITE(REG_FSP_READ_SIZE, 0x010); // 1 byte (receive from get feature)

    // Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "RFC Wait FSP Done Time Out %lx !!!!\r\n", u32Addr));
        return ERR_SPINAND_TIMEOUT;
    }

    _HAL_FSP_GetRData((pu8Data), 1);
    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    return _HAL_FSP_CHECK_SPINAND_DONE(pu8Data);
}

static U32 _HAL_SPINAND_BDMA_Write(U32 u32DataSize)
{
    U32 u32Ret = ERR_SPINAND_SUCCESS;

    if (BDMA_READ(0x00) & 1)
    {
        BDMA_WRITE(0x00, 0x10);
        if (BDMA_READ(0x00) & 0x10)
        {
            BDMA_WRITE(0x00, 0x00);
        }
    }
    // Set source and destination path
    BDMA_WRITE(0x00, 0x0000);
    BDMA_WRITE(0x02, 0X2B40); // 0:source device (channel0)
                              // 4:source device data width (16 bytes)
                              // B:destination device (MIU) 1:destination device (IMI)
                              // 2:destination device data width (16 bytes)
    BDMA_WRITE(0x03, 0x0000); // channel0: MIU0

    // Set start address
    BDMA_WRITE(0x04, (Chip_Phys_to_MIU(ALLOC_DMEM.bdma_phy_addr) & 0x0000FFFF));
    BDMA_WRITE(0x05, (Chip_Phys_to_MIU(ALLOC_DMEM.bdma_phy_addr) >> 16));

    // Set end address
    BDMA_WRITE(0x06, (0x0 & 0x0000FFFF));
    BDMA_WRITE(0x07, (0x0 >> 16));

    // Set Size
    BDMA_WRITE(0x08, (u32DataSize & 0xFFFF));
    BDMA_WRITE(0x09, (u32DataSize >> 16));

    // Trigger
    BDMA_WRITE(0x00, 1);

    return u32Ret;
}

U32 HAL_SPINAND_program_load_data(U16 u16_col_address, U8 *pu8_buf, U32 u32_size)
{
    U32 u32Ret      = ERR_SPINAND_TIMEOUT;
    U8  u8WbufIndex = 0;
    U32 u32WrteBuf  = REG_FSP_WRITE_BUFF;

    // DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printk("u16ColumnAddr %x u16DataSize %x Data %x \r\n", u16ColumnAddr,
    // u16DataSize, *pu8Data));
    //  Write Enable
    u32Ret = _HAL_SPINAND_WRITE_ENABLE();
    if (u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    // while(HAL_QSPI_FOR_DEBUG()==0);

    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP write Command
    //  FIRSET COMMAND PRELOAD

    FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), SPI_NAND_CMD_PP);
    u8WbufIndex++;
    FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), (u16_col_address >> 8) & 0xff);
    u8WbufIndex++;
    FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), u16_col_address & 0xff);
    u8WbufIndex++;

    FSP_WRITE(REG_FSP_WRITE_SIZE, u8WbufIndex);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x00);
    // Trigger FSP

    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "PL Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    u8WbufIndex = 0;

    QSPI_WRITE(REG_SPI_BURST_WRITE, REG_SPI_ENABLE_BURST);

    while (0 < u32_size)
    {
        while (SINGLE_WRITE_SIZE > u8WbufIndex)
        {
            if (u8WbufIndex == FSP_WRITE_BUF_JUMP_OFFSET)
            {
                u32WrteBuf = REG_FSP_WRITE_BUFF2;
            }

            FSP_WRITE_BYTE((u32WrteBuf + (u8WbufIndex % FSP_WRITE_BUF_JUMP_OFFSET)), *pu8_buf);
            // printk("0x%02x, ", *pu8_buf);
            pu8_buf++;
            u8WbufIndex++;
            u32_size--;

            if (0 == u32_size)
            {
                break;
            }
        }

        FSP_WRITE(REG_FSP_WRITE_SIZE, u8WbufIndex);
        FSP_WRITE(REG_FSP_READ_SIZE, 0x00);
        // Trigger FSP

        FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

        // Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE)
        {
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "PL Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
        }

        u8WbufIndex = 0;
        u32WrteBuf  = REG_FSP_WRITE_BUFF;
        // QSPI_WRITE(REG_SPI_BURST_WRITE,REG_SPI_ENABLE_BURST);
        // Clear FSP done flag
        FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    }

    QSPI_WRITE(REG_SPI_BURST_WRITE, REG_SPI_DISABLE_BURST);
    return _HAL_FSP_CHECK_SPINAND_DONE(NULL);
}

U32 HAL_SPINAND_PROGRAM_BY_BDMA(U16 u16ColumnAddr, U16 u16DataSize)
{
    U32 u32Ret      = ERR_SPINAND_TIMEOUT;
    U8  u8Addr      = 0;
    U8  u8WbufIndex = 0;
    S8  s8Index     = 0;
    U32 u32WrteBuf  = REG_FSP_WRITE_BUFF;
    U32 u32Timer    = 0;

    u32Ret = _HAL_SPINAND_WRITE_ENABLE();
    if (u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    _HAL_SPINAND_BDMA_Write(u16DataSize);

    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP write Command
#ifdef CONFIG_AUTO_DETECT_WRITE
    FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), SPI_NAND_CMD_PP);
#else
    FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), SPI_NAND_CMD_QPP);
#endif
    u8WbufIndex++;

    // PAGE Address
    for (s8Index = (SPI_NAND_PAGE_ADDR_LEN - 1); s8Index >= 0; s8Index--)
    {
        u8Addr = (u16ColumnAddr >> (8 * s8Index)) & 0xFF;
        FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), u8Addr);
        u8WbufIndex++;
    }

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x3);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x00);
    // set FSP Outside replace
    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, (u16DataSize + 1) & 0x00FFFFFF);
    FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x1003);
    // trigger
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    u32Ret = ERR_SPINAND_BDMA_TIMEOUT;
    // check BDMA don
    do
    {
        // check done
        if (BDMA_READ(0x01) & 8)
        {
            // clear done
            BDMA_WRITE(0x01, 8);
            u32Ret = ERR_SPINAND_SUCCESS;
            break;
        }
        if (++u32Timer % 1000 == 0)
            cond_resched();
        udelay(1);
    } while (u32Timer < CHK_NUM_WAITDONE);
    if (u32Ret != ERR_SPINAND_SUCCESS)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "BDMA Time Out !!!!\r\n"));
    }

    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "PL Wait FSP Done Time Out !!!!\r\n"));
        u32Ret = ERR_SPINAND_TIMEOUT;
    }
    // reset
    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, 0x0);
    FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x0);
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return u32Ret;
}

// 4:Quad mode version BDMA
U32 HAL_SPINAND_PROGRAM_BY_BDMA4(U16 u16ColumnAddr, U16 u16DataSize)
{
    U32 u32Ret      = ERR_SPINAND_TIMEOUT;
    U8  u8Addr      = 0;
    U8  u8WbufIndex = 0;
    S8  s8Index     = 0;
    U32 u32WrteBuf  = REG_FSP_WRITE_BUFF;
    U32 u32Timer    = 0;

    u32Ret = _HAL_SPINAND_WRITE_ENABLE();
    if (u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    //_HAL_SPINAND_BDMA_Write(u16DataSize);

    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP)); // h6c reg_fsp_ctrl0 = BIT0 | BIT1 | BIT2
    FSP_WRITE(REG_FSP_CTRL2, 0);                                 // h75 reg_fsp_ctrl2, reg_fsp_ctrl3 = 0
    FSP_WRITE(REG_FSP_CTRL4, 0);                                 // h76 reg_fsp_ctrl4, reg_spare_0 = 0

    // Set FSP write Command
#ifdef CONFIG_AUTO_DETECT_WRITE
    FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), SPI_NAND_CMD_PP);
#else
    FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), SPI_NAND_CMD_QPP);
    FSP_WRITE_BYTE(REG_FSP_QUAD_MODE, 0);
#endif
    u8WbufIndex++;

    // PAGE Address
    for (s8Index = (SPI_NAND_PAGE_ADDR_LEN - 1); s8Index >= 0; s8Index--)
    {
        u8Addr = (u16ColumnAddr >> (8 * s8Index)) & 0xFF;
        FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), u8Addr);
        u8WbufIndex++;
    }

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x3);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x0);

    // set FSP Outside replace
    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, 0x0);
    FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x0);
    // set FSP Outside replace
    // 0 : wbf0 = reg_fsp_wbf_size_outside + reg_fsp_wbf_size0
    // trigger
    QSPI_WRITE(REG_SPI_BURST_WRITE, 0);
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "PL Wait FSP Done Time Out !!!!\r\n"));
        u32Ret = ERR_SPINAND_TIMEOUT;
    }

    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, (u16DataSize + 1) & 0x0FFF);
    FSP_WRITE(REG_FSP_WBF_OUTSIDE,
              0x1000); // reg_fsp_wbf_outside_en = 1, reg_fsp_wbf_mode = 0, reg_fsp_wbf_replaced = 3.
    QSPI_WRITE(REG_SPI_BURST_WRITE, REG_SPI_ENABLE_BURST);
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x0);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x0);
    FSP_WRITE_BYTE(REG_FSP_QUAD_MODE, ENABLE_FSP_QUAD);
    _HAL_SPINAND_BDMA_Write(u16DataSize);
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // check BDMA done
    u32Ret = ERR_SPINAND_BDMA_TIMEOUT;
    do
    {
        // check done
        if (BDMA_READ(0x01) & 8)
        {
            // clear done
            BDMA_WRITE(0x01, 8);
            u32Ret = ERR_SPINAND_SUCCESS;
            break;
        }
        if (++u32Timer % 1000 == 0)
            cond_resched();
        udelay(1);
    } while (u32Timer < CHK_NUM_WAITDONE);
    if (u32Ret != ERR_SPINAND_SUCCESS)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "BDMA Time Out !!!!\r\n"));
    }

    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "PL Wait FSP Done Time Out !!!!\r\n"));
        u32Ret = ERR_SPINAND_TIMEOUT;
    }
    // reset
    FSP_WRITE(REG_FSP_WBF_SIZE_OUTSIDE, 0x0);
    FSP_WRITE(REG_FSP_WBF_OUTSIDE, 0x0);
    QSPI_WRITE(REG_SPI_BURST_WRITE, 0);
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x0);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x0);
#ifndef CONFIG_AUTO_DETECT_WRITE
    FSP_WRITE_BYTE(REG_FSP_QUAD_MODE, 0);
#endif
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return u32Ret;
}

U32 HAL_SPINAND_PROGRAM_LOAD_DATA(U16 u16ColumnAddr, U16 u16DataSize, U8 *pu8Data, U8 *pu8_SpareBuf)
{
    U32 u32Ret = ERR_SPINAND_TIMEOUT;
    U8  u8Addr = 0, u8DataIndex = 0;
    U16 u16RealLength = 0, u16FspWriteSize = SINGLE_WRITE_SIZE;
    U8  u8WbufIndex  = 0;
    S8  s8Index      = 0;
    U32 u32WrteBuf   = REG_FSP_WRITE_BUFF;
    U16 u16DataIndex = 0;
    U8 *pu8Wdata     = NULL;
    U8  u8Status     = 0;

    pu8Wdata = pu8Data;
    // DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printk("u16ColumnAddr %x u16DataSize %x Data %x \r\n", u16ColumnAddr,
    // u16DataSize, *pu8Data));
    //  Write Enable
    u32Ret = _HAL_SPINAND_WRITE_ENABLE();
    if (u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    // while(HAL_QSPI_FOR_DEBUG()==0);

    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP write Command
    //  FIRSET COMMAND PRELOAD
#ifdef CONFIG_AUTO_DETECT_WRITE
    FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), SPI_NAND_CMD_PP);
#else
    FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), SPI_NAND_CMD_QPP);
#endif
    FSP_WRITE(REG_FSP_READ_SIZE, 0x00);
    u8WbufIndex++;

    // PAGE Address
    for (s8Index = (SPI_NAND_PAGE_ADDR_LEN - 1); s8Index >= 0; s8Index--)
    {
        u8Addr = (u16ColumnAddr >> (8 * s8Index)) & 0xFF;
        FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), u8Addr);
        u8WbufIndex++;
    }

    FSP_WRITE(REG_FSP_READ_SIZE, 0);
#ifdef CONFIG_NAND_QUAL_WRITE
    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x003);
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "PL Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }
    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    u8WbufIndex = 0;
    FSP_WRITE_BYTE(REG_FSP_QUAD_MODE, ENABLE_FSP_QUAD);
#endif
    QSPI_WRITE(REG_SPI_BURST_WRITE, REG_SPI_ENABLE_BURST);

    for (u16DataIndex = 0; u16DataIndex < u16DataSize; u16DataIndex += u16RealLength)
    {
        u16RealLength = u16DataSize - u16DataIndex;
        if (u16RealLength >= SINGLE_WRITE_SIZE)
        {
            u16RealLength = SINGLE_WRITE_SIZE - u8WbufIndex;
        }
        else
        {
            u16FspWriteSize = u16RealLength;
        }

        // Write Data
        for (u8DataIndex = 0; u8DataIndex < u16RealLength; u8DataIndex++)
        {
            if ((u16DataIndex + u8DataIndex) == PAGE_SIZE)
            {
                pu8Wdata = pu8_SpareBuf;
            }

            // printk("u8DataIndex %x u16RealLength %x \r\n",u8DataIndex, u16RealLength);
            FSP_WRITE_BYTE((u32WrteBuf + u8WbufIndex), *pu8Wdata);
            u8WbufIndex++;
            pu8Wdata++;
            if (u8WbufIndex >= FSP_WRITE_BUF_JUMP_OFFSET)
            {
                u32WrteBuf  = REG_FSP_WRITE_BUFF2;
                u8WbufIndex = 0;
            }
        }

        FSP_WRITE(REG_FSP_WRITE_SIZE, u16FspWriteSize);
        // Trigger FSP

        FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

        // Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE)
        {
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "PL Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
        }

        u8WbufIndex = 0;
        u32WrteBuf  = REG_FSP_WRITE_BUFF;
        // Clear FSP done flag
        FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    }
    QSPI_WRITE(REG_SPI_BURST_WRITE, REG_SPI_DISABLE_BURST);
    FSP_WRITE_BYTE(REG_FSP_QUAD_MODE, 0);

    return _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);
}

U32 HAL_SPINAND_READ_STATUS(U8 *pu8Status, U8 u8Addr)
{
    U8 u8WbufIndex = 0;

    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP Read Command
    //  FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_GF);
    u8WbufIndex++;
    // SECOND COMMAND SET READ PARAMETER
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u8Addr);
    // write buffer size
    FSP_WRITE(REG_FSP_WRITE_SIZE, 2);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 1);

    // Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "RS Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_SUCCESS;
    }
    // Get Read Data
    _HAL_FSP_GetRData(pu8Status, 1);

    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return ERR_SPINAND_SUCCESS;
}

U32 HAL_SPINAND_BLOCKERASE(U32 u32_PageIdx)
{
    U8  u8WbufIndex = 0;
    S8  s8Index     = 0;
    U8  u8Addr      = 0;
    U32 u32Ret      = 0;
    U8  u8Status    = 0;

    u32Ret = _HAL_SPINAND_WRITE_ENABLE();
    if (u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    u32Ret = HAL_SPINAND_WriteProtect(FALSE);
    if (u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP | ENABLE_SEC_CMD | ENABLE_THR_CMD));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP Read Command
    //  FIRSET COMMAND PRELOAD
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_WREN);
    u8WbufIndex++;

    // SECOND COMMAND SET Erase Command
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_BE);
    u8WbufIndex++;

    // seet erase Start Address
    for (s8Index = (SPI_NAND_ADDR_LEN - 1); s8Index >= 0; s8Index--)
    {
        u8Addr = (u32_PageIdx >> (8 * s8Index)) & 0xFF;
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u8Addr);
        u8WbufIndex++;
    }

    // THIRD COMMAND GET FATURE CHECK CAHCHE READY
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_GF);
    u8WbufIndex++;
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_REG_STAT);
    u8WbufIndex++;

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x241);

    FSP_WRITE(REG_FSP_READ_SIZE, 0x100);

    // Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "BE Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    u32Ret = _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);
    if (u32Ret == ERR_SPINAND_SUCCESS)
        if (u8Status & E_FAIL)
            u32Ret = ERR_SPINAND_E_FAIL;
    u32Ret = HAL_SPINAND_WriteProtect(TRUE);

    return u32Ret;
}

U32 HAL_SPINAND_program(U32 u32_row_address, U16 u16_col_address, U8 *pu8_buf, U32 u32_size)
{
    U32 u32Ret;
    U8  u8WbufIndex = 0;
    U8  u8Status;

    u32Ret = HAL_SPINAND_WriteProtect(FALSE);
#if 0
#ifdef CONFIG_AUTO_DETECT_WRITE
        HAL_SPINAND_SetMode(WRITE_MODE);
#else
       //HAL_SPINAND_SetMode(gNandReadMode);
            HAL_SPINAND_PreHandle(E_SPINAND_QUAD_MODE_IO);
#endif
#endif
    if ((BLOCKCNT == DENSITY_2G) && (((u32_row_address / BLOCK_PAGE_SIZE) & 0x1) == 1))
        u16_col_address = (1 << 12); // plane select for MICRON

    u32Ret = HAL_SPINAND_program_load_data(u16_col_address, pu8_buf, u32_size);
    if (u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;

#ifdef CONFIG_NAND_QUAL_WRITE
    FSP_WRITE_BYTE(REG_FSP_QUAD_MODE, 0);
    HAL_SPINAND_PreHandle(E_SPINAND_SINGLE_MODE);
#endif
    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // FIRST COMMAND PAGE PROGRAM EXECUTE
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_PE);
    u8WbufIndex++;
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u32_row_address >> 16);
    u8WbufIndex++;
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), (u32_row_address >> 8 & 0Xff));
    u8WbufIndex++;
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u32_row_address & 0xff);
    u8WbufIndex++;

    FSP_WRITE(REG_FSP_WRITE_SIZE, u8WbufIndex);

    FSP_WRITE(REG_FSP_READ_SIZE, 0x000);
    // Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    u32Ret = _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);
    if (u32Ret == ERR_SPINAND_SUCCESS)
        if (u8Status & P_FAIL)
        {
            u32Ret = ERR_SPINAND_W_FAIL;
            spi_nand_err("P_FAIL!!!\n");
            return u32Ret;
        }
    u32Ret = HAL_SPINAND_WriteProtect(TRUE);

    return u32Ret;
}

U32 HAL_SPINAND_Write(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf)
{
    U32 u32Ret        = 0;
    U8  u8Addr        = 0;
    U8  u8WbufIndex   = 0;
    S8  s8Index       = 0;
    U16 u16DataSize   = 0;
    U8  u8Status      = 0;
    U16 u16ColumnAddr = 0;

    // calculate write data size
    u16DataSize = SPARE_SIZE + PAGE_SIZE;

    // DEBUG_SPINAND(E_SPINAND_DBGLV_DEBUG, printk("addr %lx u8Data %x u32PageIndex %lx\r\n", (U32)u8Data, *u8Data,
    // u32_PageIdx));
    u32Ret = HAL_SPINAND_WriteProtect(FALSE);

    if (u32Ret != ERR_SPINAND_SUCCESS)
        return u32Ret;
#if 0
#ifdef CONFIG_AUTO_DETECT_WRITE
    HAL_SPINAND_SetMode((SPINAND_MODE)WRITE_MODE);
#else
    //HAL_SPINAND_SetMode(gNandReadMode);
    //HAL_SPINAND_PreHandle(E_SPINAND_QUAD_MODE_IO);
    HAL_SPINAND_PreHandle(E_SPINAND_QUAD_MODE);
#endif
#endif
    if (PLANE && (((u32_PageIdx / BLOCK_PAGE_SIZE) & 0x1) == 1))
        u16ColumnAddr = (1 << 12); // plane select for MICRON

// under this condition, Load data would use QPP, which on some SPINAND device, it would require
// QE bit enabled, but BE bit could be cleared in UBOOT or Quad Read.
#if defined(CONFIG_NAND_QUAL_WRITE) && !defined(CONFIG_AUTO_DETECT_WRITE)
    HAL_SPINAND_PreHandle(E_SPINAND_QUAD_MODE);
#endif

    if (BDMA_W_FLAG && (PAGE_SIZE <= 2048)) // outsize bit number is insufficient for 4KB page, force using RIU mode
    {
        memcpy(ALLOC_DMEM.bdma_vir_addr, u8Data, PAGE_SIZE);
        memcpy(ALLOC_DMEM.bdma_vir_addr + PAGE_SIZE, pu8_SpareBuf, SPARE_SIZE);
        Chip_Flush_MIU_Pipe();
#if defined(CONFIG_NAND_QUAL_WRITE)
        u32Ret = HAL_SPINAND_PROGRAM_BY_BDMA4(u16ColumnAddr, u16DataSize);
#else
        u32Ret = HAL_SPINAND_PROGRAM_BY_BDMA(u16ColumnAddr, u16DataSize);
#endif
    }
    else
    {
        u32Ret = HAL_SPINAND_PROGRAM_LOAD_DATA(u16ColumnAddr, u16DataSize, u8Data, pu8_SpareBuf);
    }
    if (u32Ret != ERR_SPINAND_SUCCESS)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "PAGE LOAD FAIL %lx \r\n", u32Ret));
        return u32Ret;
    }

#ifdef CONFIG_NAND_QUAL_WRITE
    HAL_SPINAND_PreHandle(E_SPINAND_SINGLE_MODE);
#endif
    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // FIRST COMMAND PAGE PROGRAM EXECUTE
    FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), SPI_NAND_CMD_PE);
    u8WbufIndex++;

    // set Write Start Address
    for (s8Index = (SPI_NAND_ADDR_LEN - 1); s8Index >= 0; s8Index--)
    {
        u8Addr = (u32_PageIdx >> (8 * s8Index)) & 0xFF;
        FSP_WRITE_BYTE((REG_FSP_WRITE_BUFF + u8WbufIndex), u8Addr);
        u8WbufIndex++;
    }

    FSP_WRITE(REG_FSP_WRITE_SIZE, 0x004);
    FSP_WRITE(REG_FSP_READ_SIZE, 0x000);
    // Trigger FSP
    FSP_WRITE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    u32Ret = _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);

    if (u32Ret == ERR_SPINAND_SUCCESS)
    {
        if (u8Status & P_FAIL)
        {
            u32Ret = ERR_SPINAND_W_FAIL;
            return u32Ret;
        }
    }
    u32Ret = HAL_SPINAND_WriteProtect(TRUE);

    return u32Ret;
}

U32 HAL_SPINAND_Read(U32 u32Addr, U32 u32DataSize, U8 *pu8Data)
{
    U16 u16Addr = u32Addr & 0xFFFF;
    U32 ret     = ERR_SPINAND_SUCCESS;

    if (RIU_FLAG || SPI_BDMA_QUIRK)
    {
        ret = HAL_SPINAND_RIU_READ(u16Addr, u32DataSize, pu8Data);
    }
    else if (BDMA_FLAG)
    {
        // config SPI waveform for BDMA
        if (0) // pSpiNandDrv->tSpinandInfo.u8PlaneCnt
        {
            QSPI_WRITE(REG_SPI_FUNC_SET, (REG_SPI_ADDR2_EN | REG_SPI_DUMMY_EN | REG_SPI_WRAP_EN));
        }
        else
        {
            QSPI_WRITE(REG_SPI_FUNC_SET, (REG_SPI_ADDR2_EN | REG_SPI_DUMMY_EN));
        }
        // Set dummy cycle
        // QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN|REG_SPI_DUMMY_CYC_SINGAL);
        //        _HAL_SPINAND_BDMA_INIT(u32DataSize);

        ret = _HAL_SPINAND_BDMA_READ(u16Addr, u32DataSize, pu8Data);
    }
    else if (XIP_FLAG)
    {
        if (0) // pSpiNandDrv->tSpinandInfo.u8PlaneCnt
        {
            QSPI_WRITE(REG_SPI_FUNC_SET, (REG_SPI_ADDR2_EN | REG_SPI_DUMMY_EN | REG_SPI_WRAP_EN));
        }
        else
        {
            QSPI_WRITE(REG_SPI_FUNC_SET, (REG_SPI_ADDR2_EN | REG_SPI_DUMMY_EN));
        }

        if (!BASE_SPI_OFFSET)
        {
            if (!PAGE_SIZE)
                BASE_SPI_OFFSET = (MS_U32)ioremap(MS_SPI_ADDR, 2048 + 16);

            BASE_SPI_OFFSET = (MS_U32)ioremap(MS_SPI_ADDR, PAGE_SIZE + SPARE_SIZE);
        }
        if (BASE_SPI_OFFSET)
        {
            memcpy((void *)pu8Data, (const void *)(BASE_SPI_OFFSET) + u16Addr, u32DataSize);
        }

        if (ret != ERR_SPINAND_SUCCESS)
        {
            printk(KERN_ERR "R Wait BDMA Done Time Out CLK!!!!\r\n");
        }
    }
    return ret;
}

U32 HAL_SPINAND_ReadID(U32 u32DataSize, U8 *pu8Data)
{
    U16 u16Index      = 0;
    U32 u32RealLength = 0;

    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_RDID);
    // Set Start Address
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + 1, 0x00);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 2);
    FSP_WRITE(REG_FSP_READ_SIZE, 0);

    // Set Write & Read Length
    for (u16Index = 0; u16Index < u32DataSize; u16Index += u32RealLength)
    {
        if (u32DataSize > (MAX_READ_BUF_CNT))
        {
            FSP_WRITE(REG_FSP_READ_SIZE, MAX_READ_BUF_CNT);
            u32RealLength = MAX_READ_BUF_CNT;
        }
        else
        {
            FSP_WRITE(REG_FSP_READ_SIZE, u32DataSize);
            u32RealLength = u32DataSize;
        }

        // Trigger FSP
        FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

        // Check FSP done flag
        if (_HAL_FSP_ChkWaitDone() == FALSE)
        {
            DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "RID Wait FSP Done Time Out !!!!\r\n"));
            return ERR_SPINAND_TIMEOUT;
        }
        // Get Read Data
        _HAL_FSP_GetRData((pu8Data + u16Index), u32RealLength);

        // Clear FSP done flag
        FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    }
    return ERR_SPINAND_SUCCESS;
}

U32 HAL_SPINAND_WriteProtect(BOOL bEnable)
{
    U8 u8WbufIndex = 0;
    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_SF);
    u8WbufIndex++;
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex, SPI_NAND_REG_PROT);
    u8WbufIndex++;
    if (bEnable)
        FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex, 0x38); // all locked(default)
    else
        FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex, 0x00); // all unlocked

    FSP_WRITE(REG_FSP_WRITE_SIZE, 3);
    // read buffer size
    FSP_WRITE(REG_FSP_READ_SIZE, 0);
    // Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk(KERN_ERR "WP Wait FSP Done Time Out !!!!\r\n"));
        return ERR_SPINAND_TIMEOUT;
    }

    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return ERR_SPINAND_SUCCESS;
}

U32 HAL_SPINAND_SetMode(SPINAND_MODE eMode)
{
    switch (eMode)
    {
        case E_SPINAND_SINGLE_MODE:
            HAL_SPINAND_PreHandle(E_SPINAND_SINGLE_MODE);
            QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN | REG_SPI_DUMMY_CYC_SINGLE);
            QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_NORMAL_MODE);
            //	 PM_WRITE_MASK(REG_PM_SPI_IS_GPIO, PM_SPI_HOLD_IS_GPIO, PM_SPI_HOLD_GPIO_MASK);
            //	 PM_WRITE_MASK(REG_PM_SPI_IS_GPIO, PM_SPI_WP_IS_GPIO, PM_SPI_WP_GPIO_MASK);
            break;
        case E_SPINAND_FAST_MODE:
            HAL_SPINAND_PreHandle(E_SPINAND_FAST_MODE);
            QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN | REG_SPI_DUMMY_CYC_SINGLE);
            QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_FAST_READ);
            break;
        case E_SPINAND_DUAL_MODE:
            HAL_SPINAND_PreHandle(E_SPINAND_DUAL_MODE);
            QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN | REG_SPI_DUMMY_CYC_SINGLE);
            QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_3B);
            break;
        case E_SPINAND_DUAL_MODE_IO:
            HAL_SPINAND_PreHandle(E_SPINAND_DUAL_MODE_IO);
            QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN | REG_SPI_DUMMY_CYC_DUAL);
            QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_BB);
            break;
        case E_SPINAND_QUAD_MODE:
            HAL_SPINAND_PreHandle(E_SPINAND_QUAD_MODE);
            QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN | REG_SPI_DUMMY_CYC_SINGLE);
            QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_6B);
            break;
        case E_SPINAND_QUAD_MODE_IO:
            HAL_SPINAND_PreHandle(E_SPINAND_QUAD_MODE_IO);
            //	 PM_WRITE_MASK(REG_PM_SPI_IS_GPIO, PM_SPI_HOLD_NOT_GPIO, PM_SPI_HOLD_GPIO_MASK);
            //	 PM_WRITE_MASK(REG_PM_SPI_IS_GPIO, PM_SPI_WP_NOT_GPIO, PM_SPI_WP_GPIO_MASK);
            QSPI_WRITE(REG_SPI_CKG_SPI, REG_SPI_USER_DUMMY_EN | REG_SPI_DUMMY_CYC_QUAD);
            // QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_6B);
            QSPI_WRITE(REG_SPI_MODE_SEL, REG_SPI_CMD_EB);
            break;
    }
    return ERR_SPINAND_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SPINAND_SetCKG()
/// @brief \b Function \b Description: This function is used to set ckg_spi dynamically
/// @param <IN>        \b eCkgSpi    : enumerate the ckg_spi
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : Please use this function carefully , and is restricted to Flash ability
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_SPINAND_SetCKG(U8 u8CkgSpi)
{
    BOOL Ret        = FALSE;
    U8   u8nonPmIdx = 0, u8PmIdx = 0;
    U8   u8Idx  = 0;
    U8   u8Size = 0;
    u8Size      = sizeof(_hal_ckg_spi_nonpm) / sizeof(hal_clk_ckg_t);
    // DEBUG_SPINAND(E_SPINAND_DBGLV_INFO, printk("%s()\n", __FUNCTION__));
    for (u8Idx = 0; u8Idx < u8Size; u8Idx++)
    {
        if (u8CkgSpi < _hal_ckg_spi_nonpm[u8Idx].u8Clk)
        {
            if (u8Idx)
                u8nonPmIdx = u8Idx - 1;
            else
                u8nonPmIdx = u8Idx;
            break;
        }
        else
            u8nonPmIdx = u8Idx;
    }

    u8Size = sizeof(_hal_ckg_spi_pm) / sizeof(hal_clk_ckg_t);
    for (u8Idx = 0; u8Idx < u8Size; u8Idx++)
    {
        if (u8CkgSpi < _hal_ckg_spi_pm[u8Idx].u8Clk)
        {
            if (u8Idx)
                u8PmIdx = u8Idx - 1;
            else
                u8PmIdx = u8Idx;
            break;
        }
        else
            u8PmIdx = u8Idx;
    }

    if (_hal_ckg_spi_nonpm[u8nonPmIdx].eClkCkg == NULL || _hal_ckg_spi_pm[u8PmIdx].eClkCkg == NULL)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk("CLOCK NOT SUPPORT \n"));
        return Ret;
    }

    // NON-PM Doman
    CLK_WRITE_MASK(REG_CLK0_CKG_SPI, CLK0_CLK_SWITCH_OFF, CLK0_CLK_SWITCH_MASK);
    CLK_WRITE_MASK(REG_CLK0_CKG_SPI, CLK0_CKG_SPI_108MHZ, CLK0_CKG_SPI_MASK);   // set ckg_spi
    CLK_WRITE_MASK(REG_CLK0_CKG_SPI, CLK0_CLK_SWITCH_ON, CLK0_CLK_SWITCH_MASK); // run @ ckg_spi

    // PM Doman
    PM_WRITE_MASK(REG_PM_CKG_SPI, PM_SPI_CLK_SWITCH_OFF, PM_SPI_CLK_SWITCH_MASK);         // run @ 12M
    PM_WRITE_MASK(REG_PM_CKG_SPI, _hal_ckg_spi_pm[u8PmIdx].eClkCkg, PM_SPI_CLK_SEL_MASK); // set ckg_spi
    PM_WRITE_MASK(REG_PM_CKG_SPI, PM_SPI_CLK_SWITCH_ON, PM_SPI_CLK_SWITCH_MASK);          // run @ ckg_spi
    Ret = TRUE;
    return Ret;
}

void HAL_SPINAND_CSCONFIG(void)
{
    U16 u16Data = 0;
    u16Data     = CHIP_READ(REG_CHIPTOP_DUMMY3);
    u16Data |= CHIP_CS_PAD1;
    CHIP_WRITE(REG_CHIPTOP_DUMMY3, u16Data);
}

BOOL HAL_SPINAND_IsActive(void)
{
    //	    U16 u16Reg;
    // Chiptop, offset 0x04, bit 4 : spi nand mode
    //	    u16Reg = CHIP_READ(0x04);
    //	    if (u16Reg&0x10)
    return 1;
    //	    else
    //	        return 0;
}

U8 HAL_SPINAND_ReadStatusRegister(U8 *u8Status, U8 u8Addr)
{
    U8 u8WbufIndex = 0;

    HAL_SPINAND_AssertNormalMode();
    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_GF);
    u8WbufIndex++;
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex, u8Addr);
    u8WbufIndex++;

    FSP_WRITE(REG_FSP_WRITE_SIZE, 2);

    FSP_WRITE(REG_FSP_READ_SIZE, 1);

    // Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk("WP Wait FSP Done Time Out !!!!\r\n"));
        return FALSE;
    }
    _HAL_FSP_GetRData((u8Status), 1);

    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);

    return ERR_SPINAND_SUCCESS;
}

U8 HAL_SPINAND_WriteStatusRegister(U8 *u8Status, U8 u8Addr)
{
    U8 u8WbufIndex = 0;

    HAL_SPINAND_AssertNormalMode();
    // FSP init config
    FSP_WRITE(REG_FSP_CTRL, (ENABLE_FSP | RESET_FSP | INT_FSP));
    FSP_WRITE(REG_FSP_CTRL2, 0);
    FSP_WRITE(REG_FSP_CTRL4, 0);

    // Set FSP Read Command
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF, SPI_NAND_CMD_SF);
    u8WbufIndex++;
    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex, u8Addr);
    u8WbufIndex++;

    FSP_WRITE_BYTE(REG_FSP_WRITE_BUFF + u8WbufIndex, *u8Status);

    FSP_WRITE(REG_FSP_WRITE_SIZE, 3);
    FSP_WRITE(REG_FSP_READ_SIZE, 0);

    // Trigger FSP
    FSP_WRITE_BYTE(REG_FSP_TRIGGER, TRIGGER_FSP);

    // Check FSP done flag
    if (_HAL_FSP_ChkWaitDone() == FALSE)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printk("WP Wait FSP Done Time Out !!!!\r\n"));
        return FALSE;
    }

    // Clear FSP done flag
    FSP_WRITE_BYTE(REG_FSP_CLEAR_DONE, CLEAR_DONE_FSP);
    return TRUE;
}

U8 HAL_QSPI_FOR_DEBUG(void)
{
    return ((U8)(PM_READ(0x34)));
}
