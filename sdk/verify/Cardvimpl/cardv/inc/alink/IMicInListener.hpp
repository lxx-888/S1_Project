///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ALinkTypes.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \class IMicInListener
/// \brief Listener for microphone audio sink
///
///////////////////////////////////////////////////////////////////////////////
class PUBLIC_API IMicInListener : public ALinkListener {
 public:
    const Type type() const final { return kMicInListener; }

    ///////////////////////////////////////////////////////////////////////////
    /// \fn onConfig
    /// \brief Set the audio info of microphone
    /// \param[in]  cid         Connection id
    /// \param[in]  channel     Channel bound
    /// \param[in]  json        Configuration in JSON format
    /// \param[in]  size        The size of JSON string
    /// \param[in]  respond     The callback to respond the configuration
    ///
    virtual void onConfig(int32_t cid, Channel channel, const char * json, size_t size, callback_t respond) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn onAudioData
    /// \brief Microphone audio data received
    /// \param[in]  cid         Connection id
    /// \param[in]  channel     Channel bound
    /// \param[in]  buffer      The audio data received
    /// \param[in]  size        The size of the data
    ///
    virtual void onAudioData(int32_t cid, Channel channel, const uint8_t * buffer, size_t size) = 0;
};

}  // namespace alink
