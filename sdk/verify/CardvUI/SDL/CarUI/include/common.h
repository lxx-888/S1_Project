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

#ifndef _COMMON_H_
#define _COMMON_H_

#define COLOR_MAP(u32Color, SDL_Color)               \
    {                                                \
        SDL_Color.r = u32Color & 0x000000FF;         \
        SDL_Color.g = (u32Color & 0x0000FF00) >> 8;  \
        SDL_Color.b = (u32Color & 0x00FF0000) >> 16; \
        SDL_Color.a = (u32Color & 0xFF000000) >> 24; \
    }

int  CarUI_Init(void);
void CarUI_DeInit(void);

#endif