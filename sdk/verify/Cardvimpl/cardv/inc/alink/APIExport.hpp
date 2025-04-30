///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Copyright (c) 2020 AutoNavi Software Co.,Ltd. All rights reserved.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef PUBLIC_API
#if defined(_WIN32) || defined(__CYGWIN__) || defined(_WIN32_WCE)
#define PUBLIC_API
#else
#define PUBLIC_API __attribute__ ((visibility ("default")))
#endif
#endif
