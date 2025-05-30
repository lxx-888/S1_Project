#ifndef __MAC80211_DRIVER_OPS
#define __MAC80211_DRIVER_OPS

#include <net/Sstar_mac80211.h>
#include "ieee80211_i.h"
#include "driver-trace.h"

static inline void drv_tx(struct ieee80211_local *local, struct sk_buff *skb)
{
	local->ops->tx(&local->hw, skb);
}

static inline int drv_start(struct ieee80211_local *local)
{
	int ret;

	might_sleep();

	trace_drv_start(local);
	local->started = true;
	smp_mb();
	ret = local->ops->start(&local->hw);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline void drv_stop(struct ieee80211_local *local)
{
	might_sleep();

	trace_drv_stop(local);
	local->ops->stop(&local->hw);
	trace_drv_return_void(local);

	/* sync away all work on the tasklet before clearing started */
	tasklet_disable(&local->tasklet);
	tasklet_enable(&local->tasklet);

	barrier();

	local->started = false;
}

#ifdef CONFIG_PM
static inline int drv_suspend(struct ieee80211_local *local,
			      struct cfg80211_wowlan *wowlan)
{
	int ret;

	might_sleep();

	trace_drv_suspend(local);
	ret = local->ops->suspend(&local->hw, wowlan);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline int drv_resume(struct ieee80211_local *local)
{
	int ret;

	might_sleep();

	trace_drv_resume(local);
	ret = local->ops->resume(&local->hw);
	trace_drv_return_int(local, ret);
	return ret;
}
#endif

static inline int drv_add_interface(struct ieee80211_local *local,
				    struct ieee80211_vif *vif)
{
	int ret;

	might_sleep();

	trace_drv_add_interface(local, vif_to_sdata(vif));
	ret = local->ops->add_interface(&local->hw, vif);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline int drv_change_interface(struct ieee80211_local *local,
				       struct ieee80211_sub_if_data *sdata,
				       enum nl80211_iftype type, bool p2p)
{
	int ret;

	might_sleep();

	trace_drv_change_interface(local, sdata, type, p2p);
	ret = local->ops->change_interface(&local->hw, &sdata->vif, type, p2p);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline void drv_remove_interface(struct ieee80211_local *local,
					struct ieee80211_vif *vif)
{
	might_sleep();

	trace_drv_remove_interface(local, vif_to_sdata(vif));
	local->ops->remove_interface(&local->hw, vif);
	trace_drv_return_void(local);
}

static inline int drv_config(struct ieee80211_local *local, u32 changed)
{
	int ret;

	might_sleep();

	trace_drv_config(local, changed);
	ret = local->ops->config(&local->hw, changed);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline void drv_bss_info_changed(struct ieee80211_local *local,
					struct ieee80211_sub_if_data *sdata,
					struct ieee80211_bss_conf *info,
					u32 changed)
{
	might_sleep();

	trace_drv_bss_info_changed(local, sdata, info, changed);
	if (local->ops->bss_info_changed)
		local->ops->bss_info_changed(&local->hw, &sdata->vif, info,
					     changed);
	trace_drv_return_void(local);
}

static inline int drv_tx_sync(struct ieee80211_local *local,
			      struct ieee80211_sub_if_data *sdata,
			      const u8 *bssid, enum ieee80211_tx_sync_type type)
{
	int ret = 0;

	might_sleep();

	trace_drv_tx_sync(local, sdata, bssid, type);
	if (local->ops->tx_sync)
		ret = local->ops->tx_sync(&local->hw, &sdata->vif, bssid, type);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline void drv_finish_tx_sync(struct ieee80211_local *local,
				      struct ieee80211_sub_if_data *sdata,
				      const u8 *bssid,
				      enum ieee80211_tx_sync_type type)
{
	might_sleep();

	trace_drv_finish_tx_sync(local, sdata, bssid, type);
	if (local->ops->finish_tx_sync)
		local->ops->finish_tx_sync(&local->hw, &sdata->vif, bssid,
					   type);
	trace_drv_return_void(local);
}

static inline u64 drv_prepare_multicast(struct ieee80211_local *local,
					struct ieee80211_sub_if_data *sdata,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
					struct netdev_hw_addr_list *mc_list)
#else
					int mc_count,
					struct dev_addr_list *mc_list)
#endif
{
	u64 ret = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	trace_drv_prepare_multicast(local, sdata, mc_list->count);
#else
	trace_drv_prepare_multicast(local, sdata, mc_count);
#endif

	if (local->ops->prepare_multicast)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
		ret = local->ops->prepare_multicast(&local->hw, &sdata->vif,
						    mc_list);
#else
		ret = local->ops->prepare_multicast(&local->hw, &sdata->vif,
						    mc_count, mc_list);
#endif

	trace_drv_return_u64(local, ret);

	return ret;
}

static inline void drv_configure_filter(struct ieee80211_local *local,
					struct ieee80211_sub_if_data *sdata,
					unsigned int changed_flags,
					unsigned int *total_flags,
					u64 multicast)
{
	might_sleep();

	trace_drv_configure_filter(local, sdata, changed_flags, total_flags,
				   multicast);
	local->ops->configure_filter(&local->hw, &sdata->vif, changed_flags,
				     total_flags, multicast);
	trace_drv_return_void(local);
}

static inline int drv_set_tim(struct ieee80211_local *local,
			      struct ieee80211_sta *sta, bool set)
{
	int ret = 0;
	trace_drv_set_tim(local, sta, set);
	if (local->ops->set_tim)
		ret = local->ops->set_tim(&local->hw, sta, set);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline int drv_set_key(struct ieee80211_local *local,
			      enum set_key_cmd cmd,
			      struct ieee80211_sub_if_data *sdata,
			      struct ieee80211_sta *sta,
			      struct ieee80211_key_conf *key)
{
	int ret;

	might_sleep();

	trace_drv_set_key(local, cmd, sdata, sta, key);
	ret = local->ops->set_key(&local->hw, cmd, &sdata->vif, sta, key);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline void drv_update_tkip_key(struct ieee80211_local *local,
				       struct ieee80211_sub_if_data *sdata,
				       struct ieee80211_key_conf *conf,
				       struct sta_info *sta, u32 iv32,
				       u16 *phase1key)
{
	struct ieee80211_sta *ista = NULL;

	if (sta)
		ista = &sta->sta;

	trace_drv_update_tkip_key(local, sdata, conf, ista, iv32);
	if (local->ops->update_tkip_key)
		local->ops->update_tkip_key(&local->hw, &sdata->vif, conf, ista,
					    iv32, phase1key);
	trace_drv_return_void(local);
}

static inline int drv_hw_scan(struct ieee80211_local *local,
			      struct ieee80211_sub_if_data *sdata,
			      struct ieee80211_scan_req_wrap *req)
{
	int ret;

	might_sleep();

	trace_drv_hw_scan(local, sdata);
	ret = local->ops->hw_scan(&local->hw, &sdata->vif, req);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline void drv_cancel_hw_scan(struct ieee80211_local *local,
				      struct ieee80211_sub_if_data *sdata)
{
	might_sleep();

	trace_drv_cancel_hw_scan(local, sdata);
	local->ops->cancel_hw_scan(&local->hw, &sdata->vif);
	trace_drv_return_void(local);
}

static inline int drv_sched_scan_start(struct ieee80211_local *local,
				       struct ieee80211_sub_if_data *sdata,
				       struct cfg80211_sched_scan_request *req,
				       struct ieee80211_sched_scan_ies *ies)
{
	int ret;

	might_sleep();

	trace_drv_sched_scan_start(local, sdata);
	ret = local->ops->sched_scan_start(&local->hw, &sdata->vif, req, ies);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline void drv_sched_scan_stop(struct ieee80211_local *local,
				       struct ieee80211_sub_if_data *sdata)
{
	might_sleep();

	trace_drv_sched_scan_stop(local, sdata);
	local->ops->sched_scan_stop(&local->hw, &sdata->vif);
	trace_drv_return_void(local);
}

static inline void drv_sw_scan_start(struct ieee80211_local *local)
{
	might_sleep();

	trace_drv_sw_scan_start(local);
	if (local->ops->sw_scan_start)
		local->ops->sw_scan_start(&local->hw);
	trace_drv_return_void(local);
}

static inline void drv_sw_scan_complete(struct ieee80211_local *local)
{
	might_sleep();

	trace_drv_sw_scan_complete(local);
	if (local->ops->sw_scan_complete)
		local->ops->sw_scan_complete(&local->hw);
	trace_drv_return_void(local);
}

static inline int drv_get_stats(struct ieee80211_local *local,
				struct ieee80211_low_level_stats *stats)
{
	int ret = -EOPNOTSUPP;

	might_sleep();

	if (local->ops->get_stats)
		ret = local->ops->get_stats(&local->hw, stats);
	trace_drv_get_stats(local, stats, ret);

	return ret;
}

static inline void drv_get_tkip_seq(struct ieee80211_local *local,
				    u8 hw_key_idx, u32 *iv32, u16 *iv16)
{
	if (local->ops->get_tkip_seq)
		local->ops->get_tkip_seq(&local->hw, hw_key_idx, iv32, iv16);
	trace_drv_get_tkip_seq(local, hw_key_idx, iv32, iv16);
}

static inline int drv_set_frag_threshold(struct ieee80211_local *local,
					 u32 value)
{
	int ret = 0;

	might_sleep();

	trace_drv_set_frag_threshold(local, value);
	if (local->ops->set_frag_threshold)
		ret = local->ops->set_frag_threshold(&local->hw, value);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline int drv_set_rts_threshold(struct ieee80211_local *local,
					struct ieee80211_sub_if_data *sdata,
					u32 value)
{
	int ret = 0;

	might_sleep();

	trace_drv_set_rts_threshold(local, sdata, value);
	if (local->ops->set_rts_threshold)
		ret = local->ops->set_rts_threshold(&local->hw, &sdata->vif,
						    value);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline int drv_set_coverage_class(struct ieee80211_local *local,
					 u8 value)
{
	int ret = 0;
	might_sleep();

	trace_drv_set_coverage_class(local, value);
	if (local->ops->set_coverage_class)
		local->ops->set_coverage_class(&local->hw, value);
	else
		ret = -EOPNOTSUPP;

	trace_drv_return_int(local, ret);
	return ret;
}

static inline void drv_sta_notify(struct ieee80211_local *local,
				  struct ieee80211_sub_if_data *sdata,
				  enum sta_notify_cmd cmd,
				  struct ieee80211_sta *sta)
{
	trace_drv_sta_notify(local, sdata, cmd, sta);
	if (local->ops->sta_notify)
		local->ops->sta_notify(&local->hw, &sdata->vif, cmd, sta);
	trace_drv_return_void(local);
}

static inline int drv_sta_add(struct ieee80211_local *local,
			      struct ieee80211_sub_if_data *sdata,
			      struct ieee80211_sta *sta)
{
	int ret = 0;

	might_sleep();

	trace_drv_sta_add(local, sdata, sta);
	if (local->ops->sta_add)
		ret = local->ops->sta_add(&local->hw, &sdata->vif, sta);

	trace_drv_return_int(local, ret);

	return ret;
}

static inline void drv_sta_remove(struct ieee80211_local *local,
				  struct ieee80211_sub_if_data *sdata,
				  struct ieee80211_sta *sta)
{
	might_sleep();

	trace_drv_sta_remove(local, sdata, sta);
	if (local->ops->sta_remove)
		local->ops->sta_remove(&local->hw, &sdata->vif, sta);

	trace_drv_return_void(local);
}

static inline int drv_conf_tx(struct ieee80211_local *local,
			      struct ieee80211_sub_if_data *sdata, u16 ac,
			      const struct ieee80211_tx_queue_params *params)
{
	int ret = -EOPNOTSUPP;

	might_sleep();

	trace_drv_conf_tx(local, sdata, ac, params);
	if (local->ops->conf_tx)
		ret = local->ops->conf_tx(&local->hw, &sdata->vif, ac, params);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline u64 drv_get_tsf(struct ieee80211_local *local,
			      struct ieee80211_sub_if_data *sdata)
{
	u64 ret = -1ULL;

	might_sleep();

	trace_drv_get_tsf(local, sdata);
	if (local->ops->get_tsf)
		ret = local->ops->get_tsf(&local->hw, &sdata->vif);
	trace_drv_return_u64(local, ret);
	return ret;
}

static inline void drv_set_tsf(struct ieee80211_local *local,
			       struct ieee80211_sub_if_data *sdata, u64 tsf)
{
	might_sleep();

	trace_drv_set_tsf(local, sdata, tsf);
	if (local->ops->set_tsf)
		local->ops->set_tsf(&local->hw, &sdata->vif, tsf);
	trace_drv_return_void(local);
}

static inline void drv_reset_tsf(struct ieee80211_local *local,
				 struct ieee80211_sub_if_data *sdata)
{
	might_sleep();

	trace_drv_reset_tsf(local, sdata);
	if (local->ops->reset_tsf)
		local->ops->reset_tsf(&local->hw, &sdata->vif);
	trace_drv_return_void(local);
}

static inline int drv_tx_last_beacon(struct ieee80211_local *local)
{
	int ret = 0; /* default unsuported op for less congestion */

	might_sleep();

	trace_drv_tx_last_beacon(local);
	if (local->ops->tx_last_beacon)
		ret = local->ops->tx_last_beacon(&local->hw);
	trace_drv_return_int(local, ret);
	return ret;
}

static inline int drv_ampdu_action(struct ieee80211_local *local,
				   struct ieee80211_sub_if_data *sdata,
				   enum ieee80211_ampdu_mlme_action action,
				   struct ieee80211_sta *sta, u16 tid, u16 *ssn,
				   u8 buf_size)
{
	int ret = -EOPNOTSUPP;

	might_sleep();

	trace_drv_ampdu_action(local, sdata, action, sta, tid, ssn, buf_size);

	if (local->ops->ampdu_action)
		ret = local->ops->ampdu_action(&local->hw, &sdata->vif, action,
					       sta, tid, ssn, buf_size);

	trace_drv_return_int(local, ret);

	return ret;
}

static inline int drv_get_survey(struct ieee80211_local *local, int idx,
				 struct survey_info *survey)
{
	int ret = -EOPNOTSUPP;

	trace_drv_get_survey(local, idx, survey);

	if (local->ops->get_survey)
		ret = local->ops->get_survey(&local->hw, idx, survey);

	trace_drv_return_int(local, ret);

	return ret;
}

static inline void drv_rfkill_poll(struct ieee80211_local *local)
{
	might_sleep();

	if (local->ops->rfkill_poll)
		local->ops->rfkill_poll(&local->hw);
}

static inline void drv_flush(struct ieee80211_local *local,
			     struct ieee80211_sub_if_data *sdata, bool drop)
{
	might_sleep();

	trace_drv_flush(local, sdata, drop);
	if (local->ops->flush)
		local->ops->flush(&local->hw, &sdata->vif, drop);
	trace_drv_return_void(local);
}

static inline void
drv_channel_switch(struct ieee80211_local *local,
		   struct ieee80211_channel_switch *ch_switch)
{
	might_sleep();

	trace_drv_channel_switch(local, ch_switch);
	local->ops->channel_switch(&local->hw, ch_switch);
	trace_drv_return_void(local);
}

static inline int drv_set_antenna(struct ieee80211_local *local, u32 tx_ant,
				  u32 rx_ant)
{
	int ret = -EOPNOTSUPP;
	might_sleep();
	if (local->ops->set_antenna)
		ret = local->ops->set_antenna(&local->hw, tx_ant, rx_ant);
	trace_drv_set_antenna(local, tx_ant, rx_ant, ret);
	return ret;
}

static inline int drv_get_antenna(struct ieee80211_local *local, u32 *tx_ant,
				  u32 *rx_ant)
{
	int ret = -EOPNOTSUPP;
	might_sleep();
	if (local->ops->get_antenna)
		ret = local->ops->get_antenna(&local->hw, tx_ant, rx_ant);
	trace_drv_get_antenna(local, *tx_ant, *rx_ant, ret);
	return ret;
}

static inline int drv_remain_on_channel(struct ieee80211_local *local,
					struct ieee80211_sub_if_data *sdata,
					struct ieee80211_channel *chan,
					enum nl80211_channel_type chantype,
					unsigned int duration, u64 cookie)
{
	int ret;

	might_sleep();

	trace_drv_remain_on_channel(local, chan, chantype, duration);
	ret = local->ops->remain_on_channel(&local->hw, &sdata->vif, chan,
					    chantype, duration, cookie);
	trace_drv_return_int(local, ret);

	return ret;
}

static inline int drv_cancel_remain_on_channel(struct ieee80211_local *local)
{
	int ret;

	might_sleep();

	trace_drv_cancel_remain_on_channel(local);
	ret = local->ops->cancel_remain_on_channel(&local->hw);
	trace_drv_return_int(local, ret);

	return ret;
}

static inline int drv_set_ringparam(struct ieee80211_local *local, u32 tx,
				    u32 rx)
{
	int ret = -ENOTSUPP;

	might_sleep();

	trace_drv_set_ringparam(local, tx, rx);
	if (local->ops->set_ringparam)
		ret = local->ops->set_ringparam(&local->hw, tx, rx);
	trace_drv_return_int(local, ret);

	return ret;
}

static inline void drv_get_ringparam(struct ieee80211_local *local, u32 *tx,
				     u32 *tx_max, u32 *rx, u32 *rx_max)
{
	might_sleep();

	trace_drv_get_ringparam(local, tx, tx_max, rx, rx_max);
	if (local->ops->get_ringparam)
		local->ops->get_ringparam(&local->hw, tx, tx_max, rx, rx_max);
	trace_drv_return_void(local);
}

static inline bool drv_tx_frames_pending(struct ieee80211_local *local)
{
	bool ret = false;

	might_sleep();

	trace_drv_tx_frames_pending(local);
	if (local->ops->tx_frames_pending)
		ret = local->ops->tx_frames_pending(&local->hw);
	trace_drv_return_bool(local, ret);

	return ret;
}

static inline int drv_set_bitrate_mask(struct ieee80211_local *local,
				       struct ieee80211_sub_if_data *sdata,
				       const struct cfg80211_bitrate_mask *mask)
{
	int ret = -EOPNOTSUPP;

	might_sleep();

	trace_drv_set_bitrate_mask(local, sdata, mask);
	if (local->ops->set_bitrate_mask)
		ret = local->ops->set_bitrate_mask(&local->hw, &sdata->vif,
						   mask);
	trace_drv_return_int(local, ret);

	return ret;
}
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 0, 8))
static inline void drv_set_rekey_data(struct ieee80211_local *local,
				      struct ieee80211_sub_if_data *sdata,
				      struct cfg80211_gtk_rekey_data *data)
{
	trace_drv_set_rekey_data(local, sdata, data);
	if (local->ops->set_rekey_data)
		local->ops->set_rekey_data(&local->hw, &sdata->vif, data);
	trace_drv_return_void(local);
}
#endif
static inline void drv_rssi_callback(struct ieee80211_local *local,
				     const enum ieee80211_rssi_event event)
{
	trace_drv_rssi_callback(local, event);
	if (local->ops->rssi_callback)
		local->ops->rssi_callback(&local->hw, event);
	trace_drv_return_void(local);
}

static inline void
drv_release_buffered_frames(struct ieee80211_local *local, struct sta_info *sta,
			    u16 tids, int num_frames,
			    enum ieee80211_frame_release_type reason,
			    bool more_data)
{
	trace_drv_release_buffered_frames(local, &sta->sta, tids, num_frames,
					  reason, more_data);
	if (local->ops->release_buffered_frames)
		local->ops->release_buffered_frames(&local->hw, &sta->sta, tids,
						    num_frames, reason,
						    more_data);
	trace_drv_return_void(local);
}

static inline void
drv_allow_buffered_frames(struct ieee80211_local *local, struct sta_info *sta,
			  u16 tids, int num_frames,
			  enum ieee80211_frame_release_type reason,
			  bool more_data)
{
	trace_drv_allow_buffered_frames(local, &sta->sta, tids, num_frames,
					reason, more_data);
	if (local->ops->allow_buffered_frames)
		local->ops->allow_buffered_frames(&local->hw, &sta->sta, tids,
						  num_frames, reason,
						  more_data);
	trace_drv_return_void(local);
}

static inline void drv_set_data_filter(struct ieee80211_local *local,
				       struct ieee80211_sub_if_data *sdata,
				       void *data, int len)
{
	//trace_drv_set_data_filter(local, data, len);
	if (local->ops->set_data_filter)
		local->ops->set_data_filter(&local->hw, &sdata->vif, data, len);

	trace_drv_return_void(local);
}
#endif /* __MAC80211_DRIVER_OPS */
