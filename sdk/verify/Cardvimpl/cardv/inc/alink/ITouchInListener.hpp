///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ALinkTypes.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \class ITouchInListener
/// \brief Listener for touch action data sink
///
///////////////////////////////////////////////////////////////////////////////
class PUBLIC_API ITouchInListener : public ALinkListener {
 public:
    const Type type() const final { return kTouchInListener; }

    ///////////////////////////////////////////////////////////////////////////
    /// \fn onTouch
    /// \brief Touch event received
    /// \param[in]  cid         Connection id
    /// \param[in]  channel     Channel bound
    /// \param[in]  event       Touch event
    ///
    virtual void onTouch(int32_t cid, Channel channel, const TouchEvent & event) = 0;
};

}  // namespace alink
