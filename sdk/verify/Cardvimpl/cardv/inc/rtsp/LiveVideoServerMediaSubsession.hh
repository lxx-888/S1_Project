/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2016 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H264 Elementary Stream video file.
// C++ header

#ifndef _LIVE_VIDEO_SERVER_MEDIA_SUBSESSION_HH
#define _LIVE_VIDEO_SERVER_MEDIA_SUBSESSION_HH

#include "LiveStreamServerMediaSubsession.hh"
#include "mid_VideoEncoder.h"

class LiveVideoServerMediaSubsession : public LiveStreamServerMediaSubsession
{
public:
    static LiveVideoServerMediaSubsession* createNew(UsageEnvironment& env, const int streamId,
                                                     MI_VideoEncoder* videoEncoder);

    // Used to implement "getAuxSDPLine()":
    void checkForAuxSDPLine1();
    void afterPlayingDummy1();

    virtual void noteLiveness();

protected:
    LiveVideoServerMediaSubsession(UsageEnvironment& env, const int streamId, MI_VideoEncoder* videoEncoder);
    // called only by createNew();
    virtual ~LiveVideoServerMediaSubsession();

    void setDoneFlag()
    {
        fDoneFlag = ~0;
    }

protected: // redefined virtual functions
    virtual char const*   getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource);
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
    virtual void          closeStreamSource(FramedSource* inputSource);
    virtual RTPSink*      createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                                           FramedSource* inputSource);
    // virtual void deleteStream(unsigned clientSessionId, void*& streamToken);

private:
    void adjustBitRateByRtd(u_int32_t rtd);

private:
    char*    fAuxSDPLine;
    char     fDoneFlag;     // used when setting up "fAuxSDPLine"
    RTPSink* fDummyRTPSink; // ditto
    Boolean  fEncoderInit;
    int      fDumpFd;
    int      fOrigBitRate;
};

#endif
