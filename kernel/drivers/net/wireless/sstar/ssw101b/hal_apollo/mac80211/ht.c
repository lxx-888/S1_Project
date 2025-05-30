/*
 * HT handling
 *
 * Copyright 2003, Jouni Malinen <jkmaline@cc.hut.fi>
 * Copyright 2002-2005, Instant802 Networks, Inc.
 * Copyright 2005-2006, Devicescape Software, Inc.
 * Copyright 2006-2007	Jiri Benc <jbenc@suse.cz>
 * Copyright 2007, Michael Wu <flamingice@sourmilk.net>
 * Copyright 2007-2010, Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/ieee80211.h>
#include <linux/export.h>
#include <net/Sstar_mac80211.h>
#include "ieee80211_i.h"
#include "rate.h"

void ieee80211_ht_cap_ie_to_sta_ht_cap(struct ieee80211_supported_band *sband,
				       const struct ieee80211_ht_cap *ht_cap_ie,
				       struct ieee80211_sta_ht_cap *ht_cap)
{
	u8 ampdu_info, tx_mcs_set_cap;
	int i, max_tx_streams;

	BUG_ON(!ht_cap);

	memset(ht_cap, 0, sizeof(*ht_cap));

	if (!ht_cap_ie || !sband->ht_cap.ht_supported)
		return;

	ht_cap->ht_supported = true;

	/*
	 * The bits listed in this expression should be
	 * the same for the peer and us, if the station
	 * advertises more then we can't use those thus
	 * we mask them out.
	 */
	ht_cap->cap = le16_to_cpu(ht_cap_ie->cap_info) &
		      (sband->ht_cap.cap |
		       ~(IEEE80211_HT_CAP_LDPC_CODING |
			 IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
			 IEEE80211_HT_CAP_GRN_FLD | IEEE80211_HT_CAP_SGI_20 |
			 IEEE80211_HT_CAP_SGI_40 | IEEE80211_HT_CAP_DSSSCCK40));
	/*
	 * The STBC bits are asymmetric -- if we don't have
	 * TX then mask out the peer's RX and vice versa.
	 */
	if (!(sband->ht_cap.cap & IEEE80211_HT_CAP_TX_STBC))
		ht_cap->cap &= ~IEEE80211_HT_CAP_RX_STBC;
	if (!(sband->ht_cap.cap & IEEE80211_HT_CAP_RX_STBC))
		ht_cap->cap &= ~IEEE80211_HT_CAP_TX_STBC;

	ampdu_info = ht_cap_ie->ampdu_params_info;
	ht_cap->ampdu_factor = ampdu_info & IEEE80211_HT_AMPDU_PARM_FACTOR;
	ht_cap->ampdu_density =
		(ampdu_info & IEEE80211_HT_AMPDU_PARM_DENSITY) >> 2;

	/* own MCS TX capabilities */
	tx_mcs_set_cap = sband->ht_cap.mcs.tx_params;

	/* Copy peer MCS TX capabilities, the driver might need them. */
	ht_cap->mcs.tx_params = ht_cap_ie->mcs.tx_params;

	/* can we TX with MCS rates? */
	if (!(tx_mcs_set_cap & IEEE80211_HT_MCS_TX_DEFINED))
		return;

	/* Counting from 0, therefore +1 */
	if (tx_mcs_set_cap & IEEE80211_HT_MCS_TX_RX_DIFF)
		max_tx_streams = ((tx_mcs_set_cap &
				   IEEE80211_HT_MCS_TX_MAX_STREAMS_MASK) >>
				  IEEE80211_HT_MCS_TX_MAX_STREAMS_SHIFT) +
				 1;
	else
		max_tx_streams = IEEE80211_HT_MCS_TX_MAX_STREAMS;

	/*
	 * 802.11n-2009 20.3.5 / 20.6 says:
	 * - indices 0 to 7 and 32 are single spatial stream
	 * - 8 to 31 are multiple spatial streams using equal modulation
	 *   [8..15 for two streams, 16..23 for three and 24..31 for four]
	 * - remainder are multiple spatial streams using unequal modulation
	 */
	for (i = 0; i < max_tx_streams; i++)
		ht_cap->mcs.rx_mask[i] = sband->ht_cap.mcs.rx_mask[i] &
					 ht_cap_ie->mcs.rx_mask[i];

	if (tx_mcs_set_cap & IEEE80211_HT_MCS_TX_UNEQUAL_MODULATION)
		for (i = IEEE80211_HT_MCS_UNEQUAL_MODULATION_START_BYTE;
		     i < IEEE80211_HT_MCS_MASK_LEN; i++)
			ht_cap->mcs.rx_mask[i] = sband->ht_cap.mcs.rx_mask[i] &
						 ht_cap_ie->mcs.rx_mask[i];

	/* handle MCS rate 32 too */
	if (sband->ht_cap.mcs.rx_mask[32 / 8] & ht_cap_ie->mcs.rx_mask[32 / 8] &
	    1)
		ht_cap->mcs.rx_mask[32 / 8] |= 1;
}
void ieee80211_ht_cap_to_sta_channel_type(struct sta_info *sta)
{
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	char vif_channel_type = 0;
	char sta_channel_type =
		!!(sta->sta.ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40);
	if (sdata == NULL) {
		return;
	}
	switch (vif_chw(&sdata->vif)) {
	case NL80211_CHAN_NO_HT:
	case NL80211_CHAN_HT20:
		vif_channel_type = 0;
		break;
	case NL80211_CHAN_HT40PLUS:
	case NL80211_CHAN_HT40MINUS:
		vif_channel_type = 1;
		break;
	}

	if (sta_channel_type == vif_channel_type) {
		if (vif_channel_type) {
			set_sta_flag(sta, WLAN_STA_40M_CH);
			clear_sta_flag(sta, WLAN_STA_40M_CH_SEND_20M);
			Sstar_printk_always("[%pM]:40M channel\n",
					    sta->sta.addr);
		} else {
			clear_sta_flag(sta, WLAN_STA_40M_CH);
			Sstar_printk_always("[%pM]:20M channel\n",
					    sta->sta.addr);
		}
	} else {
		clear_sta_flag(sta, WLAN_STA_40M_CH);
		Sstar_printk_always("[%pM]:20M channel\n", sta->sta.addr);
	}
}
void ieee80211_sta_tear_down_BA_sessions(struct sta_info *sta, bool tx)
{
	int i;

	Sstar_cancel_work_sync(&sta->ampdu_mlme.work);

	for (i = 0; i < STA_TID_NUM; i++) {
#ifdef CONFIG_SSTAR_SW_AGGTX
		__ieee80211_stop_tx_ba_session(sta, i, WLAN_BACK_INITIATOR, tx);
#endif
		__ieee80211_stop_rx_ba_session(sta, i, WLAN_BACK_RECIPIENT,
					       WLAN_REASON_QSTA_LEAVE_QBSS, tx);
	}
}

void ieee80211_ba_session_work(struct Sstar_work_struct *work)
{
	struct sta_info *sta =
		container_of(work, struct sta_info, ampdu_mlme.work);
#ifdef CONFIG_SSTAR_SW_AGGTX
	struct tid_ampdu_tx *tid_tx;
#endif
	int tid;

	/*
	 * When this flag is set, new sessions should be
	 * blocked, and existing sessions will be torn
	 * down by the code that set the flag, so this
	 * need not run.
	 */
	if (test_sta_flag(sta, WLAN_STA_BLOCK_BA))
		return;

	mutex_lock(&sta->ampdu_mlme.mtx);
	for (tid = 0; tid < STA_TID_NUM; tid++) {
		if (test_and_clear_bit(tid,
				       sta->ampdu_mlme.tid_rx_timer_expired))
			___ieee80211_stop_rx_ba_session(
				sta, tid, WLAN_BACK_RECIPIENT,
				WLAN_REASON_QSTA_TIMEOUT, true);

		if (test_and_clear_bit(tid,
				       sta->ampdu_mlme.tid_rx_stop_requested))
			___ieee80211_stop_rx_ba_session(sta, tid,
							WLAN_BACK_RECIPIENT,
							WLAN_REASON_UNSPECIFIED,
							true);
#ifdef CONFIG_SSTAR_SW_AGGTX
		tid_tx = sta->ampdu_mlme.tid_start_tx[tid];
		if (tid_tx) {
			/*
			 * Assign it over to the normal tid_tx array
			 * where it "goes live".
			 */
			spin_lock_bh(&sta->lock);

			sta->ampdu_mlme.tid_start_tx[tid] = NULL;
			/* could there be a race? */
			if (sta->ampdu_mlme.tid_tx[tid])
				Sstar_kfree(tid_tx);
			else
				ieee80211_assign_tid_tx(sta, tid, tid_tx);
			spin_unlock_bh(&sta->lock);

			ieee80211_tx_ba_session_handle_start(sta, tid);
			continue;
		}

		tid_tx = rcu_dereference_protected_tid_tx(sta, tid);
		if (tid_tx &&
		    test_and_clear_bit(HT_AGG_STATE_WANT_STOP, &tid_tx->state))
			___ieee80211_stop_tx_ba_session(
				sta, tid, WLAN_BACK_INITIATOR, true);
#endif
	}
	mutex_unlock(&sta->ampdu_mlme.mtx);
}
#ifdef CONFIG_SSTAR_DRIVER_PROCESS_BA
void ieee80211_send_delba(struct ieee80211_sub_if_data *sdata, const u8 *da,
			  u16 tid, u16 initiator, u16 reason_code)
{
	struct ieee80211_local *local = sdata->local;
	struct sk_buff *skb;
	struct Sstar_ieee80211_mgmt *mgmt;
	u16 params;

	skb = Sstar_dev_alloc_skb(sizeof(*mgmt) + local->hw.extra_tx_headroom);
	if (!skb)
		return;

	Sstar_skb_reserve(skb, local->hw.extra_tx_headroom);
	mgmt = (struct Sstar_ieee80211_mgmt *)Sstar_skb_put(skb, 24);
	memset(mgmt, 0, 24);
	memcpy(mgmt->da, da, ETH_ALEN);
	memcpy(mgmt->sa, sdata->vif.addr, ETH_ALEN);
	if (sdata->vif.type == NL80211_IFTYPE_AP ||
	    sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
		memcpy(mgmt->bssid, sdata->vif.addr, ETH_ALEN);
	else if (sdata->vif.type == NL80211_IFTYPE_STATION)
		memcpy(mgmt->bssid, sdata->u.mgd.bssid, ETH_ALEN);

	mgmt->frame_control =
		cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);

	Sstar_skb_put(skb, 1 + sizeof(mgmt->u.action.u.delba));

	mgmt->u.action.category = SSTAR_WLAN_CATEGORY_BACK;
	mgmt->u.action.u.delba.action_code = WLAN_ACTION_DELBA;
	params = (u16)(initiator << 11); /* bit 11 initiator */
	params |= (u16)(tid << 12); /* bit 15:12 TID number */

	mgmt->u.action.u.delba.params = cpu_to_le16(params);
	mgmt->u.action.u.delba.reason_code = cpu_to_le16(reason_code);

	ieee80211_tx_skb(sdata, skb);
}
#endif
void ieee80211_process_delba(struct ieee80211_sub_if_data *sdata,
			     struct sta_info *sta,
			     struct Sstar_ieee80211_mgmt *mgmt, size_t len)
{
	u16 tid, params;
	u16 initiator;

	params = le16_to_cpu(mgmt->u.action.u.delba.params);
	tid = (params & IEEE80211_DELBA_PARAM_TID_MASK) >> 12;
	initiator = (params & IEEE80211_DELBA_PARAM_INITIATOR_MASK) >> 11;

#ifdef CONFIG_MAC80211_SSTAR_HT_DEBUG
	if (net_ratelimit())
		Sstar_printk_always(
			"delba from %pM (%s) tid %d reason code %d\n", mgmt->sa,
			initiator ? "initiator" : "recipient", tid,
			le16_to_cpu(mgmt->u.action.u.delba.reason_code));
#endif /* CONFIG_MAC80211_SSTAR_HT_DEBUG */

	if (initiator == WLAN_BACK_INITIATOR)
		__ieee80211_stop_rx_ba_session(sta, tid, WLAN_BACK_INITIATOR, 0,
					       true);
#ifdef CONFIG_SSTAR_SW_AGGTX
	else
		__ieee80211_stop_tx_ba_session(sta, tid, WLAN_BACK_RECIPIENT,
					       true);
#endif
}
#ifdef CONFIG_SSTAR_SMPS
int ieee80211_send_smps_action(struct ieee80211_sub_if_data *sdata,
			       enum ieee80211_smps_mode smps, const u8 *da,
			       const u8 *bssid)
{
	struct ieee80211_local *local = sdata->local;
	struct sk_buff *skb;
	struct Sstar_ieee80211_mgmt *action_frame;

	/* 27 = header + category + action + smps mode */
	skb = Sstar_dev_alloc_skb(27 + local->hw.extra_tx_headroom);
	if (!skb)
		return -ENOMEM;

	Sstar_skb_reserve(skb, local->hw.extra_tx_headroom);
	action_frame = (void *)Sstar_skb_put(skb, 27);
	memcpy(action_frame->da, da, ETH_ALEN);
	memcpy(action_frame->sa, sdata->dev->dev_addr, ETH_ALEN);
	memcpy(action_frame->bssid, bssid, ETH_ALEN);
	action_frame->frame_control =
		cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
	action_frame->u.action.category = SSTAR_WLAN_CATEGORY_HT;
	action_frame->u.action.u.ht_smps.action = WLAN_HT_ACTION_SMPS;
	switch (smps) {
	case IEEE80211_SMPS_AUTOMATIC:
	case IEEE80211_SMPS_NUM_MODES:
		WARN_ON(1);
	case IEEE80211_SMPS_OFF:
		action_frame->u.action.u.ht_smps.smps_control =
			WLAN_HT_SMPS_CONTROL_DISABLED;
		break;
	case IEEE80211_SMPS_STATIC:
		action_frame->u.action.u.ht_smps.smps_control =
			WLAN_HT_SMPS_CONTROL_STATIC;
		break;
	case IEEE80211_SMPS_DYNAMIC:
		action_frame->u.action.u.ht_smps.smps_control =
			WLAN_HT_SMPS_CONTROL_DYNAMIC;
		break;
	}

	/* we'll do more on status of this frame */
	IEEE80211_SKB_CB(skb)->flags |= IEEE80211_TX_CTL_REQ_TX_STATUS;
	ieee80211_tx_skb(sdata, skb);

	return 0;
}

void ieee80211_request_smps_work(struct Sstar_work_struct *work)
{
	struct ieee80211_sub_if_data *sdata = container_of(
		work, struct ieee80211_sub_if_data, u.mgd.request_smps_work);

	mutex_lock(&sdata->u.mgd.mtx);
	__ieee80211_request_smps(sdata, sdata->u.mgd.driver_smps_mode);
	mutex_unlock(&sdata->u.mgd.mtx);
}
void ieee80211_request_smps(struct ieee80211_vif *vif,
			    enum ieee80211_smps_mode smps_mode)
{
	struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);

	if (WARN_ON(vif->type != NL80211_IFTYPE_STATION))
		return;

	if (WARN_ON(smps_mode == IEEE80211_SMPS_OFF))
		smps_mode = IEEE80211_SMPS_AUTOMATIC;

	sdata->u.mgd.driver_smps_mode = smps_mode;

	ieee80211_queue_work(&sdata->local->hw,
			     &sdata->u.mgd.request_smps_work);
}
#endif
/* this might change ... don't want non-open drivers using it */
//EXPORT_SYMBOL_GPL(ieee80211_request_smps);
