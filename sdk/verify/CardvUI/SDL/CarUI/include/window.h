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

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <pthread.h>
#include "SDL.h"

class Window
{
public:
    Window(const char *title, int x, int y, int w, int h, Uint32 color);
    virtual ~Window();

    SDL_Renderer *getRender();
    SDL_Color     getColor();

    void Lock();
    void UnLock();

private:
    SDL_Window *    m_pWindow;
    SDL_Renderer *  m_pRenderer;
    SDL_Rect        m_Rect;
    SDL_Color       m_Color;
    pthread_mutex_t m_Mutex;
};

#endif