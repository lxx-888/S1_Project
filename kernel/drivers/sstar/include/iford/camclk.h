/*
 * camclk.h- Sigmastar
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

#ifndef __CAMCLK_H__
#define __CAMCLK_H__

#define CAMCLK_VOID                   0
#define CAMCLK_utmi_480m              1
#define CAMCLK_mpll_432m              2
#define CAMCLK_upll_384m              3
#define CAMCLK_mpll_345m              4
#define CAMCLK_upll_320m              5
#define CAMCLK_mpll_288m              6
#define CAMCLK_utmi_240m              7
#define CAMCLK_mpll_216m              8
#define CAMCLK_utmi_192m              9
#define CAMCLK_mpll_172m              10
#define CAMCLK_utmi_160m              11
#define CAMCLK_mpll_123m              12
#define CAMCLK_mpll_86m               13
#define CAMCLK_mpll_288m_div2         14
#define CAMCLK_mpll_288m_div4         15
#define CAMCLK_mpll_288m_div8         16
#define CAMCLK_mpll_216m_div2         17
#define CAMCLK_mpll_216m_div4         18
#define CAMCLK_mpll_216m_div8         19
#define CAMCLK_mpll_123m_div2         20
#define CAMCLK_mpll_86m_div2          21
#define CAMCLK_mpll_86m_div4          22
#define CAMCLK_mpll_86m_div16         23
#define CAMCLK_utmi_192m_div4         24
#define CAMCLK_utmi_160m_div4         25
#define CAMCLK_utmi_160m_div5         26
#define CAMCLK_utmi_160m_div8         27
#define CAMCLK_xtali_12m              28
#define CAMCLK_xtali_12m_div2         29
#define CAMCLK_xtali_12m_div4         30
#define CAMCLK_xtali_12m_div8         31
#define CAMCLK_xtali_12m_div16        32
#define CAMCLK_xtali_12m_div40        33
#define CAMCLK_xtali_12m_div64        34
#define CAMCLK_xtali_12m_div128       35
#define CAMCLK_xtali_24m              36
#define CAMCLK_RTC_CLK_32K            37
#define CAMCLK_pm_riu_w_clk_in        38
#define CAMCLK_miupll_clk             39
#define CAMCLK_ddrpll_clk             40
#define CAMCLK_lpll_clk               41
#define CAMCLK_ven_pll                42
#define CAMCLK_ven_pll_div6           43
#define CAMCLK_lpll_div2              44
#define CAMCLK_lpll_div4              45
#define CAMCLK_lpll_div8              46
#define CAMCLK_armpll_37p125m         47
#define CAMCLK_riu_w_clk_in           48
#define CAMCLK_riu_w_clk_top          49
#define CAMCLK_riu_w_clk_sc_gp        50
#define CAMCLK_riu_w_clk_vhe_gp       51
#define CAMCLK_riu_w_clk_hemcu_gp     52
#define CAMCLK_riu_w_clk_mipi_if_gp   53
#define CAMCLK_riu_w_clk_mcu_if_gp    54
#define CAMCLK_fuart0_synth_out       55
#define CAMCLK_miu_p                  56
#define CAMCLK_mspi0_p                57
#define CAMCLK_mspi1_p                58
#define CAMCLK_miu_vhe_gp_p           59
#define CAMCLK_miu_sc_gp_p            60
#define CAMCLK_mcu_p                  61
#define CAMCLK_fclk1_p                62
#define CAMCLK_sdio_p                 63
#define CAMCLK_tck_buf                64
#define CAMCLK_eth_buf                65
#define CAMCLK_rmii_buf               66
#define CAMCLK_emac_testrx125_in_lan  67
#define CAMCLK_gop0                   68
#define CAMCLK_rtc_32k                69
#define CAMCLK_fro                    70
#define CAMCLK_fro_div2               71
#define CAMCLK_fro_div8               72
#define CAMCLK_fro_div16              73
#define CAMCLK_cpupll_clk             74
#define CAMCLK_utmi                   75
#define CAMCLK_bach                   76
#define CAMCLK_miu                    77
#define CAMCLK_miu_boot               78
#define CAMCLK_ddr_syn                79
#define CAMCLK_miu_rec                80
#define CAMCLK_mcu                    81
#define CAMCLK_riubrdg                82
#define CAMCLK_bdma                   83
#define CAMCLK_spi_arb                84
#define CAMCLK_spi_flash              85
#define CAMCLK_pwm                    86
#define CAMCLK_uart0                  87
#define CAMCLK_uart1                  88
#define CAMCLK_fuart0_synth_in        89
#define CAMCLK_fuart                  90
#define CAMCLK_mspi0                  91
#define CAMCLK_mspi1                  92
#define CAMCLK_mspi                   93
#define CAMCLK_miic0                  94
#define CAMCLK_miic1                  95
#define CAMCLK_miic2                  96
#define CAMCLK_bist                   97
#define CAMCLK_pwr_ctl                98
#define CAMCLK_xtali                  99
#define CAMCLK_live                   100
#define CAMCLK_sr00_mclk              101
#define CAMCLK_sr01_mclk              102
#define CAMCLK_sr1_mclk               103
#define CAMCLK_bist_pm                104
#define CAMCLK_bist_ipu_gp            105
#define CAMCLK_ipu                    106
#define CAMCLK_ipuff                  107
#define CAMCLK_bist_usb30_gp          108
#define CAMCLK_csi_mac_lptx_top_i_m00 109
#define CAMCLK_csi_mac_top_i_m00      110
#define CAMCLK_ns_top_i_m00           111
#define CAMCLK_csi_mac_lptx_top_i_m01 112
#define CAMCLK_csi_mac_top_i_m01      113
#define CAMCLK_ns_top_i_m01           114
#define CAMCLK_csi_mac_lptx_top_i_m1  115
#define CAMCLK_csi_mac_top_i_m1       116
#define CAMCLK_ns_top_i_m1            117
#define CAMCLK_mipi1_tx_csi           118
#define CAMCLK_bist_vhe_gp            119
#define CAMCLK_vhe                    120
#define CAMCLK_mfe                    121
#define CAMCLK_xtali_sc_gp            122
#define CAMCLK_bist_sc_gp             123
#define CAMCLK_emac_ahb               124
#define CAMCLK_jpe                    125
#define CAMCLK_aesdma                 126
#define CAMCLK_sdio                   127
#define CAMCLK_sd                     128
#define CAMCLK_ecc                    129
#define CAMCLK_isp                    130
#define CAMCLK_fclk1                  131
#define CAMCLK_odclk                  132
#define CAMCLK_dip                    133
#define CAMCLK_emac_tx                134
#define CAMCLK_emac_rx                135
#define CAMCLK_emac_tx_ref            136
#define CAMCLK_emac_rx_ref            137
#define CAMCLK_ive                    138
#define CAMCLK_ldcfeye                139
#define CAMCLK_live_pm                140
#define CAMCLK_mcu_pm_p1              141
#define CAMCLK_spi_pm                 142
#define CAMCLK_miic_pm                143
#define CAMCLK_pm_sleep               144
#define CAMCLK_rtc                    145
#define CAMCLK_sar                    146
#define CAMCLK_pir                    147
#define CAMCLK_pm_uart                148
#endif
