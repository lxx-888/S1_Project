/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef __G711_H_
#define __G711_H_

#ifdef __cplusplus
extern "C" {
#endif

void G711Encoder(short *pcm, unsigned char *code, int size, int lawflag);
void G711Decoder(short *pcm, unsigned char *code, int size, int lawflag);
void G711Covert(unsigned char *dst, unsigned char *src, int size, int flag);

#ifdef __cplusplus
}
#endif

#endif //__G711_H_
