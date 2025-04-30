///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IAbstractDataSourceListener.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \class IVideoOutListener
/// \brief Listener for video source
///
/// Video is configured with format and resolution. As H.264 is the only format
/// supported, the configuration message in JSON format may like this,
/// {
///     "video_format": "H.264",
///     "width": 1024,
///     "height": 600
/// }
/// The video source as the listener shall respond the configuration message
/// in JSON format also. If the resolution configuration can be accepted, the
/// same JSON shall be the response. Otherwise, the resolution of the outgoing
/// video stream shall be sent as the response which may be similar to this,
/// {
///     "video_format": "H.264",
///     "width": 800,
///     "height": 600
/// }
/// When the video is started, the outgoing data should be aligned with the
/// configuration response. As for H.264, the data fed every time shall be a
/// NALU with 4 bytes start code as known as 0x00000001. When the video is
/// stopped or paused, no NALU shall be fed. If a configured and stopped video
/// is required to be started again, previous configuration parameters those
/// sent by the video source shall be kept using for new video data.
///
///////////////////////////////////////////////////////////////////////////////
class PUBLIC_API IVideoOutListener : public IAbstractDataSourceListener {
 public:
    const Type type() const final { return kVideoOutListener; }
};

}  // namespace alink
