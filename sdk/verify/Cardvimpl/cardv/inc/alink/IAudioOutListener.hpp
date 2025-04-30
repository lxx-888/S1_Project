///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IAbstractDataSourceListener.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \class IAudioOutListener
/// \brief Listener for audio source
///
///////////////////////////////////////////////////////////////////////////////
class PUBLIC_API IAudioOutListener : public IAbstractDataSourceListener {
 public:
    const Type type() const final { return kAudioOutListener; }
};

}  // namespace alink
