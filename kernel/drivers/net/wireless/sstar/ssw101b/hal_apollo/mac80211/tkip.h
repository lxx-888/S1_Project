/*
 * Copyright 2002-2004, Instant802 Networks, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef TKIP_H
#define TKIP_H
#ifdef CONFIG_SSTAR_USE_SW_ENC
#include <linux/types.h>
#include <linux/crypto.h>
#include "key.h"

u8 *ieee80211_tkip_add_iv(u8 *pos, struct ieee80211_key *key);

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 4, 0))
int ieee80211_tkip_encrypt_data(struct crypto_cipher *tfm,
				struct ieee80211_key *key, struct sk_buff *skb,
				u8 *payload, size_t payload_len);

enum {
	TKIP_DECRYPT_OK = 0,
	TKIP_DECRYPT_NO_EXT_IV = -1,
	TKIP_DECRYPT_INVALID_KEYIDX = -2,
	TKIP_DECRYPT_REPLAY = -3,
};
int ieee80211_tkip_decrypt_data(struct crypto_cipher *tfm,
				struct ieee80211_key *key, u8 *payload,
				size_t payload_len, u8 *ta, u8 *ra, int only_iv,
				int queue, u32 *out_iv32, u16 *out_iv16);
#else
int ieee80211_tkip_encrypt_data(struct arc4_ctx *ctx, struct ieee80211_key *key,
				struct sk_buff *skb, u8 *payload,
				size_t payload_len);

enum {
	TKIP_DECRYPT_OK = 0,
	TKIP_DECRYPT_NO_EXT_IV = -1,
	TKIP_DECRYPT_INVALID_KEYIDX = -2,
	TKIP_DECRYPT_REPLAY = -3,
};
int ieee80211_tkip_decrypt_data(struct arc4_ctx *ctx, struct ieee80211_key *key,
				u8 *payload, size_t payload_len, u8 *ta, u8 *ra,
				int only_iv, int queue, u32 *out_iv32,
				u16 *out_iv16);
#endif
#endif /* CONFIG_SSTAR_USE_SW_ENC */
#endif /* TKIP_H */
