/*
 * cmdqueue.h- Sigmastar
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 *
 * Author: jeff.li <jeff.li@sigmastar.com.cn>
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

#ifndef __CMD_QUEUE_H__
#define __CMD_QUEUE_H__

#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <semaphore.h>
#include <string.h>

#define MODULE_MSG  0x01
#define MODULE_EXIT 0x02

#ifdef __cplusplus
extern "C" {
#endif

class Msg
{
public:
    Msg(unsigned int value, const void *msg = NULL, unsigned int msg_len = 0, unsigned int param = 0);
    Msg(unsigned int value, unsigned int param);
    ~Msg(void);
    int         init(unsigned int value, const void *msg = NULL, unsigned int msg_len = 0, unsigned int param = 0);
    const void *get_message(unsigned int &len);
    void        free_message();
    Msg *       get_next(void)
    {
        return m_next;
    };
    void set_next(Msg *next)
    {
        m_next = next;
    };
    unsigned int get_value(void)
    {
        return m_value;
    };
    int has_param(void)
    {
        return m_has_param;
    };
    unsigned int get_param(void)
    {
        return m_param;
    };

private:
    Msg *        m_next;
    unsigned int m_value;
    int          m_has_param;
    unsigned int m_param;
    unsigned int m_msg_len;
    const void * m_msg;
};

class MsgQueue
{
public:
    MsgQueue(void);
    ~MsgQueue(void);
    int  send_message(unsigned int msgval, const void *msg = NULL, unsigned int msg_len = 0, sem_t *sem = NULL,
                      unsigned int param = 0);
    Msg *get_message(void);
    void release();

private:
    int             send_message(Msg *msg, sem_t *sem);
    Msg *           m_msg_queue;
    pthread_mutex_t m_cmdq_mutex;
};

int cmd_parse_msg(Msg *pMsg, unsigned long *RMsg);
#ifdef __cplusplus
}
#endif

#endif //__CMD_QUEUE_H__
