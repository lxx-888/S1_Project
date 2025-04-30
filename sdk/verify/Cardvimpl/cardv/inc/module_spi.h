/*
 * module_spi.h- Sigmastar
 *
 * Copyright (C) 2020 Sigmastar Technology Corp.
 *
 * Author: jiahong.xiong <jiahong.xiong@sigmastar.com.cn>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef _MODULE_SPI_H_
#define _MODULE_SPI_H_

#define DBG_LEVEL_FATAL (1)
#define DBG_LEVEL_ERROR (2)
#define DBG_LEVEL_WARN  (3)
#define DBG_LEVEL_INFO  (4)
#define DBG_LEVEL_DEBUG (5)
#define DBG_LEVEL_ENTRY (6)

#define DBG_LEVEL DBG_LEVEL_ENTRY

#define COLOR_NONE   "\033[0m"
#define COLOR_BLACK  "\033[0;30m"
#define COLOR_BLUE   "\033[0;34m"
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_CYAN   "\033[0;36m"
#define COLOR_RED    "\033[0;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_WHITE  "\033[1;37m"

#ifdef DBG
#define DBG_ENTRY(fmt, args...)                             \
    ({                                                      \
        do                                                  \
        {                                                   \
            if (DBG_LEVEL >= DBG_LEVEL_ENTRY)               \
            {                                               \
                printf("%s[%d]: ", __FUNCTION__, __LINE__); \
                printf(fmt, ##args);                        \
            }                                               \
        } while (0);                                        \
    })
#define DBG_DEBUG(fmt, args...)                             \
    ({                                                      \
        do                                                  \
        {                                                   \
            if (DBG_LEVEL >= DBG_LEVEL_DEBUG)               \
            {                                               \
                printf("%s[%d]: ", __FUNCTION__, __LINE__); \
                printf(fmt, ##args);                        \
            }                                               \
        } while (0);                                        \
    })
#define DBG_INFO(fmt, args...)                              \
    ({                                                      \
        do                                                  \
        {                                                   \
            if (DBG_LEVEL >= DBG_LEVEL_INFO)                \
            {                                               \
                printf("%s[%d]: ", __FUNCTION__, __LINE__); \
                printf(fmt, ##args);                        \
            }                                               \
        } while (0);                                        \
    })
#define DBG_WRN(fmt, args...)                               \
    ({                                                      \
        do                                                  \
        {                                                   \
            if (DBG_LEVEL >= DBG_LEVEL_WARN)                \
            {                                               \
                printf("%s[%d]: ", __FUNCTION__, __LINE__); \
                printf(fmt, ##args);                        \
            }                                               \
        } while (0);                                        \
    })
#define DBG_ERR(fmt, args...)                               \
    ({                                                      \
        do                                                  \
        {                                                   \
            if (DBG_LEVEL >= DBG_LEVEL_ERROR)               \
            {                                               \
                printf("%s[%d]: ", __FUNCTION__, __LINE__); \
                printf(fmt, ##args);                        \
            }                                               \
        } while (0);                                        \
    })
#define DBG_FATAL(fmt, args...)                             \
    ({                                                      \
        do                                                  \
        {                                                   \
            if (DBG_LEVEL >= DBG_LEVEL_FATAL)               \
            {                                               \
                printf("%s[%d]: ", __FUNCTION__, __LINE__); \
                printf(fmt, ##args);                        \
            }                                               \
        } while (0);                                        \
    })
#else
#define DBG_ENTRY(fmt, args...)
#define DBG_DEBUG(fmt, args...)
#define DBG_INFO(fmt, args...)
#define DBG_WRN(fmt, args...)
#define DBG_ERR(fmt, args...)                               \
    ({                                                      \
        do                                                  \
        {                                                   \
            if (DBG_LEVEL >= DBG_LEVEL_ERROR)               \
            {                                               \
                printf("%s[%d]: ", __FUNCTION__, __LINE__); \
                printf(fmt, ##args);                        \
            }                                               \
        } while (0);                                        \
    })
#define DBG_FATAL(fmt, args...)                             \
    ({                                                      \
        do                                                  \
        {                                                   \
            if (DBG_LEVEL >= DBG_LEVEL_FATAL)               \
            {                                               \
                printf("%s[%d]: ", __FUNCTION__, __LINE__); \
                printf(fmt, ##args);                        \
            }                                               \
        } while (0);                                        \
    })
#endif

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

void cardv_spi_transfer(int fd);
void cardv_spi_send_data(int fd, MI_U16 data, MI_U16 size);
void cardv_spi_read_data(int fd, MI_U8 size);
int  cardv_spi_open(const char *device, MI_U8 u8mode, MI_U8 u8bits, MI_U32 u32speed, MI_U16 u16delay);
void cardv_spi_close(int fd);

#endif //#define _MODULE_SPI_H_
