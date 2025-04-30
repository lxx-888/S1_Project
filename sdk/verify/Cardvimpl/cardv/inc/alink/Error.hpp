///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>

namespace alink {

const int32_t ERR_OK                                                    = 0;
const int32_t ERR_INVALID_CONNECTION                                    = -1;
const int32_t ERR_INVALID_CHANNEL                                       = -2;
const int32_t ERR_INVALID_JSON                                          = -3;
const int32_t ERR_SERVICE_PORT_OCCUPIED                                 = -4;
const int32_t ERR_BAD_SERVICE_INSTANCE                                  = -5;
const int32_t ERR_INTERFACE_DEPRECATED                                  = -6;
const int32_t ERR_NOT_IMPLEMENTED                                       = -7;
const int32_t ERR_WRONG_LISTENER                                        = -8;
const int32_t ERR_LISTENER_IN_USING                                     = -9;
const int32_t ERR_WRONG_EVENT                                           = -10;

}  // namespace alink
