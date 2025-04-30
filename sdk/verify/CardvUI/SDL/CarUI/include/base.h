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

#ifndef _BASE_H_
#define _BASE_H_

#include "SDL.h"
#include "window.h"
#include "common.h"

class Base
{
public:
    Base(Window *pWindow, int x, int y, int w, int h);
    virtual ~Base();

    virtual void setVisible(bool isVisible);
    bool isVisible();
    void setBackgroundPic(const char *pPicPath);
    void setBackgroundColor(uint32_t color);
    void setPosition(int x, int y, int w, int h);
    void getPosition(int *x, int *y, int *w, int *h);

protected:
    Window *      m_pWindow;
    SDL_Renderer *m_pRenderer;
    SDL_Texture * m_pTexture;
    SDL_Rect      m_Rect;
    SDL_Color     m_BgColor;
    bool          m_Visable;
};

#endif