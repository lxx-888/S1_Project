/*
 * Software WAPI encryption implementation
 * Copyright (c) 2011, sigmastar
 * Author: Janusz Dziedzic <janusz.dziedzic@tieto.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef WAPI_H
#define WAPI_H
#ifdef CONFIG_SSTAR_USE_SW_ENC

#include <linux/skbuff.h>
#include <linux/types.h>
#include "ieee80211_i.h"
#include "key.h"

#ifndef ETH_P_WAPI
#define ETH_P_WAPI 0x88B4
#endif

ieee80211_rx_result ieee80211_crypto_wapi_decrypt(struct ieee80211_rx_data *rx);
#endif
#endif /* WAPI_H */
