///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ALinkTypes.hpp"
#include "ALinkListener.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \class IAbstractDataSourceListener
/// \brief Interfaces for data source listener
///
///////////////////////////////////////////////////////////////////////////////
class PUBLIC_API IAbstractDataSourceListener : public ALinkListener {
 public:
    ///////////////////////////////////////////////////////////////////////////
    /// \fn onConfig
    /// \brief Configure the data source
    /// \param[in]  cid         Connection id
    /// \param[in]  channel     Channel bound
    /// \param[in]  json        Configurations in JSON format
    /// \param[in]  size        The size of configuration string
    /// \param[in]  respond     The callback to respond the configuration
    ///
    virtual void onConfig(int32_t cid, Channel channel, const char * json, size_t size, callback_t respond) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn onStart
    /// \brief The channel is ready to be fed data
    /// \param[in]  cid         Connection id
    /// \param[in]  channel     Channel bound
    ///
    virtual void onStart(int32_t cid, Channel channel) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn onPause
    /// \brief No more data feeding for channel
    /// \param[in]  cid         Connection id
    /// \param[in]  channel     Channel bound
    ///
    virtual void onPause(int32_t cid, Channel channel) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn onResume
    /// \brief The channel is available again for data feeding
    /// \param[in]  cid         Connection id
    /// \param[in]  channel     Channel bound
    ///
    virtual void onResume(int32_t cid, Channel channel) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn onStop
    /// \brief The channel is closed
    /// \param[in]  cid         Connection id
    /// \param[in]  channel     Channel bound
    ///
    virtual void onStop(int32_t cid, Channel channel) = 0;
};

}  // namespace alink
