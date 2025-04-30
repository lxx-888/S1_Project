#ifndef _LIVE_AUDIO_SOURCE_HH
#define _LIVE_AUDIO_SOURCE_HH

#include "MediaSource.hh"
#include "FramedSource.hh"
#include "ms_notify.h"

class LiveAudioSource : public FramedSource
{
public:
    static LiveAudioSource* createNew(UsageEnvironment& env, const int streamId);

private:
    LiveAudioSource(UsageEnvironment& env, const int streamId);
    virtual ~LiveAudioSource();
    virtual void doGetNextFrame();
    static void  incomingDataHandler(void* clientData);
    void         incomingDataHandler1();

private:
    int                fNotifyFd;
    char*              fIpcMem;
    long               fFrameNum;
    int                fStreamId;
    unsigned long long fLastFrameTimestamp;
};

#endif
