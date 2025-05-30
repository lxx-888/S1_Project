/*
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __RC_MINSTREL_H
#define __RC_MINSTREL_H

#ifdef MINSTREL_RSSI_USED
//#define MINSTREL_PHY_DEBUG_PRINT
#endif
struct minstrel_rate {
	int bitrate;
	int rix;

	unsigned int perfect_tx_time;
	unsigned int ack_time;

	int sample_limit;
	unsigned int retry_count;
	unsigned int retry_count_cts;
	unsigned int retry_count_rtscts;
	unsigned int adjusted_retry_count;

	u32 success;
	u32 attempts;
	u32 last_attempts;
	u32 last_success;

	/* parts per thousand */
	u32 cur_prob;
	u32 probability;

	/* per-rate throughput */
	u32 cur_tp;
#ifdef MINSTREL_RSSI_USED
	u64 succ_hist1;
	u64 att_hist1;
	u32 succ_hist2;
	u32 att_hist2;
	u32 success_prob1;
#endif
	u64 succ_hist;
	u64 att_hist;
};

struct minstrel_sta_info {
	unsigned long stats_update;
#ifdef MINSTREL_RSSI_USED
	unsigned long stats_update1;
	unsigned int initial_flag;
#endif
	unsigned int sp_ack_dur;
	unsigned int rate_avg;

	unsigned int lowest_rix;

	unsigned int max_tp_rate;
	unsigned int max_tp_rate2;
	unsigned int max_prob_rate;
	unsigned int packet_count;
	unsigned int sample_count;
	int sample_deferred;

	unsigned int sample_idx;
	unsigned int sample_column;

#ifdef MINSTREL_PHY_DEBUG_PRINT
	unsigned short high_not_sample_cnt;
	unsigned short high_sample_cnt;
	unsigned short low_sample_cnt;
	unsigned short not_sample_cnt;
#endif
	int n_rates;
	struct minstrel_rate *r;
	bool prev_sample;
#ifdef MINSTREL_RSSI_USED
	int mean_rssi; //add rx rssi
	int max_rssi;
	int min_rssi;
	int total_rssi; //add rx rssi
	int rssi_count;
	int table_flag;
	int table_count;
#endif
	/* sampling table */
	u8 *sample_table;

#ifdef CONFIG_MAC80211_SSTAR_DEBUGFS
	struct dentry *dbg_stats;
#endif
};

struct minstrel_priv {
	struct ieee80211_hw *hw;
	bool has_mrr;
	unsigned int cw_min;
	unsigned int cw_max;
	unsigned int max_retry;
	unsigned int ewma_level;
	unsigned int segment_size;
	unsigned int update_interval;
	unsigned int lookaround_rate;
	unsigned int lookaround_rate_mrr;

#ifdef CONFIG_MAC80211_SSTAR_DEBUGFS
	/*
	 * enable fixed rate processing per RC
	 *   - write static index to debugfs:ieee80211/phyX/rc/fixed_rate_idx
	 *   - write -1 to enable RC processing again
	 *   - setting will be applied on next update
	 */
	u32 fixed_rate_idx;
	struct dentry *dbg_fixed_rate;
#endif
};

struct minstrel_debugfs_info {
	size_t len;
	char buf[];
};

extern struct rate_control_ops mac80211_minstrel;
void minstrel_add_sta_debugfs(void *priv, void *priv_sta, struct dentry *dir);
void minstrel_remove_sta_debugfs(void *priv, void *priv_sta);

/* debugfs */
int minstrel_stats_open(struct inode *inode, struct file *file);
ssize_t minstrel_stats_read(struct file *file, char __user *buf, size_t len,
			    loff_t *ppos);
int minstrel_stats_release(struct inode *inode, struct file *file);

#endif
