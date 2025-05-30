/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/

#define _RTL8723DU_XMIT_C_

#include <rtl8723d_hal.h>

void _dbg_dump_tx_info(PADAPTER padapter, int frame_tag, struct tx_desc *ptxdesc)
{
	u8 bDumpTxPkt;
	u8 bDumpTxDesc = _FALSE;

	rtw_hal_get_def_var(padapter, HAL_DEF_DBG_DUMP_TXPKT, &(bDumpTxPkt));

	if (bDumpTxPkt == 1) { /* dump txdesc for data frame */
		RTW_INFO("dump tx_desc for data frame\n");
		if ((frame_tag & 0x0f) == DATA_FRAMETAG)
			bDumpTxDesc = _TRUE;
	} else if (bDumpTxPkt == 2) { /* dump txdesc for mgnt frame */
		RTW_INFO("dump tx_desc for mgnt frame\n");
		if ((frame_tag & 0x0f) == MGNT_FRAMETAG)
			bDumpTxDesc = _TRUE;
	} else if (bDumpTxPkt == 3) { /* dump early info */
	}

	if (bDumpTxDesc) {
		RTW_INFO("=====================================\n");
		RTW_INFO("Offset00(0x%08x)\n", ptxdesc->txdw0);
		RTW_INFO("Offset04(0x%08x)\n", ptxdesc->txdw1);
		RTW_INFO("Offset08(0x%08x)\n", ptxdesc->txdw2);
		RTW_INFO("Offset12(0x%08x)\n", ptxdesc->txdw3);
		RTW_INFO("Offset16(0x%08x)\n", ptxdesc->txdw4);
		RTW_INFO("Offset20(0x%08x)\n", ptxdesc->txdw5);
		RTW_INFO("Offset24(0x%08x)\n", ptxdesc->txdw6);
		RTW_INFO("Offset28(0x%08x)\n", ptxdesc->txdw7);
#if defined(TXDESC_40_BYTES) || defined(TXDESC_64_BYTES)
		RTW_INFO("Offset32(0x%08x)\n", ptxdesc->txdw8);
		RTW_INFO("Offset36(0x%08x)\n", ptxdesc->txdw9);
#endif
#if defined(TXDESC_64_BYTES)
		RTW_INFO("Offset40(0x%08x)\n", ptxdesc->txdw10);
		RTW_INFO("Offset44(0x%08x)\n", ptxdesc->txdw11);
		RTW_INFO("Offset48(0x%08x)\n", ptxdesc->txdw12);
		RTW_INFO("Offset52(0x%08x)\n", ptxdesc->txdw13);
		RTW_INFO("Offset56(0x%08x)\n", ptxdesc->txdw14);
		RTW_INFO("Offset58(0x%08x)\n", ptxdesc->txdw15);
#endif
		RTW_INFO("=====================================\n");
	}
}

s32 rtl8723du_init_xmit_priv(PADAPTER padapter)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
#ifdef PLATFORM_LINUX
    unsigned long adapter_addr = 0;
    adapter_addr = (unsigned long)padapter;
	tasklet_init(&pxmitpriv->xmit_tasklet,
		     rtl8723du_xmit_tasklet,
		     adapter_addr);
#endif
	return _SUCCESS;
}

void rtl8723du_free_xmit_priv(PADAPTER padapter)
{
}

int urb_zero_packet_chk(PADAPTER padapter, int sz)
{
	u8 blnSetTxDescOffset;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);

	blnSetTxDescOffset = (((sz + TXDESC_SIZE) % pHalData->UsbBulkOutSize) == 0) ? 1 : 0;

	return blnSetTxDescOffset;
}

static s32 update_txdesc(struct xmit_frame *pxmitframe, u8 *pmem, s32 sz, u8 bagg_pkt)
{
	int pull = 0;

	PADAPTER padapter = pxmitframe->padapter;
	struct tx_desc *ptxdesc = (struct tx_desc *)pmem;

#ifndef CONFIG_USE_USB_BUFFER_ALLOC_TX
	if ((PACKET_OFFSET_SZ != 0)
	    && (_FALSE == bagg_pkt)
	    && (urb_zero_packet_chk(padapter, sz) == 0)) {
		ptxdesc = (struct tx_desc *)(pmem + PACKET_OFFSET_SZ);
		pull = 1;
		pxmitframe->pkt_offset--;
	}
#endif	/* CONFIG_USE_USB_BUFFER_ALLOC_TX */

	_rtw_memset(ptxdesc, 0, sizeof(struct tx_desc));

	rtl8723d_update_txdesc(pxmitframe, (u8 *)ptxdesc);
	_dbg_dump_tx_info(padapter, pxmitframe->frame_tag, ptxdesc);
	return pull;
}

#ifdef CONFIG_XMIT_THREAD_MODE
/*
 * Description
 *	Transmit xmitbuf to hardware tx fifo
 *
 * Return
 *	_SUCCESS	ok
 *	_FAIL		something error
 */
s32 rtl8723du_xmit_buf_handler(PADAPTER padapter)
{
	/* PHAL_DATA_TYPE phal; */
	struct xmit_priv *pxmitpriv;
	struct xmit_buf *pxmitbuf;
	struct xmit_frame *pxmitframe;
	s32 ret;


	/* phal = GET_HAL_DATA(padapter); */
	pxmitpriv = &padapter->xmitpriv;

	ret = _rtw_down_sema(&pxmitpriv->xmit_sema);
	if (_FAIL == ret) {
		return _FAIL;
	}

	if (RTW_CANNOT_RUN(padapter)) {
		RTW_DBG(FUNC_ADPT_FMT "- bDriverStopped(%s) bSurpriseRemoved(%s)\n",
			FUNC_ADPT_ARG(padapter),
			rtw_is_drv_stopped(padapter) ? "True" : "False",
			rtw_is_surprise_removed(padapter) ? "True" : "False");
		return _FAIL;
	}

	if (check_pending_xmitbuf(pxmitpriv) == _FALSE)
		return _SUCCESS;

#ifdef CONFIG_LPS_LCLK
	ret = rtw_register_tx_alive(padapter);
	if (ret != _SUCCESS) {
		return _SUCCESS;
	}
#endif

	do {
		pxmitbuf = dequeue_pending_xmitbuf(pxmitpriv);
		if (pxmitbuf == NULL)
			break;
		pxmitframe = (struct xmit_frame *) pxmitbuf->priv_data;
		rtw_write_port(padapter, pxmitbuf->ff_hwaddr, pxmitbuf->len, (unsigned char *)pxmitbuf);
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

	} while (1);

#ifdef CONFIG_LPS_LCLK
	rtw_unregister_tx_alive(padapter);
#endif

	return _SUCCESS;
}
#endif


static s32 rtw_dump_xframe(PADAPTER padapter, struct xmit_frame *pxmitframe)
{
	s32 ret = _SUCCESS;
	s32 inner_ret = _SUCCESS;
	int t, sz, w_sz, pull = 0;
	u8 *mem_addr;
	u32 ff_hwaddr;
	struct xmit_buf *pxmitbuf = pxmitframe->pxmitbuf;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
#ifdef CONFIG_80211N_HT
	if ((pxmitframe->frame_tag == DATA_FRAMETAG) &&
	    (pxmitframe->attrib.ether_type != 0x0806) &&
	    (pxmitframe->attrib.ether_type != 0x888e) &&
	    (pxmitframe->attrib.dhcp_pkt != 1))
		rtw_issue_addbareq_cmd(padapter, pxmitframe, _FALSE);
#endif /* CONFIG_80211N_HT */
	mem_addr = pxmitframe->buf_addr;


	for (t = 0; t < pattrib->nr_frags; t++) {
		if (inner_ret != _SUCCESS && ret == _SUCCESS)
			ret = _FAIL;

		if (t != (pattrib->nr_frags - 1)) {

			sz = pxmitpriv->frag_len;
			sz = sz - 4 - (psecuritypriv->sw_encrypt ? 0 : pattrib->icv_len);
		} else /* no frag */
			sz = pattrib->last_txcmdsz;

		pull = update_txdesc(pxmitframe, mem_addr, sz, _FALSE);
		/* rtl8723d_update_txdesc(pxmitframe, mem_addr+PACKET_OFFSET_SZ); */

		if (pull) {
			mem_addr += PACKET_OFFSET_SZ; /* pull txdesc head */

			/* pxmitbuf->pbuf = mem_addr; */
			pxmitframe->buf_addr = mem_addr;

			w_sz = sz + TXDESC_SIZE;
		} else
			w_sz = sz + TXDESC_SIZE + PACKET_OFFSET_SZ;

		ff_hwaddr = rtw_get_ff_hwaddr(pxmitframe);
#ifdef CONFIG_XMIT_THREAD_MODE
		pxmitbuf->len = w_sz;
		pxmitbuf->ff_hwaddr = ff_hwaddr;

		if (pxmitframe->attrib.qsel == QSLT_BEACON)
			/* download rsvd page*/
			rtw_write_port(padapter, ff_hwaddr, w_sz, (u8 *)pxmitbuf);
		else
			enqueue_pending_xmitbuf(pxmitpriv, pxmitbuf);		

#else
		inner_ret = rtw_write_port(padapter, ff_hwaddr, w_sz, (unsigned char *)pxmitbuf);
#endif
		rtw_count_tx_stats(padapter, pxmitframe, sz);


		/* RTW_INFO("rtw_write_port, w_sz=%d, sz=%d, txdesc_sz=%d, tid=%d\n", w_sz, sz, w_sz-sz, pattrib->priority); */

		mem_addr += w_sz;

		mem_addr = (u8 *)RND4(((SIZE_PTR)(mem_addr)));

	}

#ifdef CONFIG_XMIT_THREAD_MODE
	if (pxmitframe->attrib.qsel == QSLT_BEACON)
#endif
	rtw_free_xmitframe(pxmitpriv, pxmitframe);

	if (ret != _SUCCESS)
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_UNKNOWN);

	return ret;
}

#ifdef CONFIG_USB_TX_AGGREGATION
#define IDEA_CONDITION 1	/* check all packets before enqueue */
s32 rtl8723du_xmitframe_complete(PADAPTER padapter, struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);
	struct xmit_frame *pxmitframe = NULL;
	struct xmit_frame *pfirstframe = NULL;

	/* aggregate variable */
	struct hw_xmit *phwxmit;
	struct sta_info *psta = NULL;
	struct tx_servq *ptxservq = NULL;

	_irqL irqL;
	_list *xmitframe_plist = NULL, *xmitframe_phead = NULL;

	u32 pbuf;       /* next pkt address */
	u32 pbuf_tail;  /* last pkt tail */
	u32 len;        /* packet length, except TXDESC_SIZE and PKT_OFFSET */

	u32 bulkSize = pHalData->UsbBulkOutSize;
	u8 descCount;
	u32 bulkPtr;

	/* dump frame variable */
	u32 ff_hwaddr;

	_list *sta_plist, *sta_phead;
	u8 single_sta_in_queue = _FALSE;

#ifndef IDEA_CONDITION
	int res = _SUCCESS;
#endif

#ifdef CONFIG_RTW_MGMT_QUEUE
	/* dump management frame directly */
	pxmitframe = rtw_dequeue_mgmt_xframe(pxmitpriv);
	if (pxmitframe) {
		rtw_dump_xframe(padapter, pxmitframe);
		return _TRUE;
	}
#endif

	/* check xmitbuffer is ok */
	if (pxmitbuf == NULL) {
		pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
		if (pxmitbuf == NULL)
			return _FALSE;
	}


	/* 3 1. pick up first frame */
	do {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		pxmitframe = rtw_dequeue_xframe(pxmitpriv, pxmitpriv->hwxmits, pxmitpriv->hwxmit_entry);
		if (pxmitframe == NULL) {
			/* no more xmit frame, release xmit buffer */
			rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
			return _FALSE;
		}


#ifndef IDEA_CONDITION
		if (pxmitframe->frame_tag != DATA_FRAMETAG) {
			/* rtw_free_xmitframe(pxmitpriv, pxmitframe); */
			continue;
		}

		/* TID 0~15 */
		if ((pxmitframe->attrib.priority < 0) ||
		    (pxmitframe->attrib.priority > 15)) {
			/* rtw_free_xmitframe(pxmitpriv, pxmitframe); */
			continue;
		}
#endif

		pxmitframe->pxmitbuf = pxmitbuf;
		pxmitframe->buf_addr = pxmitbuf->pbuf;
		pxmitbuf->priv_data = pxmitframe;

		/* pxmitframe->agg_num = 1; */ /* alloc xmitframe should assign to 1. */
		/* pxmitframe->pkt_offset = 1; */ /* first frame of aggregation, reserve offset */
		pxmitframe->pkt_offset = (PACKET_OFFSET_SZ / 8);

		if (rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe) == _FALSE) {
			RTW_INFO("%s coalesce 1st xmitframe failed\n", __func__);
			continue;
		}


		/* always return ndis_packet after rtw_xmitframe_coalesce */
		rtw_os_xmit_complete(padapter, pxmitframe);

		break;
	} while (1);

	/* 3 2. aggregate same priority and same DA(AP or STA) frames */
	pfirstframe = pxmitframe;
	len = rtw_wlan_pkt_size(pfirstframe) + TXDESC_OFFSET;
	pbuf_tail = len;
	pbuf = _RND8(pbuf_tail);

	/* check pkt amount in one bluk */
	descCount = 0;
	bulkPtr = bulkSize;
	if (pbuf < bulkPtr)
		descCount++;
	else {
		descCount = 0;
		bulkPtr = ((pbuf / bulkSize) + 1) * bulkSize; /* round to next bulkSize */
	}

	/* dequeue same priority packet from station tx queue */
	psta = pfirstframe->attrib.psta;
	switch (pfirstframe->attrib.priority) {
	case 1:
	case 2:
		ptxservq = &(psta->sta_xmitpriv.bk_q);
		phwxmit = pxmitpriv->hwxmits + 3;
		break;

	case 4:
	case 5:
		ptxservq = &(psta->sta_xmitpriv.vi_q);
		phwxmit = pxmitpriv->hwxmits + 1;
		break;

	case 6:
	case 7:
		ptxservq = &(psta->sta_xmitpriv.vo_q);
		phwxmit = pxmitpriv->hwxmits;
		break;

	case 0:
	case 3:
	default:
		ptxservq = &(psta->sta_xmitpriv.be_q);
		phwxmit = pxmitpriv->hwxmits + 2;
		break;
	}

	_enter_critical_bh(&pxmitpriv->lock, &irqL);

	sta_phead = get_list_head(phwxmit->sta_queue);
	sta_plist = get_next(sta_phead);
	single_sta_in_queue = rtw_end_of_queue_search(sta_phead, get_next(sta_plist));

	xmitframe_phead = get_list_head(&ptxservq->sta_pending);
	xmitframe_plist = get_next(xmitframe_phead);
	while (rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist) == _FALSE) {
		pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);
		xmitframe_plist = get_next(xmitframe_plist);

		if (_FAIL == rtw_hal_busagg_qsel_check(padapter, pfirstframe->attrib.qsel, pxmitframe->attrib.qsel))
			break;

		len = rtw_wlan_pkt_size(pxmitframe) + TXDESC_SIZE; /* no offset */
		if (pbuf + len > MAX_XMITBUF_SZ)
			break;

		rtw_list_delete(&pxmitframe->list);
		ptxservq->qcnt--;
		phwxmit->accnt--;

#ifndef IDEA_CONDITION
		/* suppose only data frames would be in queue */
		if (pxmitframe->frame_tag != DATA_FRAMETAG) {
			rtw_free_xmitframe(pxmitpriv, pxmitframe);
			continue;
		}

		/* TID 0~15 */
		if ((pxmitframe->attrib.priority < 0) ||
		    (pxmitframe->attrib.priority > 15)) {
			rtw_free_xmitframe(pxmitpriv, pxmitframe);
			continue;
		}
#endif

		/* pxmitframe->pxmitbuf = pxmitbuf; */
		pxmitframe->buf_addr = pxmitbuf->pbuf + pbuf;

		pxmitframe->agg_num = 0; /* not first frame of aggregation */
		pxmitframe->pkt_offset = 0; /* not first frame of aggregation, no need to reserve offset */

		if (rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe) == _FALSE) {
			RTW_INFO("%s coalesce failed\n", __func__);
			rtw_free_xmitframe(pxmitpriv, pxmitframe);
			continue;
		}


		/* always return ndis_packet after rtw_xmitframe_coalesce */
		rtw_os_xmit_complete(padapter, pxmitframe);

		/* (len - TXDESC_SIZE) == pxmitframe->attrib.last_txcmdsz */
		update_txdesc(pxmitframe, pxmitframe->buf_addr, pxmitframe->attrib.last_txcmdsz, _TRUE);

		/* don't need xmitframe any more */
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		/* handle pointer and stop condition */
		pbuf_tail = pbuf + len;
		pbuf = _RND8(pbuf_tail);

		pfirstframe->agg_num++;
		if (MAX_TX_AGG_PACKET_NUMBER == pfirstframe->agg_num)
			break;

		if (pbuf < bulkPtr) {
			descCount++;
			if (descCount == pHalData->UsbTxAggDescNum)
				break;
		} else {
			descCount = 0;
			bulkPtr = ((pbuf / bulkSize) + 1) * bulkSize;
		}
	}
	if (_rtw_queue_empty(&ptxservq->sta_pending) == _TRUE)
		rtw_list_delete(&ptxservq->tx_pending);
	else if (single_sta_in_queue == _FALSE) {
		/* Re-arrange the order of stations in this ac queue to balance the service for these stations */
		rtw_list_delete(&ptxservq->tx_pending);
		rtw_list_insert_tail(&ptxservq->tx_pending, get_list_head(phwxmit->sta_queue));
	}

	_exit_critical_bh(&pxmitpriv->lock, &irqL);
#ifdef CONFIG_80211N_HT
	if ((pfirstframe->attrib.ether_type != 0x0806) &&
	    (pfirstframe->attrib.ether_type != 0x888e) &&
	    (pfirstframe->attrib.dhcp_pkt != 1))
		rtw_issue_addbareq_cmd(padapter, pfirstframe, _FALSE);
#endif /* CONFIG_80211N_HT */
#ifndef CONFIG_USE_USB_BUFFER_ALLOC_TX
	/* 3 3. update first frame txdesc */
	if ((PACKET_OFFSET_SZ != 0)
	    && (pbuf_tail % bulkSize) == 0) {
		/* remove pkt_offset */
		pbuf_tail -= PACKET_OFFSET_SZ;
		pfirstframe->buf_addr += PACKET_OFFSET_SZ;
		pfirstframe->pkt_offset = 0;
	}
#endif	/* CONFIG_USE_USB_BUFFER_ALLOC_TX */
	update_txdesc(pfirstframe, pfirstframe->buf_addr, pfirstframe->attrib.last_txcmdsz, _TRUE);

	/* 3 4. write xmit buffer to USB FIFO */
	ff_hwaddr = rtw_get_ff_hwaddr(pfirstframe);

	/* xmit address == ((xmit_frame*)pxmitbuf->priv_data)->buf_addr */
#ifdef CONFIG_XMIT_THREAD_MODE
	pxmitbuf->len = pbuf_tail;
	pxmitbuf->ff_hwaddr = ff_hwaddr;

	if (pfirstframe->attrib.qsel == QSLT_BEACON)
		/* download rsvd page*/
		rtw_write_port(padapter, ff_hwaddr, pbuf_tail, (u8 *)pxmitbuf);
	else
		enqueue_pending_xmitbuf(pxmitpriv, pxmitbuf);
#else
	rtw_write_port(padapter, ff_hwaddr, pbuf_tail, (u8 *)pxmitbuf);
#endif



	/* 3 5. update statisitc */
	pbuf_tail -= (pfirstframe->agg_num * TXDESC_SIZE);
	if (pfirstframe->pkt_offset == 1)
		pbuf_tail -= PACKET_OFFSET_SZ;

	rtw_count_tx_stats(padapter, pfirstframe, pbuf_tail);

#ifdef CONFIG_XMIT_THREAD_MODE
	if (pfirstframe->attrib.qsel == QSLT_BEACON)
#endif
	rtw_free_xmitframe(pxmitpriv, pfirstframe);

	return _TRUE;
}

#else

s32 rtl8723du_xmitframe_complete(PADAPTER padapter, struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{

	struct hw_xmit *phwxmits;
	sint hwentry;
	struct xmit_frame *pxmitframe = NULL;
	int res = _SUCCESS, xcnt = 0;

	phwxmits = pxmitpriv->hwxmits;
	hwentry = pxmitpriv->hwxmit_entry;

#ifdef CONFIG_RTW_MGMT_QUEUE
	/* dump management frame directly */
	pxmitframe = rtw_dequeue_mgmt_xframe(pxmitpriv);
	if (pxmitframe) {
		rtw_dump_xframe(padapter, pxmitframe);
		return _TRUE;
	}
#endif

	if (pxmitbuf == NULL) {
		pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
		if (!pxmitbuf)
			return _FALSE;
	}


	do {
		pxmitframe = rtw_dequeue_xframe(pxmitpriv, phwxmits, hwentry);

		if (pxmitframe) {
			pxmitframe->pxmitbuf = pxmitbuf;

			pxmitframe->buf_addr = pxmitbuf->pbuf;

			pxmitbuf->priv_data = pxmitframe;

			if ((pxmitframe->frame_tag & 0x0f) == DATA_FRAMETAG) {
				if (pxmitframe->attrib.priority <= 15) /* TID0~15 */
					res = rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);

				rtw_os_xmit_complete(padapter, pxmitframe); /* always return ndis_packet after rtw_xmitframe_coalesce */
			}




			if (res == _SUCCESS)
				rtw_dump_xframe(padapter, pxmitframe);
			else {
				rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
				rtw_free_xmitframe(pxmitpriv, pxmitframe);
			}

			xcnt++;

		} else {
			rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
			return _FALSE;
		}

		break;

	} while (0/*xcnt < (NR_XMITFRAME >> 3)*/);

	return _TRUE;

}
#endif



static s32 xmitframe_direct(PADAPTER padapter, struct xmit_frame *pxmitframe)
{
	s32 res = _SUCCESS;


	res = rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
	if (res == _SUCCESS)
		rtw_dump_xframe(padapter, pxmitframe);

	return res;
}

/*
 * Return
 *	_TRUE	dump packet directly
 *	_FALSE	enqueue packet
 */
static s32 pre_xmitframe(PADAPTER padapter, struct xmit_frame *pxmitframe)
{
	_irqL irqL;
	s32 res;
	struct xmit_buf *pxmitbuf = NULL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	_enter_critical_bh(&pxmitpriv->lock, &irqL);

	if (rtw_txframes_sta_ac_pending(padapter, pattrib) > 0)
		goto enqueue;

	if (rtw_xmit_ac_blocked(padapter) == _TRUE)
		goto enqueue;

	if (DEV_STA_LG_NUM(padapter->dvobj))
		goto enqueue;

	pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
	if (pxmitbuf == NULL)
		goto enqueue;

	_exit_critical_bh(&pxmitpriv->lock, &irqL);

	pxmitframe->pxmitbuf = pxmitbuf;
	pxmitframe->buf_addr = pxmitbuf->pbuf;
	pxmitbuf->priv_data = pxmitframe;

	if (xmitframe_direct(padapter, pxmitframe) != _SUCCESS) {
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
		rtw_free_xmitframe(pxmitpriv, pxmitframe);
	}

	return _TRUE;

enqueue:
	res = rtw_xmitframe_enqueue(padapter, pxmitframe);
	_exit_critical_bh(&pxmitpriv->lock, &irqL);

	if (res != _SUCCESS) {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		pxmitpriv->tx_drop++;
		return _TRUE;
	}

	return _FALSE;
}

s32 rtl8723du_mgnt_xmit(PADAPTER padapter, struct xmit_frame *pmgntframe)
{
	return rtw_dump_xframe(padapter, pmgntframe);
}

/*
 * Return
 *	_TRUE	dump packet directly ok
 *	_FALSE	temporary can't transmit packets to hardware
 */
s32 rtl8723du_hal_xmit(PADAPTER padapter, struct xmit_frame *pxmitframe)
{
	return pre_xmitframe(padapter, pxmitframe);
}

#ifdef CONFIG_RTW_MGMT_QUEUE
s32 rtl8723du_hal_mgmt_xmitframe_enqueue(PADAPTER padapter, struct xmit_frame *pxmitframe)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	s32 err;

	err = rtw_mgmt_xmitframe_enqueue(padapter, pxmitframe);
	if (err != _SUCCESS) {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);
		pxmitpriv->tx_drop++;
	} else {
#ifdef PLATFORM_LINUX
		tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
#endif
	}
	return err;
}
#endif

s32 rtl8723du_hal_xmitframe_enqueue(PADAPTER padapter, struct xmit_frame *pxmitframe)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	s32 err;

	err = rtw_xmitframe_enqueue(padapter, pxmitframe);
	if (err != _SUCCESS) {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		pxmitpriv->tx_drop++;
	} else {
#ifdef PLATFORM_LINUX
		tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
#endif
	}

	return err;

}

void rtl8723d_cal_txdesc_chksum(struct tx_desc *ptxdesc)
{
	u16 *usPtr = (u16 *)ptxdesc;
	u32 count;
	u32 index;
	u16 checksum = 0;


	/* Clear first */
	ptxdesc->txdw7 &= cpu_to_le32(0xffff0000);

	/*
	 * checksume is always calculated by first 32 bytes,
	 * and it doesn't depend on TX DESC length.
	 * Thomas,Lucas@SD4,20130515
	 */
	count = 16;
	for (index = 0; index < count; index++)
		checksum ^= le16_to_cpu(*(usPtr + index));

	/* avoid zero checksum make tx hang */
	checksum = ~checksum;

	ptxdesc->txdw7 |= cpu_to_le32(checksum & 0x0000ffff);
}

#ifdef CONFIG_HOSTAPD_MLME

static void rtl8723du_hostap_mgnt_xmit_cb(struct urb *urb)
{
#ifdef PLATFORM_LINUX
	struct sk_buff *skb = (struct sk_buff *)urb->context;

	/* RTW_INFO("%s\n", __func__); */

	rtw_skb_free(skb);
#endif
}

s32 rtl8723du_hostap_mgnt_xmit_entry(PADAPTER padapter, _pkt *pkt)
{
#ifdef PLATFORM_LINUX
	u16 fc;
	int rc, len, pipe;
	unsigned int bmcst, tid, qsel;
	struct sk_buff *skb, *pxmit_skb;
	struct urb *urb;
	unsigned char *pxmitbuf;
	struct tx_desc *ptxdesc;
	struct ieee80211_hdr *tx_hdr;
	struct hostapd_priv *phostapdpriv = padapter->phostapdpriv;
	struct net_device *pnetdev = padapter->pnetdev;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);
	struct dvobj_priv *pdvobj = adapter_to_dvobj(padapter);


	/* RTW_INFO("%s\n", __func__); */

	skb = pkt;

	len = skb->len;
	tx_hdr = (struct ieee80211_hdr *)(skb->data);
	fc = le16_to_cpu(tx_hdr->frame_ctl);
	bmcst = IS_MCAST(tx_hdr->addr1);

	if ((fc & IEEE80211_FCTL_FTYPE) != IEEE80211_FTYPE_MGMT)
		goto _exit;

	pxmit_skb = rtw_skb_alloc(len + TXDESC_SIZE);

	if (!pxmit_skb)
		goto _exit;

	pxmitbuf = pxmit_skb->data;

	urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!urb)
		goto _exit;

	/* ----- fill tx desc ----- */
	ptxdesc = (struct tx_desc *)pxmitbuf;
	_rtw_memset(ptxdesc, 0, sizeof(*ptxdesc));

	/* offset 0 */
	ptxdesc->txdw0 |= cpu_to_le32(len & 0x0000ffff);
	ptxdesc->txdw0 |= cpu_to_le32(((TXDESC_SIZE + OFFSET_SZ) << OFFSET_SHT) & 0x00ff0000); /*default = 32 bytes for TX Desc */
	ptxdesc->txdw0 |= cpu_to_le32(OWN | FSG | LSG);

	if (bmcst)
		ptxdesc->txdw0 |= cpu_to_le32(BIT(24));

	/* offset 4 */
	ptxdesc->txdw1 |= cpu_to_le32(0x00); /* MAC_ID */

	ptxdesc->txdw1 |= cpu_to_le32((0x12 << QSEL_SHT) & 0x00001f00);

	ptxdesc->txdw1 |= cpu_to_le32((0x06 << 16) & 0x000f0000); /* b mode */

	/* offset 8 */

	/* offset 12 */
	ptxdesc->txdw3 |= cpu_to_le32((le16_to_cpu(tx_hdr->seq_ctl) << 16) & 0xffff0000);

	/* offset 16 */
	ptxdesc->txdw4 |= cpu_to_le32(BIT(8)); /* driver uses rate */

	/* offset 20 */


	/* HW append seq */
	ptxdesc->txdw4 |= cpu_to_le32(BIT(7)); /* Hw set sequence number */
	ptxdesc->txdw3 |= cpu_to_le32((8 << 28)); /* set bit3 to 1. Suugested by TimChen. 2009.12.29. */


	rtl8723d_cal_txdesc_chksum(ptxdesc);
	/* ----- end of fill tx desc ----- */

	skb_put(pxmit_skb, len + TXDESC_SIZE);
	pxmitbuf = pxmitbuf + TXDESC_SIZE;
	_rtw_memcpy(pxmitbuf, skb->data, len);

	/* RTW_INFO("mgnt_xmit, len=%x\n", pxmit_skb->len); */


	/* ----- prepare urb for submit ----- */

	/* translate DMA FIFO addr to pipehandle */
	/* pipe = ffaddr2pipehdl(pdvobj, MGT_QUEUE_INX); */
	pipe = usb_sndbulkpipe(pdvobj->pusbdev, pHalData->Queue2EPNum[(u8)MGT_QUEUE_INX] & 0x0f);

	usb_fill_bulk_urb(urb, pdvobj->pusbdev, pipe,
		pxmit_skb->data, pxmit_skb->len, rtl8723du_hostap_mgnt_xmit_cb, pxmit_skb);

	urb->transfer_flags |= URB_ZERO_PACKET;
	usb_anchor_urb(urb, &phostapdpriv->anchored);
	rc = usb_submit_urb(urb, GFP_ATOMIC);
	if (rc < 0) {
		usb_unanchor_urb(urb);
		kfree_skb(skb);
	}
	usb_free_urb(urb);


_exit:

	rtw_skb_free(skb);

#endif

	return 0;

}
#endif
