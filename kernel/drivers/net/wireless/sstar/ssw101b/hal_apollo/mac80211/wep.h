/*
 * Software WEP encryption implementation
 * Copyright 2002, Jouni Malinen <jkmaline@cc.hut.fi>
 * Copyright 2003, Instant802 Networks, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef WEP_H
#define WEP_H
#ifdef CONFIG_SSTAR_USE_SW_ENC
#include <linux/skbuff.h>
#include <linux/types.h>
#include "ieee80211_i.h"
#include "key.h"

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 4, 0))
int ieee80211_wep_init(struct ieee80211_local *local);
void ieee80211_wep_free(struct ieee80211_local *local);
bool ieee80211_wep_is_weak_iv(struct sk_buff *skb, struct ieee80211_key *key);
int ieee80211_wep_encrypt_data(struct crypto_cipher *tfm, u8 *rc4key,
			       size_t klen, u8 *data, size_t data_len);
int ieee80211_wep_encrypt(struct ieee80211_local *local, struct sk_buff *skb,
			  const u8 *key, int keylen, int keyidx);
int ieee80211_wep_decrypt_data(struct crypto_cipher *tfm, u8 *rc4key,
			       size_t klen, u8 *data, size_t data_len);
#else
void ieee80211_wep_init(struct ieee80211_local *local);
bool ieee80211_wep_is_weak_iv(struct sk_buff *skb, struct ieee80211_key *key);
int ieee80211_wep_encrypt_data(struct arc4_ctx *ctx, u8 *rc4key, size_t klen,
			       u8 *data, size_t data_len);
int ieee80211_wep_encrypt(struct ieee80211_local *local, struct sk_buff *skb,
			  const u8 *key, int keylen, int keyidx);
int ieee80211_wep_decrypt_data(struct arc4_ctx *ctx, u8 *rc4key, size_t klen,
			       u8 *data, size_t data_len);
#endif
//bool ieee80211_wep_is_weak_iv(struct sk_buff *skb, struct ieee80211_key *key);

ieee80211_rx_result ieee80211_crypto_wep_decrypt(struct ieee80211_rx_data *rx);
ieee80211_tx_result ieee80211_crypto_wep_encrypt(struct ieee80211_tx_data *tx);
#endif
#endif /* WEP_H */
