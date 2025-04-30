#if (defined ALINK)
#include "alink.h"

void AlinkDemo::start()
{
    mEventListener = std::make_shared<EventListener>(*this);
    mVideoListener = std::make_shared<VideoListener>(*this);

    mServer->registerChannelListener(alink::kEvent, mEventListener.get());
    mServer->registerChannelListener(alink::kVideoOut, mVideoListener.get());

    mServer->start();
}

void AlinkDemo::stop()
{
    mServer->stop();
}
#endif