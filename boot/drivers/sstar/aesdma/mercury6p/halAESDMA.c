/*
 * halAESDMA.c- Sigmastar
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

#ifndef _HAL_AESDMA_H_
#include "halAESDMA.h"
#endif

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) U16 HAL_RSA_DummyRead(U16 u16Index, U8 dummy)
{
    U8  c;
    U16 data = 0;

    for (c = 0; dummy + 5 > c; c++)
        data = RIU[(AESDMA_BASE_ADDR + (u16Index << 1))];

    return data;
}
#else
#define HAL_RSA_DummyRead(x, y)
#endif

void HAL_AESDMA_SetXIULength(U32 u32Size)
{
    // AESDMA XIU length (byte):54~55
    RIU[(AESDMA_BASE_ADDR + (0x54 << 1))] = (U16)((0x0000ffff) & (u32Size));
    RIU[(AESDMA_BASE_ADDR + (0x55 << 1))] = (U16)(((0xffff0000) & (u32Size)) >> 16);
}

BOOL HAL_AESDMA_CheckAesKey(U16 u32KeySel)
{
    // Check the validity of the AES key id
    return (u32KeySel <= AESDMA_EFUSE_HW_KEY_MAX ? TRUE : FALSE);
}

void HAL_AESDMA_UseAesKey(U16 u32KeySel)
{
    // AESDMA using key
    if (u32KeySel != AESDMA_KEY_OTP_EFUSE_KEY8)
    {
        RIU[(AESDMA_BASE_ADDR + (0x79 << 1))] =
            (((RIU[(AESDMA_BASE_ADDR + (0x79 << 1))]) & (~(AESDMA_USE_SECRET_KEY_MASK | AESDMA_KEY_OTP_EFUSE_USE_KEY8)))
             | AESDMA_USE_KEY(u32KeySel));
    }
    else
    {
        RIU[(AESDMA_BASE_ADDR + (0x79 << 1))] =
            (((RIU[(AESDMA_BASE_ADDR + (0x79 << 1))]) & (~AESDMA_USE_SECRET_KEY_MASK)) | AESDMA_KEY_OTP_EFUSE_USE_KEY8);
    }
}

void HAL_AESDMA_CipherEncrypt(void)
{
    RIU[(AESDMA_BASE_ADDR + (0x51 << 1))] = ((RIU[(AESDMA_BASE_ADDR + (0x51 << 1))]) & (~AESDMA_CTRL_CIPHER_DECRYPT));
}
void HAL_AESDMA_CipherDecrypt(void)
{
    // AESDMA ctrl(decrypt/aes_en)
    RIU[(AESDMA_BASE_ADDR + (0x51 << 1))] = ((RIU[(AESDMA_BASE_ADDR + (0x51 << 1))]) | (AESDMA_CTRL_CIPHER_DECRYPT));
}

void HAL_AESDMA_Enable(void)
{
    // AESDMA ctrl(decrypt/aes_en)
    RIU[(AESDMA_BASE_ADDR + (0x51 << 1))] = ((RIU[(AESDMA_BASE_ADDR + (0x51 << 1))]) | AESDMA_CTRL_AES_EN);
}

void HAL_AESDMA_Disable(void)
{
    // AESDMA ctrl(decrypt/aes_en)
    RIU[(AESDMA_BASE_ADDR + (0x51 << 1))] = ((RIU[(AESDMA_BASE_ADDR + (0x51 << 1))]) & (~AESDMA_CTRL_AES_EN));
}

void HAL_AESDMA_FileOutEnable(U8 u8FileOutEnable)
{
    // AESDMA fout_en
    if (u8FileOutEnable == 1)
    {
        RIU[(AESDMA_BASE_ADDR + (0x50 << 1))] = ((RIU[(AESDMA_BASE_ADDR + (0x50 << 1))]) | AESDMA_CTRL_FOUT_EN);
    }
    else
    {
        RIU[(AESDMA_BASE_ADDR + (0x50 << 1))] = ((RIU[(AESDMA_BASE_ADDR + (0x50 << 1))]) & (~AESDMA_CTRL_FOUT_EN));
    }
}

void HAL_AESDMA_SetFileinAddr(U32 u32addr)
{
    if (u32addr >= 0x20000000)
    {
        u32addr -= 0x20000000;
    }

    // SHA_SetLength:5c~5d(sha_message_length)
    RIU[(AESDMA_BASE_ADDR + (0x52 << 1))] = (U16)((0x0000ffff) & (u32addr));
    RIU[(AESDMA_BASE_ADDR + (0x53 << 1))] = (U16)(((0xffff0000) & (u32addr)) >> 16);
}

void HAL_AESDMA_SetFileoutAddr(U32 u32addr, U32 u32Size)
{
    if (u32addr >= 0x20000000)
    {
        u32addr -= 0x20000000;
    }

    // SHA_SetLength:5c~5d(sha_message_length)
    RIU[(AESDMA_BASE_ADDR + (0x56 << 1))] = (U16)((0x0000ffff) & (u32addr));
    RIU[(AESDMA_BASE_ADDR + (0x57 << 1))] = (U16)(((0xffff0000) & (u32addr)) >> 16);

    RIU[(AESDMA_BASE_ADDR + (0x58 << 1))] = (U16)((0x0000ffff) & (u32addr + u32Size - 1));
    RIU[(AESDMA_BASE_ADDR + (0x59 << 1))] = (U16)(((0xffff0000) & (u32addr + u32Size - 1)) >> 16);
}

void HAL_AESDMA_SetCipherKey(U16 *pu16Key)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        RIU[(AESDMA_BASE_ADDR + ((0x67 - i) << 1))] = ((pu16Key[i] & 0x00FF) << 8) | ((pu16Key[i] & 0xFF00) >> 8);
        HAL_RSA_DummyRead((0x67 - i), 1);
    }
}

void HAL_AESDMA_SetIV(U16 *pu16IV)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        RIU[(AESDMA_BASE_ADDR + ((0x6F - i) << 1))] = ((pu16IV[i] & 0x00FF) << 8) | ((pu16IV[i] & 0xFF00) >> 8);
    }
}

void HAL_AESDMA_SetChainModeECB(void)
{
    RIU[(AESDMA_BASE_ADDR + (0x51 << 1))] =
        (RIU[(AESDMA_BASE_ADDR + (0x51 << 1))] & AESDMA_CTRL_CHAINMODE_CLEAR) | AESDMA_CTRL_CHAINMODE_ECB;
}

void HAL_AESDMA_SetChainModeCTR(void)
{
    RIU[(AESDMA_BASE_ADDR + (0x51 << 1))] =
        (RIU[(AESDMA_BASE_ADDR + (0x51 << 1))] & AESDMA_CTRL_CHAINMODE_CLEAR) | AESDMA_CTRL_CHAINMODE_CTR;
}

void HAL_AESDMA_SetChainModeCBC(void)
{
    RIU[(AESDMA_BASE_ADDR + (0x51 << 1))] =
        (RIU[(AESDMA_BASE_ADDR + (0x51 << 1))] & AESDMA_CTRL_CHAINMODE_CLEAR) | AESDMA_CTRL_CHAINMODE_CBC;
}

void HAL_AESDMA_Reset(void)
{
    int i;
    RIU[(AESDMA_BASE_ADDR + (0x50 << 1))] = AESDMA_CTRL_SW_RST;
    RIU[(AESDMA_BASE_ADDR + (0x50 << 1))] = 0;
    RIU[(AESDMA_BASE_ADDR + (0x79 << 1))] = 0;
    RIU[(AESDMA_BASE_ADDR + (0x51 << 1))] = 0;
    RIU[(AESDMA_BASE_ADDR + (0x52 << 1))] = 0;
    RIU[(AESDMA_BASE_ADDR + (0x53 << 1))] = 0;
    RIU[(AESDMA_BASE_ADDR + (0x54 << 1))] = 0;
    RIU[(AESDMA_BASE_ADDR + (0x55 << 1))] = 0;
    RIU[(AESDMA_BASE_ADDR + (0x56 << 1))] = 0;
    RIU[(AESDMA_BASE_ADDR + (0x57 << 1))] = 0;
    RIU[(AESDMA_BASE_ADDR + (0x58 << 1))] = 0;
    RIU[(AESDMA_BASE_ADDR + (0x59 << 1))] = 0;
    RIU[(AESDMA_BASE_ADDR + (0x79 << 1))] =
        (((RIU[(AESDMA_BASE_ADDR + (0x79 << 1))]) & (~AESDMA_USE_SECRET_KEY_MASK)) | AESDMA_USE_CIPHER_KEY);
    for (i = 0; i < 8; i++)
    {
        RIU[(AESDMA_BASE_ADDR + ((0x67 - i) << 1))] = 0;
    }
    for (i = 0; i < 8; i++)
    {
        RIU[(AESDMA_BASE_ADDR + ((0x6F - i) << 1))] = 0;
    }
}

void HAL_AESDMA_Start(U8 u8AESDMAStart)
{
    // AESDMA file start
    if (u8AESDMAStart == 1)
    {
        RIU[(AESDMA_BASE_ADDR + (0x50 << 1))] = ((RIU[(AESDMA_BASE_ADDR + (0x50 << 1))]) | (AESDMA_CTRL_FILE_ST));
        RIU[(AESDMA_BASE_ADDR + (0x50 << 1))] = ((RIU[(AESDMA_BASE_ADDR + (0x50 << 1))]) & (~AESDMA_CTRL_FILE_ST));
    }
    else
    {
        RIU[(AESDMA_BASE_ADDR + (0x50 << 1))] = ((RIU[(AESDMA_BASE_ADDR + (0x50 << 1))]) & (~AESDMA_CTRL_FILE_ST));
    }

    HAL_RSA_DummyRead(0x50, 1);
}

U16 HAL_AESDMA_GetStatus(void)
{
    return RIU[(AESDMA_BASE_ADDR + (0x7F << 1))];
}

#ifdef PKA_SUPPORT_RSA

#define RSA_LOOP_CNT (0xfffffff0UL)
#define RSA_CHECK_SATTUS(x, cnt)                    \
    {                                               \
        if (x >= cnt)                               \
        {                                           \
            printf("<AESDMA> timeout! %llx \n", x); \
            return FALSE;                           \
        }                                           \
        x = 0;                                      \
    }
/*
void HAL_RSA_LoadKeyN(u32 *ptr_N, u8 u8Nlentgh) {}
void HAL_RSA_LoadKeyE(u32 *ptr_E, u8 u8Elentgh) {}
void HAL_RSA_ClearInt(void) {}
void HAL_RSA_Reset(void) {}
void HAL_RSA_SetKeyType(u8 u8hwkey, u8 u8pubkey) {}
void HAL_RSA_Start(void) {}
void HAL_RSA_FileOutStart(void) {}
void HAL_RSA_FileOutEnd(void) {}
void HAL_RSA_SetFileOutAddr(u32 u32offset) {}
void HAL_RSA_LoadSignInverse_end(void) {}

u16 HAL_RSA_GetStatus(void)
{
    return 0;
}
*/
void HAL_RSA_Reset(U8 u8lentgh, U32 operation)
{
    /*
        U32 tmp[512 / 4]={0};

        HAL_RSA_LoadKeyNInverse(tmp, u8lentgh);

        if (PKA_RSA_OPERAND_4096 == operation)
            HAL_RSA_LoadKeyEInverse(tmp, u8lentgh, PKA_RSA_OPERAND_4096);
        if (PKA_RSA_OPERAND_2048 == operation)
            HAL_RSA_LoadKeyEInverse(tmp, u8lentgh, PKA_RSA_OPERAND_2048);

        HAL_RSA_LoadSignInverse_start();
        HAL_RSA_LoadSignInverse(tmp, u8lentgh, 0);
        */
}

void HAL_RSA_LoadKeyNInverse(U32 *ptr_N, U8 u8Nlentgh)
{
    U8                 lentgh = u8Nlentgh;
    U32                i;
    unsigned long long value;

    PKA2_AESDMA_REG32_WRITEE(PKA2_RSA_SELECT_CTRL, PKA2_RSA_SELECT_D0D1_RAM);
    for (i = 0; i < lentgh; i++)
    {
        value = ((*(ptr_N + (lentgh - 1) - i)) & 0xff000000) >> 24 | ((*(ptr_N + (lentgh - 1) - i)) & 0x00ff0000) >> 8
                | ((*(ptr_N + (lentgh - 1) - i)) & 0x0000ff00) << 8
                | ((*(ptr_N + (lentgh - 1) - i)) & 0x000000ff) << 24;
        PKA0_AESDMA_REG32_WRITEE(i, value);
    }
}

void HAL_RSA_LoadKeyE(U8 u8Elentgh, U32 operation)
{
    U32 i;
    U8  lentgh = u8Elentgh;

    if (PKA_RSA_OPERAND_4096 == operation)
    {
        PKA2_AESDMA_REG32_WRITEE(PKA2_RSA_SELECT_CTRL, PKA2_RSA_SELECT_D2D3_RAM);
        PKA0_AESDMA_REG32_WRITEE(0, 0x10001); // default value 0x10001
    }
    else
    {
        PKA2_AESDMA_REG32_WRITEE(PKA2_RSA_SELECT_CTRL, PKA2_RSA_SELECT_D0D1_RAM);
        PKA1_AESDMA_REG32_WRITEE(0, 0x10001); // default value 0x10001
    }

    for (i = 1; i < lentgh; i++)
    {
        if (PKA_RSA_OPERAND_4096 == operation)
        {
            PKA0_AESDMA_REG32_WRITEE(i, 0x0UL);
        }
        else
        {
            PKA1_AESDMA_REG32_WRITEE(i, 0x0UL);
        }
    }
}

void HAL_RSA_LoadKeyEInverse(U32 *ptr_E, U8 u8Elentgh, U32 operation)
{
    unsigned long long value;
    U32                i;
    U8                 lentgh = u8Elentgh;

    if (PKA_RSA_OPERAND_4096 == operation)
    {
        PKA2_AESDMA_REG32_WRITEE(PKA2_RSA_SELECT_CTRL, PKA2_RSA_SELECT_D2D3_RAM);
    }
    else
    {
        PKA2_AESDMA_REG32_WRITEE(PKA2_RSA_SELECT_CTRL, PKA2_RSA_SELECT_D0D1_RAM);
    }

    for (i = 0; i < lentgh; i++)
    {
        value = ((*(ptr_E + (lentgh - 1) - i)) & 0xff000000) >> 24 | ((*(ptr_E + (lentgh - 1) - i)) & 0x00ff0000) >> 8
                | ((*(ptr_E + (lentgh - 1) - i)) & 0x0000ff00) << 8
                | ((*(ptr_E + (lentgh - 1) - i)) & 0x000000ff) << 24;

        if (PKA_RSA_OPERAND_4096 == operation)
        {
            PKA0_AESDMA_REG32_WRITEE(i, value);
        }
        else
        {
            PKA1_AESDMA_REG32_WRITEE(i, value);
        }
    }
}

void HAL_RSA_LoadSignInverse_start(void)
{
    PKA2_AESDMA_REG32_WRITEE(PKA2_RSA_SELECT_CTRL, PKA2_RSA_SELECT_A_RAM);

    return;
}

void HAL_RSA_LoadSignInverse(U32 *ptr_Sign, U32 Signlentgh, U32 position)
{
    U32                i;
    U8                 lentgh = Signlentgh;
    unsigned long long value;

    for (i = 0; i < Signlentgh; i++)
    {
        value = ((*(ptr_Sign + (lentgh - 1) - i)) & 0xff000000) >> 24
                | ((*(ptr_Sign + (lentgh - 1) - i)) & 0x00ff0000) >> 8
                | ((*(ptr_Sign + (lentgh - 1) - i)) & 0x0000ff00) << 8
                | ((*(ptr_Sign + (lentgh - 1) - i)) & 0x000000ff) << 24;

        PKA0_AESDMA_REG32_WRITEE((i + position), value);
    }
}
void HAL_RSA_FileOut_Start(void)
{
    PKA2_AESDMA_REG32_WRITEE(PKA2_RSA_SELECT_CTRL, PKA2_RSA_SELECT_A_RAM); // A RAM

    return;
}

U32 HAL_RSA_FileOut(U32 i)
{
    unsigned long long value;

    // for (i = 0; i < 128; i++)
    {
        value = PKA0_AESDMA_REG32_READ(i);

        value = ((value & 0xff000000) >> 24 | (value & 0x00ff0000) >> 8 | (value & 0x0000ff00) << 8
                 | (value & 0x000000ff) << 24);

        PKA_AESDMA_DBG("view %llx Long %llx\n", (PKA0_AESDMA_BASE + (i << 2)), value);
    }

    return value;
}

void HAL_RSA_Init_Status(U32 piont, U32 operation)
{
    PKA2_AESDMA_REG32_WRITEE(PKA_BANK_MAIN_CTL, 0x00000000);                 // Control Register selection
    PKA0_AESDMA_REG32_WRITEE(PKA_BANK_FLAGS, 0x00000000);                    // Write flags
    PKA0_AESDMA_REG32_WRITEE(PKA_BANK_ENTRY_PIONT, piont);                   // Entry Point – Set to calc_r_inv = 11
    PKA0_AESDMA_REG32_WRITEE(PKA_BANK_PROBABILITY, 0x00UL);                  // Setup Dial-a-DTA for 0%
    PKA0_AESDMA_REG32_WRITEE(PKA_BANK_IRQ_EN, 0x40000000);                   // Setup interrupt enable register
    PKA0_AESDMA_REG32_WRITEE(PKA_BANK_MAIN_CTL, (0x80000000UL | operation)); // Start CLUE operation for 2048-bits
}

U32 HAL_RSA_WaitDone(U32 reason_way)
{
    unsigned long long loop = 0;
    // Check stack point is 0
    while ((PKA0_AESDMA_REG32_READ(PKA_BANK_STACK_PIONT) & 0xf) != 0x0 && (loop < RSA_LOOP_CNT))
    {
        loop++;
    }
    PKA_AESDMA_DBG("//wait 0x%lx to 0xf != 0x0\n", (PKA0_AESDMA_BASE + (PKA_BANK_STACK_PIONT << 2)));

    RSA_CHECK_SATTUS(loop, RSA_LOOP_CNT);

    if (reason_way == TRUE)
    {
        // Check cleared status
        while ((PKA0_AESDMA_REG32_READ(PKA_BANK_STAT) & 0xf0000000) != 0x40000000 && (loop < RSA_LOOP_CNT))
        {
            loop++;
        }

        PKA_AESDMA_DBG("//wait 0x%lx to 0xf0000000 != 0x40000000\n", (PKA0_AESDMA_BASE + (PKA_BANK_STAT << 2)));
        RSA_CHECK_SATTUS(loop, RSA_LOOP_CNT);
        // Check expected status: stop reason should be 0 (Normal)
        while ((PKA0_AESDMA_REG32_READ(PKA_BANK_ISR_STATUS) & 0x50ff0000) != 0x50000000 && (loop < RSA_LOOP_CNT))
        {
            loop++;
        }

        PKA_AESDMA_DBG("//wait 0x%lx to 0x50ff0000 != 0x50000000\n", (PKA0_AESDMA_BASE + (PKA_BANK_ISR_STATUS << 2)));
        // Clear Status
        PKA0_AESDMA_REG32_WRITEE(PKA_BANK_STAT, 0x40000000);

        RSA_CHECK_SATTUS(loop, RSA_LOOP_CNT);
        // Check cleared status
        while ((PKA0_AESDMA_REG32_READ(PKA_BANK_STAT) & 0xffffffff) != 0x00000000 && (loop < RSA_LOOP_CNT))
        {
            loop++;
        }

        PKA_AESDMA_DBG("//wait 0x%lx to 0xffffffff != 0x00000000\n", (PKA0_AESDMA_BASE + (PKA_BANK_STAT << 2)));
    }
    else
    {
        // Check expected status: stop reason should be 0 (Normal)
        while ((PKA0_AESDMA_REG32_READ(PKA_BANK_ISR_STATUS) & 0x40ff0000) != 0x40000000 && (loop < RSA_LOOP_CNT))
        {
            loop++;
        }
        PKA_AESDMA_DBG("//wait 0x%lx to 0x40ff0000 != 0x40000000\n", (PKA0_AESDMA_BASE + (PKA_BANK_STAT << 2)));

        RSA_CHECK_SATTUS(loop, RSA_LOOP_CNT);
        // Clear Status
        PKA0_AESDMA_REG32_WRITEE(PKA_BANK_STAT, 0x40000000);

        // Check cleared status
        while ((PKA0_AESDMA_REG32_READ(PKA_BANK_STAT) & 0xf0ff0000) != 0x00000000 && (loop < RSA_LOOP_CNT))
        {
            loop++;
        }
        PKA_AESDMA_DBG("//wait 0x%lx to 0xf0ff0000 != 0x00000000\n", (PKA0_AESDMA_BASE + (PKA_BANK_STAT << 2)));
    }

    return TRUE;
}

#else

void HAL_RSA_ClearInt(void)
{
    // RSA interrupt clear
    RIU[(RSA_BASE_ADDR + (0x27 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x27 << 1))]) | (RSA_INT_CLR));
}

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) void HAL_RSA_Reset(void)
#else
void HAL_RSA_Reset(void)
#endif
{
    // RSA Rst
    RIU[(RSA_BASE_ADDR + (0x28 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x28 << 1))]) | (RSA_CTRL_RSA_RST));
    HAL_RSA_DummyRead(0x28, 1);
    RIU[(RSA_BASE_ADDR + (0x28 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x28 << 1))]) & (~RSA_CTRL_RSA_RST));
    HAL_RSA_DummyRead(0x28, 1);
}

void HAL_RSA_Ind32Ctrl(U8 u8dirction)
{
    //[1] reg_ind32_direction 0: Read. 1: Write
    if (u8dirction == 1)
    {
        RIU[(RSA_BASE_ADDR + (0x21 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x21 << 1))]) | (RSA_IND32_CTRL_DIRECTION_WRITE));
    }
    else
    {
        RIU[(RSA_BASE_ADDR + (0x21 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x21 << 1))]) & (~RSA_IND32_CTRL_DIRECTION_WRITE));
    }
    HAL_RSA_DummyRead(0x21, 1);
    //[2] reg_addr_auto_inc : Set 1 to enable address auto-increment after finishing read/write
    RIU[(RSA_BASE_ADDR + (0x21 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x21 << 1))]) | (RSA_IND32_CTRL_ADDR_AUTO_INC));
    HAL_RSA_DummyRead(0x21, 1);
    //[3] Set 1 to enable access auto-start after writing Data[31:16]
    RIU[(RSA_BASE_ADDR + (0x21 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x21 << 1))]) | (RSA_IND32_CTRL_ACCESS_AUTO_START));
    HAL_RSA_DummyRead(0x21, 1);
}

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) void HAL_RSA_LoadSignInverse(U32 *ptr_Sign)
#else
void HAL_RSA_LoadSignInverse(U32 *ptr_Sign)
#endif
{
    U32 i;

    RIU[(RSA_BASE_ADDR + (0x22 << 1))] = RSA_A_BASE_ADDR;                                            // RSA A addr
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) | (RSA_IND32_START)); // RSA start

    for (i = 0; i < 64; i++)
    {
        RIU[(RSA_BASE_ADDR + (0x23 << 1))] =
            (U16)(((*(ptr_Sign + 63 - i)) >> 8) & 0xFF00) | (((*(ptr_Sign + 63 - i)) >> 24) & 0xFF);
        HAL_RSA_DummyRead(0x23, 1);
        RIU[(RSA_BASE_ADDR + (0x24 << 1))] =
            (U16)(((*(ptr_Sign + 63 - i)) >> 8) & 0xFF) | (((*(ptr_Sign + 63 - i)) << 8) & 0xFF00);
        HAL_RSA_DummyRead(0x24, 1);
    }
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) & (~RSA_IND32_START)); // RSA stop
}

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) void HAL_RSA_LoadSignInverse_2byte(U32 *ptr_Sign, U8 length)
#else
void HAL_RSA_LoadSignInverse_2byte(U32 *ptr_Sign, U8 length)
#endif
{
    S32 i;

    RIU[(RSA_BASE_ADDR + (0x22 << 1))] = RSA_A_BASE_ADDR;                                            // RSA A addr
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) | (RSA_IND32_START)); // RSA start
    for (i = 0; i < length; i++)
    {
        RIU[(RSA_BASE_ADDR + (0x23 << 1))] =
            (U16)(((*(ptr_Sign + (length - 1) - i)) >> 8) & 0xFF00) | (((*(ptr_Sign + (length - 1) - i)) >> 24) & 0xFF);
        HAL_RSA_DummyRead(0x23, 1);
        RIU[(RSA_BASE_ADDR + (0x24 << 1))] =
            (U16)(((*(ptr_Sign + (length - 1) - i)) >> 8) & 0xFF) | (((*(ptr_Sign + (length - 1) - i)) << 8) & 0xFF00);
        HAL_RSA_DummyRead(0x24, 1);
    }

    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) & (~RSA_IND32_START)); // RSA stop
}

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) void HAL_RSA_LoadKeyE(void)
#else
void HAL_RSA_LoadKeyE(void)
#endif
{
    RIU[(RSA_BASE_ADDR + (0x22 << 1))] = RSA_E_BASE_ADDR; // RSA E addr
    HAL_RSA_DummyRead(0x22, 1);
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) | (RSA_IND32_START)); // RSA start
    HAL_RSA_DummyRead(0x20, 1);
    // RIU[(POR_STATUS_BASE_ADDR+(0xA<<1))]=(U16)((0x0000ffff)&(U32)(ptr_E)); //write ptr_E addr to por_status(0x10050A)
#if 0
    for( i = 0; i < 64; i++ )
    {
        RIU[(RSA_BASE_ADDR+(0x23<<1))]= (U16)(((*(ptr_E+i))>>8)&0xFF00)|(((*(ptr_E+i))>>24)&0xFF);
        RIU[(RSA_BASE_ADDR+(0x24<<1))]= (U16)(((*(ptr_E+i))>>8)&0xFF)|(((*(ptr_E+i))<<8)&0xFF00);
    }
#endif
    RIU[(RSA_BASE_ADDR + (0x23 << 1))] = 0x0001;
    HAL_RSA_DummyRead(0x23, 1);
    RIU[(RSA_BASE_ADDR + (0x24 << 1))] = 0x0001;
    HAL_RSA_DummyRead(0x24, 1);

    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) & (~RSA_IND32_START)); // RSA stop
}

void HAL_RSA_LoadKeyEInverse(U32 *ptr_E)
{
    U32 i;
    U8  length = 64;

    RIU[(RSA_BASE_ADDR + (0x22 << 1))] = RSA_E_BASE_ADDR;
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) | (RSA_IND32_START)); // RSA start

    // RIU[(POR_STATUS_BASE_ADDR+(0xB<<1))]=(U16)((0x0000ffff)&(U32)(ptr_N)); //write ptr_N addr to por_status(0x10050B)

    for (i = 0; i < length; i++)
    {
        RIU[(RSA_BASE_ADDR + (0x23 << 1))] =
            (U16)(((*(ptr_E + (length - 1) - i)) >> 8) & 0xFF00) | (((*(ptr_E + (length - 1) - i)) >> 24) & 0xFF);
        HAL_RSA_DummyRead(0x23, 1);
        RIU[(RSA_BASE_ADDR + (0x24 << 1))] =
            (U16)(((*(ptr_E + (length - 1) - i)) >> 8) & 0xFF) | (((*(ptr_E + (length - 1) - i)) << 8) & 0xFF00);
        HAL_RSA_DummyRead(0x24, 1);
    }
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) & (~RSA_IND32_START)); // RSA stop
}

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) void HAL_RSA_LoadKeyN(U32 *ptr_N)
#else
void HAL_RSA_LoadKeyN(U32 *ptr_N)
#endif
{
    U32 i;

    RIU[(RSA_BASE_ADDR + (0x22 << 1))] = RSA_N_BASE_ADDR;                                            // RSA N addr
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) | (RSA_IND32_START)); // RSA start

    // RIU[(POR_STATUS_BASE_ADDR+(0xB<<1))]=(U16)((0x0000ffff)&(U32)(ptr_N)); //write ptr_N addr to por_status(0x10050B)

    for (i = 0; i < 64; i++)
    {
        RIU[(RSA_BASE_ADDR + (0x23 << 1))] = (U16)(((*(ptr_N + i)) >> 8) & 0xFF00) | (((*(ptr_N + i)) >> 24) & 0xFF);
        HAL_RSA_DummyRead(0x23, 1);
        RIU[(RSA_BASE_ADDR + (0x24 << 1))] = (U16)(((*(ptr_N + i)) >> 8) & 0xFF) | (((*(ptr_N + i)) << 8) & 0xFF00);
        HAL_RSA_DummyRead(0x24, 1);
    }
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) & (~RSA_IND32_START)); // RSA stop
}

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) void HAL_RSA_LoadKeyNInverse(U32 *ptr_N)
#else
void HAL_RSA_LoadKeyNInverse(U32 *ptr_N)
#endif
{
    U32 i;
    U8  length = 64;

    RIU[(RSA_BASE_ADDR + (0x22 << 1))] = RSA_N_BASE_ADDR;                                            // RSA N addr
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) | (RSA_IND32_START)); // RSA start

    // RIU[(POR_STATUS_BASE_ADDR+(0xB<<1))]=(U16)((0x0000ffff)&(U32)(ptr_N)); //write ptr_N addr to por_status(0x10050B)

    for (i = 0; i < length; i++)
    {
        RIU[(RSA_BASE_ADDR + (0x23 << 1))] =
            (U16)(((*(ptr_N + (length - 1) - i)) >> 8) & 0xFF00) | (((*(ptr_N + (length - 1) - i)) >> 24) & 0xFF);
        HAL_RSA_DummyRead(0x23, 1);
        RIU[(RSA_BASE_ADDR + (0x24 << 1))] =
            (U16)(((*(ptr_N + (length - 1) - i)) >> 8) & 0xFF) | (((*(ptr_N + (length - 1) - i)) << 8) & 0xFF00);
        HAL_RSA_DummyRead(0x24, 1);
    }
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) & (~RSA_IND32_START)); // RSA stop
}

void HAL_RSA_SetKeyLength(U16 u16keylen)
{
    //[13:8] n_len_e: key length, if hardware key set, this register is ignored and hardware internal using 3f
    RIU[(RSA_BASE_ADDR + (0x28 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x28 << 1))]) | (u16keylen << 8));
    HAL_RSA_DummyRead(0x28, 1);
}

void HAL_RSA_SetKeyType(U8 u8hwkey, U8 u8pubkey)
{
    //[1] hw_key_e : 0 : software key, 1: hardware key
    //[2] e_pub_e : 0: pvivate key, 1: public key
    if (u8hwkey == 1)
    {
        RIU[(RSA_BASE_ADDR + (0x28 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x28 << 1))]) | (RSA_SEL_HW_KEY));
    }
    else
    {
        RIU[(RSA_BASE_ADDR + (0x28 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x28 << 1))]) & (~RSA_SEL_HW_KEY));
    }
    HAL_RSA_DummyRead(0x28, 1);
    if (u8pubkey == 1)
    {
        RIU[(RSA_BASE_ADDR + (0x28 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x28 << 1))]) | (RSA_SEL_PUBLIC_KEY));
    }
    else
    {
        RIU[(RSA_BASE_ADDR + (0x28 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x28 << 1))]) & (~RSA_SEL_PUBLIC_KEY));
    }
    HAL_RSA_DummyRead(0x28, 1);
}

void HAL_RSA_ExponetialStart(void)
{
    // RSA exp start
    RIU[(RSA_BASE_ADDR + (0x27 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x27 << 1))]) | (RSA_EXP_START));
}

U16 HAL_RSA_GetStatus(void)
{
    return RIU[(RSA_BASE_ADDR + (0x29 << 1))];
}

void HAL_RSA_FileOutStart(void)
{
    // RSA ind32_start
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) | (RSA_IND32_START));
    HAL_RSA_DummyRead(0x20, 1);
}

void HAL_RSA_FileOutEnd(void)
{
    RIU[(RSA_BASE_ADDR + (0x20 << 1))] = ((RIU[(RSA_BASE_ADDR + (0x20 << 1))]) & (~RSA_IND32_START));
    HAL_RSA_DummyRead(0x20, 1);
}
void HAL_RSA_SetFileOutAddr(U32 u32offset)
{
    // RSA ind32_addr
    RIU[(RSA_BASE_ADDR + (0x22 << 1))] = (U16)(RSA_Z_BASE_ADDR + u32offset);
    HAL_RSA_DummyRead(0x22, 1);
}

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) U32 HAL_RSA_FileOut(void)
#else
U32  HAL_RSA_FileOut(void)
#endif
{
    U32 output;
    U32 dummy;

    dummy = RIU[(RSA_BASE_ADDR + (0x26 << 1))];
    HAL_RSA_DummyRead(0x26, 2);
    output = (U16)(((dummy >> 8) & 0xFF) | ((dummy << 8) & 0xFF00));

    dummy = RIU[(RSA_BASE_ADDR + (0x25 << 1))];
    HAL_RSA_DummyRead(0x25, 2);
    output = output | ((((dummy >> 8) & 0xFF) | ((dummy << 8) & 0xFF00)) << 16);

    return output;
}
#endif
void HAL_SHA_Reset(void)
{
    // SHA_Reset
    RIU[SHARNG_BASE_ADDR + (0x08 << 1)] = (RIU[SHARNG_BASE_ADDR + (0x08 << 1)] | (SHARNG_CTRL_SHA_RST));
    RIU[SHARNG_BASE_ADDR + (0x08 << 1)] = (RIU[SHARNG_BASE_ADDR + (0x08 << 1)] & (~SHARNG_CTRL_SHA_RST));
    RIU[SHARNG_BASE_ADDR + (0x5d << 1)] = 0;
}

void HAL_SHA_SetAddress(U32 u32Address)
{
    if (u32Address >= 0x20000000)
    {
        u32Address -= 0x20000000;
    }

    // SHA_SetLength:5c~5d(sha_message_length)
    RIU[(SHARNG_BASE_ADDR + (0x0A << 1))] = (U16)((0x0000ffff) & (u32Address));
    HAL_RSA_DummyRead(0x0A, 1);
    RIU[(SHARNG_BASE_ADDR + (0x0B << 1))] = (U16)(((0xffff0000) & (u32Address)) >> 16);
    HAL_RSA_DummyRead(0x0B, 1);

    // Bypass scatter & gather address
    RIU[(SHARNG_BASE_ADDR + (0x08 << 1))] = ((RIU[(SHARNG_BASE_ADDR + (0x08 << 1))]) | (1 << 11));
}

void HAL_SHA_SetLength(U32 u32Size)
{
    // SHA_SetLength:5c~5d(sha_message_length)
    RIU[(SHARNG_BASE_ADDR + (0x0C << 1))] = (U16)((0x0000ffff) & (u32Size));
    HAL_RSA_DummyRead(0x0C, 1);
    RIU[(SHARNG_BASE_ADDR + (0x0D << 1))] = (U16)(((0xffff0000) & (u32Size)) >> 16);
    HAL_RSA_DummyRead(0x0D, 1);
}

void HAL_SHA_SelMode(U8 u8sha256)
{
    // SHA_SelMode:58~59(sha_ctrl & sha_scattergather_size)
    if (u8sha256 == 1)
    {
        RIU[(SHARNG_BASE_ADDR + (0x08 << 1))] =
            ((RIU[(SHARNG_BASE_ADDR + (0x08 << 1))]) | (SHARNG_CTRL_SHA_SEL_SHA256));
    }
    else
    {
        RIU[(SHARNG_BASE_ADDR + (0x08 << 1))] =
            ((RIU[(SHARNG_BASE_ADDR + (0x08 << 1))]) & (~SHARNG_CTRL_SHA_SEL_SHA256));
    }
    HAL_RSA_DummyRead(0x08, 1);
}

U16 HAL_SHA_GetStatus(void)
{
    return (RIU[(SHARNG_BASE_ADDR + (0x0F << 1))]);
}

void HAL_SHA_Clear(void)
{
    // Set "1" to idle state after reg_read_sha_ready = 1
    RIU[(SHARNG_BASE_ADDR + (0x08 << 1))] = ((RIU[(SHARNG_BASE_ADDR + (0x08 << 1))]) & (~SHARNG_CTRL_SHA_CLR));
    RIU[(SHARNG_BASE_ADDR + (0x08 << 1))] = ((RIU[(SHARNG_BASE_ADDR + (0x08 << 1))]) | (SHARNG_CTRL_SHA_CLR));
}

void HAL_SHA_Start(void)
{
    RIU[(SHARNG_BASE_ADDR + (0x08 << 1))] = ((RIU[(SHARNG_BASE_ADDR + (0x08 << 1))]) | (SHARNG_CTRL_SHA_FIRE_ONCE));
    RIU[(SHARNG_BASE_ADDR + (0x08 << 1))] = ((RIU[(SHARNG_BASE_ADDR + (0x08 << 1))]) & (~SHARNG_CTRL_SHA_FIRE_ONCE));
    HAL_RSA_DummyRead(0x08, 1);
}

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) void HAL_SHA_Out(U32 u32Buf)
#else
void HAL_SHA_Out(U32 u32Buf)
#endif
{
    U32 index;

    // SHA_Out
    for (index = 0; index < 16; index++)
    {
        HAL_RSA_DummyRead((0x10 + index), 2);
        *((U16 *)u32Buf + index) = (RIU[(SHARNG_BASE_ADDR + (0x10 << 1) + index * 2)]);
    }
}
