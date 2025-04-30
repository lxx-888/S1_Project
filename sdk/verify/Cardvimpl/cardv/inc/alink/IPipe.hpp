///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ALinkTypes.hpp"

namespace alink {

///////////////////////////////////////////////////////////////////////////////
/// \class IPipe
/// \brief Interfaces for IO pipe
///
///////////////////////////////////////////////////////////////////////////////
class PUBLIC_API IPipe {
 public:
    ///////////////////////////////////////////////////////////////////////////
    /// \fn read
    /// \brief Synchronous read operation from the pipe
    /// \param[in]  buffer      The buffer to receive data
    /// \param[in]  size        The size of the buffer
    /// \param[in]  timout      The maxinum period for the operation in
    ///                         milliseconds, -1 for waiting till data received
    /// \return                 The size of data received
    ///
    virtual size_t read(uint8_t * buffer, size_t size, uint32_t timeout = -1) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \fn write
    /// \brief Synchronous write operation to the pipe
    /// \param[in]  buffer      The data to be sent
    /// \param[in]  size        The size of the buffer
    /// \param[in]  timout      The maxinum period for the operation in
    ///                         milliseconds, -1 for waiting till data sent
    /// \return                 The size of data sent
    ///
    virtual size_t write(const uint8_t * buffer, size_t size, uint32_t timeout = -1) = 0;
};

}  // namespace alink
