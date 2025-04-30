#ifndef _LIVE_VIDEO_SOURCE_HH
#define _LIVE_VIDEO_SOURCE_HH

#include "MediaSource.hh"
#include "FramedSource.hh"
#include "mid_VideoEncoder.h"
#include "live555.h"

class LiveVideoSource : public FramedSource
{
public:
    static LiveVideoSource *createNew(UsageEnvironment &env, const int streamId, MI_VideoEncoder *videoEncoder, int fd);

protected:
    LiveVideoSource(UsageEnvironment &env, const int streamId, MI_VideoEncoder *videoEncoder, int fd);
    virtual ~LiveVideoSource();

protected:
    int              fNotifyFd;
    int              fDumpFd;
    char *           fFrameEnd;
    char *           fNextFrameToStream;
    long             fFrameBytesToStream;
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t   stPack[STREAM_PACK_CNT];

private:
    virtual void doGetNextFrame();
    static void  incomingDataHandler(void *clientData);
    void         incomingDataHandler1();
#if (LIVE555_SUPPORT_H264_H265)
    int findH264NalUnit(const char *frameToStream, const int frameBytesToStream, char **naluToStream,
                        unsigned int *naluBytesToStream);
    int findH265NalUnit(const char *frameToStream, const int frameBytesToStream, char **naluToStream,
                        unsigned int *naluBytesToStream);
#endif

private:
    int                fHaveSeenIdrFrame;
    MI_VideoEncoder *  fVideoEncoder;
    unsigned long long fLastFrameTimestamp;
    int                streamCnt;
};

#endif
