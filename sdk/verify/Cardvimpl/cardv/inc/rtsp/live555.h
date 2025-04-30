/*
 * live555.h- Sigmastar
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 *
 * Author: jeff.li <jeff.li@sigmastar.com.cn>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef _LIVE555_H__
#define _LIVE555_H__

#pragma once
#include <BasicUsageEnvironment.hh>
#include "liveMedia.hh"
#include "mid_VideoEncoder.h"
#include "mid_AudioEncoder.h"

#define MAX_SERRVER_MEDIA_SESSION (3)
#define LIVE555_SUPPORT_H264_H265 (1)
#define LIVE555_SUPPORT_MJPEG     (1)
#define LIVE555_DUMP_STREAM       (0)

class live555
{
public:
    static live555* createNew();
    ~live555();
    int        startLive555Server();
    int        stopLive555Server();
    int        createServerMediaSession(MI_VideoEncoder* videoEncoder, MI_AudioEncoder* audioEncoder, char* streamName,
                                        int streamId);
    void       closeServerMediaSession(int smsIdx);
    static int isRunning()
    {
        return fIsRunning;
    };
    static void setRunning(int isRunning)
    {
        fIsRunning = isRunning;
    };

    char m_watchVariable;
    int  m_liveState;

    TaskScheduler*              m_scheduler;
    UsageEnvironment*           m_env;
    UserAuthenticationDatabase* m_authDB;
    RTSPServer*                 m_rtspServer;

    ServerMediaSession* m_sms[MAX_SERRVER_MEDIA_SESSION];
    int                 m_smsCount;

    static int fRtspServerPortNum;
    static int fIsRunning;

private:
    live555();
};

#endif //_LIVE555_H__