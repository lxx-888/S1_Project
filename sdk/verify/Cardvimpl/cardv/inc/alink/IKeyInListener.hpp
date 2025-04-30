///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ALinkTypes.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \class IKeyInListener
/// \brief Listener for key stroke data sink
///
///////////////////////////////////////////////////////////////////////////////
class PUBLIC_API IKeyInListener : public ALinkListener {
 public:
    const Type type() const final { return kKeyInListener; }

    ///////////////////////////////////////////////////////////////////////////
    /// \fn onKey
    /// \brief Key event received
    /// \param[in]  cid         Connection id
    /// \param[in]  channel     Channel bound
    /// \param[in]  event       Key event
    ///
    virtual void onKey(int32_t cid, Channel channel, const KeyEvent & event) = 0;
};

}  // namespace alink
