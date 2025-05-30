/*
 * Copyright 2002-2005, Instant802 Networks, Inc.
 * Copyright 2005-2006, Devicescape Software, Inc.
 * Copyright 2006-2007	Jiri Benc <jbenc@suse.cz>
 * Copyright 2007-2010	Johannes Berg <johannes@sipsolutions.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rcupdate.h>
#include <linux/export.h>
#include <net/Sstar_mac80211.h>
#include <net/ieee80211_radiotap.h>

#include "ieee80211_i.h"
#include "driver-ops.h"
#include "led.h"
#include "mesh.h"
#include "wep.h"
#include "wpa.h"
#include "tkip.h"
#include "wme.h"
#include "wapi.h"

static bool ieee80211_frame_eapol(u8 *data, int len);
/*
 * monitor mode reception
 *
 * This function cleans up the SKB, i.e. it removes all the stuff
 * only useful for monitoring.
 */
static struct sk_buff *remove_monitor_info(struct ieee80211_local *local,
					   struct sk_buff *skb)
{
	if (local->hw.flags & IEEE80211_HW_RX_INCLUDES_FCS) {
		if (likely(skb->len > FCS_LEN))
			__Sstar_pskb_trim(skb, skb->len - FCS_LEN);
		else {
			/* driver bug */
			WARN_ON(1);
			Sstar_dev_kfree_skb(skb);
			skb = NULL;
		}
	}

	return skb;
}

static inline int should_drop_frame(struct sk_buff *skb, int present_fcs_len)
{
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;

	if (status->flag & (RX_FLAG_FAILED_FCS_CRC | RX_FLAG_FAILED_PLCP_CRC))
		return 1;
	if (unlikely(skb->len < 16 + present_fcs_len))
		return 1;
	if (ieee80211_is_ctl(hdr->frame_control) &&
	    !ieee80211_is_pspoll(hdr->frame_control) &&
	    !ieee80211_is_back_req(hdr->frame_control))
		return 1;
	return 0;
}

static int ieee80211_rx_radiotap_len(struct ieee80211_local *local,
				     struct ieee80211_rx_status *status)
{
	int len;

	/* always present fields */
	len = sizeof(struct ieee80211_radiotap_header) + 9;

	if (status->flag & RX_FLAG_MACTIME_MPDU)
		len += 8;
	if (local->hw.flags & IEEE80211_HW_SIGNAL_DBM)
		len += 1;

	if (len & 1) /* padding for RX_FLAGS if necessary */
		len++;

	if (status->flag & RX_FLAG_HT) /* HT info */
		len += 3;

	return len;
}

/*
 * ieee80211_add_rx_radiotap_header - add radiotap header
 *
 * add a radiotap header containing all the fields which the hardware provided.
 */
static void ieee80211_add_rx_radiotap_header(struct ieee80211_local *local,
					     struct sk_buff *skb,
					     struct ieee80211_rate *rate,
					     int rtap_len)
{
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
	struct ieee80211_radiotap_header *rthdr;
	unsigned char *pos;
	u16 rx_flags = 0;

	rthdr = (struct ieee80211_radiotap_header *)Sstar_skb_push(skb,
								   rtap_len);
	memset(rthdr, 0, rtap_len);

	/* radiotap header, set always present flags */
	rthdr->it_present = cpu_to_le32((1 << IEEE80211_RADIOTAP_FLAGS) |
					(1 << IEEE80211_RADIOTAP_CHANNEL) |
					(1 << IEEE80211_RADIOTAP_ANTENNA) |
					(1 << IEEE80211_RADIOTAP_RX_FLAGS));
	rthdr->it_len = cpu_to_le16(rtap_len);

	pos = (unsigned char *)(rthdr + 1);

	/* the order of the following fields is important */

	/* IEEE80211_RADIOTAP_TSFT */
	if (status->flag & RX_FLAG_MACTIME_MPDU) {
		put_unaligned_le64(status->mactime, pos);
		rthdr->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_TSFT);
		pos += 8;
	}

	/* IEEE80211_RADIOTAP_FLAGS */
	if (local->hw.flags & IEEE80211_HW_RX_INCLUDES_FCS)
		*pos |= IEEE80211_RADIOTAP_F_FCS;
	if (status->flag & (RX_FLAG_FAILED_FCS_CRC | RX_FLAG_FAILED_PLCP_CRC))
		*pos |= IEEE80211_RADIOTAP_F_BADFCS;
	if (status->flag & RX_FLAG_SHORTPRE)
		*pos |= IEEE80211_RADIOTAP_F_SHORTPRE;
	pos++;

	/* IEEE80211_RADIOTAP_RATE */
	if (!rate || status->flag & RX_FLAG_HT) {
		/*
		 * Without rate information don't add it. If we have,
		 * MCS information is a separate field in radiotap,
		 * added below. The byte here is needed as padding
		 * for the channel though, so initialise it to 0.
		 */
		*pos = 0;
	} else {
		rthdr->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_RATE);
		*pos = rate->bitrate / 5;
	}
	pos++;

	/* IEEE80211_RADIOTAP_CHANNEL */
	put_unaligned_le16(status->freq, pos);
	pos += 2;
	if (status->band == IEEE80211_BAND_5GHZ)
		put_unaligned_le16(IEEE80211_CHAN_OFDM | IEEE80211_CHAN_5GHZ,
				   pos);
	else if (status->flag & RX_FLAG_HT)
		put_unaligned_le16(IEEE80211_CHAN_DYN | IEEE80211_CHAN_2GHZ,
				   pos);
	else if (rate && rate->flags & IEEE80211_RATE_ERP_G)
		put_unaligned_le16(IEEE80211_CHAN_OFDM | IEEE80211_CHAN_2GHZ,
				   pos);
	else if (rate)
		put_unaligned_le16(IEEE80211_CHAN_CCK | IEEE80211_CHAN_2GHZ,
				   pos);
	else
		put_unaligned_le16(IEEE80211_CHAN_2GHZ, pos);
	pos += 2;

	/* IEEE80211_RADIOTAP_DBM_ANTSIGNAL */
	if (local->hw.flags & IEEE80211_HW_SIGNAL_DBM) {
		*pos = status->signal;
		rthdr->it_present |=
			cpu_to_le32(1 << IEEE80211_RADIOTAP_DBM_ANTSIGNAL);
		pos++;
	}

	/* IEEE80211_RADIOTAP_LOCK_QUALITY is missing */

	/* IEEE80211_RADIOTAP_ANTENNA */
	*pos = status->antenna;
	pos++;

	/* IEEE80211_RADIOTAP_DB_ANTNOISE is not used */

	/* IEEE80211_RADIOTAP_RX_FLAGS */
	/* ensure 2 byte alignment for the 2 byte field as required */
	if ((pos - (u8 *)rthdr) & 1)
		pos++;
	if (status->flag & RX_FLAG_FAILED_PLCP_CRC)
		rx_flags |= IEEE80211_RADIOTAP_F_RX_BADPLCP;
	put_unaligned_le16(rx_flags, pos);
	pos += 2;

	if (status->flag & RX_FLAG_HT) {
		rthdr->it_present |= cpu_to_le32(1 << IEEE80211_RADIOTAP_MCS);
		*pos++ = IEEE80211_RADIOTAP_MCS_HAVE_MCS |
			 IEEE80211_RADIOTAP_MCS_HAVE_GI |
			 IEEE80211_RADIOTAP_MCS_HAVE_BW;
		*pos = 0;
		if (status->flag & RX_FLAG_SHORT_GI)
			*pos |= IEEE80211_RADIOTAP_MCS_SGI;
		if (status->flag & RX_FLAG_40MHZ)
			*pos |= IEEE80211_RADIOTAP_MCS_BW_40;
		pos++;
		*pos++ = status->rate_idx;
	}
}
#ifdef CONFIG_SSTAR_MONITOR_HDR_PRISM
static bool ieee80211_add_rx_prism_header(struct ieee80211_sub_if_data *sdata,
					  struct sk_buff *skb,
					  struct ieee80211_rate *rate)
{
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_prism_header *prism_header;
	size_t hdr_len = ieee80211_hdrlen(hdr->frame_control);
	size_t data_size = skb->len - hdr_len;

	if (Sstar_skb_headroom(skb) < sizeof(*prism_header) &&
	    Sstar_pskb_expand_head(skb, sizeof(*prism_header), 0, GFP_ATOMIC))
		goto fail;
	prism_header = (void *)Sstar_skb_push(skb, sizeof(*prism_header));
	memset(prism_header, 0, sizeof(*prism_header));

	prism_header->msg_code = ieee80211_prism_sniffrm;
	prism_header->msg_len = sizeof(struct ieee80211_prism_header);
	memcpy(prism_header->dev_name, sdata->name, IFNAMSIZ);

	prism_header->hosttime.did = ieee80211_prism_sniffrm_hosttime;
	prism_header->hosttime.status = 0;
	prism_header->hosttime.len = 4;
	prism_header->hosttime.data = jiffies;

	prism_header->mactime.did = ieee80211_prism_sniffrm_mactime;
	prism_header->mactime.status = 0;
	prism_header->mactime.len = 0;
	prism_header->mactime.data = 0;

	prism_header->istx.did = ieee80211_prism_sniffrm_istx;
	prism_header->istx.status = 0;
	prism_header->istx.len = 0;
	prism_header->istx.data = 0;

	prism_header->channel.did = ieee80211_prism_sniffrm_channel;
	prism_header->channel.status = 0;
	prism_header->channel.len = 4;
	prism_header->channel.data = status->freq;

	prism_header->rssi.did = ieee80211_prism_sniffrm_rssi;
	prism_header->rssi.status = 0;
	prism_header->rssi.len = 4;
	prism_header->rssi.data = status->signal;

	prism_header->signal.did = ieee80211_prism_sniffrm_signal;
	prism_header->signal.status = 0;
	prism_header->signal.len = 4;
	prism_header->signal.data = 0;

	prism_header->noise.did = ieee80211_prism_sniffrm_noise;
	prism_header->noise.status = 0;
	prism_header->noise.len = 4;
	prism_header->noise.data = 0;

	prism_header->rate.did = ieee80211_prism_sniffrm_rate;
	prism_header->rate.status = 0;
	prism_header->rate.len = 4;
	prism_header->rate.data = 0;

	prism_header->frmlen.did = ieee80211_prism_sniffrm_frmlen;
	prism_header->frmlen.status = 0;
	prism_header->frmlen.len = 4;
	prism_header->frmlen.data = data_size;

	return true;
fail:
	return false;
}
#endif
/*
 * This function copies a received frame to all monitor interfaces and
 * returns a cleaned-up SKB that no longer includes the FCS nor the
 * radiotap header the driver might have added.
 */
static struct sk_buff *ieee80211_rx_monitor(struct ieee80211_local *local,
					    struct sk_buff *origskb,
					    struct ieee80211_rate *rate)
{
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(origskb);
	struct ieee80211_sub_if_data *sdata;
	int needed_headroom = 0;
	struct sk_buff *skb;
	struct net_device *prev_dev = NULL;
	int present_fcs_len = 0;

	/*
	 * First, we may need to make a copy of the skb because
	 *  (1) we need to modify it for radiotap (if not present), and
	 *  (2) the other RX handlers will modify the skb we got.
	 *
	 * We don't need to, of course, if we aren't going to return
	 * the SKB because it has a bad FCS/PLCP checksum.
	 */
	if (local->hw.flags & IEEE80211_HW_RX_INCLUDES_FCS)
		present_fcs_len = FCS_LEN;

	/* make sure hdr->frame_control is on the linear part */
	if (!Sstar_pskb_may_pull(origskb, 2)) {
		Sstar_dev_kfree_skb(origskb);
		return NULL;
	}

	if (!local->monitors) {
		if (should_drop_frame(origskb, present_fcs_len)) {
			Sstar_dev_kfree_skb(origskb);
			return NULL;
		}

		return remove_monitor_info(local, origskb);
	}

	if (should_drop_frame(origskb, present_fcs_len)) {
		/* only need to expand headroom if necessary */
		skb = origskb;
		origskb = NULL;
	} else {
		/*
		 * Need to make a copy and possibly remove radiotap header
		 * and FCS from the original.
		 */
		skb = Sstar_skb_copy(origskb, GFP_ATOMIC);

		origskb = remove_monitor_info(local, origskb);

		if (!skb)
			return origskb;
	}
#if 0
	{

		u8 *frame_type = NULL;
		struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;

		if(ieee80211_is_data(hdr->frame_control))
			frame_type = "data";
		else if(ieee80211_is_mgmt(hdr->frame_control))
			frame_type = "mgmt";
		else if(ieee80211_is_ctl(hdr->frame_control))
			frame_type = "ctl";
		else
			frame_type = "NULL";
		if(memcmp(frame_type,"mgmt",4))
			printk( "[monitor]:RX:name[%s],fc[%x],addr1[%pM]\n",frame_type,hdr->frame_control,hdr->addr1);
	}
#endif
	/*
	*only support one monitor
	*/
	list_for_each_entry_rcu (sdata, &local->interfaces, list) {
		if (sdata->vif.type != NL80211_IFTYPE_MONITOR)
			continue;

		if (WARN_ON_ONCE(sdata->u.mntr_flags &
				 MONITOR_FLAG_COOK_FRAMES))
			continue;

		if (!ieee80211_sdata_running(sdata))
			continue;

		prev_dev = sdata->dev;
		sdata->dev->stats.rx_packets++;
		sdata->dev->stats.rx_bytes += skb->len;
		break;
	}

	if (prev_dev) {
		skb->dev = prev_dev;
#ifdef CONFIG_SSTAR_MONITOR_HDR_PRISM
		if (prev_dev->type == ARPHRD_IEEE80211_PRISM) {
			if (ieee80211_add_rx_prism_header(
				    IEEE80211_DEV_TO_SUB_IF(prev_dev), skb,
				    rate) == false) {
				Sstar_dev_kfree_skb(skb);
				return origskb;
			}
		} else
#endif
		{
			WARN_ON(prev_dev->type != ARPHRD_IEEE80211_RADIOTAP);
			status = IEEE80211_SKB_RXCB(skb);
			needed_headroom =
				ieee80211_rx_radiotap_len(local, status);

			if (Sstar_skb_headroom(skb) < needed_headroom &&
			    Sstar_pskb_expand_head(skb, needed_headroom, 0,
						   GFP_ATOMIC)) {
				Sstar_dev_kfree_skb(skb);
				return origskb;
			}
			ieee80211_add_rx_radiotap_header(local, skb, rate,
							 needed_headroom);
		}

		skb_reset_mac_header(skb);
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		skb->pkt_type = PACKET_OTHERHOST;
		skb->protocol = htons(ETH_P_802_2);
		Sstar_netif_receive_skb(skb);

		if (origskb) {
			Sstar_dev_kfree_skb(origskb);
			origskb = NULL;
		}
	} else {
		Sstar_dev_kfree_skb(skb);
	}
	return origskb;
}
static void ieee80211_parse_qos(struct ieee80211_rx_data *rx)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);
	int tid, seqno_idx, security_idx;

	/* does the frame have a qos control field? */
	if (ieee80211_is_data_qos(hdr->frame_control)) {
		u8 *qc = ieee80211_get_qos_ctl(hdr);
		/* frame has qos control */
		tid = *qc & IEEE80211_QOS_CTL_TID_MASK;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0))
		if (*qc & IEEE80211_QOS_CONTROL_A_MSDU_PRESENT)
#else
		if (*qc & IEEE80211_QOS_CTL_A_MSDU_PRESENT)
#endif
			status->rx_flags |= IEEE80211_RX_AMSDU;

		seqno_idx = tid;
		security_idx = tid;
	} else {
		/*
		 * IEEE 802.11-2007, 7.1.3.4.1 ("Sequence Number field"):
		 *
		 *	Sequence numbers for management frames, QoS data
		 *	frames with a broadcast/multicast address in the
		 *	Address 1 field, and all non-QoS data frames sent
		 *	by QoS STAs are assigned using an additional single
		 *	modulo-4096 counter, [...]
		 *
		 * We also use that counter for non-QoS STAs.
		 */
		seqno_idx = NUM_RX_DATA_QUEUES;
		security_idx = 0;
		if (ieee80211_is_mgmt(hdr->frame_control))
			security_idx = NUM_RX_DATA_QUEUES;
		tid = 0;
	}

	rx->seqno_idx = seqno_idx;
	rx->security_idx = security_idx;
	/* Set skb->priority to 1d tag if highest order bit of TID is not set.
	 * For now, set skb->priority to 0 for other cases. */
	rx->skb->priority = (tid > 7) ? 0 : tid;
}

/**
 * DOC: Packet alignment
 *
 * Drivers always need to pass packets that are aligned to two-byte boundaries
 * to the stack.
 *
 * Additionally, should, if possible, align the payload data in a way that
 * guarantees that the contained IP header is aligned to a four-byte
 * boundary. In the case of regular frames, this simply means aligning the
 * payload to a four-byte boundary (because either the IP header is directly
 * contained, or IV/RFC1042 headers that have a length divisible by four are
 * in front of it).  If the payload data is not properly aligned and the
 * architecture doesn't support efficient unaligned operations, mac80211
 * will align the data.
 *
 * With A-MSDU frames, however, the payload data address must yield two modulo
 * four because there are 14-byte 802.3 headers within the A-MSDU frames that
 * push the IP header further back to a multiple of four again. Thankfully, the
 * specs were sane enough this time around to require padding each A-MSDU
 * subframe to a length that is a multiple of four.
 *
 * Padding like Atheros hardware adds which is between the 802.11 header and
 * the payload is not supported, the driver is required to move the 802.11
 * header to be directly in front of the payload in that case.
 */
static void ieee80211_verify_alignment(struct ieee80211_rx_data *rx)
{
#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG
	WARN_ONCE((unsigned long)rx->skb->data & 1,
		  "unaligned packet at 0x%p\n", rx->skb->data);
#endif
}

/* rx handlers */

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_passive_scan(struct ieee80211_rx_data *rx)
{
	struct ieee80211_local *local = rx->local;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);
	struct sk_buff *skb = rx->skb;

	if (likely(!(status->rx_flags & IEEE80211_RX_IN_SCAN)
#ifdef CONFIG_SSTAR_SUPPORT_SCHED_SCAN
		   && !local->sched_scanning
#endif
		   ))
		return RX_CONTINUE;

	if (test_bit(SCAN_HW_SCANNING, &local->scanning)
#ifdef CONFIG_SSTAR_SUPPORT_SCHED_SCAN
	    || local->sched_scanning
#endif
	) {
		if (test_bit(SCAN_INTERNAL_SCANNING, &local->scanning))
			return ieee80211_scan_rx_internal(rx->sdata, skb);
		else if (test_bit(SCAN_CFG80211_SCANNING, &local->scanning)) {
			return ieee80211_scan_rx(rx->sdata, skb);
		} else {
			Sstar_printk_debug("scaning end,so drop\n");
		}
	}

	/* scanning finished during invoking of handlers */
	I802_DEBUG_INC(local->rx_handlers_drop_passive_scan);
	return RX_DROP_UNUSABLE;
}

static int ieee80211_is_unicast_robust_mgmt_frame(struct sk_buff *skb)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;

	if (skb->len < 24 || is_multicast_ether_addr(hdr->addr1))
		return 0;
	return Sstar_ieee80211_is_robust_mgmt_frame(skb);
}

static int ieee80211_is_multicast_robust_mgmt_frame(struct sk_buff *skb)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;

	if (skb->len < 24 || !is_multicast_ether_addr(hdr->addr1))
		return 0;
	return Sstar_ieee80211_is_robust_mgmt_frame(skb);
}

/* Get the BIP key index from MMIE; return -1 if this is not a BIP frame */
static int ieee80211_get_mmie_keyidx(struct sk_buff *skb)
{
	struct Sstar_ieee80211_mgmt *hdr =
		(struct Sstar_ieee80211_mgmt *)skb->data;
	struct ieee80211_mmie *mmie;

	if (skb->len < 24 + sizeof(*mmie) || !is_multicast_ether_addr(hdr->da))
		return -1;
	if (!Sstar_ieee80211_is_robust_mgmt_frame(skb))
		return -1; /* not a robust management frame */

	mmie = (struct ieee80211_mmie *)(skb->data + skb->len - sizeof(*mmie));
	if (mmie->element_id != SSTAR_WLAN_EID_MMIE ||
	    mmie->length != sizeof(*mmie) - 2)
		return -1;

	return le16_to_cpu(mmie->key_id);
}

#ifdef CONFIG_MAC80211_SSTAR_MESH
static ieee80211_rx_result ieee80211_rx_mesh_check(struct ieee80211_rx_data *rx)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
	char *dev_addr = rx->sdata->vif.addr;

	if (ieee80211_is_data(hdr->frame_control)) {
		if (is_multicast_ether_addr(hdr->addr1)) {
			if (ieee80211_has_tods(hdr->frame_control) ||
			    !ieee80211_has_fromds(hdr->frame_control))
				return RX_DROP_MONITOR;
			if (memcmp(hdr->addr3, dev_addr, ETH_ALEN) == 0)
				return RX_DROP_MONITOR;
		} else {
			if (!ieee80211_has_a4(hdr->frame_control))
				return RX_DROP_MONITOR;
			if (memcmp(hdr->addr4, dev_addr, ETH_ALEN) == 0)
				return RX_DROP_MONITOR;
		}
	}

	/* If there is not an established peer link and this is not a peer link
	 * establisment frame, beacon or probe, drop the frame.
	 */

	if (!rx->sta || sta_plink_state(rx->sta) != NL80211_PLINK_ESTAB) {
		struct Sstar_ieee80211_mgmt *mgmt;

		if (!ieee80211_is_mgmt(hdr->frame_control))
			return RX_DROP_MONITOR;

		if (ieee80211_is_action(hdr->frame_control)) {
			u8 category;
			mgmt = (struct Sstar_ieee80211_mgmt *)hdr;
			category = mgmt->u.action.category;
			if (category != SSTAR_WLAN_CATEGORY_MESH_ACTION &&
			    category != SSTAR_WLAN_CATEGORY_SELF_PROTECTED)
				return RX_DROP_MONITOR;
			return RX_CONTINUE;
		}

		if (ieee80211_is_probe_req(hdr->frame_control) ||
		    ieee80211_is_probe_resp(hdr->frame_control) ||
		    ieee80211_is_beacon(hdr->frame_control) ||
		    ieee80211_is_auth(hdr->frame_control))
			return RX_CONTINUE;

		return RX_DROP_MONITOR;
	}

	return RX_CONTINUE;
}
#endif
#define SEQ_MODULO 0x1000
#define SEQ_MASK 0xfff

static inline int seq_less(u16 sq1, u16 sq2)
{
	return ((sq1 - sq2) & SEQ_MASK) > (SEQ_MODULO >> 1);
}

static inline u16 seq_inc(u16 sq)
{
	return (sq + 1) & SEQ_MASK;
}

static inline u16 seq_sub(u16 sq1, u16 sq2)
{
	return (sq1 - sq2) & SEQ_MASK;
}

static void ieee80211_release_reorder_frame(struct ieee80211_hw *hw,
					    struct tid_ampdu_rx *tid_agg_rx,
					    int index,
					    struct sk_buff_head *frames)
{
	struct sk_buff *skb = tid_agg_rx->reorder_buf[index];
	struct ieee80211_rx_status *status;

	lockdep_assert_held(&tid_agg_rx->reorder_lock);

	if (!skb)
		goto no_frame;

	/* release the frame from the reorder ring buffer */
	tid_agg_rx->stored_mpdu_num--;
	tid_agg_rx->reorder_buf[index] = NULL;
	status = IEEE80211_SKB_RXCB(skb);
	status->rx_flags |= IEEE80211_RX_DEFERRED_RELEASE;
	__Sstar_skb_queue_tail(frames, skb);

no_frame:
	tid_agg_rx->head_seq_num = seq_inc(tid_agg_rx->head_seq_num);
}

static void ieee80211_release_reorder_frames(struct ieee80211_hw *hw,
					     struct tid_ampdu_rx *tid_agg_rx,
					     u16 head_seq_num,
					     struct sk_buff_head *frames)
{
	int index;

	lockdep_assert_held(&tid_agg_rx->reorder_lock);

	while (seq_less(tid_agg_rx->head_seq_num, head_seq_num)) {
		index = seq_sub(tid_agg_rx->head_seq_num, tid_agg_rx->ssn) %
			tid_agg_rx->buf_size;
		ieee80211_release_reorder_frame(hw, tid_agg_rx, index, frames);
	}
}

/*
 * Timeout (in jiffies) for skb's that are waiting in the RX reorder buffer. If
 * the skb was added to the buffer longer than this time ago, the earlier
 * frames that have not yet been received are assumed to be lost and the skb
 * can be released for processing. This may also release other skb's from the
 * reorder buffer if there are no additional gaps between the frames.
 *
 * Callers must hold tid_agg_rx->reorder_lock.
 */
static int HT_RX_REORDER_BUF_TIMEOUT = (HZ / 10);
static int Sta_shortTcp_RxCnt = 0;
static void ieee80211_sta_reorder_release(struct ieee80211_hw *hw,
					  struct tid_ampdu_rx *tid_agg_rx,
					  struct sk_buff_head *frames)
{
	int index, j;

	lockdep_assert_held(&tid_agg_rx->reorder_lock);

	/* release the buffer until next missing frame */
	index = seq_sub(tid_agg_rx->head_seq_num, tid_agg_rx->ssn) %
		tid_agg_rx->buf_size;
	if (!tid_agg_rx->reorder_buf[index] && tid_agg_rx->stored_mpdu_num) {
		/*
		 * No buffers ready to be released, but check whether any
		 * frames in the reorder buffer have timed out.
		 */
		int skipped = 1;
		for (j = (index + 1) % tid_agg_rx->buf_size; j != index;
		     j = (j + 1) % tid_agg_rx->buf_size) {
			if (!tid_agg_rx->reorder_buf[j]) {
				skipped++;
				continue;
			}
			if (skipped &&
			    !time_after(jiffies,
					tid_agg_rx->reorder_time[j] +
						HT_RX_REORDER_BUF_TIMEOUT))
				goto set_release_timer;

#ifdef CONFIG_MAC80211_SSTAR_HT_DEBUG
			if (net_ratelimit())
				wiphy_debug(
					hw->wiphy,
					"release an RX reorder frame due to timeout on earlier frames\n");
#endif
			if (skipped) {
				//printk("release an RX reorder frame due to timeout sn %x\n",tid_agg_rx->head_seq_num);
			}
			ieee80211_release_reorder_frame(hw, tid_agg_rx, j,
							frames);

			/*
			 * Increment the head seq# also for the skipped slots.
			 */
			tid_agg_rx->head_seq_num =
				(tid_agg_rx->head_seq_num + skipped) & SEQ_MASK;
			skipped = 0;
		}
	} else
		while (tid_agg_rx->reorder_buf[index]) {
			ieee80211_release_reorder_frame(hw, tid_agg_rx, index,
							frames);
			index = seq_sub(tid_agg_rx->head_seq_num,
					tid_agg_rx->ssn) %
				tid_agg_rx->buf_size;
		}

	if (tid_agg_rx->stored_mpdu_num) {
		j = index = seq_sub(tid_agg_rx->head_seq_num, tid_agg_rx->ssn) %
			    tid_agg_rx->buf_size;

		for (; j != (index - 1) % tid_agg_rx->buf_size;
		     j = (j + 1) % tid_agg_rx->buf_size) {
			if (tid_agg_rx->reorder_buf[j])
				break;
		}

	set_release_timer:

		Sstar_mod_timer(&tid_agg_rx->reorder_timer,
				tid_agg_rx->reorder_time[j] + 1 +
					HT_RX_REORDER_BUF_TIMEOUT);
	} else {
		Sstar_del_timer(&tid_agg_rx->reorder_timer);
	}
}

/*
 * As this function belongs to the RX path it must be under
 * rcu_read_lock protection. It returns false if the frame
 * can be processed immediately, true if it was consumed.
 */
static bool ieee80211_sta_manage_reorder_buf(struct ieee80211_hw *hw,
					     struct tid_ampdu_rx *tid_agg_rx,
					     struct sk_buff *skb,
					     struct sk_buff_head *frames)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	u16 sc = le16_to_cpu(hdr->seq_ctrl);
	u16 mpdu_seq_num = (sc & IEEE80211_SCTL_SEQ) >> 4;
	u16 head_seq_num, buf_size;
	int index;
	bool ret = true;
	bool isEapol = (ieee80211_is_data(hdr->frame_control) &&
			ieee80211_frame_eapol(skb->data, skb->len));
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);

	if (skb->len < 150) {
		Sta_shortTcp_RxCnt++;
		if (Sta_shortTcp_RxCnt > 50) {
			Sta_shortTcp_RxCnt = 50;
			HT_RX_REORDER_BUF_TIMEOUT = HZ / 4;
		}
	} else {
		if (Sta_shortTcp_RxCnt >= 20)
			Sta_shortTcp_RxCnt -= 20;
		if (Sta_shortTcp_RxCnt < 20) {
			Sta_shortTcp_RxCnt = 0;
			HT_RX_REORDER_BUF_TIMEOUT = HZ / 2;
		}
	}
	spin_lock(&tid_agg_rx->reorder_lock);

	buf_size = tid_agg_rx->buf_size;
	head_seq_num = tid_agg_rx->head_seq_num;

	/* frame with out of date sequence number */
	if (seq_less(mpdu_seq_num, head_seq_num)) {
		if (!(status->flag & RX_FLAG_AMPDU) ||
		    !(status->flag & RX_FLAG_HT)) {
			ret = false;
		} else {
			Sstar_dev_kfree_skb(skb);
		}
		goto out;
	}

	/*
	 * If frame the sequence number exceeds our buffering window
	 * size release some previous frames to make room for this one.
	 */
	if (!seq_less(mpdu_seq_num, head_seq_num + buf_size)) {
		head_seq_num = seq_inc(seq_sub(mpdu_seq_num, buf_size));
		/* release stored frames up to new head to stack */
		ieee80211_release_reorder_frames(hw, tid_agg_rx, head_seq_num,
						 frames);
	}

	/* Now the new frame is always in the range of the reordering buffer */

	index = seq_sub(mpdu_seq_num, tid_agg_rx->ssn) % tid_agg_rx->buf_size;

	/* check if we already stored this frame */
	if (tid_agg_rx->reorder_buf[index]) {
		Sstar_dev_kfree_skb(skb);
		goto out;
	}

	/*
	 * If the current MPDU is in the right order and nothing else
	 * is stored we can process it directly, no need to buffer it.
	 * If it is first but there's something stored, we may be able
	 * to release frames after this one.
	 */
	if (mpdu_seq_num == tid_agg_rx->head_seq_num &&
	    tid_agg_rx->stored_mpdu_num == 0) {
		tid_agg_rx->head_seq_num = seq_inc(tid_agg_rx->head_seq_num);
		ret = false;
		goto out;
	}

	if (isEapol && mpdu_seq_num == (tid_agg_rx->head_seq_num + 1) &&
	    tid_agg_rx->stored_mpdu_num == 0) {
		tid_agg_rx->head_seq_num = seq_inc(tid_agg_rx->head_seq_num);
		ret = false;
		goto out;
	}

	/* put the frame in the reordering buffer */
	tid_agg_rx->reorder_buf[index] = skb;
	tid_agg_rx->reorder_time[index] = jiffies;
	tid_agg_rx->stored_mpdu_num++;
	ieee80211_sta_reorder_release(hw, tid_agg_rx, frames);

out:
	spin_unlock(&tid_agg_rx->reorder_lock);
	return ret;
}

/*
 * Reorder MPDUs from A-MPDUs, keeping them on a buffer. Returns
 * true if the MPDU was buffered, false if it should be processed.
 */
static void ieee80211_rx_reorder_ampdu(struct ieee80211_rx_data *rx,
				       struct sk_buff_head *frames)
{
	struct sk_buff *skb = rx->skb;
	struct ieee80211_local *local = rx->local;
	struct ieee80211_hw *hw = &local->hw;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct sta_info *sta = rx->sta;
	struct tid_ampdu_rx *tid_agg_rx;
	u16 sc;
	int tid;

	if (!ieee80211_is_data_qos(hdr->frame_control))
		goto dont_reorder;

	/*
	 * filter the QoS data rx stream according to
	 * STA/TID and check if this STA/TID is on aggregation
	 */

	if (!sta)
		goto dont_reorder;

	tid = *ieee80211_get_qos_ctl(hdr) & IEEE80211_QOS_CTL_TID_MASK;

	tid_agg_rx = rcu_dereference(sta->ampdu_mlme.tid_rx[tid]);
	if (!tid_agg_rx)
		goto dont_reorder;

	/* qos null data frames are excluded */
	if (unlikely(hdr->frame_control &
		     cpu_to_le16(IEEE80211_STYPE_NULLFUNC)))
		goto dont_reorder;

	/* new, potentially un-ordered, ampdu frame - process it */

	/* reset session timer */
	if (tid_agg_rx->timeout)
		Sstar_mod_timer(&tid_agg_rx->session_timer,
				TU_TO_EXP_TIME(tid_agg_rx->timeout));

	/* if this mpdu is fragmented - terminate rx aggregation session */
	sc = le16_to_cpu(hdr->seq_ctrl);
	if (sc & IEEE80211_SCTL_FRAG) {
		skb->pkt_type = IEEE80211_SDATA_QUEUE_TYPE_FRAME;
		Sstar_skb_queue_tail(&rx->sdata->skb_queue, skb);
		ieee80211_queue_work(&local->hw, &rx->sdata->work);
		return;
	}

	/*
	 * No locking needed -- we will only ever process one
	 * RX packet at a time, and thus own tid_agg_rx. All
	 * other code manipulating it needs to (and does) make
	 * sure that we cannot get to it any more before doing
	 * anything with it.
	 */
	if (ieee80211_sta_manage_reorder_buf(hw, tid_agg_rx, skb, frames))
		return;

dont_reorder:
	__Sstar_skb_queue_tail(frames, skb);
}

static bool ieee80211_frame_eapol(u8 *data, int len)
{
	__le16 fc;
	bool status = false;

#define ETH_TYPE_OFFSET 30

	fc = ((struct ieee80211_hdr *)data)->frame_control;
	if ((ieee80211_is_data(fc)) && (len > 34)) {
		u8 eapol_index = ETH_TYPE_OFFSET;
		u16 ethertype;

		if (ieee80211_is_data_qos(fc))
			eapol_index = ETH_TYPE_OFFSET + 2;

		ethertype =
			((data[eapol_index] << 8) | (data[eapol_index + 1]));
		if (ethertype == ETH_P_PAE)
			status = true;
	}
	return status;
}

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_check(struct ieee80211_rx_data *rx)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);

	/* Drop duplicate 802.11 retransmissions (IEEE 802.11 Chap. 9.2.9) */
	if (rx->sta && !is_multicast_ether_addr(hdr->addr1)) {
		if (unlikely(ieee80211_has_retry(hdr->frame_control) &&
			     rx->sta->last_seq_ctrl[rx->seqno_idx] ==
				     hdr->seq_ctrl)) {
			if ((le16_to_cpu(
				     rx->sta->last_frame_ctrl[rx->seqno_idx]) &
			     IEEE80211_FCTL_STYPE) ==
			    (le16_to_cpu(hdr->frame_control) &
			     IEEE80211_FCTL_STYPE)) {
				if (status->rx_flags & IEEE80211_RX_RA_MATCH) {
					rx->local->dot11FrameDuplicateCount++;
					rx->sta->num_duplicates++;
				}
				return RX_DROP_UNUSABLE;
			} else {
				Sstar_printk_err(
					"%s:fc last [%x] ,fc current[%x]\n",
					__func__,
					rx->sta->last_frame_ctrl[rx->seqno_idx],
					hdr->frame_control);
				rx->sta->last_seq_ctrl[rx->seqno_idx] =
					hdr->seq_ctrl;
				rx->sta->last_frame_ctrl[rx->seqno_idx] =
					hdr->frame_control;
			}
		} else {
			rx->sta->last_seq_ctrl[rx->seqno_idx] = hdr->seq_ctrl;
			rx->sta->last_frame_ctrl[rx->seqno_idx] =
				hdr->frame_control;
		}
	}

	if (unlikely(rx->skb->len < 16)) {
		I802_DEBUG_INC(rx->local->rx_handlers_drop_short);
		return RX_DROP_MONITOR;
	}

	/* Drop disallowed frame classes based on STA auth/assoc state;
	 * IEEE 802.11, Chap 5.5.
	 *
	 * mac80211 filters only based on association state, i.e. it drops
	 * Class 3 frames from not associated stations. hostapd sends
	 * deauth/disassoc frames when needed. In addition, hostapd is
	 * responsible for filtering on both auth and assoc states.
	 */
#ifdef CONFIG_MAC80211_SSTAR_MESH
	if (ieee80211_vif_is_mesh(&rx->sdata->vif))
		return ieee80211_rx_mesh_check(rx);
#endif
	if ((status->flag & RX_FLAG_UNKOWN_STA_FRAME) &&
	    (rx->sdata->vif.type == NL80211_IFTYPE_AP)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
		if (rx->sdata->vif.type == NL80211_IFTYPE_AP) {
			if (cfg80211_rx_spurious_frame(rx->sdata->dev,
						       hdr->addr2,
						       GFP_ATOMIC) == true)
				return RX_DROP_UNUSABLE;
		}
#endif
		return RX_DROP_MONITOR;
	}
	if (unlikely((ieee80211_is_data(hdr->frame_control) ||
		      ieee80211_is_pspoll(hdr->frame_control)) &&
		     rx->sdata->vif.type != NL80211_IFTYPE_ADHOC &&
		     rx->sdata->vif.type != NL80211_IFTYPE_WDS &&
		     (!rx->sta || !test_sta_flag(rx->sta, WLAN_STA_ASSOC)))) {
		if (rx->sta && rx->sta->dummy &&
		    ieee80211_is_data_present(hdr->frame_control)) {
			u16 ethertype;
			u8 *payload;

			payload = rx->skb->data +
				  ieee80211_hdrlen(hdr->frame_control);
			ethertype = (payload[6] << 8) | payload[7];
			if (cpu_to_be16(ethertype) ==
			    rx->sdata->control_port_protocol)
				return RX_CONTINUE;
		}
		if (!ieee80211_frame_eapol(rx->skb->data, rx->skb->len)) {
			//if(rx->sta)
			//	printk(KERN_ERR "%s:[%s]:sta[%pM] not assoc\n",__func__,rx->sdata->name,hdr->addr2);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
			if (rx->sdata->vif.type == NL80211_IFTYPE_AP) {
				if (cfg80211_rx_spurious_frame(
					    rx->sdata->dev, hdr->addr2,
					    GFP_ATOMIC) == true)
					return RX_DROP_UNUSABLE;
			}
#endif
			return RX_DROP_MONITOR;
		}
	}

	return RX_CONTINUE;
}
#ifndef CONFIG_SSTAR_USE_SW_ENC
static ieee80211_rx_result
ieee80211_crypto_wep_decrypt(struct ieee80211_rx_data *rx)
{
	struct sk_buff *skb = rx->skb;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;

	if (!ieee80211_is_data(hdr->frame_control) &&
	    !ieee80211_is_auth(hdr->frame_control))
		return RX_CONTINUE;
	if (!(status->flag & RX_FLAG_DECRYPTED)) {
		Sstar_printk_err("wep decr err\n");
		return RX_DROP_UNUSABLE;
	}
	if (!(status->flag & RX_FLAG_IV_STRIPPED)) {
		unsigned int hdrlen;

		hdrlen = ieee80211_hdrlen(hdr->frame_control);
		memmove(skb->data + WEP_IV_LEN, skb->data, hdrlen);
		Sstar_skb_pull(skb, WEP_IV_LEN);
		/* remove ICV */
		Sstar_skb_trim(rx->skb, rx->skb->len - WEP_ICV_LEN);
	}

	return RX_CONTINUE;
}
static ieee80211_rx_result
ieee80211_crypto_tkip_decrypt(struct ieee80211_rx_data *rx)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
	int hdrlen = 0;
	struct sk_buff *skb = rx->skb;

	hdrlen = ieee80211_hdrlen(hdr->frame_control);

	if (!ieee80211_is_data(hdr->frame_control))
		return RX_CONTINUE;

	if (!rx->sta || skb->len - hdrlen < 12)
		return RX_DROP_UNUSABLE;

	/* Trim ICV */
	Sstar_skb_trim(skb, skb->len - TKIP_ICV_LEN);

	/* Remove IV */
	memmove(skb->data + TKIP_IV_LEN, skb->data, hdrlen);
	Sstar_skb_pull(skb, TKIP_IV_LEN);

	return RX_CONTINUE;
}

static ieee80211_rx_result
ieee80211_crypto_ccmp_decrypt(struct ieee80211_rx_data *rx)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
	int hdrlen;
	struct sk_buff *skb = rx->skb;
	int data_len;

	hdrlen = ieee80211_hdrlen(hdr->frame_control);

	if (!ieee80211_is_data(hdr->frame_control) &&
	    !Sstar_ieee80211_is_robust_mgmt_frame(skb))
		return RX_CONTINUE;

	data_len = skb->len - hdrlen - CCMP_HDR_LEN - CCMP_MIC_LEN;
	if (!rx->sta || data_len < 0)
		return RX_DROP_UNUSABLE;

	/* Remove CCMP header and MIC */
	Sstar_skb_trim(skb, skb->len - CCMP_MIC_LEN);
	memmove(skb->data + CCMP_HDR_LEN, skb->data, hdrlen);
	Sstar_skb_pull(skb, CCMP_HDR_LEN);

	return RX_CONTINUE;
}
static ieee80211_rx_result
ieee80211_crypto_aes_cmac_decrypt(struct ieee80211_rx_data *rx)
{
	struct sk_buff *skb = rx->skb;
	struct ieee80211_mmie *mmie;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;

	if (!ieee80211_is_mgmt(hdr->frame_control))
		return RX_CONTINUE;

	if (skb->len < 24 + sizeof(*mmie))
		return RX_DROP_UNUSABLE;

	mmie = (struct ieee80211_mmie *)(skb->data + skb->len - sizeof(*mmie));
	if (mmie->element_id != SSTAR_WLAN_EID_MMIE ||
	    mmie->length != sizeof(*mmie) - 2)
		return RX_DROP_UNUSABLE; /* Invalid MMIE */
	/* Remove MMIE */
	Sstar_skb_trim(skb, skb->len - sizeof(*mmie));

	return RX_CONTINUE;
}
#ifdef CONFIG_WAPI_SUPPORT
static ieee80211_rx_result
ieee80211_crypto_wapi_decrypt(struct ieee80211_rx_data *rx)
{
	struct sk_buff *skb = rx->skb;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	int hdrlen = ieee80211_hdrlen(hdr->frame_control);
	int data_len;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);

	if (!ieee80211_is_data(hdr->frame_control))
		return RX_CONTINUE;

	if (!(status->flag & RX_FLAG_DECRYPTED)) {
		/* TODO - SMS4 decryption for firmware without
		 * SMS4 support */
		return RX_DROP_UNUSABLE;
	}

	data_len = skb->len - hdrlen - WAPI_IV_LEN - WAPI_ICV_LEN;
	if (data_len < 0)
		return RX_DROP_UNUSABLE;

	/* Trim ICV */
	Sstar_skb_trim(skb, skb->len - WAPI_ICV_LEN);

	/* Remove IV */
	memmove(skb->data + WAPI_IV_LEN, skb->data, hdrlen);
	Sstar_skb_pull(skb, WAPI_IV_LEN);

	return RX_CONTINUE;
}
#endif
#endif
static ieee80211_rx_result debug_noinline
ieee80211_rx_h_decrypt(struct ieee80211_rx_data *rx)
{
	struct sk_buff *skb = rx->skb;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	int keyidx;
	int hdrlen;
	ieee80211_rx_result result = RX_DROP_UNUSABLE;
	struct ieee80211_key *sta_ptk = NULL;
	int mmie_keyidx = -1;
	__le16 fc;

	/*
	 * Key selection 101
	 *
	 * There are four types of keys:
	 *  - GTK (group keys)
	 *  - IGTK (group keys for management frames)
	 *  - PTK (pairwise keys)
	 *  - STK (station-to-station pairwise keys)
	 *
	 * When selecting a key, we have to distinguish between multicast
	 * (including broadcast) and unicast frames, the latter can only
	 * use PTKs and STKs while the former always use GTKs and IGTKs.
	 * Unless, of course, actual WEP keys ("pre-RSNA") are used, then
	 * unicast frames can also use key indices like GTKs. Hence, if we
	 * don't have a PTK/STK we check the key index for a WEP key.
	 *
	 * Note that in a regular BSS, multicast frames are sent by the
	 * AP only, associated stations unicast the frame to the AP first
	 * which then multicasts it on their behalf.
	 *
	 * There is also a slight problem in IBSS mode: GTKs are negotiated
	 * with each station, that is something we don't currently handle.
	 * The spec seems to expect that one negotiates the same key with
	 * every station but there's no such requirement; VLANs could be
	 * possible.
	 */

	/*
	 * No point in finding a key and decrypting if the frame is neither
	 * addressed to us nor a multicast frame.
	 */
	if (!(status->rx_flags & IEEE80211_RX_RA_MATCH))
		return RX_CONTINUE;

	/* start without a key */
	rx->key = NULL;

	if (rx->sta)
		sta_ptk = rcu_dereference(rx->sta->ptk);

	fc = hdr->frame_control;

	if (!ieee80211_has_protected(fc))
		mmie_keyidx = ieee80211_get_mmie_keyidx(rx->skb);

	if (!is_multicast_ether_addr(hdr->addr1) && sta_ptk) {
		rx->key = sta_ptk;
		if ((status->flag & RX_FLAG_DECRYPTED) &&
		    (status->flag & RX_FLAG_IV_STRIPPED))
			return RX_CONTINUE;
		/* Skip decryption if the frame is not protected. */
		if (!ieee80211_has_protected(fc))
			return RX_CONTINUE;
	} else if (mmie_keyidx >= 0) {
		/* Broadcast/multicast robust management frame / BIP */
		if ((status->flag & RX_FLAG_DECRYPTED) &&
		    (status->flag & RX_FLAG_IV_STRIPPED))
			return RX_CONTINUE;

		if (mmie_keyidx < NUM_DEFAULT_KEYS ||
		    mmie_keyidx >= NUM_DEFAULT_KEYS + NUM_DEFAULT_MGMT_KEYS)
			return RX_DROP_MONITOR; /* unexpected BIP keyidx */
		if (rx->sta)
			rx->key = rcu_dereference(rx->sta->gtk[mmie_keyidx]);
		if (!rx->key)
			rx->key = rcu_dereference(rx->sdata->keys[mmie_keyidx]);
	} else if (!ieee80211_has_protected(fc)) {
		/*
		 * The frame was not protected, so skip decryption. However, we
		 * need to set rx->key if there is a key that could have been
		 * used so that the frame may be dropped if encryption would
		 * have been expected.
		 */
		struct ieee80211_key *key = NULL;
		struct ieee80211_sub_if_data *sdata = rx->sdata;
		int i;

		if (ieee80211_is_mgmt(fc) &&
		    is_multicast_ether_addr(hdr->addr1) &&
		    (key = rcu_dereference(rx->sdata->default_mgmt_key)))
			rx->key = key;
		else {
			if (rx->sta) {
				for (i = 0; i < NUM_DEFAULT_KEYS; i++) {
					key = rcu_dereference(rx->sta->gtk[i]);
					if (key)
						break;
				}
			}
			if (!key) {
				for (i = 0; i < NUM_DEFAULT_KEYS; i++) {
					key = rcu_dereference(sdata->keys[i]);
					if (key)
						break;
				}
			}
			if (key)
				rx->key = key;
		}
		return RX_CONTINUE;
	} else {
		u8 keyid;
		/*
		 * The device doesn't give us the IV so we won't be
		 * able to look up the key. That's ok though, we
		 * don't need to decrypt the frame, we just won't
		 * be able to keep statistics accurate.
		 * Except for key threshold notifications, should
		 * we somehow allow the driver to tell us which key
		 * the hardware used if this flag is set?
		 */
		if ((status->flag & RX_FLAG_DECRYPTED) &&
		    (status->flag & RX_FLAG_IV_STRIPPED))
			return RX_CONTINUE;

		hdrlen = ieee80211_hdrlen(fc);

		if (rx->skb->len < 8 + hdrlen)
			return RX_DROP_UNUSABLE; /* TODO: count this? */

		/*
		 * no need to call ieee80211_wep_get_keyidx,
		 * it verifies a bunch of things we've done already
		 */
		skb_copy_bits(rx->skb, hdrlen + 3, &keyid, 1);
		keyidx = keyid >> 6;

		/* check per-station GTK first, if multicast packet */
		if (is_multicast_ether_addr(hdr->addr1) && rx->sta)
			rx->key = rcu_dereference(rx->sta->gtk[keyidx]);

		/* if not found, try default key */
		if (!rx->key) {
			struct ieee80211_sub_if_data *sdata = rx->sdata;
			rx->key = rcu_dereference(rx->sdata->keys[keyidx]);

			/*
			 * RSNA-protected unicast frames should always be
			 * sent with pairwise or station-to-station keys,
			 * but for WEP we allow using a key index as well.
			 */
			if (rx->key &&
			    rx->key->conf.cipher != WLAN_CIPHER_SUITE_WEP40 &&
			    rx->key->conf.cipher != WLAN_CIPHER_SUITE_WEP104 &&
			    !is_multicast_ether_addr(hdr->addr1) &&
			    /*adhoc wpa-none may used key index ,add by wp for adhoc,may need check*/
			    (sdata->vif.type != NL80211_IFTYPE_ADHOC)) {
				rx->key = NULL;
			}
		}
	}

	if (rx->key) {
		if (unlikely(rx->key->flags & KEY_FLAG_TAINTED))
			return RX_DROP_MONITOR;

		rx->key->tx_rx_count++;
		/* TODO: add threshold stuff again */
	} else {
		if (ieee80211_is_deauth(hdr->frame_control)) {
			Sstar_printk_err(
				"deauth loss :addr1: [%pM] addr2:[%pM] \n",
				hdr->addr1, hdr->addr2);
		}
		return RX_DROP_MONITOR;
	}

	if (Sstar_skb_linearize(rx->skb))
		return RX_DROP_UNUSABLE;
	/* the hdr variable is invalid now! */

	switch (rx->key->conf.cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
#ifdef CONFIG_SSTAR_USE_SW_ENC
		/* Check for weak IVs if possible */
		if (rx->sta && ieee80211_is_data(fc) &&
		    (!(status->flag & RX_FLAG_IV_STRIPPED) ||
		     !(status->flag & RX_FLAG_DECRYPTED)) &&
		    ieee80211_wep_is_weak_iv(rx->skb, rx->key))
			rx->sta->wep_weak_iv_count++;
#endif
		result = ieee80211_crypto_wep_decrypt(rx);
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		result = ieee80211_crypto_tkip_decrypt(rx);
		break;
	case WLAN_CIPHER_SUITE_CCMP:
		result = ieee80211_crypto_ccmp_decrypt(rx);
		break;
	case WLAN_CIPHER_SUITE_AES_CMAC:
		result = ieee80211_crypto_aes_cmac_decrypt(rx);
		break;
#ifdef CONFIG_WAPI_SUPPORT
	case WLAN_CIPHER_SUITE_SMS4:
		result = ieee80211_crypto_wapi_decrypt(rx);
		break;
#endif
	default:
		/*
		 * We can reach here only with HW-only algorithms
		 * but why didn't it decrypt the frame?!
		 */
		return RX_DROP_UNUSABLE;
	}

	/* either the frame has been decrypted or will be dropped */
	status->flag |= RX_FLAG_DECRYPTED;

	return result;
}
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
static ieee80211_rx_result debug_noinline
ieee80211_rx_h_check_more_data(struct ieee80211_rx_data *rx)
{
	struct ieee80211_local *local;
	struct ieee80211_sub_if_data *sdata;
	struct ieee80211_hdr *hdr;
	struct sk_buff *skb;

	local = rx->local;
	sdata = rx->sdata;
	skb = rx->skb;
	hdr = (struct ieee80211_hdr *)skb->data;

	if (!sdata->pspolling)
		return RX_CONTINUE;

	if (!ieee80211_has_fromds(hdr->frame_control))
		/* this is not from AP */
		return RX_CONTINUE;

	if (!ieee80211_is_data(hdr->frame_control))
		return RX_CONTINUE;

	if (!ieee80211_has_moredata(hdr->frame_control)) {
		/* AP has no more frames buffered for us */
		sdata->pspolling = false;
		return RX_CONTINUE;
	}

	/* more data bit is set, let's request a new frame from the AP */
	ieee80211_send_pspoll(local, rx->sdata);

	return RX_CONTINUE;
}
#endif
static void ap_sta_ps_start(struct sta_info *sta)
{
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	struct ieee80211_local *local = sdata->local;

	atomic_inc(&sdata->bss->num_sta_ps);
	set_sta_flag(sta, WLAN_STA_PS_STA);
#ifdef CONFIG_TX_NO_CONFIRM
#pragma message("sta ps recalc tim")
	sta_info_recalc_tim(sta);
#else
#pragma message("sta ps not recalc tim")
#endif
	if (!(local->hw.flags & IEEE80211_HW_AP_LINK_PS))
		drv_sta_notify(local, sdata, STA_NOTIFY_SLEEP, &sta->sta);
#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_PS_DEBUG
	Sstar_printk_debug("%s: STA %pM aid %d enters power save mode\n",
			   sdata->name, sta->sta.addr, sta->sta.aid);
#endif /* CONFIG_MAC80211_SSTAR_VERBOSE_PS_DEBUG */
}

static void ap_sta_ps_end(struct sta_info *sta)
{
	struct ieee80211_sub_if_data *sdata = sta->sdata;

	atomic_dec(&sdata->bss->num_sta_ps);

#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_PS_DEBUG
	Sstar_printk_debug("%s: STA %pM aid %d exits power save mode\n",
			   sdata->name, sta->sta.addr, sta->sta.aid);
#endif /* CONFIG_MAC80211_SSTAR_VERBOSE_PS_DEBUG */

	if (test_sta_flag(sta, WLAN_STA_PS_DRIVER)) {
#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_PS_DEBUG
		Sstar_printk_debug("%s: STA %pM aid %d driver-ps-blocked\n",
				   sdata->name, sta->sta.addr, sta->sta.aid);
#endif /* CONFIG_MAC80211_SSTAR_VERBOSE_PS_DEBUG */
		return;
	}

	ieee80211_sta_ps_deliver_wakeup(sta);
}

int ieee80211_sta_ps_transition(struct ieee80211_sta *sta, bool start)
{
	struct sta_info *sta_inf = container_of(sta, struct sta_info, sta);
	bool in_ps;
#if 0
	WARN_ON(!(sta_inf->local->hw.flags & IEEE80211_HW_AP_LINK_PS));
#endif

	/* Don't let the same PS state be set twice */
	in_ps = test_sta_flag(sta_inf, WLAN_STA_PS_STA);
	if ((start && in_ps) || (!start && !in_ps))
		return -EINVAL;

	if (start)
		ap_sta_ps_start(sta_inf);
	else
		ap_sta_ps_end(sta_inf);

	return 0;
}
//EXPORT_SYMBOL(ieee80211_sta_ps_transition);

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_uapsd_and_pspoll(struct ieee80211_rx_data *rx)
{
	struct ieee80211_sub_if_data *sdata = rx->sdata;
	struct ieee80211_hdr *hdr = (void *)rx->skb->data;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);
	int tid, ac;

	if (!rx->sta || !(status->rx_flags & IEEE80211_RX_RA_MATCH))
		return RX_CONTINUE;

	if (sdata->vif.type != NL80211_IFTYPE_AP &&
	    sdata->vif.type != NL80211_IFTYPE_AP_VLAN)
		return RX_CONTINUE;

	/*
	 * The device handles station powersave, so don't do anything about
	 * uAPSD and PS-Poll frames (the latter shouldn't even come up from
	 * it to mac80211 since they're handled.)
	 */
	if (sdata->local->hw.flags & IEEE80211_HW_AP_LINK_PS)
		return RX_CONTINUE;

	/*
	 * Don't do anything if the station isn't already asleep. In
	 * the uAPSD case, the station will probably be marked asleep,
	 * in the PS-Poll case the station must be confused ...
	 */
	if (!test_sta_flag(rx->sta, WLAN_STA_PS_STA))
		return RX_CONTINUE;

	if (unlikely(ieee80211_is_pspoll(hdr->frame_control))) {
		if (!test_sta_flag(rx->sta, WLAN_STA_SP)) {
			if (!test_sta_flag(rx->sta, WLAN_STA_PS_DRIVER))
				ieee80211_sta_ps_deliver_poll_response(rx->sta);
			else
				set_sta_flag(rx->sta, WLAN_STA_PSPOLL);
		}

		/* Free PS Poll skb here instead of returning RX_DROP that would
		 * count as an dropped frame. */
		Sstar_dev_kfree_skb(rx->skb);

		return RX_QUEUED;
	} else if (!ieee80211_has_morefrags(hdr->frame_control) &&
		   !(status->rx_flags & IEEE80211_RX_DEFERRED_RELEASE) &&
		   ieee80211_has_pm(hdr->frame_control) &&
		   (ieee80211_is_data_qos(hdr->frame_control) ||
		    ieee80211_is_qos_nullfunc(hdr->frame_control))) {
		tid = *ieee80211_get_qos_ctl(hdr) & IEEE80211_QOS_CTL_TID_MASK;
		ac = ieee802_1d_to_ac[tid & 7];

		/*
		 * If this AC is not trigger-enabled do nothing.
		 *
		 * NB: This could/should check a separate bitmap of trigger-
		 * enabled queues, but for now we only implement uAPSD w/o
		 * TSPEC changes to the ACs, so they're always the same.
		 */
		if (!(rx->sta->sta.uapsd_queues & BIT(ac)))
			return RX_CONTINUE;

		/* if we are in a service period, do nothing */
		if (test_sta_flag(rx->sta, WLAN_STA_SP))
			return RX_CONTINUE;

		if (!test_sta_flag(rx->sta, WLAN_STA_PS_DRIVER))
			ieee80211_sta_ps_deliver_uapsd(rx->sta);
		else
			set_sta_flag(rx->sta, WLAN_STA_UAPSD);
	}

	return RX_CONTINUE;
}

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_sta_process(struct ieee80211_rx_data *rx)
{
	struct sta_info *sta = rx->sta;
	struct sk_buff *skb = rx->skb;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	int cur_signal = 0;

	if (!sta)
		return RX_CONTINUE;
#ifdef CONFIG_SSTAR_SUPPORT_IBSS
	/*
	 * Update last_rx only for IBSS packets which are for the current
	 * BSSID to avoid keeping the current IBSS network alive in cases
	 * where other STAs start using different BSSID.
	 */
	if (rx->sdata->vif.type == NL80211_IFTYPE_ADHOC) {
		u8 *bssid = ieee80211_get_bssid(hdr, rx->skb->len,
						NL80211_IFTYPE_ADHOC);
		if (Sstar_compare_ether_addr(bssid, rx->sdata->u.ibss.bssid) ==
		    0) {
			sta->last_rx = jiffies;
			if (ieee80211_is_data(hdr->frame_control)) {
				sta->last_rx_rate_idx = status->rate_idx;
				sta->last_rx_rate_flag = status->flag;
			}
		}
	} else
#endif
		if (!is_multicast_ether_addr(hdr->addr1)) {
		/*
		 * Mesh beacons will update last_rx when if they are found to
		 * match the current local configuration when processed.
		 */
		sta->last_rx = jiffies;
		if (ieee80211_is_data(hdr->frame_control)) {
			sta->last_rx_rate_idx = status->rate_idx;
			sta->last_rx_rate_flag = status->flag;
		}
	}

	if (!(status->rx_flags & IEEE80211_RX_RA_MATCH))
		return RX_CONTINUE;
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
	if (rx->sdata->vif.type == NL80211_IFTYPE_STATION)
		ieee80211_sta_rx_notify(rx->sdata, hdr);
#endif
	sta->rx_fragments++;
	sta->rx_bytes += rx->skb->len;
	sta->last_signal = status->signal;
	Sstar_ewma_add(&sta->avg_signal, -status->signal);

	cur_signal = status->signal;
	if (cur_signal >= 2)
		cur_signal = 2;

	if (!ieee80211_is_data(hdr->frame_control)) {
		cur_signal -= 3;
	}

	sta->last_signal2 = cur_signal;
	//	if(status->flag & RX_FLAG_HT || status->rate_idx >= 4){
	Sstar_ewma_add(&sta->avg_signal2, -sta->last_signal2);
	//	}
	/*
	 * Change STA power saving mode only at the end of a frame
	 * exchange sequence.
	 */
	if (!(sta->local->hw.flags & IEEE80211_HW_AP_LINK_PS) &&
	    !ieee80211_has_morefrags(hdr->frame_control) &&
	    !(status->rx_flags & IEEE80211_RX_DEFERRED_RELEASE) &&
	    (rx->sdata->vif.type == NL80211_IFTYPE_AP ||
	     rx->sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
#ifdef SSTAR_AP_SME
	    && test_sta_flag(sta, WLAN_STA_PS_STA)
#endif
	) {
		if (test_sta_flag(sta, WLAN_STA_PS_STA)) {
			/*
			 * Ignore doze->wake transitions that are
			 * indicated by non-data frames, the standard
			 * is unclear here, but for example going to
			 * PS mode and then scanning would cause a
			 * doze->wake transition for the probe request,
			 * and that is clearly undesirable.
			 */
			if (ieee80211_is_data(hdr->frame_control) &&
			    !ieee80211_has_pm(hdr->frame_control))
				ap_sta_ps_end(sta);
		} else {
			if (ieee80211_has_pm(hdr->frame_control))
				ap_sta_ps_start(sta);
		}
	}

	/*
	 * Drop (qos-)data::nullfunc frames silently, since they
	 * are used only to control station power saving mode.
	 */
	if (ieee80211_is_nullfunc(hdr->frame_control) ||
	    ieee80211_is_qos_nullfunc(hdr->frame_control)) {
		I802_DEBUG_INC(rx->local->rx_handlers_drop_nullfunc);

		/*
		 * If we receive a 4-addr nullfunc frame from a STA
		 * that was not moved to a 4-addr STA vlan yet, drop
		 * the frame to the monitor interface, to make sure
		 * that hostapd sees it
		 */
		if (ieee80211_has_a4(hdr->frame_control) &&
		    (rx->sdata->vif.type == NL80211_IFTYPE_AP ||
		     (rx->sdata->vif.type == NL80211_IFTYPE_AP_VLAN &&
		      !rx->sdata->u.vlan.sta)))
			return RX_DROP_MONITOR;
		/*
		 * Update counter and free packet here to avoid
		 * counting this as a dropped packed.
		 */
		sta->rx_packets++;
		Sstar_dev_kfree_skb(rx->skb);
		return RX_QUEUED;
	}

	return RX_CONTINUE;
} /* ieee80211_rx_h_sta_process */

static inline struct ieee80211_fragment_entry *
ieee80211_reassemble_add(struct ieee80211_sub_if_data *sdata, unsigned int frag,
			 unsigned int seq, int rx_queue, struct sk_buff **skb)
{
	struct ieee80211_fragment_entry *entry;
	//	int idx;

	//	idx = sdata->fragment_next;
	entry = &sdata->fragments[sdata->fragment_next++];
	if (sdata->fragment_next >= IEEE80211_FRAGMENT_MAX)
		sdata->fragment_next = 0;

	if (!Sstar_skb_queue_empty(&entry->skb_list)) {
#ifdef CONFIG_MAC80211_SSTAR_VERBOSE_DEBUG
		struct ieee80211_hdr *hdr =
			(struct ieee80211_hdr *)entry->skb_list.next->data;
		Sstar_printk_debug(
			"%s: RX reassembly removed oldest "
			"fragment entry (idx=%d age=%lu seq=%d last_frag=%d "
			"addr1=%pM addr2=%pM\n",
			sdata->name, idx, jiffies - entry->first_frag_time,
			entry->seq, entry->last_frag, hdr->addr1, hdr->addr2);
#endif
		__Sstar_skb_queue_purge(&entry->skb_list);
	}

	__Sstar_skb_queue_tail(&entry->skb_list,
			       *skb); /* no need for locking */
	*skb = NULL;
	entry->first_frag_time = jiffies;
	entry->seq = seq;
	entry->rx_queue = rx_queue;
	entry->last_frag = frag;
	entry->ccmp = 0;
	entry->extra_len = 0;

	return entry;
}

static inline struct ieee80211_fragment_entry *
ieee80211_reassemble_find(struct ieee80211_sub_if_data *sdata,
			  unsigned int frag, unsigned int seq, int rx_queue,
			  struct ieee80211_hdr *hdr)
{
	struct ieee80211_fragment_entry *entry;
	int i, idx;

	idx = sdata->fragment_next;
	for (i = 0; i < IEEE80211_FRAGMENT_MAX; i++) {
		struct ieee80211_hdr *f_hdr;

		idx--;
		if (idx < 0)
			idx = IEEE80211_FRAGMENT_MAX - 1;

		entry = &sdata->fragments[idx];
		if (Sstar_skb_queue_empty(&entry->skb_list) ||
		    entry->seq != seq || entry->rx_queue != rx_queue ||
		    entry->last_frag + 1 != frag)
			continue;

		f_hdr = (struct ieee80211_hdr *)entry->skb_list.next->data;

		/*
		 * Check ftype and addresses are equal, else check next fragment
		 */
		if (((hdr->frame_control ^ f_hdr->frame_control) &
		     cpu_to_le16(IEEE80211_FCTL_FTYPE)) ||
		    Sstar_compare_ether_addr(hdr->addr1, f_hdr->addr1) != 0 ||
		    Sstar_compare_ether_addr(hdr->addr2, f_hdr->addr2) != 0)
			continue;

		if (time_after(jiffies, entry->first_frag_time + 2 * HZ)) {
			__Sstar_skb_queue_purge(&entry->skb_list);
			continue;
		}
		return entry;
	}

	return NULL;
}

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_defragment(struct ieee80211_rx_data *rx)
{
	struct ieee80211_hdr *hdr;
	u16 sc;
	__le16 fc;
	unsigned int frag, seq;
	struct ieee80211_fragment_entry *entry;
	struct sk_buff *skb;
	struct ieee80211_rx_status *status;

	hdr = (struct ieee80211_hdr *)rx->skb->data;
	fc = hdr->frame_control;
	sc = le16_to_cpu(hdr->seq_ctrl);
	frag = sc & IEEE80211_SCTL_FRAG;

	if (likely((!ieee80211_has_morefrags(fc) && frag == 0) ||
		   (rx->skb)->len < 24 ||
		   is_multicast_ether_addr(hdr->addr1))) {
		/* not fragmented */
		goto out;
	}
	I802_DEBUG_INC(rx->local->rx_handlers_fragments);

	if (Sstar_skb_linearize(rx->skb))
		return RX_DROP_UNUSABLE;

	/*
	 *  skb_linearize() might change the skb->data and
	 *  previously cached variables (in this case, hdr) need to
	 *  be refreshed with the new data.
	 */
	hdr = (struct ieee80211_hdr *)rx->skb->data;
	seq = (sc & IEEE80211_SCTL_SEQ) >> 4;

	if (frag == 0) {
		/* This is the first fragment of a new frame. */
		entry = ieee80211_reassemble_add(rx->sdata, frag, seq,
						 rx->seqno_idx, &(rx->skb));
		if (rx->key && rx->key->conf.cipher == WLAN_CIPHER_SUITE_CCMP &&
		    ieee80211_has_protected(fc)) {
			int queue = rx->security_idx;
			/* Store CCMP PN so that we can verify that the next
			 * fragment has a sequential PN value. */
			entry->ccmp = 1;
			memcpy(entry->last_pn, rx->key->u.ccmp.rx_pn[queue],
			       CCMP_PN_LEN);
			entry->first_key = rx->key;
		}
		return RX_QUEUED;
	}

	/* This is a fragment for a frame that should already be pending in
	 * fragment cache. Add this fragment to the end of the pending entry.
	 */
	entry = ieee80211_reassemble_find(rx->sdata, frag, seq, rx->seqno_idx,
					  hdr);
	if (!entry) {
		I802_DEBUG_INC(rx->local->rx_handlers_drop_defrag);
		return RX_DROP_MONITOR;
	}

	/* Verify that MPDUs within one MSDU have sequential PN values.
	 * (IEEE 802.11i, 8.3.3.4.5) */
	if (entry->ccmp) {
		int i;
		u8 pn[CCMP_PN_LEN], *rpn;
		int queue;
		if (!rx->key ||
		    rx->key->conf.cipher != WLAN_CIPHER_SUITE_CCMP ||
		    !ieee80211_has_protected(fc)) {
			Sstar_printk_err("%s:rx new frag enc err\n", __func__);
			return RX_DROP_UNUSABLE;
		}
		/*
		*fragment must be enc with only one key.
		*/
		Sstar_printk_debug("%s:first_key(%p),key(%p)\n", __func__,
				   entry->first_key, rx->key);

		if ((entry->first_key != rx->key) ||
		    (memcmp(entry->first_key->conf.key, rx->key->conf.key,
			    entry->first_key->conf.keylen))) {
			__Sstar_skb_queue_purge(&entry->skb_list);
			Sstar_printk_err("%s:key change\n", __func__);
			return RX_DROP_UNUSABLE;
		}
		memcpy(pn, entry->last_pn, CCMP_PN_LEN);
		for (i = CCMP_PN_LEN - 1; i >= 0; i--) {
			pn[i]++;
			if (pn[i])
				break;
		}
		queue = rx->security_idx;
		rpn = rx->key->u.ccmp.rx_pn[queue];
		if (memcmp(pn, rpn, CCMP_PN_LEN)) {
			Sstar_printk_err("pn[%pM],rpn(%pM)\n", pn, rpn);
			return RX_DROP_UNUSABLE;
		}
		memcpy(entry->last_pn, pn, CCMP_PN_LEN);
	} else if (entry->first_key != rx->key) {
		Sstar_printk_err("%s:entry->first_key != rx->key \n", __func__);
		__Sstar_skb_queue_purge(&entry->skb_list);
		return RX_DROP_UNUSABLE;
	} else if (entry->first_key && rx->key &&
		   (memcmp(entry->first_key->conf.key, rx->key->conf.key,
			   entry->first_key->conf.keylen))) {
		Sstar_printk_err("%s:rx->key has update\n", __func__);
		__Sstar_skb_queue_purge(&entry->skb_list);
		return RX_DROP_UNUSABLE;
	} else if (entry->first_key && !ieee80211_has_protected(fc)) {
		Sstar_printk_err("%s:new frag not enc\n", __func__);
		__Sstar_skb_queue_purge(&entry->skb_list);
		return RX_DROP_UNUSABLE;
	}

	Sstar_skb_pull(rx->skb, ieee80211_hdrlen(fc));
	__Sstar_skb_queue_tail(&entry->skb_list, rx->skb);
	entry->last_frag = frag;
	entry->extra_len += rx->skb->len;
	if (ieee80211_has_morefrags(fc)) {
		rx->skb = NULL;
		return RX_QUEUED;
	}

	rx->skb = __Sstar_skb_dequeue(&entry->skb_list);
	if (Sstar_skb_tailroom(rx->skb) < entry->extra_len) {
		I802_DEBUG_INC(rx->local->rx_expand_skb_head2);
		if (unlikely(Sstar_pskb_expand_head(
			    rx->skb, 0, entry->extra_len, GFP_ATOMIC))) {
			Sstar_printk_err("%s:tailroom(%d),extra_len(%d)\n",
					 __func__, Sstar_skb_tailroom(rx->skb),
					 entry->extra_len);
			I802_DEBUG_INC(rx->local->rx_handlers_drop_defrag);
			__Sstar_skb_queue_purge(&entry->skb_list);
			return RX_DROP_UNUSABLE;
		}
	}
	while ((skb = __Sstar_skb_dequeue(&entry->skb_list))) {
		memcpy(Sstar_skb_put(rx->skb, skb->len), skb->data, skb->len);
		Sstar_dev_kfree_skb(skb);
	}

	/* Complete frame has been reassembled - process it now */
	status = IEEE80211_SKB_RXCB(rx->skb);
	status->rx_flags |= IEEE80211_RX_FRAGMENTED;

out:
	if (rx->sta)
		rx->sta->rx_packets++;
	if (is_multicast_ether_addr(hdr->addr1))
		rx->local->dot11MulticastReceivedFrameCount++;
	else
		ieee80211_led_rx(rx->local);
	return RX_CONTINUE;
}

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_remove_qos_control(struct ieee80211_rx_data *rx)
{
	u8 *data = rx->skb->data;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)data;

	if (!ieee80211_is_data_qos(hdr->frame_control))
		return RX_CONTINUE;

	/* remove the qos control field, update frame type and meta-data */
	memmove(data + IEEE80211_QOS_CTL_LEN, data,
		ieee80211_hdrlen(hdr->frame_control) - IEEE80211_QOS_CTL_LEN);
	hdr = (struct ieee80211_hdr *)Sstar_skb_pull(rx->skb,
						     IEEE80211_QOS_CTL_LEN);
	/* change frame type to non QOS */
	hdr->frame_control &= ~cpu_to_le16(IEEE80211_STYPE_QOS_DATA);

	return RX_CONTINUE;
}

static int ieee80211_802_1x_port_control(struct ieee80211_rx_data *rx)
{
	if (unlikely(!rx->sta || !test_sta_flag(rx->sta, WLAN_STA_AUTHORIZED)))
		return -EACCES;

	return 0;
}

static int ieee80211_drop_unencrypted(struct ieee80211_rx_data *rx, __le16 fc)
{
	struct sk_buff *skb = rx->skb;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);

	/*
	 * Pass through unencrypted frames if the hardware has
	 * decrypted them already.
	 */
	if (status->flag & RX_FLAG_DECRYPTED)
		return 0;

	/* Drop unencrypted frames if key is set. */
	if (unlikely(!ieee80211_has_protected(fc) &&
		     !ieee80211_is_nullfunc(fc) && ieee80211_is_data(fc) &&
		     (rx->key || rx->sdata->drop_unencrypted)))
		return -EACCES;

	return 0;
}

static int ieee80211_drop_unencrypted_mgmt(struct ieee80211_rx_data *rx)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);
	__le16 fc = hdr->frame_control;

	/*
	 * Pass through unencrypted frames if the hardware has
	 * decrypted them already.
	 */
	if (status->flag & RX_FLAG_DECRYPTED)
		return 0;

	if (rx->sta && test_sta_flag(rx->sta, WLAN_STA_MFP)) {
		if (unlikely(!ieee80211_has_protected(fc) &&
			     ieee80211_is_unicast_robust_mgmt_frame(rx->skb) &&
			     rx->key)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0))
			if (ieee80211_is_deauth(fc))
				cfg80211_send_unprot_deauth(rx->sdata->dev,
							    rx->skb->data,
							    rx->skb->len);
			else if (ieee80211_is_disassoc(fc))
				cfg80211_send_unprot_disassoc(rx->sdata->dev,
							      rx->skb->data,
							      rx->skb->len);
#else
			if (ieee80211_is_deauth(fc) ||
			    ieee80211_is_disassoc(fc))
				cfg80211_rx_unprot_mlme_mgmt(rx->sdata->dev,
							     rx->skb->data,
							     rx->skb->len);
#endif
			return -EACCES;
		}
		/* BIP does not use Protected field, so need to check MMIE */
		if (unlikely(
			    ieee80211_is_multicast_robust_mgmt_frame(rx->skb) &&
			    ieee80211_get_mmie_keyidx(rx->skb) < 0)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0))
			if (ieee80211_is_deauth(fc))
				cfg80211_send_unprot_deauth(rx->sdata->dev,
							    rx->skb->data,
							    rx->skb->len);
			else if (ieee80211_is_disassoc(fc))
				cfg80211_send_unprot_disassoc(rx->sdata->dev,
							      rx->skb->data,
							      rx->skb->len);
#else
			if (ieee80211_is_deauth(fc) ||
			    ieee80211_is_disassoc(fc))
				cfg80211_rx_unprot_mlme_mgmt(rx->sdata->dev,
							     rx->skb->data,
							     rx->skb->len);

#endif
			return -EACCES;
		}
		/*
		 * When using MFP, Action frames are not allowed prior to
		 * having configured keys.
		 */
		if (unlikely(ieee80211_is_action(fc) && !rx->key &&
			     Sstar_ieee80211_is_robust_mgmt_frame(rx->skb)))
			return -EACCES;
	}

	return 0;
}

static int __ieee80211_data_to_8023(struct ieee80211_rx_data *rx,
				    bool *port_control)
{
	struct ieee80211_sub_if_data *sdata = rx->sdata;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
	bool check_port_control = false;
	struct ethhdr *ehdr;
	int ret;

	*port_control = false;
	if (ieee80211_has_a4(hdr->frame_control) &&
	    sdata->vif.type == NL80211_IFTYPE_AP_VLAN && !sdata->u.vlan.sta)
		return -1;

	if (sdata->vif.type == NL80211_IFTYPE_STATION &&
	    !!sdata->u.mgd.use_4addr !=
		    !!ieee80211_has_a4(hdr->frame_control)) {
		if (!sdata->u.mgd.use_4addr)
			return -1;
		else
			check_port_control = true;
	}

	if (is_multicast_ether_addr(hdr->addr1) &&
	    sdata->vif.type == NL80211_IFTYPE_AP_VLAN && sdata->u.vlan.sta)
		return -1;

	ret = ieee80211_data_to_8023(rx->skb, sdata->vif.addr, sdata->vif.type);
	if (ret < 0)
		return ret;

	ehdr = (struct ethhdr *)rx->skb->data;
	if (ehdr->h_proto == rx->sdata->control_port_protocol)
		*port_control = true;
	else if (check_port_control)
		return -1;

	return 0;
}

/*
 * requires that rx->skb is a frame with ethernet header
 */
static bool ieee80211_frame_allowed(struct ieee80211_rx_data *rx, __le16 fc)
{
	static const u8 pae_group_addr[ETH_ALEN] __aligned(
		2) = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x03 };
	struct ethhdr *ehdr = (struct ethhdr *)rx->skb->data;

	/*
	 * Allow EAPOL frames to us/the PAE group address regardless
	 * of whether the frame was encrypted or not.
	 */
	if (ehdr->h_proto == rx->sdata->control_port_protocol &&
	    (Sstar_compare_ether_addr(ehdr->h_dest, rx->sdata->vif.addr) == 0 ||
	     Sstar_compare_ether_addr(ehdr->h_dest, pae_group_addr) == 0))
		return true;

	if (ieee80211_802_1x_port_control(rx) ||
	    ieee80211_drop_unencrypted(rx, fc))
		return false;

	return true;
}

/* ******************************************************************** */
#include <linux/udp.h>
#include <net/ip.h>
/* D	HCP port*/
#define IS_BOOTP_PORT(src_port, des_port)                                      \
	((((src_port) == 67) && ((des_port) == 68)) ||                         \
	 (((src_port) == 68) && ((des_port) == 67)))

int ieee80211_is_dhcp_frame(struct sk_buff *skb)
{
	const struct iphdr *ip;
	struct ethhdr *ehdr = (struct ethhdr *)skb->data;

	ip = (struct iphdr *)((u8 *)ehdr + sizeof(struct ethhdr));
	if (ehdr->h_proto == htons(ETH_P_IP)) {
		if (IPPROTO_UDP == ip->protocol) {
			struct udphdr *udph =
				(struct udphdr *)((u8 *)ip + (ip->ihl << 2));
			if (IS_BOOTP_PORT(ntohs(udph->source),
					  ntohs(udph->dest))) {
				return 1;
			}
		}
	}
	return 0;
}

/*
 * requires that rx->skb is a frame with ethernet header
 */
static void ieee80211_deliver_skb(struct ieee80211_rx_data *rx)
{
	struct ieee80211_sub_if_data *sdata = rx->sdata;
	struct net_device *dev = sdata->dev;
	struct sk_buff *skb, *xmit_skb;
	//	struct sk_buff  *deliver_skb=NULL;
	struct ethhdr *ehdr = (struct ethhdr *)rx->skb->data;
	struct sta_info *dsta;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);

	skb = rx->skb;
	xmit_skb = NULL;
#ifdef CONFIG_MAC80211_BRIDGE
	{
		if (ieee80211_brigde_change_rxhdr(sdata, skb) < 0) {
			Sstar_printk_err(
				"ieee80211_deliver_skb drop frame ,[%s]\n",
				sdata->name);
			Sstar_dev_kfree_skb(skb);
			return;
		}
	}
#endif // CONFIG_MAC80211_BRIDGE

	if ((sdata->vif.type == NL80211_IFTYPE_AP ||
	     sdata->vif.type == NL80211_IFTYPE_AP_VLAN) &&
	    !(sdata->flags & IEEE80211_SDATA_DONT_BRIDGE_PACKETS) &&
	    (status->rx_flags & IEEE80211_RX_RA_MATCH) &&
	    (sdata->vif.type != NL80211_IFTYPE_AP_VLAN || !sdata->u.vlan.sta)) {
		if (is_multicast_ether_addr(ehdr->h_dest)) {
			/*
			 * send multicast frames both to higher layers in
			 * local net stack and back to the wireless medium
			 */
			xmit_skb = Sstar_skb_copy(skb, GFP_ATOMIC);
			if (!xmit_skb && net_ratelimit())
				Sstar_printk_err("%s: failed to clone "
						 "multicast frame\n",
						 dev->name);
		} else {
			dsta = sta_info_get(sdata, skb->data);
			if (dsta) {
				/*
				 * The destination station is associated to
				 * this AP (in this VLAN), so send the frame
				 * directly to it and do not pass it to local
				 * net stack.
				 */
				xmit_skb = skb;
				skb = NULL;
			}
		}
	}

#ifdef CONFIG_MAC80211_BRIDGE1
	if (skb) {
		struct ieee80211_local *local = rx->local;
		struct ieee80211_sub_if_data *deliver_sdata;
		struct ethhdr *ehdr_deliver = NULL;
		deliver_skb = NULL;
		list_for_each_entry_rcu (deliver_sdata, &local->interfaces,
					 list) {
			if (!ieee80211_sdata_running(deliver_sdata))
				continue;
			if (deliver_sdata == sdata)
				continue;
			if ((deliver_sdata->vif.type != NL80211_IFTYPE_AP) &&
			    (deliver_sdata->vif.type !=
			     NL80211_IFTYPE_STATION)) {
				continue;
			}
			if (/*(is_multicast_ether_addr(ehdr->h_dest))||*/
			    (ieee80211_is_dhcp_frame(skb))) {
				/*
				 * send multicast frames both to higher layers in
				 * local net stack and back to the wireless medium
				 */
				deliver_skb = Sstar_skb_copy(skb, GFP_ATOMIC);
				deliver_skb->dev = deliver_sdata->dev;
				ehdr_deliver =
					(struct ethhdr *)deliver_skb->data;
				memcpy(ehdr_deliver->h_source,
				       deliver_sdata->vif.addr, 6);
				if (!deliver_skb && net_ratelimit())
					Sstar_printk_err("%s: failed to clone "
							 "multicast frame\n",
							 dev->name);

				break;
			}
		}
		if (deliver_skb) {
			/* send to wireless media */
			deliver_skb->protocol = htons(ETH_P_802_3);
			skb_reset_network_header(deliver_skb);
			skb_reset_mac_header(deliver_skb);
			//printk("deliver skb iftype22 %d \n",deliver_sdata->vif.type);
			local_bh_disable();
			//	frame_hexdump("\ndeliver skb \n",deliver_skb->data,32);
			if (ieee80211_subif_start_xmit(deliver_skb,
						       deliver_sdata->dev) ==
			    NETDEV_TX_OK) {
			} else {
				Sstar_dev_kfree_skb(deliver_skb);
			}
			local_bh_enable();
		}
	}
#endif //SSTAR_AP_STA_BRIDGE
	if (skb) {
		int align __maybe_unused;

#if !defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS) ||                        \
	defined(CONFIG_SSTAR_ALIGNED_ACCESS)
		/*
		 * 'align' will only take the values 0 or 2 here
		 * since all frames are required to be aligned
		 * to 2-byte boundaries when being passed to
		 * mac80211. That also explains the __skb_push()
		 * below.
		 */
		align = ((unsigned long)(skb->data + sizeof(struct ethhdr))) &
			(L1_CACHE_BYTES - 1);
		if (align) {
			if (WARN_ON(Sstar_skb_headroom(skb) <
				    (L1_CACHE_BYTES - 1))) {
				Sstar_dev_kfree_skb(skb);
				skb = NULL;
			} else {
				u8 *data = skb->data;
				size_t len = skb_headlen(skb);
				skb->data -= align;
				memmove(skb->data, data, len);
				Sstar_skb_set_tail_pointer(skb, len);
			}
		}
#endif

		if (skb) {
			/* deliver to local stack */
			skb->protocol = eth_type_trans(skb, dev);
#ifdef CHKSUM_HW_SUPPORT
			if ((dev->features & NETIF_F_RXCSUM) &&
			    (!(status->flag & RX_FLAG_HW_CHKSUM_ERROR))) {
				skb->ip_summed = CHECKSUM_UNNECESSARY;
			} else {
				skb->ip_summed = CHECKSUM_NONE;
			}
#endif
			memset(skb->cb, 0, sizeof(skb->cb));
			//extern void frame_hexdump(char *prefix, u8 *data, int len);
			//frame_hexdump("netif_receive_skb1 ",skb->data,64);
			//Sstar_netif_receive_skb(skb);
			Sstar_netif_rx(skb);
		}
	}

	if (xmit_skb) {
		/* send to wireless media */
		xmit_skb->protocol = htons(ETH_P_802_3);
		skb_reset_network_header(xmit_skb);
		skb_reset_mac_header(xmit_skb);
		Sstar_dev_queue_xmit(xmit_skb);
	}
}

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_amsdu(struct ieee80211_rx_data *rx)
{
	struct net_device *dev = rx->sdata->dev;
	struct sk_buff *skb = rx->skb;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	__le16 fc = hdr->frame_control;
	struct sk_buff_head frame_list;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);
	u8 da[6];
	u8 sa[6];
	u8 llc[6] = { 0xaa, 0xaa, 0x03, 0, 0, 0 };

	bool first = true;

	if (unlikely(!ieee80211_is_data(fc)))
		return RX_CONTINUE;

	if (unlikely(!ieee80211_is_data_present(fc)))
		return RX_DROP_MONITOR;

	if (!(status->rx_flags & IEEE80211_RX_AMSDU))
		return RX_CONTINUE;

	if (ieee80211_has_a4(hdr->frame_control) &&
	    rx->sdata->vif.type == NL80211_IFTYPE_AP_VLAN &&
	    !rx->sdata->u.vlan.sta)
		return RX_DROP_UNUSABLE;

	if (is_multicast_ether_addr(hdr->addr1) &&
	    ((rx->sdata->vif.type == NL80211_IFTYPE_AP_VLAN &&
	      rx->sdata->u.vlan.sta) ||
	     (rx->sdata->vif.type == NL80211_IFTYPE_STATION &&
	      rx->sdata->u.mgd.use_4addr)))
		return RX_DROP_UNUSABLE;

	if (rx->key) {
		/*
		 * We should not receive A-MSDUs on pre-HT connections,
		 * and HT connections cannot use old ciphers. Thus drop
		 * them, as in those cases we couldn't even have SPP
		 * A-MSDUs or such.
		 */
		switch (rx->key->conf.cipher) {
		case WLAN_CIPHER_SUITE_WEP40:
		case WLAN_CIPHER_SUITE_WEP104:
		case WLAN_CIPHER_SUITE_TKIP:
			return RX_DROP_UNUSABLE;
		default:
			break;
		}
	}
	memcpy(da, ieee80211_get_DA(hdr), ETH_ALEN);
	memcpy(sa, ieee80211_get_SA(hdr), ETH_ALEN);

	skb->dev = dev;
	__Sstar_skb_queue_head_init(&frame_list);

	if (Sstar_skb_linearize(skb))
		return RX_DROP_UNUSABLE;

	Sstar_amsdu_to_8023s(skb, &frame_list, dev->dev_addr,
			     rx->sdata->vif.type,
			     rx->local->hw.extra_tx_headroom);

	while (!Sstar_skb_queue_empty(&frame_list)) {
		struct ethhdr *ehdr;
		rx->skb = __Sstar_skb_dequeue(&frame_list);
		ehdr = (struct ethhdr *)rx->skb->data;
		if (!ieee80211_frame_allowed(rx, fc)) {
			Sstar_dev_kfree_skb(rx->skb);
			first = false;
			continue;
		}

		if ((first == true) && (memcmp(ehdr->h_dest, da, 6) ||
					memcmp(ehdr->h_source, sa, 6) ||
					memcmp(llc, da, 6) == 0)) {
			Sstar_printk_err("%s:amsdu first frame err\n",
					 __func__);
			Sstar_dev_kfree_skb(rx->skb);
			__Sstar_skb_queue_purge(&frame_list);
			Sstar_printk_err("%s:amsdu first frame err2\n",
					 __func__);
			break;
		}
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += rx->skb->len;
		first = false;
		Sstar_printk_debug("%s:amsdu deliver skb\n", __func__);
		ieee80211_deliver_skb(rx);
	}

	return RX_QUEUED;
}

#ifdef CONFIG_MAC80211_SSTAR_MESH
static ieee80211_rx_result
ieee80211_rx_h_mesh_fwding(struct ieee80211_rx_data *rx)
{
	struct ieee80211_hdr *hdr;
	struct ieee80211s_hdr *mesh_hdr;
	unsigned int hdrlen;
	struct sk_buff *skb = rx->skb, *fwd_skb;
	struct ieee80211_local *local = rx->local;
	struct ieee80211_sub_if_data *sdata = rx->sdata;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);

	hdr = (struct ieee80211_hdr *)skb->data;
	hdrlen = ieee80211_hdrlen(hdr->frame_control);
	mesh_hdr = (struct ieee80211s_hdr *)(skb->data + hdrlen);

	/* frame is in RMC, don't forward */
	if (ieee80211_is_data(hdr->frame_control) &&
	    is_multicast_ether_addr(hdr->addr1) &&
	    mesh_rmc_check(hdr->addr3, mesh_hdr, rx->sdata))
		return RX_DROP_MONITOR;

	if (!ieee80211_is_data(hdr->frame_control))
		return RX_CONTINUE;

	if (!mesh_hdr->ttl)
		/* illegal frame */
		return RX_DROP_MONITOR;

	if (ieee80211_queue_stopped(&local->hw, skb_get_queue_mapping(skb))) {
		IEEE80211_IFSTA_MESH_CTR_INC(&sdata->u.mesh,
					     dropped_frames_congestion);
		return RX_DROP_MONITOR;
	}

	if (mesh_hdr->flags & MESH_FLAGS_AE) {
		struct mesh_path *mppath;
		char *proxied_addr;
		char *mpp_addr;

		if (is_multicast_ether_addr(hdr->addr1)) {
			mpp_addr = hdr->addr3;
			proxied_addr = mesh_hdr->eaddr1;
		} else {
			mpp_addr = hdr->addr4;
			proxied_addr = mesh_hdr->eaddr2;
		}

		rcu_read_lock();
		mppath = mpp_path_lookup(proxied_addr, sdata);
		if (!mppath) {
			mpp_path_add(proxied_addr, mpp_addr, sdata);
		} else {
			spin_lock_bh(&mppath->state_lock);
			if (Sstar_compare_ether_addr(mppath->mpp, mpp_addr) !=
			    0)
				memcpy(mppath->mpp, mpp_addr, ETH_ALEN);
			spin_unlock_bh(&mppath->state_lock);
		}
		rcu_read_unlock();
	}

	/* Frame has reached destination.  Don't forward */
	if (!is_multicast_ether_addr(hdr->addr1) &&
	    Sstar_compare_ether_addr(sdata->vif.addr, hdr->addr3) == 0)
		return RX_CONTINUE;

	mesh_hdr->ttl--;

	if (status->rx_flags & IEEE80211_RX_RA_MATCH) {
		if (!mesh_hdr->ttl)
			IEEE80211_IFSTA_MESH_CTR_INC(&rx->sdata->u.mesh,
						     dropped_frames_ttl);
		else {
			struct ieee80211_hdr *fwd_hdr;
			struct ieee80211_tx_info *info;

			fwd_skb = Sstar_skb_copy(skb, GFP_ATOMIC);

			if (!fwd_skb && net_ratelimit())
				Sstar_printk_err(
					"%s: failed to clone mesh frame\n",
					sdata->name);
			if (!fwd_skb)
				goto out;

			fwd_hdr = (struct ieee80211_hdr *)fwd_skb->data;
			memcpy(fwd_hdr->addr2, sdata->vif.addr, ETH_ALEN);
			info = IEEE80211_SKB_CB(fwd_skb);
			memset(info, 0, sizeof(*info));
			info->flags |= IEEE80211_TX_INTFL_NEED_TXPROCESSING;
			info->control.vif = &rx->sdata->vif;
			if (is_multicast_ether_addr(fwd_hdr->addr1)) {
				IEEE80211_IFSTA_MESH_CTR_INC(&sdata->u.mesh,
							     fwded_mcast);
				Sstar_skb_set_queue_mapping(
					fwd_skb,
					ieee80211_select_queue(sdata, fwd_skb));
				ieee80211_set_qos_hdr(sdata, fwd_skb);
			} else {
				int err;
				/*
				 * Save TA to addr1 to send TA a path error if a
				 * suitable next hop is not found
				 */
				memcpy(fwd_hdr->addr1, fwd_hdr->addr2,
				       ETH_ALEN);
				err = mesh_nexthop_lookup(fwd_skb, sdata);
				/* Failed to immediately resolve next hop:
				 * fwded frame was dropped or will be added
				 * later to the pending skb queue.  */
				if (err)
					return RX_DROP_MONITOR;

				IEEE80211_IFSTA_MESH_CTR_INC(&sdata->u.mesh,
							     fwded_unicast);
			}
			IEEE80211_IFSTA_MESH_CTR_INC(&sdata->u.mesh,
						     fwded_frames);
			ieee80211_add_pending_skb(local, fwd_skb);
		}
	}

out:
	if (is_multicast_ether_addr(hdr->addr1) ||
	    sdata->dev->flags & IFF_PROMISC)
		return RX_CONTINUE;
	else
		return RX_DROP_MONITOR;
}
#endif

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_data(struct ieee80211_rx_data *rx)
{
	struct ieee80211_sub_if_data *sdata = rx->sdata;
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
	struct ieee80211_local *local = rx->local;
#endif
	struct net_device *dev = sdata->dev;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
	__le16 fc = hdr->frame_control;
	bool port_control;
	int err;

	if (unlikely(!ieee80211_is_data(hdr->frame_control)))
		return RX_CONTINUE;

	if (unlikely(!ieee80211_is_data_present(hdr->frame_control)))
		return RX_DROP_MONITOR;

	/*
	 * Allow the cooked monitor interface of an AP to see 4-addr frames so
	 * that a 4-addr station can be detected and moved into a separate VLAN
	 */
	if (ieee80211_has_a4(hdr->frame_control) &&
	    sdata->vif.type == NL80211_IFTYPE_AP)
		return RX_DROP_MONITOR;

	err = __ieee80211_data_to_8023(rx, &port_control);
	if (unlikely(err))
		return RX_DROP_UNUSABLE;

	if (!ieee80211_frame_allowed(rx, fc))
		return RX_DROP_MONITOR;

	if (rx->sdata->vif.type == NL80211_IFTYPE_AP_VLAN &&
	    unlikely(port_control) && sdata->bss) {
		sdata = container_of(sdata->bss, struct ieee80211_sub_if_data,
				     u.ap);
		dev = sdata->dev;
		rx->sdata = sdata;
	}

	rx->skb->dev = dev;

	dev->stats.rx_packets++;
	dev->stats.rx_bytes += rx->skb->len;
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
	if (!(local->hw.flags & IEEE80211_HW_SUPPORTS_DYNAMIC_PS)) {
		if (sdata->ps_allowed &&
		    sdata->vif.bss_conf.dynamic_ps_timeout > 0 &&
		    !is_multicast_ether_addr(
			    ((struct ethhdr *)rx->skb->data)->h_dest) &&
		    (!local->scanning &&
		     !test_bit(SDATA_STATE_OFFCHANNEL, &sdata->state))) {
			Sstar_mod_timer(
				&sdata->dynamic_ps_timer,
				jiffies + msecs_to_jiffies(
						  sdata->vif.bss_conf
							  .dynamic_ps_timeout));
		}
	}
#endif
	ieee80211_deliver_skb(rx);

	return RX_QUEUED;
}
#ifdef CONFIG_SSTAR_PROCESS_CTRL
static ieee80211_rx_result debug_noinline
ieee80211_rx_h_ctrl(struct ieee80211_rx_data *rx)
{
	struct ieee80211_local *local = rx->local;
	struct ieee80211_hw *hw = &local->hw;
	struct sk_buff *skb = rx->skb;
	struct ieee80211_bar *bar = (struct ieee80211_bar *)skb->data;
	struct tid_ampdu_rx *tid_agg_rx;
	u16 start_seq_num;
	u16 tid;

	if (likely(!ieee80211_is_ctl(bar->frame_control)))
		return RX_CONTINUE;

	if (ieee80211_is_back_req(bar->frame_control)) {
		struct {
			__le16 control, start_seq_num;
		} __packed bar_data;

		if (!rx->sta)
			return RX_DROP_MONITOR;

		if (skb_copy_bits(skb, offsetof(struct ieee80211_bar, control),
				  &bar_data, sizeof(bar_data)))
			return RX_DROP_MONITOR;

		tid = le16_to_cpu(bar_data.control) >> 12;

		tid_agg_rx = rcu_dereference(rx->sta->ampdu_mlme.tid_rx[tid]);
		if (!tid_agg_rx)
			return RX_DROP_MONITOR;

		start_seq_num = le16_to_cpu(bar_data.start_seq_num) >> 4;

		/* reset session timer */
		if (tid_agg_rx->timeout)
			Sstar_mod_timer(&tid_agg_rx->session_timer,
					TU_TO_EXP_TIME(tid_agg_rx->timeout));

		spin_lock(&tid_agg_rx->reorder_lock);
		/* release stored frames up to start of BAR */
		ieee80211_release_reorder_frames(hw, tid_agg_rx, start_seq_num);
		spin_unlock(&tid_agg_rx->reorder_lock);

		Sstar_kfree_skb(skb);
		return RX_QUEUED;
	}

	/*
	 * After this point, we only want management frames,
	 * so we can drop all remaining control frames to
	 * cooked monitor interfaces.
	 */
	return RX_DROP_MONITOR;
}
#endif
#ifdef CONFIG_SSTAR_SA_QUERY
static void ieee80211_process_sa_query_req(struct ieee80211_sub_if_data *sdata,
					   struct Sstar_ieee80211_mgmt *mgmt,
					   size_t len)
{
	struct ieee80211_local *local = sdata->local;
	struct sk_buff *skb;
	struct Sstar_ieee80211_mgmt *resp;

	if (Sstar_compare_ether_addr(mgmt->da, sdata->vif.addr) != 0) {
		/* Not to own unicast address */
		return;
	}

	if (Sstar_compare_ether_addr(mgmt->sa, sdata->u.mgd.bssid) != 0 ||
	    Sstar_compare_ether_addr(mgmt->bssid, sdata->u.mgd.bssid) != 0) {
		/* Not from the current AP or not associated yet. */
		return;
	}

	if (len < 24 + 1 + sizeof(resp->u.action.u.sa_query)) {
		/* Too short SA Query request frame */
		return;
	}

	skb = Sstar_dev_alloc_skb(sizeof(*resp) + local->hw.extra_tx_headroom);
	if (skb == NULL)
		return;

	Sstar_skb_reserve(skb, local->hw.extra_tx_headroom);
	resp = (struct Sstar_ieee80211_mgmt *)Sstar_skb_put(skb, 24);
	memset(resp, 0, 24);
	memcpy(resp->da, mgmt->sa, ETH_ALEN);
	memcpy(resp->sa, sdata->vif.addr, ETH_ALEN);
	memcpy(resp->bssid, sdata->u.mgd.bssid, ETH_ALEN);
	resp->frame_control =
		cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
	Sstar_skb_put(skb, 1 + sizeof(resp->u.action.u.sa_query));
	resp->u.action.category = SSTAR_WLAN_CATEGORY_SA_QUERY;
	resp->u.action.u.sa_query.action = WLAN_ACTION_SA_QUERY_RESPONSE;
	memcpy(resp->u.action.u.sa_query.trans_id,
	       mgmt->u.action.u.sa_query.trans_id, WLAN_SA_QUERY_TR_ID_LEN);

	ieee80211_tx_skb(sdata, skb);
}
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))

static void ieee80211_rx_report_ap_beacon(struct ieee80211_rx_data *rx)
{
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
	int sig = 0;
	if (rx->local->hw.flags & IEEE80211_HW_SIGNAL_DBM)
		sig = status->signal;
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0))
	cfg80211_report_obss_beacon(rx->local->hw.wiphy, rx->skb->data,
				    rx->skb->len, status->freq, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
	cfg80211_report_obss_beacon(rx->local->hw.wiphy, rx->skb->data,
				    rx->skb->len, status->freq, sig,
				    GFP_ATOMIC);
#else
	cfg80211_report_obss_beacon(rx->local->hw.wiphy, rx->skb->data,
				    rx->skb->len, status->freq, sig);
#endif
}

#endif
static ieee80211_rx_result debug_noinline
ieee80211_rx_h_mgmt_check(struct ieee80211_rx_data *rx)
{
	struct Sstar_ieee80211_mgmt *mgmt =
		(struct Sstar_ieee80211_mgmt *)rx->skb->data;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);
	struct ieee80211_sub_if_data *sdata = rx->sdata;

	/*
	 * From here on, look only at management frames.
	 * Data and control frames are already handled,
	 * and unknown (reserved) frames are useless.
	 */
	if ((status->rx_flags & IEEE80211_RX_ERP_BEACON) &&
	    (sdata->vif.type == NL80211_IFTYPE_AP)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0))
		rx->skb->pkt_type = IEEE80211_SDATA_QUEUE_TYPE_FRAME;
		Sstar_skb_queue_tail(&sdata->skb_queue, rx->skb);
		ieee80211_queue_work(&rx->local->hw, &sdata->work);
		return RX_QUEUED;
#else
		ieee80211_rx_report_ap_beacon(rx);
		return RX_DROP_UNUSABLE;
#endif
	}

	if (status->rx_flags & IEEE80211_RX_SPECIAL_PROBE_REQ) {
		rx->skb->pkt_type = IEEE80211_SDATA_QUEUE_TYPE_FRAME;
		Sstar_skb_queue_tail(&sdata->skb_queue, rx->skb);
		ieee80211_queue_work(&rx->local->hw, &sdata->work);
		return RX_QUEUED;
	}
	if (rx->skb->len < 24)
		return RX_DROP_MONITOR;

	if (!ieee80211_is_mgmt(mgmt->frame_control))
		return RX_DROP_MONITOR;

	if (!(status->rx_flags & IEEE80211_RX_RA_MATCH))
		return RX_DROP_MONITOR;

	if (ieee80211_drop_unencrypted_mgmt(rx))
		return RX_DROP_UNUSABLE;

	return RX_CONTINUE;
}

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_action(struct ieee80211_rx_data *rx)
{
	struct ieee80211_local *local = rx->local;
	struct ieee80211_sub_if_data *sdata = rx->sdata;
#ifdef CONFIG_SSTAR_SPECTRUM_MGMT
	struct ieee80211_channel_state *chan_state =
		ieee80211_get_channel_state(local, sdata);
#endif
	struct Sstar_ieee80211_mgmt *mgmt =
		(struct Sstar_ieee80211_mgmt *)rx->skb->data;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);
	int len = rx->skb->len;

	if (!ieee80211_is_action(mgmt->frame_control))
		return RX_CONTINUE;

	/* drop too small frames */
	if (len < IEEE80211_MIN_ACTION_SIZE)
		return RX_DROP_UNUSABLE;

	if (!rx->sta && mgmt->u.action.category != SSTAR_WLAN_CATEGORY_PUBLIC)
		return RX_DROP_UNUSABLE;

	if (!(status->rx_flags & IEEE80211_RX_RA_MATCH))
		return RX_DROP_UNUSABLE;

	switch (mgmt->u.action.category) {
#ifdef SSTAR_SUPPORT_WIDTH_40M
	case SSTAR_WLAN_CATEGORY_PUBLIC:
		if (sdata->vif.type != NL80211_IFTYPE_STATION)
			break;
		if (SSTAR_WLAN_PUB_ACTION_EX_CHL_SW_ANNOUNCE !=
		    mgmt->u.action.u.ext_chan_switch.action_code)
			break;
		if (memcmp(mgmt->bssid, sdata->u.mgd.bssid, ETH_ALEN))
			break;

		goto queue;
#endif
	case SSTAR_WLAN_CATEGORY_BACK:
		/*
		 * The aggregation code is not prepared to handle
		 * anything but STA/AP due to the BSSID handling;
		 * IBSS could work in the code but isn't supported
		 * by drivers or the standard.
		 */
		if (sdata->vif.type != NL80211_IFTYPE_STATION &&
		    sdata->vif.type != NL80211_IFTYPE_AP_VLAN &&
		    sdata->vif.type != NL80211_IFTYPE_AP)
			break;

		/* verify action_code is present */
		if (len < IEEE80211_MIN_ACTION_SIZE + 1)
			break;

		switch (mgmt->u.action.u.addba_req.action_code) {
		case WLAN_ACTION_ADDBA_REQ:
			if (len < (IEEE80211_MIN_ACTION_SIZE +
				   sizeof(mgmt->u.action.u.addba_req)))
				goto invalid;
			break;
		case WLAN_ACTION_ADDBA_RESP:
			if (len < (IEEE80211_MIN_ACTION_SIZE +
				   sizeof(mgmt->u.action.u.addba_resp)))
				goto invalid;
			break;
		case WLAN_ACTION_DELBA:
			if (len < (IEEE80211_MIN_ACTION_SIZE +
				   sizeof(mgmt->u.action.u.delba)))
				goto invalid;
			break;
		default:
			goto invalid;
		}

		goto queue;
#ifdef CONFIG_SSTAR_SPECTRUM_MGMT
	case SSTAR_WLAN_CATEGORY_SPECTRUM_MGMT:
		if (chan_state->conf.channel->band != IEEE80211_BAND_5GHZ)
			break;

		if (sdata->vif.type != NL80211_IFTYPE_STATION)
			break;

		/* verify action_code is present */
		if (len < IEEE80211_MIN_ACTION_SIZE + 1)
			break;

		switch (mgmt->u.action.u.measurement.action_code) {
		case WLAN_ACTION_SPCT_MSR_REQ:
			if (len < (IEEE80211_MIN_ACTION_SIZE +
				   sizeof(mgmt->u.action.u.measurement)))
				break;
			ieee80211_process_measurement_req(sdata, mgmt, len);
			goto handled;
		case WLAN_ACTION_SPCT_CHL_SWITCH:
			if (len < (IEEE80211_MIN_ACTION_SIZE +
				   sizeof(mgmt->u.action.u.chan_switch)))
				break;

			if (sdata->vif.type != NL80211_IFTYPE_STATION)
				break;

			if (memcmp(mgmt->bssid, sdata->u.mgd.bssid, ETH_ALEN))
				break;

			goto queue;
		}
		break;
#endif
#ifdef CONFIG_SSTAR_SA_QUERY
	case SSTAR_WLAN_CATEGORY_SA_QUERY:
		if (len < (IEEE80211_MIN_ACTION_SIZE +
			   sizeof(mgmt->u.action.u.sa_query)))
			break;

		switch (mgmt->u.action.u.sa_query.action) {
		case WLAN_ACTION_SA_QUERY_REQUEST:
			if (sdata->vif.type != NL80211_IFTYPE_STATION)
				break;
			ieee80211_process_sa_query_req(sdata, mgmt, len);
			goto handled;
		}
		break;
#endif
#ifdef CONFIG_MAC80211_SSTAR_MESH
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0))
	case SSTAR_WLAN_CATEGORY_SELF_PROTECTED:
		switch (mgmt->u.action.u.self_prot.action_code) {
		case WLAN_SP_MESH_PEERING_OPEN:
		case WLAN_SP_MESH_PEERING_CLOSE:
		case WLAN_SP_MESH_PEERING_CONFIRM:
			if (!ieee80211_vif_is_mesh(&sdata->vif))
				goto invalid;
			if (sdata->u.mesh.security != IEEE80211_MESH_SEC_NONE)
				/* userspace handles this frame */
				break;
			goto queue;
		case WLAN_SP_MGK_INFORM:
		case WLAN_SP_MGK_ACK:
			if (!ieee80211_vif_is_mesh(&sdata->vif))
				goto invalid;
			break;
		}
		break;
#endif
	case SSTAR_WLAN_CATEGORY_MESH_ACTION:
		if (!ieee80211_vif_is_mesh(&sdata->vif))
			break;
		if (mesh_action_is_path_sel(mgmt) &&
		    (!mesh_path_sel_is_hwmp(sdata)))
			break;
		goto queue;
#else
	default:
		goto handled;
#endif
	}

	return RX_CONTINUE;

invalid:
	status->rx_flags |= IEEE80211_RX_MALFORMED_ACTION_FRM;
	/* will return in the next handlers */
	return RX_CONTINUE;

handled:
	if (rx->sta)
		rx->sta->rx_packets++;
	Sstar_dev_kfree_skb(rx->skb);
	return RX_QUEUED;

queue:
	rx->skb->pkt_type = IEEE80211_SDATA_QUEUE_TYPE_FRAME;
	Sstar_skb_queue_tail(&sdata->skb_queue, rx->skb);
	ieee80211_queue_work(&local->hw, &sdata->work);
	if (rx->sta)
		rx->sta->rx_packets++;
	return RX_QUEUED;
}

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_userspace_mgmt(struct ieee80211_rx_data *rx)
{
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);

	/* skip known-bad action frames and return them in the next handler */
	if (status->rx_flags & IEEE80211_RX_MALFORMED_ACTION_FRM)
		return RX_CONTINUE;

	if (ieee80211_is_assoc_req(
		    ((struct Sstar_ieee80211_mgmt *)rx->skb->data)
			    ->frame_control)) {
		u8 Sstar_event[] = { 221,  9,	 0xF1, 0xF2, 0xF3, 0xF4,
				     0x21, 0x22, 0x23, 0x24, 0x25 };
		int rxmgm_len = rx->skb->len;
		//int rxmgm_buf = rx->skb->data;
		u8 *rxmgm_buf = Sstar_kmalloc(rxmgm_len + sizeof(Sstar_event),
					      GFP_ATOMIC);

		memcpy(rxmgm_buf, rx->skb->data, rxmgm_len);
		memcpy(&rxmgm_buf[rxmgm_len], Sstar_event, sizeof(Sstar_event));
		rxmgm_len += sizeof(Sstar_event);
		/*
		 * Getting here means the kernel doesn't know how to handle
		 * it, but maybe userspace does ... include returned frames
		 * so userspace can register for those to know whether ones
		 * it transmitted were processed or returned.
		 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)) &&                          \
	!defined(COMPAT_KERNEL_RELEASE)
		if (cfg80211_rx_mgmt(rx->sdata->dev, status->freq, rxmgm_buf,
				     rxmgm_len, GFP_ATOMIC))
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
		if (cfg80211_rx_mgmt(rx->sdata->dev, status->freq, -90,
				     rxmgm_buf, rxmgm_len, GFP_ATOMIC))
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0))
		if (cfg80211_rx_mgmt(&rx->sdata->wdev, status->freq, -90,
				     rxmgm_buf, rxmgm_len, GFP_ATOMIC))
//#elif  (LINUX_VERSION_CODE < KERNEL_VERSION(3,18,25))
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 00))
		if (cfg80211_rx_mgmt(&rx->sdata->wdev, status->freq, -90,
				     rxmgm_buf, rxmgm_len, 0, GFP_ATOMIC))
#else
		if (cfg80211_rx_mgmt(&rx->sdata->wdev, status->freq, -90,
				     rxmgm_buf, rxmgm_len, 0))
#endif
		{
			if (rx->sta)
				rx->sta->rx_packets++;
			Sstar_dev_kfree_skb(rx->skb);
			Sstar_kfree(rxmgm_buf);
			return RX_QUEUED;
		}
		Sstar_kfree(rxmgm_buf);
		return RX_CONTINUE;
	}
	/*
	 * Getting here means the kernel doesn't know how to handle
	 * it, but maybe userspace does ... include returned frames
	 * so userspace can register for those to know whether ones
	 * it transmitted were processed or returned.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)) &&                          \
	!defined(COMPAT_KERNEL_RELEASE)
	if (cfg80211_rx_mgmt(rx->sdata->dev, status->freq, rx->skb->data,
			     rx->skb->len, GFP_ATOMIC))
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
	if (cfg80211_rx_mgmt(rx->sdata->dev, status->freq, -90, rx->skb->data,
			     rx->skb->len, GFP_ATOMIC))
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0))
	if (cfg80211_rx_mgmt(&rx->sdata->wdev, status->freq, -90, rx->skb->data,
			     rx->skb->len, GFP_ATOMIC))
//#elif  (LINUX_VERSION_CODE < KERNEL_VERSION(3,18,25))
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 00))
	if (cfg80211_rx_mgmt(&rx->sdata->wdev, status->freq, -90, rx->skb->data,
			     rx->skb->len, 0, GFP_ATOMIC))
#else
	if (cfg80211_rx_mgmt(&rx->sdata->wdev, status->freq, -90, rx->skb->data,
			     rx->skb->len, 0))
#endif
	{
		if (rx->sta)
			rx->sta->rx_packets++;
		Sstar_dev_kfree_skb(rx->skb);
		return RX_QUEUED;
	}

	return RX_CONTINUE;
}

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_action_return(struct ieee80211_rx_data *rx)
{
	struct ieee80211_local *local = rx->local;
	struct Sstar_ieee80211_mgmt *mgmt =
		(struct Sstar_ieee80211_mgmt *)rx->skb->data;
	struct sk_buff *nskb;
	struct ieee80211_sub_if_data *sdata = rx->sdata;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);

	if (!ieee80211_is_action(mgmt->frame_control))
		return RX_CONTINUE;

	/*
	 * For AP mode, hostapd is responsible for handling any action
	 * frames that we didn't handle, including returning unknown
	 * ones. For all other modes we will return them to the sender,
	 * setting the 0x80 bit in the action category, as required by
	 * 802.11-2007 7.3.1.11.
	 * Newer versions of hostapd shall also use the management frame
	 * registration mechanisms, but older ones still use cooked
	 * monitor interfaces so push all frames there.
	 */
	if (!(status->rx_flags & IEEE80211_RX_MALFORMED_ACTION_FRM) &&
	    (sdata->vif.type == NL80211_IFTYPE_AP ||
	     sdata->vif.type == NL80211_IFTYPE_AP_VLAN))
		return RX_DROP_MONITOR;

	if (is_multicast_ether_addr(mgmt->da)) {
		Sstar_printk_debug("[%s]: drop multicast action\n",
				   sdata->name);
		return RX_DROP_UNUSABLE;
	}
	/* do not return rejected action frames */
	if (mgmt->u.action.category & 0x80)
		return RX_DROP_UNUSABLE;

	nskb = Sstar_skb_copy_expand(rx->skb, local->hw.extra_tx_headroom, 0,
				     GFP_ATOMIC);
	if (nskb) {
		struct Sstar_ieee80211_mgmt *nmgmt = (void *)nskb->data;

		nmgmt->u.action.category |= 0x80;
		memcpy(nmgmt->da, nmgmt->sa, ETH_ALEN);
		memcpy(nmgmt->sa, rx->sdata->vif.addr, ETH_ALEN);

		memset(nskb->cb, 0, sizeof(nskb->cb));

		ieee80211_tx_skb(rx->sdata, nskb);
	}
	Sstar_dev_kfree_skb(rx->skb);
	return RX_QUEUED;
}

static ieee80211_rx_result debug_noinline
ieee80211_rx_h_mgmt(struct ieee80211_rx_data *rx)
{
	struct ieee80211_sub_if_data *sdata = rx->sdata;
	ieee80211_rx_result rxs;
	struct Sstar_ieee80211_mgmt *mgmt = (void *)rx->skb->data;
	__le16 stype;
	rxs = ieee80211_work_rx_mgmt(rx->sdata, rx->skb);
	if (rxs != RX_CONTINUE)
		return rxs;
	stype = mgmt->frame_control & cpu_to_le16(IEEE80211_FCTL_STYPE);
#ifdef SSTAR_AP_SME
	if ((sdata->vif.type == NL80211_IFTYPE_AP) ||
	    (sdata->vif.type == NL80211_IFTYPE_P2P_GO)) {
		Sstar_printk_ap("%s:sdata(%x),type(%d)\n", __func__, (int)sdata,
				sdata->vif.type);
		if (ieee80211_ap_sme_rx_mgmt(sdata, rx->skb))
			return RX_DROP_MONITOR;

		return RX_QUEUED;
	}
#endif
	if (!ieee80211_vif_is_mesh(&sdata->vif) &&
	    sdata->vif.type != NL80211_IFTYPE_ADHOC &&
	    sdata->vif.type != NL80211_IFTYPE_STATION)
		return RX_DROP_MONITOR;

	switch (stype) {
	case cpu_to_le16(IEEE80211_STYPE_BEACON):
	case cpu_to_le16(IEEE80211_STYPE_PROBE_RESP):
		/* process for all: mesh, mlme, ibss */
		break;
	case cpu_to_le16(IEEE80211_STYPE_DEAUTH):
	case cpu_to_le16(IEEE80211_STYPE_DISASSOC):
		if (is_multicast_ether_addr(mgmt->da) &&
		    !is_broadcast_ether_addr(mgmt->da))
			return RX_DROP_MONITOR;

		/* process only for station */
		if (sdata->vif.type != NL80211_IFTYPE_STATION)
			return RX_DROP_MONITOR;
		break;
	case cpu_to_le16(IEEE80211_STYPE_PROBE_REQ):
	case cpu_to_le16(IEEE80211_STYPE_AUTH):
		/* process only for ibss */
		if (sdata->vif.type != NL80211_IFTYPE_ADHOC)
			return RX_DROP_MONITOR;
		break;
	default:
		return RX_DROP_MONITOR;
	}
	/* queue up frame and kick off work to process it */
	rx->skb->pkt_type = IEEE80211_SDATA_QUEUE_TYPE_FRAME;
	Sstar_skb_queue_tail(&sdata->skb_queue, rx->skb);
	ieee80211_queue_work(&rx->local->hw, &sdata->work);
	if (rx->sta)
		rx->sta->rx_packets++;

	return RX_QUEUED;
}

/* TODO: use IEEE80211_RX_FRAGMENTED */
static bool ieee8211_cook_rtap_hdr(struct ieee80211_rx_data *rx,
				   struct ieee80211_rate *rate)
{
	struct sk_buff *skb = rx->skb;
	struct ieee80211_rtap_hdr {
		struct ieee80211_radiotap_header hdr;
		u8 flags;
		u8 rate_or_pad;
		__le16 chan_freq;
		__le16 chan_flags;
	} __packed *rthdr;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);

	if (Sstar_skb_headroom(skb) < sizeof(*rthdr) &&
	    Sstar_pskb_expand_head(skb, sizeof(*rthdr), 0, GFP_ATOMIC))
		goto fail;

	rthdr = (void *)Sstar_skb_push(skb, sizeof(*rthdr));
	memset(rthdr, 0, sizeof(*rthdr));
	rthdr->hdr.it_len = cpu_to_le16(sizeof(*rthdr));
	rthdr->hdr.it_present = cpu_to_le32((1 << IEEE80211_RADIOTAP_FLAGS) |
					    (1 << IEEE80211_RADIOTAP_CHANNEL));

	if (rate) {
		rthdr->rate_or_pad = rate->bitrate / 5;
		rthdr->hdr.it_present |=
			cpu_to_le32(1 << IEEE80211_RADIOTAP_RATE);
	}
	rthdr->chan_freq = cpu_to_le16(status->freq);

	if (status->band == IEEE80211_BAND_5GHZ)
		rthdr->chan_flags =
			cpu_to_le16(IEEE80211_CHAN_OFDM | IEEE80211_CHAN_5GHZ);
	else
		rthdr->chan_flags =
			cpu_to_le16(IEEE80211_CHAN_DYN | IEEE80211_CHAN_2GHZ);

	return true;
fail:
	return false;
}
#ifdef CONFIG_SSTAR_MONITOR_HDR_PRISM
static bool ieee8211_cook_prism_hdr(struct ieee80211_rx_data *rx,
				    struct ieee80211_rate *rate)
{
	struct sk_buff *skb = rx->skb;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_prism_header *prism_header;
	size_t hdr_len = ieee80211_hdrlen(hdr->frame_control);
	size_t data_size = skb->len - hdr_len;

	if (Sstar_skb_headroom(skb) < sizeof(*prism_header) &&
	    Sstar_pskb_expand_head(skb, sizeof(*prism_header), 0, GFP_ATOMIC))
		goto fail;
	prism_header = (void *)Sstar_skb_push(skb, sizeof(*prism_header));
	memset(prism_header, 0, sizeof(*prism_header));

	prism_header->msg_code = ieee80211_prism_sniffrm;
	prism_header->msg_len = sizeof(struct ieee80211_prism_header);
	memcpy(prism_header->dev_name, rx->sdata->name, IFNAMSIZ);

	prism_header->hosttime.did = ieee80211_prism_sniffrm_hosttime;
	prism_header->hosttime.status = 0;
	prism_header->hosttime.len = 4;
	prism_header->hosttime.data = jiffies;

	prism_header->mactime.did = ieee80211_prism_sniffrm_mactime;
	prism_header->mactime.status = 0;
	prism_header->mactime.len = 0;
	prism_header->mactime.data = 0;

	prism_header->istx.did = ieee80211_prism_sniffrm_istx;
	prism_header->istx.status = 0;
	prism_header->istx.len = 0;
	prism_header->istx.data = 0;

	prism_header->channel.did = ieee80211_prism_sniffrm_channel;
	prism_header->channel.status = 0;
	prism_header->channel.len = 4;
	prism_header->channel.data = status->freq;

	prism_header->rssi.did = ieee80211_prism_sniffrm_rssi;
	prism_header->rssi.status = 0;
	prism_header->rssi.len = 4;
	prism_header->rssi.data = status->signal;

	prism_header->signal.did = ieee80211_prism_sniffrm_signal;
	prism_header->signal.status = 0;
	prism_header->signal.len = 4;
	prism_header->signal.data = 0;

	prism_header->noise.did = ieee80211_prism_sniffrm_noise;
	prism_header->noise.status = 0;
	prism_header->noise.len = 4;
	prism_header->noise.data = 0;

	prism_header->rate.did = ieee80211_prism_sniffrm_rate;
	prism_header->rate.status = 0;
	prism_header->rate.len = 4;
	prism_header->rate.data = 0;

	prism_header->frmlen.did = ieee80211_prism_sniffrm_frmlen;
	prism_header->frmlen.status = 0;
	prism_header->frmlen.len = 4;
	prism_header->frmlen.data = data_size;

	return true;
fail:
	return false;
}
#endif
static void ieee80211_rx_cooked_monitor(struct ieee80211_rx_data *rx,
					struct ieee80211_rate *rate)
{
	struct ieee80211_sub_if_data *sdata;
	struct ieee80211_local *local = rx->local;
	struct sk_buff *skb = rx->skb;
	struct net_device *prev_dev = NULL;
	bool ret = true;
	/*
	 * If cooked monitor has been processed already, then
	 * don't do it again. If not, set the flag.
	 */
	if (rx->flags & IEEE80211_RX_CMNTR)
		goto out_free_skb;
	rx->flags |= IEEE80211_RX_CMNTR;

	list_for_each_entry_rcu (sdata, &local->interfaces, list) {
		if (!ieee80211_sdata_running(sdata))
			continue;

		if (sdata->vif.type != NL80211_IFTYPE_MONITOR ||
		    !(sdata->u.mntr_flags & MONITOR_FLAG_COOK_FRAMES))
			continue;

#ifdef CONFIG_SSTAR_SUPPORT_MULTIAP
		if (strstr(sdata->name, rx->sdata->name) == NULL)
			continue;
#endif
		prev_dev = sdata->dev;
		sdata->dev->stats.rx_packets++;
		sdata->dev->stats.rx_bytes += skb->len;
		break;
	}

	if (prev_dev) {
#ifdef CONFIG_SSTAR_MONITOR_HDR_PRISM
		if (prev_dev->type == ARPHRD_IEEE80211_PRISM) {
			ret = ieee8211_cook_prism_hdr(rx, rate);
		} else
#endif
		{
			WARN_ON(prev_dev->type != ARPHRD_IEEE80211_RADIOTAP);
			ret = ieee8211_cook_rtap_hdr(rx, rate);
		}
		if (ret == false) {
			goto out_free_skb;
		}

		Sstar_skb_set_mac_header(skb, 0);
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		skb->pkt_type = PACKET_OTHERHOST;
		skb->protocol = htons(ETH_P_802_2);
		skb->dev = prev_dev;
		Sstar_netif_receive_skb(skb);
		return;
	}
out_free_skb:
	Sstar_dev_kfree_skb(skb);
}
bool ieee80211_rx_sta_cook_deauthen(struct sta_info *sta)
{
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	struct ieee80211_local *local = sta->local;
	struct ieee80211_rx_data rx;
	struct Sstar_ieee80211_mgmt *deauth;
	struct ieee80211_rx_status *status;

	if (sdata->local != local) {
		return false;
	}
	memset(&rx, 0, sizeof(rx));

	rx.sdata = sdata;
	rx.local = local;
	rx.flags = 0;
	rx.skb = Sstar_alloc_skb(local->hw.extra_tx_headroom +
					 sizeof(struct Sstar_ieee80211_mgmt) +
					 64,
				 GFP_ATOMIC);
	if (rx.skb == NULL) {
		return false;
	}
	Sstar_skb_reserve(rx.skb, local->hw.extra_tx_headroom);
	Sstar_printk_mgmt("%s:[%pM]\n", __func__, sta->sta.addr);
	deauth = (struct Sstar_ieee80211_mgmt *)Sstar_skb_put(
		rx.skb, sizeof(struct Sstar_ieee80211_mgmt));
	deauth->duration = 0;
	memcpy(deauth->da, sdata->vif.addr, ETH_ALEN);
	memcpy(deauth->sa, sta->sta.addr, ETH_ALEN);
	memcpy(deauth->bssid, sdata->vif.addr, ETH_ALEN);
	deauth->frame_control =
		cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_DEAUTH |
			    IEEE80211_FCTL_TODS);
	deauth->u.deauth.reason_code = WLAN_REASON_DEAUTH_LEAVING;
	deauth->seq_ctrl = 0;

	status = IEEE80211_SKB_RXCB(rx.skb);
	status->band = IEEE80211_BAND_2GHZ;

	ieee80211_rx_cooked_monitor(&rx, NULL);

	return true;
}
static void ieee80211_rx_handlers_result(struct ieee80211_rx_data *rx,
					 ieee80211_rx_result res)
{
	switch (res) {
	case RX_DROP_MONITOR:
		I802_DEBUG_INC(rx->sdata->local->rx_handlers_drop);
		if (rx->sta)
			rx->sta->rx_dropped++;
		/* fall through */
	case RX_CONTINUE: {
		struct ieee80211_rate *rate = NULL;
		struct ieee80211_supported_band *sband;
		struct ieee80211_rx_status *status;

		status = IEEE80211_SKB_RXCB((rx->skb));

		sband = rx->local->hw.wiphy->bands[status->band];
		if (!(status->flag & RX_FLAG_HT))
			rate = &sband->bitrates[status->rate_idx];

		ieee80211_rx_cooked_monitor(rx, rate);
		break;
	}
	case RX_DROP_UNUSABLE:
		I802_DEBUG_INC(rx->sdata->local->rx_handlers_drop);
		if (rx->sta)
			rx->sta->rx_dropped++;
		Sstar_dev_kfree_skb(rx->skb);
		break;
	case RX_QUEUED:
		I802_DEBUG_INC(rx->sdata->local->rx_handlers_queued);
		break;
	}
}

#ifndef CONFIG_SSTAR_USE_SW_ENC
ieee80211_rx_result
ieee80211_rx_h_michael_mic_verify(struct ieee80211_rx_data *rx)
{
	unsigned int hdrlen;
	struct sk_buff *skb = rx->skb;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;

	/*
	 * it makes no sense to check for MIC errors on anything other
	 * than data frames.
	 */
	if (!ieee80211_is_data_present(hdr->frame_control))
		return RX_CONTINUE;

	/*
	 * No way to verify the MIC if the hardware stripped it or
	 * the IV with the key index. In this case we have solely rely
	 * on the driver to set RX_FLAG_MMIC_ERROR in the event of a
	 * MIC failure report.
	 */
	if (status->flag & (RX_FLAG_MMIC_STRIPPED | RX_FLAG_IV_STRIPPED)) {
		if (status->flag & RX_FLAG_MMIC_ERROR)
			goto mic_fail;

		if (!(status->flag & RX_FLAG_IV_STRIPPED) && rx->key)
			goto update_iv;

		return RX_CONTINUE;
	}

	/*
	 * Some hardware seems to generate Michael MIC failure reports; even
	 * though, the frame was not encrypted with TKIP and therefore has no
	 * MIC. Ignore the flag them to avoid triggering countermeasures.
	 */
	if (!rx->key || rx->key->conf.cipher != WLAN_CIPHER_SUITE_TKIP ||
	    !(status->flag & RX_FLAG_DECRYPTED))
		return RX_CONTINUE;

	if (rx->sdata->vif.type == NL80211_IFTYPE_AP && rx->key->conf.keyidx) {
		/*
		 * APs with pairwise keys should never receive Michael MIC
		 * errors for non-zero keyidx because these are reserved for
		 * group keys and only the AP is sending real multicast
		 * frames in the BSS. (
		 */
		return RX_DROP_UNUSABLE;
	}

	if (status->flag & RX_FLAG_MMIC_ERROR)
		goto mic_fail;

	hdrlen = ieee80211_hdrlen(hdr->frame_control);
	if (skb->len < hdrlen + SSTAR_MICHAEL_MIC_LEN)
		return RX_DROP_UNUSABLE;

	/* remove Michael MIC from payload */
	Sstar_skb_trim(skb, skb->len - SSTAR_MICHAEL_MIC_LEN);

update_iv:
	/* update IV in key information to be able to detect replays */
	rx->key->u.tkip.rx[rx->security_idx].iv32 = rx->tkip_iv32;
	rx->key->u.tkip.rx[rx->security_idx].iv16 = rx->tkip_iv16;

	return RX_CONTINUE;

mic_fail:
#ifdef CONFIG_SSTAR_SUPPORT_MIC_FAILURE_REPORT
	/*
	 * In some cases the key can be unset - e.g. a multicast packet, in
	 * a driver that supports HW encryption. Send up the key idx only if
	 * the key is set.
	 */
	mac80211_ev_michael_mic_failure(rx->sdata,
					rx->key ? rx->key->conf.keyidx : -1,
					(void *)skb->data, NULL, GFP_ATOMIC);
#else
	Sstar_printk_debug("Mic Err\n");
#endif
	return RX_DROP_UNUSABLE;
}
#endif
static void ieee80211_rx_handlers(struct ieee80211_rx_data *rx,
				  struct sk_buff_head *frames)
{
	ieee80211_rx_result res = RX_DROP_MONITOR;
	struct sk_buff *skb;

#define CALL_RXH(rxh)                                                          \
	do {                                                                   \
		res = rxh(rx);                                                 \
		if (res != RX_CONTINUE)                                        \
			goto rxh_next;                                         \
	} while (0);

	spin_lock_bh(&rx->local->rx_path_lock);
	while ((skb = __Sstar_skb_dequeue(frames))) {
		/*
		 * all the other fields are valid across frames
		 * that belong to an aMPDU since they are on the
		 * same TID from the same station
		 */
		rx->skb = skb;
		CALL_RXH(ieee80211_rx_h_decrypt)
#ifdef CONFIG_SSTAR_MAC80211_NO_USE
		CALL_RXH(ieee80211_rx_h_check_more_data)
#endif
		CALL_RXH(ieee80211_rx_h_uapsd_and_pspoll)
		CALL_RXH(ieee80211_rx_h_sta_process)
		CALL_RXH(ieee80211_rx_h_defragment)
		CALL_RXH(ieee80211_rx_h_michael_mic_verify)
		/* must be after MMIC verify so header is counted in MPDU mic */
#ifdef CONFIG_MAC80211_SSTAR_MESH
		if (ieee80211_vif_is_mesh(&rx->sdata->vif))
			CALL_RXH(ieee80211_rx_h_mesh_fwding);
#endif
		CALL_RXH(ieee80211_rx_h_remove_qos_control)
		CALL_RXH(ieee80211_rx_h_amsdu)
		CALL_RXH(ieee80211_rx_h_data)
#ifdef CONFIG_SSTAR_PROCESS_CTRL
		CALL_RXH(ieee80211_rx_h_ctrl)
#endif
		CALL_RXH(ieee80211_rx_h_mgmt_check)
		CALL_RXH(ieee80211_rx_h_action)
		CALL_RXH(ieee80211_rx_h_userspace_mgmt)
		CALL_RXH(ieee80211_rx_h_action_return)
		CALL_RXH(ieee80211_rx_h_mgmt)
	rxh_next:
		ieee80211_rx_handlers_result(rx, res);
#undef CALL_RXH
	}
	spin_unlock_bh(&rx->local->rx_path_lock);
}
/* rx handlers */
#ifdef CONFIG_SSTAR_STA_LISTEN
static ieee80211_rx_result debug_noinline
ieee80211_rx_h_listen(struct ieee80211_rx_data *rx)
{
	struct ieee80211_local *local = rx->local;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(rx->skb);

	if (local->listen_started && (rx->sdata == local->listen_sdata) &&
	    (status->flag & RX_FLAG_STA_LISTEN)) {
		/* queue up frame and kick off work to process it */
		rx->skb->pkt_type = IEEE80211_SDATA_QUEUE_TYPE_FRAME;
		Sstar_skb_queue_tail(&rx->sdata->skb_queue, rx->skb);
		ieee80211_queue_work(&rx->local->hw, &rx->sdata->work);
		return RX_QUEUED;
	}
	return RX_CONTINUE;
}
#endif
static void ieee80211_invoke_rx_handlers(struct ieee80211_rx_data *rx)
{
	ieee80211_rx_result res = RX_DROP_MONITOR;
	struct sk_buff_head reorder_release;

	__Sstar_skb_queue_head_init(&reorder_release);
#define CALL_RXH(rxh)                                                          \
	do {                                                                   \
		res = rxh(rx);                                                 \
		if (res != RX_CONTINUE)                                        \
			goto rxh_next;                                         \
	} while (0)
#ifdef CONFIG_SSTAR_STA_LISTEN
	CALL_RXH(ieee80211_rx_h_listen);
#endif
	CALL_RXH(ieee80211_rx_h_passive_scan);
	CALL_RXH(ieee80211_rx_h_check);
	ieee80211_rx_reorder_ampdu(rx, &reorder_release);
	ieee80211_rx_handlers(rx, &reorder_release);
	return;

rxh_next:
	ieee80211_rx_handlers_result(rx, res);

#undef CALL_RXH
}

/*
 * This function makes calls into the RX path, therefore
 * it has to be invoked under RCU read lock.
 */
void ieee80211_release_reorder_timeout(struct sta_info *sta, int tid)
{
	struct ieee80211_rx_data rx = {
		.sta = sta,
		.sdata = sta->sdata,
		.local = sta->local,
		/* This is OK -- must be QoS data frame */
		.security_idx = tid,
		.seqno_idx = tid,
		.flags = 0,
	};
	struct tid_ampdu_rx *tid_agg_rx;
	struct sk_buff_head frames;

	tid_agg_rx = rcu_dereference(sta->ampdu_mlme.tid_rx[tid]);
	if (!tid_agg_rx)
		return;

	__Sstar_skb_queue_head_init(&frames);
	spin_lock(&tid_agg_rx->reorder_lock);
	ieee80211_sta_reorder_release(&sta->local->hw, tid_agg_rx, &frames);
	spin_unlock(&tid_agg_rx->reorder_lock);

	ieee80211_rx_handlers(&rx, &frames);
}

/* main receive path */

static int prepare_for_handlers(struct ieee80211_rx_data *rx,
				struct ieee80211_hdr *hdr)
{
	struct ieee80211_sub_if_data *sdata = rx->sdata;
	struct sk_buff *skb = rx->skb;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
	u8 *bssid = ieee80211_get_bssid(hdr, skb->len, sdata->vif.type);
	int multicast = is_multicast_ether_addr(hdr->addr1);
	switch (sdata->vif.type) {
	case NL80211_IFTYPE_STATION:
		if (!bssid && !sdata->u.mgd.use_4addr)
			return 0;
		if (!multicast && Sstar_compare_ether_addr(sdata->vif.addr,
							   hdr->addr1) != 0) {
			if (!(sdata->dev->flags & IFF_PROMISC) ||
			    sdata->u.mgd.use_4addr)
				return 0;
			status->rx_flags &= ~IEEE80211_RX_RA_MATCH;
		}
		break;
#ifdef CONFIG_SSTAR_SUPPORT_IBSS
	case NL80211_IFTYPE_ADHOC:
		if (!bssid)
			return 0;
		if (ieee80211_is_beacon(hdr->frame_control)) {
			return 1;
		} else if (!ieee80211_bssid_match(bssid, sdata->u.ibss.bssid)) {
			if (!(status->rx_flags & IEEE80211_RX_IN_SCAN))
				return 0;
			status->rx_flags &= ~IEEE80211_RX_RA_MATCH;
		} else if (!multicast &&
			   Sstar_compare_ether_addr(sdata->vif.addr,
						    hdr->addr1) != 0) {
			if (!(sdata->dev->flags & IFF_PROMISC))
				return 0;
			status->rx_flags &= ~IEEE80211_RX_RA_MATCH;
		} else if (!rx->sta) {
			int rate_idx;
			if (status->flag & RX_FLAG_HT)
				rate_idx = 0; /* TODO: HT rates */
			else
				rate_idx = status->rate_idx;
			rx->sta =
				ieee80211_ibss_add_sta(sdata, bssid, hdr->addr2,
						       BIT(rate_idx),
						       GFP_ATOMIC);
		}
		break;
#endif
#ifdef CONFIG_MAC80211_SSTAR_MESH
	case NL80211_IFTYPE_MESH_POINT:
		if (!multicast && Sstar_compare_ether_addr(sdata->vif.addr,
							   hdr->addr1) != 0) {
			if (!(sdata->dev->flags & IFF_PROMISC))
				return 0;

			status->rx_flags &= ~IEEE80211_RX_RA_MATCH;
		}
		break;
#endif
	case NL80211_IFTYPE_AP_VLAN:
	case NL80211_IFTYPE_AP:
		if (!bssid) {
			if (Sstar_compare_ether_addr(sdata->vif.addr,
						     hdr->addr1))
				return 0;
		} else if (!ieee80211_bssid_match(bssid, sdata->vif.addr)) {
			if (ieee80211_is_beacon(hdr->frame_control))
				status->rx_flags |= IEEE80211_RX_ERP_BEACON;
			else if (!(status->rx_flags & IEEE80211_RX_IN_SCAN) &&
				 ieee80211_is_public_action(hdr, skb->len))
				return 1;
			else if (ieee80211_is_probe_req(hdr->frame_control)) {
				struct Sstar_ieee80211_mgmt *mgmt =
					(struct Sstar_ieee80211_mgmt *)hdr;
				if (Sstar_ieee80211_find_ie(
					    SSTAR_WLAN_EID_PRIVATE,
					    mgmt->u.probe_req.variable,
					    skb->len -
						    offsetof(
							    struct Sstar_ieee80211_mgmt,
							    u.probe_req
								    .variable))) {
					status->rx_flags |=
						IEEE80211_RX_SPECIAL_PROBE_REQ;
					return 1;
				}
			}
			if (!(status->rx_flags & IEEE80211_RX_IN_SCAN) &&
			    !ieee80211_is_beacon(hdr->frame_control))
				return 0;
			status->rx_flags &= ~IEEE80211_RX_RA_MATCH;
		} else if (ieee80211_is_probe_req(hdr->frame_control)) {
			struct Sstar_ieee80211_mgmt *mgmt =
				(struct Sstar_ieee80211_mgmt *)hdr;
			if (Sstar_ieee80211_find_ie(
				    SSTAR_WLAN_EID_PRIVATE,
				    mgmt->u.probe_req.variable,
				    skb->len -
					    offsetof(struct Sstar_ieee80211_mgmt,
						     u.probe_req.variable))) {
				status->rx_flags |=
					IEEE80211_RX_SPECIAL_PROBE_REQ;
				return 1;
			}
		}
		break;
#ifdef CONFIG_SSTAR_SUPPORT_WDS
	case NL80211_IFTYPE_WDS:
		if (bssid || !ieee80211_is_data(hdr->frame_control))
			return 0;
		if (Sstar_compare_ether_addr(sdata->u.wds.remote_addr,
					     hdr->addr2))
			return 0;
		break;
#endif
	default:
		/* should never get here */
		WARN_ON(1);
		break;
	}

	return 1;
}

/*
 * This function returns whether or not the SKB
 * was destined for RX processing or not, which,
 * if consume is true, is equivalent to whether
 * or not the skb was consumed.
 */
static bool ieee80211_prepare_and_rx_handle(struct ieee80211_rx_data *rx,
					    struct sk_buff *skb, bool consume)
{
	struct ieee80211_sub_if_data *sdata = rx->sdata;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
	struct ieee80211_hdr *hdr = (void *)skb->data;
	int prepares;

	rx->skb = skb;
	status->rx_flags |= IEEE80211_RX_RA_MATCH;
	prepares = prepare_for_handlers(rx, hdr);

	if (!prepares) {
		return false;
	}

	if (!consume) {
		skb = Sstar_skb_copy(skb, GFP_ATOMIC);
		if (!skb) {
			if (net_ratelimit())
				Sstar_printk_debug(
					"failed to copy skb for %s\n",
					sdata->name);
			return true;
		}

		rx->skb = skb;
	}

	ieee80211_invoke_rx_handlers(rx);
	return true;
}

/*
 * This is the actual Rx frames handler. as it blongs to Rx path it must
 * be called with rcu_read_lock protection.
 */
static void __ieee80211_rx_handle_packet(struct ieee80211_hw *hw,
					 struct sk_buff *skb)
{
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);
	struct ieee80211_local *local = hw_to_local(hw);
	struct ieee80211_sub_if_data *sdata;
	struct ieee80211_hdr *hdr;
	__le16 fc;
	struct ieee80211_rx_data rx;
	struct ieee80211_sub_if_data *prev;
	struct sta_info *sta, *tmp, *prev_sta;
	int err = 0;

	fc = ((struct ieee80211_hdr *)skb->data)->frame_control;
	memset(&rx, 0, sizeof(rx));
	rx.skb = skb;
	rx.local = local;

	if (ieee80211_is_data(fc) || ieee80211_is_mgmt(fc))
		local->dot11ReceivedFragmentCount++;

	if (unlikely(test_bit(SCAN_HW_SCANNING, &local->scanning) ||
		     test_bit(SCAN_OFF_CHANNEL, &local->scanning)))
		status->rx_flags |= IEEE80211_RX_IN_SCAN;

	if (ieee80211_is_mgmt(fc))
		err = Sstar_skb_linearize(skb);
	else
		err = !Sstar_pskb_may_pull(skb, ieee80211_hdrlen(fc));

	if (err) {
		Sstar_dev_kfree_skb(skb);
		return;
	}

	hdr = (struct ieee80211_hdr *)skb->data;
	ieee80211_parse_qos(&rx);
	ieee80211_verify_alignment(&rx);

	if (ieee80211_is_data(fc)) {
		prev_sta = NULL;

		for_each_sta_info_rx(local, hdr->addr2, sta, tmp)
		{
			if (!prev_sta) {
				prev_sta = sta;
				continue;
			}

			rx.sta = prev_sta;
			rx.sdata = prev_sta->sdata;
			ieee80211_prepare_and_rx_handle(&rx, skb, false);

			prev_sta = sta;
		}

		if (prev_sta) {
			rx.sta = prev_sta;
			rx.sdata = prev_sta->sdata;

			if (ieee80211_prepare_and_rx_handle(&rx, skb, true))
				return;
			goto out;
		}
	}

	prev = NULL;

	list_for_each_entry_rcu (sdata, &local->interfaces, list) {
		if (!ieee80211_sdata_running(sdata))
			continue;

		if (sdata->vif.type == NL80211_IFTYPE_MONITOR ||
		    sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
			continue;

		/*
		 * frame is destined for this interface, but if it's
		 * not also for the previous one we handle that after
		 * the loop to avoid copying the SKB once too much
		 */

		if (!prev) {
			prev = sdata;
			continue;
		}

		rx.sta = sta_info_get_bss_rx(prev, hdr->addr2);
		rx.sdata = prev;
		ieee80211_prepare_and_rx_handle(&rx, skb, false);

		prev = sdata;
	}

	if (prev) {
		rx.sta = sta_info_get_bss_rx(prev, hdr->addr2);
		rx.sdata = prev;

		if (ieee80211_prepare_and_rx_handle(&rx, skb, true))
			return;
	}

out:
	Sstar_dev_kfree_skb(skb);
}

/*
 * This is the receive path handler. It is called by a low level driver when an
 * 802.11 MPDU is received from the hardware.
 */
void ieee80211_rx(struct ieee80211_hw *hw, struct sk_buff *skb)
{
	struct ieee80211_local *local = hw_to_local(hw);
	struct ieee80211_rate *rate = NULL;
	struct ieee80211_supported_band *sband;
	struct ieee80211_rx_status *status = IEEE80211_SKB_RXCB(skb);

	WARN_ON_ONCE(softirq_count() == 0);

	if (WARN_ON(status->band < 0 || status->band >= IEEE80211_NUM_BANDS))
		goto drop;

	sband = local->hw.wiphy->bands[status->band];
	if (WARN_ON(!sband))
		goto drop;

	/*
	 * If we're suspending, it is possible although not too likely
	 * that we'd be receiving frames after having already partially
	 * quiesced the stack. We can't process such frames then since
	 * that might, for example, cause stations to be added or other
	 * driver callbacks be invoked.
	 */
	if (unlikely(local->quiescing || local->suspended))
		goto drop;

	/*
	 * The same happens when we're not even started,
	 * but that's worth a warning.
	 */
	if (WARN_ON(!local->started))
		goto drop;

	if (likely(!(status->flag & RX_FLAG_FAILED_PLCP_CRC))) {
		/*
		 * Validate the rate, unless a PLCP error means that
		 * we probably can't have a valid rate here anyway.
		 */

		if (status->flag & RX_FLAG_HT) {
			/*
			 * rate_idx is MCS index, which can be [0-76]
			 * as documented on:
			 *
			 * http://wireless.kernel.org/en/developers/Documentation/ieee80211/802.11n
			 *
			 * Anything else would be some sort of driver or
			 * hardware error. The driver should catch hardware
			 * errors.
			 */
			if (WARN((status->rate_idx < 0 ||
				  status->rate_idx > 76),
				 "Ht rate err\n"))
				goto drop;
		} else {
			if (WARN_ON(status->rate_idx < 0 ||
				    status->rate_idx >= sband->n_bitrates))
				goto drop;
			rate = &sband->bitrates[status->rate_idx];
		}
	}

	status->rx_flags = 0;

	/*
	 * key references and virtual interfaces are protected using RCU
	 * and this requires that we are in a read-side RCU section during
	 * receive processing
	 */
	rcu_read_lock();

	/*
	 * Frames with failed FCS/PLCP checksum are not returned,
	 * all other frames are returned without radiotap header
	 * if it was previously present.
	 * Also, frames with less than 16 bytes are dropped.
	 */
	skb = ieee80211_rx_monitor(local, skb, rate);
	if (!skb) {
		rcu_read_unlock();
		return;
	}

	ieee80211_tpt_led_trig_rx(
		local, ((struct ieee80211_hdr *)skb->data)->frame_control,
		skb->len);
	__ieee80211_rx_handle_packet(hw, skb);

	rcu_read_unlock();

	return;
drop:
	Sstar_kfree_skb(skb);
}

/* This is a version of the rx handler that can be called from hard irq
 * context. Post the skb on the queue and schedule the tasklet */
void ieee80211_rx_irqsafe(struct ieee80211_hw *hw, struct sk_buff *skb)
{
	struct ieee80211_local *local = hw_to_local(hw);
	unsigned long flags;
	bool tasklet_running;

	BUILD_BUG_ON(sizeof(struct ieee80211_rx_status) > sizeof(skb->cb));

	skb->pkt_type = IEEE80211_RX_MSG;
	spin_lock_irqsave(&local->skb_queue.lock, flags);
	__Sstar_skb_queue_tail(&local->skb_queue, skb);
	tasklet_running = local->tasklet_running;
	spin_unlock_irqrestore(&local->skb_queue.lock, flags);

	if (tasklet_running == false)
		tasklet_schedule(&local->tasklet);
}
//EXPORT_SYMBOL(ieee80211_rx_irqsafe);
