/*
 * hal_rtcpwc.h- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#ifndef __HAL_RTCPWC_H__
#define __HAL_RTCPWC_H__

#define ISO_ACK_RETRY_TIME 50

struct hal_base_time
{
    u8  is_vaild;
    u32 base_time;
};

struct hal_fail_count
{
    u32 iso_fail;
    u32 read_fail;
    u32 clock_fail;
};

typedef struct
{
    u16 cmd0;   // offset:0x00
    u16 cmd1;   // offset:0x01
    u16 pwc0;   // offset:0x0f
    u16 pwc1;   // offset:0x20
    u16 wosv;   // offset:0x21
    u16 wosc;   // offset:0x22
    u16 swctrl; // offset:0x49
} suspend_rtc_info;

struct hal_rtcpwc_t
{
    unsigned long         rtc_base;
    u32                   default_base;
    u8                    iso_auto_regen;
    u8                    pwc_io0_hiz;
    u8                    pwc_io2_valid;
    u32                   pwc_io2_cmp;
    u32                   pwc_io2_vlsel;
    u32                   pwc_io2_vhsel;
    u8                    pwc_io3_pu;
    u8                    pwc_io4_valid;
    u32                   pwc_io4_value;
    u8                    pwc_io5_valid;
    u32                   pwc_io5_value;
    u8                    pwc_io5_no_poweroff;
    s16                   offset_count;
    struct hal_base_time  base_time;
    struct hal_fail_count fail_count;
    suspend_rtc_info      rtc_suspend_info;
};

enum hal_rtc_wakeup
{
    RTC_IO0_WAKEUP   = 1,
    RTC_IO1_WAKEUP   = 2,
    RTC_IO2_WAKEUP   = 3,
    RTC_IO3_WAKEUP   = 4,
    RTC_ALARM_WAKEUP = 6,
    RTC_WAKEUP_MAX   = 7,
};

enum hal_rtc_event
{
    RTC_IO0_STATE = 1,
    RTC_IO1_STATE = 2,
    RTC_IO2_STATE = 3,
    RTC_IO3_STATE = 4,
    RTC_STATE_MAX = 5,
};

extern void   hal_rtc_init(struct hal_rtcpwc_t *hal);
extern int    hal_rtc_read_time(struct hal_rtcpwc_t *hal, u32 *seconds);
extern int    hal_rtc_set_time(struct hal_rtcpwc_t *hal, u32 seconds);
extern void   hal_rtc_set_alarm(struct hal_rtcpwc_t *hal, u32 seconds);
extern u32    hal_rtc_get_alarm_int(struct hal_rtcpwc_t *hal);
extern void   hal_rtc_clear_alarm_int(struct hal_rtcpwc_t *hal);
extern void   hal_rtc_power_off(struct hal_rtcpwc_t *hal);
extern void   hal_rtc_alarm_enable(struct hal_rtcpwc_t *hal, u8 enable);
extern u8     hal_rtc_get_alarm_enable(struct hal_rtcpwc_t *hal);
extern void   hal_rtc_get_alarm(struct hal_rtcpwc_t *hal, u32 *seconds);
extern void   hal_rtc_interrupt(struct hal_rtcpwc_t *hal);
extern char * hal_rtc_get_wakeup_name(struct hal_rtcpwc_t *hal);
extern u16    hal_rtc_get_wakeup_value(struct hal_rtcpwc_t *hal);
extern u16    hal_rtc_get_event_state(struct hal_rtcpwc_t *hal);
extern char **hal_rtc_get_event_name(struct hal_rtcpwc_t *hal);
extern s16    hal_rtc_get_offset(struct hal_rtcpwc_t *hal);
extern void   hal_rtc_set_offset(struct hal_rtcpwc_t *hal, s16 offset);
extern void   hal_rtc_suspend(struct hal_rtcpwc_t *hal);
extern void   hal_rtc_resume(struct hal_rtcpwc_t *hal);
extern u16    hal_rtc_get_xtal_state(struct hal_rtcpwc_t *hal);

#endif
