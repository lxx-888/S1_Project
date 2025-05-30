/* *
 * Copyright (c) 2016, sigmastar
 * Author:
 *
 *Based on apollo code
 * Copyright (C) ST-Ericsson SA 2011
 *
 * Author: Dmitry Tarnyagin <dmitry.tarnyagin@stericsson.com>
 * License terms: GNU General Public License (GPL) version 2
 */

#ifndef SSTAR_APOLLO_PLAT_H_INCLUDED
#define SSTAR_APOLLO_PLAT_H_INCLUDED

/*
*********************************************************
*
*PLATFORM_XUNWEI: based on linux3.0
*
*PLATFORM_SUN6I: based on linux3.3
*
*PLATFORM_FRIENDLY:based on linux3.086
*
*********************************************************
*/
#define PLATFORM_XUNWEI (1)
#define PLATFORM_SUN6I (2)
#define PLATFORM_FRIENDLY (3)
#define PLATFORM_SUN6I_64 (4)
#define PLATFORM_CDLINUX (12)
#define PLATFORM_AMLOGIC_S805 (13)
#define PLATFORM_AMLOGIC_905 (8)
#define PLATFORM_INGENICT31 (22)
#define PLATFORM_INGENICT41 (23)

#ifndef SSTAR_WIFI_PLATFORM
#define SSTAR_WIFI_PLATFORM PLATFORM_INGENICT31
#endif

#define APOLLO_1505 0
#define APOLLO_1601 1
#define APOLLO_1606 0
#define APOLLO_C 2
#define APOLLO_D 3
#define APOLLO_E 4
#define APOLLO_F 5
#define ATHENA_B 6
#define ATHENA_LITE 7
#define ATHENA_LITE_ECO 8
#define ARES_A 9
#define ARES_B 10
#define HERA 11
#define ARES_6012B 12

#ifndef PROJ_TYPE
#define PROJ_TYPE ATHENA_B
#endif

#define CRYSTAL_MODE 0
#define EXTERNAL_MODE 1
#ifndef DCXO_TYPE
#define DCXO_TYPE CRYSTAL_MODE
#endif

#define DPLL_CLOCK_26M 0
#define DPLL_CLOCK_40M 1
#define DPLL_CLOCK_24M 2

#ifndef DPLL_CLOCK
#define DPLL_CLOCK DPLL_CLOCK_40M
#endif

#include <linux/ioport.h>

struct Sstar_platform_data {
	const char *mmc_id;
	const int irq_gpio;
	const int power_gpio;
	const int reset_gpio;
	int (*power_ctrl)(const struct Sstar_platform_data *pdata, bool enable);
	int (*clk_ctrl)(const struct Sstar_platform_data *pdata, bool enable);
	int (*insert_ctrl)(const struct Sstar_platform_data *pdata,
			   bool enable);
};

/* Declaration only. Should be implemented in arch/xxx/mach-yyy */
struct Sstar_platform_data *Sstar_get_platform_data(void);

#endif /* SSTAR_APOLLO_PLAT_H_INCLUDED */
