///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ALinkTypes.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \class IEventListener
/// \brief Listener for connection event
///
/// Connection events may reported in JSON format.
/// a) Navigation status
/// {
///     "event": "navi_started/navi_stopped"
/// }
///
/// Events may also be fed.
/// a) Navigation status
/// {
///     "event": "navi_status",
///     "status": "started"                 // Status: started or stopped
/// }
/// b) Audio status
/// {
///     "event": "audio_status",
///     "channel": 0,                       // Channel name, e.g. 0 for kAudio
///     "application": "app_name",          // Application name
///     "status": "playing"                 // Status: playing, stopped, paused,
///                                         // seek_forward or seek_backward
/// }
/// c) Track status
/// {
///     "event": "track_status",
///     "channel": 0,                       // Channel name, e.g. 0 for kAudio
///     "application": "app_name",          // Application name
///     "track": {
///         "title": "Boundless Oceans, Vast Skies",
///         "duration": 284000,             // Duration in milliseconds
///         "album": "Rock and Roll",
///         "artist": "Beyond",
///         "playlist_size": 10,
///         "playlist_id": 0,
///         "cover_url": "http://somewhere"
///     },
///     "elapsed_time": 0   // milliseconds
/// }
///////////////////////////////////////////////////////////////////////////////
class PUBLIC_API IEventListener : public ALinkListener {
 public:
    const Type type() const final { return kEventListener; }

    ///////////////////////////////////////////////////////////////////////////
    /// \fn onEvent
    /// \brief Connection event received
    /// \param[in]  cid         Connection id
    /// \param[in]  channel     Channel bound
    /// \param[in]  json        Event in JSON format
    /// \param[in]  size        The size of JSON string
    ///
    virtual void onEvent(int32_t cid, Channel channel, const char * json, size_t size) = 0;
};

}  // namespace alink
