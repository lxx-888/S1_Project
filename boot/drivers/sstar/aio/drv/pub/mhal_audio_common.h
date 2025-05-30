/*
 * mhal_audio_common.h - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#ifndef __MHAL_AUDIO_COMMON_H__

#define __MHAL_AUDIO_COMMON_H__

// -------------------------------------------------------------------------------
#define AIO_BIT0  0x00000001
#define AIO_BIT1  0x00000002
#define AIO_BIT2  0x00000004
#define AIO_BIT3  0x00000008
#define AIO_BIT4  0x00000010
#define AIO_BIT5  0x00000020
#define AIO_BIT6  0x00000040
#define AIO_BIT7  0x00000080
#define AIO_BIT8  0x00000100
#define AIO_BIT9  0x00000200
#define AIO_BIT10 0x00000400
#define AIO_BIT11 0x00000800
#define AIO_BIT12 0x00001000
#define AIO_BIT13 0x00002000
#define AIO_BIT14 0x00004000
#define AIO_BIT15 0x00008000
#define AIO_BIT16 0x00010000
#define AIO_BIT17 0x00020000
#define AIO_BIT18 0x00040000
#define AIO_BIT19 0x00080000
#define AIO_BIT20 0x00100000
#define AIO_BIT21 0x00200000
#define AIO_BIT22 0x00400000
#define AIO_BIT23 0x00800000
#define AIO_BIT24 0x01000000
#define AIO_BIT25 0x02000000
#define AIO_BIT26 0x04000000
#define AIO_BIT27 0x08000000
#define AIO_BIT28 0x10000000
#define AIO_BIT29 0x20000000
#define AIO_BIT30 0x40000000
#define AIO_BIT31 0x80000000

// -------------------------------------------------------------------------------
typedef enum
{
    E_MHAL_AI_DMA_A,
    E_MHAL_AI_DMA_B,
    E_MHAL_AI_DMA_C,
    E_MHAL_AI_DMA_D,
    E_MHAL_AI_DMA_E,
    E_MHAL_AI_DMA_DIRECT_A,
    E_MHAL_AI_DMA_DIRECT_B,
    E_MHAL_AI_DMA_TOTAL,

} MHAL_AI_Dma_e;

typedef enum
{
    E_MHAL_AO_DMA_A,
    E_MHAL_AO_DMA_B,
    E_MHAL_AO_DMA_C,
    E_MHAL_AO_DMA_D,
    E_MHAL_AO_DMA_E,
    E_MHAL_AO_DMA_DIRECT_A,
    E_MHAL_AO_DMA_DIRECT_B,
    E_MHAL_AO_DMA_TOTAL,

} MHAL_AO_Dma_e;

typedef enum
{
    E_MHAL_AI_DMA_CH_SLOT_0_1,
    E_MHAL_AI_DMA_CH_SLOT_2_3,
    E_MHAL_AI_DMA_CH_SLOT_4_5,
    E_MHAL_AI_DMA_CH_SLOT_6_7,
    E_MHAL_AI_DMA_CH_SLOT_TOTAL,

} MHAL_AI_DmaChSlot_e;

typedef enum
{
    E_MHAL_AO_DMA_CH_SLOT_0,
    E_MHAL_AO_DMA_CH_SLOT_1,
    E_MHAL_AO_DMA_CH_SLOT_TOTAL,

} MHAL_AO_DmaChSlot_e;

typedef enum
{
    E_MHAL_AI_IF_NONE = 0,
    E_MHAL_AI_IF_ADC_A_0_B_0,
    E_MHAL_AI_IF_ADC_C_0_D_0,
    E_MHAL_AI_IF_DMIC_A_0_1,
    E_MHAL_AI_IF_DMIC_A_2_3,
    E_MHAL_AI_IF_HDMI_A_0_1,
    E_MHAL_AI_IF_ECHO_A_0_1,
    E_MHAL_AI_IF_I2S_RX_A_0_1,
    E_MHAL_AI_IF_I2S_RX_A_2_3,
    E_MHAL_AI_IF_I2S_RX_A_4_5,
    E_MHAL_AI_IF_I2S_RX_A_6_7,
    E_MHAL_AI_IF_I2S_RX_A_8_9,
    E_MHAL_AI_IF_I2S_RX_A_10_11,
    E_MHAL_AI_IF_I2S_RX_A_12_13,
    E_MHAL_AI_IF_I2S_RX_A_14_15,
    E_MHAL_AI_IF_I2S_RX_B_0_1,
    E_MHAL_AI_IF_I2S_RX_B_2_3,
    E_MHAL_AI_IF_I2S_RX_B_4_5,
    E_MHAL_AI_IF_I2S_RX_B_6_7,
    E_MHAL_AI_IF_I2S_RX_B_8_9,
    E_MHAL_AI_IF_I2S_RX_B_10_11,
    E_MHAL_AI_IF_I2S_RX_B_12_13,
    E_MHAL_AI_IF_I2S_RX_B_14_15,
    E_MHAL_AI_IF_I2S_RX_C_0_1,
    E_MHAL_AI_IF_I2S_RX_C_2_3,
    E_MHAL_AI_IF_I2S_RX_C_4_5,
    E_MHAL_AI_IF_I2S_RX_C_6_7,
    E_MHAL_AI_IF_I2S_RX_C_8_9,
    E_MHAL_AI_IF_I2S_RX_C_10_11,
    E_MHAL_AI_IF_I2S_RX_C_12_13,
    E_MHAL_AI_IF_I2S_RX_C_14_15,
    E_MHAL_AI_IF_I2S_RX_D_0_1,
    E_MHAL_AI_IF_I2S_RX_D_2_3,
    E_MHAL_AI_IF_I2S_RX_D_4_5,
    E_MHAL_AI_IF_I2S_RX_D_6_7,
    E_MHAL_AI_IF_I2S_RX_D_8_9,
    E_MHAL_AI_IF_I2S_RX_D_10_11,
    E_MHAL_AI_IF_I2S_RX_D_12_13,
    E_MHAL_AI_IF_I2S_RX_D_14_15,

} MHAL_AI_IF_e;

typedef enum
{
    E_MHAL_AO_IF_NONE       = 0,
    E_MHAL_AO_IF_DAC_A_0    = AIO_BIT0,
    E_MHAL_AO_IF_DAC_B_0    = AIO_BIT1,
    E_MHAL_AO_IF_DAC_C_0    = AIO_BIT2,
    E_MHAL_AO_IF_DAC_D_0    = AIO_BIT3,
    E_MHAL_AO_IF_HDMI_A_0   = AIO_BIT4,
    E_MHAL_AO_IF_HDMI_A_1   = AIO_BIT5,
    E_MHAL_AO_IF_ECHO_A_0   = AIO_BIT6,
    E_MHAL_AO_IF_ECHO_A_1   = AIO_BIT7,
    E_MHAL_AO_IF_I2S_TX_A_0 = AIO_BIT8,
    E_MHAL_AO_IF_I2S_TX_A_1 = AIO_BIT9,
    E_MHAL_AO_IF_I2S_TX_B_0 = AIO_BIT10,
    E_MHAL_AO_IF_I2S_TX_B_1 = AIO_BIT11,

} MHAL_AO_IF_e;

typedef enum
{
    E_MHAL_AO_CH_MODE_STEREO = 0,
    E_MHAL_AO_CH_MODE_DOUBLE_MONO,
    E_MHAL_AO_CH_MODE_DOUBLE_LEFT,
    E_MHAL_AO_CH_MODE_DOUBLE_RIGHT,
    E_MHAL_AO_CH_MODE_EXCHANGE,
    E_MHAL_AO_CH_MODE_ONLY_LEFT,
    E_MHAL_AO_CH_MODE_ONLY_RIGHT,

} MHAL_AO_ChMode_e;

typedef enum
{
    E_MHAL_SINEGEN_AI_DMA_A,
    E_MHAL_SINEGEN_AI_DMA_B,
    E_MHAL_SINEGEN_AI_DMA_C,
    E_MHAL_SINEGEN_AI_DMA_D,
    E_MHAL_SINEGEN_AO_DMA_A,
    E_MHAL_SINEGEN_AO_DMA_B,
    E_MHAL_SINEGEN_AO_DMA_C,
    E_MHAL_SINEGEN_AO_DMA_D,
    E_MHAL_SINEGEN_AI_IF_DMIC_A,

} MHAL_SineGen_e;

typedef enum
{
    E_MHAL_SINEGEN_FREQ_250HZ,
    E_MHAL_SINEGEN_FREQ_500HZ,
    E_MHAL_SINEGEN_FREQ_1000HZ,
    E_MHAL_SINEGEN_FREQ_1500HZ,
    E_MHAL_SINEGEN_FREQ_2000HZ,
    E_MHAL_SINEGEN_FREQ_3000HZ,
    E_MHAL_SINEGEN_FREQ_4000HZ,
    E_MHAL_SINEGEN_FREQ_6000HZ,
    E_MHAL_SINEGEN_FREQ_8000HZ,
    E_MHAL_SINEGEN_FREQ_12000HZ,
    E_MHAL_SINEGEN_FREQ_16000HZ,

} MHAL_SineGen_Freq_e;

// -------------------------------------------------------------------------------

#endif //__MHAL_AUDIO_COMMON_H__
