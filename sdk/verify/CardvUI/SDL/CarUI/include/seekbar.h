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

#ifndef _SEEKBAR_H_
#define _SEEKBAR_H_

#include "SDL.h"
#include "window.h"
#include "base.h"

class SeekBar : public Base
{
public:
    SeekBar(Window *pWindow, int x, int y, int w, int h);
    virtual ~SeekBar();

    void setMax(int max);
    void setProgress(int progress);
    int          getMax();
    int          getProgress();
    virtual void setVisible(bool isVisible);

private:
    int       m_Max;
    int       m_Progress;
    bool      m_Horizon;
    SDL_Color m_ProgressColor;
    SDL_Color m_LeftColor;
};

#endif