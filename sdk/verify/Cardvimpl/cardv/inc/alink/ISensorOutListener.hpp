///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IAbstractDataSourceListener.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \class ISensorOutListener
/// \brief Listener for sensor data source
///
/// Sensor data shall not be sent before schema negotiation. The negotiation
/// starts by a schema request in JSON format from client. Server shall response
/// the request with the schema for outgoing data. The negotiation is done when
/// client accepts the schema responded. If client does not accept the schema
/// responded by server, it may launch a new turn of negotiation.
/// The onConfig interface may receive following json as a schema request,
/// {
///     "command": "schema_request",
///     "schema": [0,1,2,3,9,8,7]
/// }
/// It indicates the client wants to receive a block of buffer which has
/// latitude, longitude, altitude, gyro-z, gyro-y and gyro-x in order after the
/// sensor data head.
/// The server may agree it with the schema response in JSON format,
/// {
///     "command": "schema_response",
///     "schema": [0,1,2,3,9,8,7]
/// }
/// Or, the server may disagree and respond another schema,
/// {
///     "command": "schema_response",
///     "schema": [0,1,7,8,9]
/// }
/// Whatever the server respond, the outgoing sensor data must be aligned with
/// the latest schema response. In previous case, the buffer fed for sensor
/// data may be as following if only GNSS values are available.
///    0                                           63
///    ---------------------------------------------
///  0 |              Reserved (0)                 |
///    ---------------------------------------------
///  8 |       Timestamp (1585671179815225)        |
///    ---------------------------------------------
/// 16 |             Category (0x8)                |
///    ---------------------------------------------
/// 24 |          Latitude (31.216453)             |
///    ---------------------------------------------
/// 32 |          Longitude (121.475287)           |
///    ---------------------------------------------
/// 40 |            GyroscopeX (0.0)               |
///    ---------------------------------------------
/// 48 |            GyroscopeY (0.0)               |
///    ---------------------------------------------
/// 56 |            GyroscopeZ (0.0)               |
///    ---------------------------------------------
///
///////////////////////////////////////////////////////////////////////////////
class PUBLIC_API ISensorOutListener : public IAbstractDataSourceListener {
 public:
    const Type type() const final { return kSensorOutListener; }
};

}  // namespace alink
