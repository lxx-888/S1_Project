/*
 * drv_audio_api.h - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#ifndef __DRV_AUDIO_API_H__
#define __DRV_AUDIO_API_H__

typedef int (*DMA_STOP_CHANNEL_CB_FN)(CHIP_AIO_DMA_e);

extern int  DrvAudApiIrqInit(void);
extern void DrvAudApiIrqSetDmaStopCb(DMA_STOP_CHANNEL_CB_FN pfnCb);
extern int  DrvAudApiIrqRequest(const char *name, void *dev_id);
extern BOOL DrvAudApiIrqFree(void *dev_id);
extern u64  DrvAudApiIoAddress(u32 addr);

#endif /* __DRV_AUDIO_API_H__ */
