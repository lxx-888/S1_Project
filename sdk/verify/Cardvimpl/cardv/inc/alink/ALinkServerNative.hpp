///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ALinkTypes.hpp"
#include "IAudioOutListener.hpp"
#include "IEventListener.hpp"
#include "IKeyInListener.hpp"
#include "IMicInListener.hpp"
#include "IPipe.hpp"
#include "ISensorOutListener.hpp"
#include "ITouchInListener.hpp"
#include "IVideoOutListener.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \class ALinkServerNative
/// \brief Main class for ALink server
///
///////////////////////////////////////////////////////////////////////////////
class PUBLIC_API ALinkServerNative {
 public:
    ///////////////////////////////////////////////////////////////////////////
    /// \fn version
    /// \brief Get the version
    /// \return                 The version string
    ///
    static const char * version();

    ///////////////////////////////////////////////////////////////////////////
    /// \fn getInstance
    /// \brief Get the instance of ALink server
    /// \param[in]  json        The configurations in JSON format for new
    ///                         created instance, ignored by living instance
    /// \param[in]  size        The size of JSON string
    /// \return                 The instance of ALink server
    ///
    static ALinkServerNative & getInstance(const char * json, size_t size);

    ///////////////////////////////////////////////////////////////////////////
    /// \fn releaseInstance
    /// \brief Release a living ALink server instance
    /// \param[in]  server      The server instance to be released
    /// \return                 Error code for the operation
    ///
    static int32_t releaseInstance(const ALinkServerNative & server);

    ///////////////////////////////////////////////////////////////////////////
    /// \fn registerChannelListener
    /// \brief Register a listener for specified channel
    /// \param[in]  channel     Target channel
    /// \param[in]  listener    A specific listener for target channel
    /// \return                 Error code for the operation
    ///
    virtual int32_t registerChannelListener(Channel channel, ALinkListener * listener) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn unregisterChannelListener
    /// \brief Unregister all listeners for specified channel
    /// \param[in]  channel     Target channel
    /// \return                 Error code for the operation
    ///
    virtual int32_t unregisterChannelListener(Channel channel) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn registerUsbDelegate
    /// \brief Register USB pipe for USB connection
    /// \param[in]  io          USB visit delegate
    /// \return                 Error code for the operation
    ///
    virtual int32_t registerUsbDelegate(IPipe * io) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn start
    /// \brief Start ALink service
    /// \return                 Error code for the operation
    ///
    virtual int32_t start() = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn stop
    /// \brief Stop ALink service
    /// \return                 Error code for the operation
    ///
    virtual int32_t stop() = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn feed
    /// \brief Feed data to specified channel of specified connection
    /// \param[in]  cid         Connection id
    /// \param[in]  channel     Target channel
    /// \param[in]  buffer      Data to send
    /// \param[in]  size        The size of data
    /// \return                 Error code for the operation
    ///
    virtual int32_t feed(int32_t cid, Channel channel, uint8_t * buffer, size_t size) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn notify
    /// \brief Send notification relative with specified connection
    /// \param[in]  cid         Connection id
    /// \param[in]  json        Message to be notified in JSON format
    /// \param[in]  size        The size of data
    /// \return                 Error code for the operation
    ///
    virtual int32_t notify(int32_t cid, const char * json, size_t size) = 0;
};

}  // namespace alink
