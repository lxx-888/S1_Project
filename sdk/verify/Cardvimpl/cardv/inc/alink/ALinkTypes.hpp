///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "Error.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \enum Channel
/// \brief ALink channels
///
///////////////////////////////////////////////////////////////////////////////
enum Channel {
    kReserved       = 0,
    kVideoOut       = 1,
    kAudioOut       = 2,
    kMicIn          = 3,
    kUserInputIn    = 4,
    kSensorOut      = 5,
    kMediaAudioOut  = 6,
    kSpeechAudioOut = 7,
    kNaviAudioOut   = 8,
    kEvent          = 9,
};

///////////////////////////////////////////////////////////////////////////////
/// \struct TouchPoint
/// \brief Touch point
///
///////////////////////////////////////////////////////////////////////////////
struct TouchPoint {
    int32_t x;                  ///< Screen coordinate x
    int32_t y;                  ///< Screen coordinate y
    int32_t slot;               ///< Slot number for finger
};

///////////////////////////////////////////////////////////////////////////////
/// \struct TouchEvent
/// \brief Touch event
///
///////////////////////////////////////////////////////////////////////////////
struct TouchEvent {
    int32_t action;             ///< 0 for down; 1 for up; 2 for move
    int32_t pointCount;         ///< The number of touch points
    TouchPoint points[10];      ///< Touch points
};

///////////////////////////////////////////////////////////////////////////////
/// \struct KeyEvent
/// \brief Key event
///
///////////////////////////////////////////////////////////////////////////////
struct KeyEvent {
    int32_t action;             ///< 0 for down; 1 for up
    int32_t keyCode;            ///< Key code
};

///////////////////////////////////////////////////////////////////////////////
/// \enum SensorDataCategory
/// \brief ALink sensor category
///
///////////////////////////////////////////////////////////////////////////////
enum SensorDataCategory {
    kGnssBit            = 0x8,
    kAccelerometerBit   = 0x80,
    kGyroscopeBit       = 0x800,
    kMagnetometerBit    = 0x8000,
};

///////////////////////////////////////////////////////////////////////////////
/// \enum SensorDataType
/// \brief ALink sensor value type for schema
///
///////////////////////////////////////////////////////////////////////////////
enum SensorDataType {
    kGnssLatitude   = 0,
    kGnssLongitude  = 1,
    kGnssAltitude   = 2,
    kGnssAccuracy   = 3,
    kAccelerometerX = 4,
    kAccelerometerY = 5,
    kAccelerometerZ = 6,
    kGyroscopeX     = 7,
    kGyroscopeY     = 8,
    kGyrosceopZ     = 9,
    kMagnetometerX  = 10,
    kMagnetometerY  = 11,
    kMagnetometerZ  = 12,
};

///////////////////////////////////////////////////////////////////////////////
/// \struct SensorDataHead
/// \brief Head of every sensor data followed by data aligned with schema
///
///////////////////////////////////////////////////////////////////////////////
struct SensorDataHead {
    int64_t reserved;           ///< Reserved
    int64_t timestamp;          ///< Microseconds from Unix Epoch
    int64_t category;           ///< Mask for active sensor catogory
};

///////////////////////////////////////////////////////////////////////////////
/// \typedef callback_t
/// \brief Type of callback which accept 4 parameters
/// \param[in]  cid         Connection id
/// \param[in]  channel     Channel bound
/// \param[in]  json        Parameters in JSON format
/// \param[in]  size        The size of parameter string
///
///////////////////////////////////////////////////////////////////////////////
using callback_t = void (*)(int32_t, Channel, const char *, size_t);

}  // namespace alink
