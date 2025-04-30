/*
 * hal_ddrfreq.c - Sigmastar
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
#include <linux/devfreq.h>
#include <linux/delay.h>
#include "registers.h"
#include "ms_platform.h"
#include "hal_ddrfreq.h"

typedef struct st_hal_ddrfreq
{
    /* sdram type */
    SSTAR_DRAM_TYPE type;
    /* frequency parameters */
    unsigned long cur_freq;
    unsigned int  cur_ddfset;
    /* 4X/8X mode */
    unsigned int xmode;
    /* timing parameters */
    unsigned int t_refi;
    /* initial flag */
    bool initialized;
} hal_ddrfreq;

typedef struct
{
    unsigned int   sidd;
    unsigned int   osc;
    unsigned int   osc_threshold_x10;
    unsigned int   sidd_hreshold;
    SSTAR_OSC_TYPE osc_type;
} osc_info;

/* ROSC/20>12.12: FF, others: SS */
static osc_info m_osc_data = {0, 0, 2424, 0, 0};

static hal_ddrfreq m_ddrfreq;
// static DEFINE_SPINLOCK(ddrfreq_lock);

#if IS_ENABLED(CONFIG_MSTAR_MIU)
extern void miu_bw_get_max_load(ulong *busy, ulong *total);
#endif
extern void recode_timestamp(int mark, const char *name);

static inline void hal_ddrfreq_ddfset(int from, int to, int step)
{
    return;
}

static int hal_ddrfreq_set_rate(unsigned int mhz)
{
    unsigned int ddr_region, target = 0;
    unsigned int pll_ch, pll_bank;

    pll_ch = INREGMSK16(BASE_REG_MIU_DFS1_AC + REG_ID_58, 0x01);
    if (pll_ch == 0)
        pll_bank = BASE_REG_MIU_ATOP;
    else
        pll_bank = BASE_REG_MIU_ATOP_G;

    ddr_region = INREGMSK16(pll_bank + REG_ID_1A, 0x7C) >> 2;
    ddr_region = ddr_region ? ddr_region : 1;
    ddr_region *= 1 << INREGMSK16(pll_bank + REG_ID_1A, 0x03);

    target = ((u64)(432) << 19) * ddr_region / mhz;

    m_ddrfreq.cur_ddfset = target;
    m_ddrfreq.cur_freq   = mhz * 1000000;

    // for dbg
    // printk("%s %d mhz=%d target=%d cur_freq=%lX\r\n", __FUNCTION__, __LINE__, mhz, target, m_ddrfreq.cur_freq);

    // switch ddr freq
    OUTREG16(BASE_REG_MIU_DFS1_AC + REG_ID_50, 0x01);

    if (INREGMSK16(BASE_REG_MIU_DFS1_AC + REG_ID_58, 0x02))
    {
        mdelay(1);
    }
    // printk("%s %d set pll_ch=%x\r\n", __FUNCTION__, __LINE__, INREGMSK16(BASE_REG_MIU_DFS1_AC + REG_ID_58, 0x01));

    return 0;
}

int hal_ddrfreq_target(unsigned long freq)
{
    if (freq == m_ddrfreq.cur_freq)
        return 0;

    return hal_ddrfreq_set_rate((unsigned int)(freq / 1000000));
}

int hal_ddrfreq_get_load(struct devfreq_dev_status *stat)
{
    stat->current_frequency = m_ddrfreq.cur_freq;

    return 0;
}

static unsigned long read_ddrfreq(unsigned char pll_ch)
{
    unsigned int freq_mhz;
    unsigned int ddr_region, ddfset;
    unsigned int pll_bank;

    if (pll_ch == 0)
        pll_bank = BASE_REG_MIU_ATOP;
    else
        pll_bank = BASE_REG_MIU_ATOP_G;

    ddfset     = (INREGMSK16(pll_bank + REG_ID_19, 0xFF) << 16) + INREG16(pll_bank + REG_ID_18);
    ddr_region = INREGMSK16(pll_bank + REG_ID_1A, 0x7C) >> 2;
    ddr_region = ddr_region ? ddr_region : 1;
    ddr_region *= 1 << INREGMSK16(pll_bank + REG_ID_1A, 0x03);
    freq_mhz = ((u64)432 << 19) * ddr_region / ddfset;

    return freq_mhz;
}

unsigned long hal_ddrfreq_get_freq(void)
{
    unsigned int freq_mhz;
    unsigned int ddfset;
    unsigned int pll_ch, pll_bank;

    pll_ch = INREGMSK16(BASE_REG_MIU_DFS1_AC + REG_ID_58, 0x01);
    if (pll_ch == 0)
        pll_bank = BASE_REG_MIU_ATOP;
    else
        pll_bank = BASE_REG_MIU_ATOP_G;

    ddfset   = (INREGMSK16(pll_bank + REG_ID_19, 0xFF) << 16) + INREG16(pll_bank + REG_ID_18);
    freq_mhz = read_ddrfreq(pll_ch);

    m_ddrfreq.cur_freq   = freq_mhz * 1000000;
    m_ddrfreq.cur_ddfset = ddfset;

    // check ddr 4X/8X mode
    switch (INREGMSK16(BASE_REG_MIU_DIG + REG_ID_01, 0x0300) >> 8)
    {
        case 0:
            m_ddrfreq.xmode = 1;
            break;
        case 1:
            m_ddrfreq.xmode = 2;
            break;
        case 2:
            m_ddrfreq.xmode = 4;
            break;
        case 3:
            m_ddrfreq.xmode = 8;
            break;
    }

    // printk("%s %d freq_mhz=%d, ddfset=%x ddr_mode=%dX\r\n", __FUNCTION__, __LINE__, freq_mhz, m_ddrfreq.cur_ddfset,
    //       m_ddrfreq.xmode);

    return m_ddrfreq.cur_freq;
}

SSTAR_DRAM_TYPE hal_ddrfreq_get_sdram_type(void)
{
    int ddr_type = SSTAR_DRAM_TYPE_UNKNOWN;

    if (m_ddrfreq.initialized && m_ddrfreq.type)
        return m_ddrfreq.type;

    if (INREGMSK16(BASE_REG_MIU_ATOP + REG_ID_5A, 0x0100))
    {
        // 0x1647 0x5a [8] -> 0: not lp, 1: lp
        if (INREGMSK16(BASE_REG_MIU_ATOP + REG_ID_5B, 0x8000))
        {
            return SSTAR_TYPE_LPDDR3;
        }
        else
        {
            if (INREGMSK16(BASE_REG_MIU_ATOP + REG_ID_72, 0x0010))
                return SSTAR_TYPE_LPDDR4X;
            else
                return SSTAR_TYPE_LPDDR4;
        }
    }
    else
    {
        if (INREGMSK16(BASE_REG_MIU_ATOP + REG_ID_5B, 0x8000))
            return SSTAR_TYPE_DDR3;
        else
            return SSTAR_TYPE_DDR4;
    }

    return ddr_type;
}

SSTAR_OSC_TYPE hal_ddrfreq_get_osc(void)
{
    return m_osc_data.osc_type;
}

void hal_ddrfreq_init(void)
{
    memset(&m_ddrfreq, 0, sizeof(hal_ddrfreq));
    m_ddrfreq.type        = hal_ddrfreq_get_sdram_type();
    m_ddrfreq.initialized = 1;

    m_osc_data.sidd = INREG16(BASE_REG_OTP2_PA + REG_ID_0A) & 0x3FF;
    m_osc_data.osc  = INREG16(BASE_REG_OTP2_PA + REG_ID_0B) & 0x3FF;
    if ((m_osc_data.osc * 10) > m_osc_data.osc_threshold_x10)
        m_osc_data.osc_type = E_OSC_TYPE1;
    else
        m_osc_data.osc_type = E_OSC_TYPE0;
}
