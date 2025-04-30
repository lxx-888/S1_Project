///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ALinkTypes.hpp"
#include "APIExport.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \class ALinkListener
/// \brief Base class for all listeners
///
///////////////////////////////////////////////////////////////////////////////
class PUBLIC_API ALinkListener {
 public:
    ///////////////////////////////////////////////////////////////////////////
    /// \enum Type
    /// \brief Listener type
    ///
    enum Type {
        kReservedListener       = 0,
        kEventListener          = 1,
        kVideoOutListener       = 2,
        kAudioOutListener       = 3,
        kMicInListener          = 4,
        kKeyInListener          = 5,
        kTouchInListener        = 6,
        kSensorOutListener      = 7,
    };

    ///////////////////////////////////////////////////////////////////////////
    /// \fn type
    /// \brief Get the type of current listener
    /// \return                 The type of current listener
    ///
    virtual const Type type() const = 0;

    virtual ~ALinkListener() = 0;
};

}  // namespace alink
