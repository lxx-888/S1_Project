/*
 * Copyright 2002-2004, Instant802 Networks, Inc.
 * Copyright 2005, Devicescape Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef IEEE80211_KEY_H
#define IEEE80211_KEY_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/crypto.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#include <crypto/arc4.h>
#endif
#include <linux/rcupdate.h>
#include <net/Sstar_mac80211.h>

#define NUM_DEFAULT_KEYS 4
#define NUM_DEFAULT_MGMT_KEYS 2

#define WEP_IV_LEN 4
#define WEP_ICV_LEN 4
#define ALG_CCMP_KEY_LEN 16
#define CCMP_HDR_LEN 8
#define CCMP_MIC_LEN 8
#define CCMP_TK_LEN 16
#define CCMP_PN_LEN 6
#define TKIP_IV_LEN 8
#define TKIP_ICV_LEN 4
#define CMAC_PN_LEN 6
#define WAPI_IV_LEN 18
#define WAPI_ICV_LEN 16

#define NUM_RX_DATA_QUEUES 16

struct ieee80211_local;
struct ieee80211_sub_if_data;
struct sta_info;

/**
 * enum ieee80211_internal_key_flags - internal key flags
 *
 * @KEY_FLAG_UPLOADED_TO_HARDWARE: Indicates that this key is present
 *	in the hardware for TX crypto hardware acceleration.
 * @KEY_FLAG_TAINTED: Key is tainted and packets should be dropped.
 */
enum ieee80211_internal_key_flags {
	KEY_FLAG_UPLOADED_TO_HARDWARE = BIT(0),
	KEY_FLAG_TAINTED = BIT(1),
};

enum ieee80211_internal_tkip_state {
	TKIP_STATE_NOT_INIT,
	TKIP_STATE_PHASE1_DONE,
	TKIP_STATE_PHASE1_HW_UPLOADED,
};

struct tkip_ctx {
	u32 iv32; /* current iv32 */
	u16 iv16; /* current iv16 */
	u16 p1k[5]; /* p1k cache */
	u32 p1k_iv32; /* iv32 for which p1k computed */
	enum ieee80211_internal_tkip_state state;
};

struct ieee80211_key {
	struct ieee80211_local *local;
	struct ieee80211_sub_if_data *sdata;
	struct sta_info *sta;

	/* for sdata list */
	struct list_head list;

	/* protected by key mutex */
	unsigned int flags;

	union {
		struct {
			/* protects tx context */
			spinlock_t txlock;

			/* last used TSC */
			struct tkip_ctx tx;

			/* last received RSC */
			struct tkip_ctx rx[NUM_RX_DATA_QUEUES];
		} tkip;
		struct {
			atomic64_t tx_pn;
			/*
			 * Last received packet number. The first
			 * NUM_RX_DATA_QUEUES counters are used with Data
			 * frames and the last counter is used with Robust
			 * Management frames.
			 */
			u8 rx_pn[NUM_RX_DATA_QUEUES + 1][CCMP_PN_LEN];
			u8 prev_rx_pn[NUM_RX_DATA_QUEUES + 1][6];
#ifdef CONFIG_SSTAR_USE_SW_ENC
			struct crypto_cipher *tfm;
#endif
			u32 replays; /* dot11RSNAStatsCCMPReplays */
		} ccmp;
		struct {
			atomic64_t tx_pn;
			u8 rx_pn[CMAC_PN_LEN];
			struct crypto_cipher *tfm;
			u32 replays; /* dot11RSNAStatsCMACReplays */
			u32 icverrors; /* dot11RSNAStatsCMACICVErrors */
		} aes_cmac;
	} u;

	/* number of times this key has been used */
	int tx_rx_count;

#ifdef CONFIG_MAC80211_SSTAR_DEBUGFS
	struct {
		struct dentry *stalink;
		struct dentry *dir;
		int cnt;
	} debugfs;
#endif

#ifdef CONFIG_MAC80211_SSTAR_ROAMING_CHANGES
	/* rcu: used for non blocking key freeing */
	struct rcu_head rcu;
#endif
	/*
	 * key config, must be last because it contains key
	 * material as variable length member
	 */
	struct ieee80211_key_conf conf;
};
struct ieee80211_key *ieee80211_key_alloc(u32 cipher, int idx, size_t key_len,
					  const u8 *key_data, size_t seq_len,
					  const u8 *seq);
/*
 * Insert a key into data structures (sdata, sta if necessary)
 * to make it used, free old key.
 */
int __must_check ieee80211_key_link(struct ieee80211_key *key,
				    struct ieee80211_sub_if_data *sdata,
				    struct sta_info *sta);
void __ieee80211_key_free(struct ieee80211_key *key);
void ieee80211_key_free(struct ieee80211_local *local,
			struct ieee80211_key *key);
void ieee80211_set_default_key(struct ieee80211_sub_if_data *sdata, int idx,
			       bool uni, bool multi);
void ieee80211_set_default_mgmt_key(struct ieee80211_sub_if_data *sdata,
				    int idx);
void ieee80211_free_keys(struct ieee80211_sub_if_data *sdata);
void ieee80211_enable_keys(struct ieee80211_sub_if_data *sdata);
void ieee80211_disable_keys(struct ieee80211_sub_if_data *sdata);
#define key_mtx_dereference(local, ref)                                        \
	rcu_dereference_protected(ref, lockdep_is_held(&((local)->key_mtx)))

#endif /* IEEE80211_KEY_H */
