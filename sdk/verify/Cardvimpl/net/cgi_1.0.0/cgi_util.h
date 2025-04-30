/*
 *	cgi_util.h- Sigmastar
 *
 * Copyright (C) 2019 Sigmastar Technology Corp.
 *
 * Author: XXXX <XXXX@sigmastar.com.cn>
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

#ifndef CGI_UTIL_H_
#define CGI_UTIL_H_

typedef struct cmdnode
{
    char *key;
    char *name;
    int   cmd;
    void (*callback)(void *parg);
    struct cmdnode *next;
} CMDNODE, *PCMDNODE;

enum CGI_TYPE
{
    KEYVALUE = 0,
    EXECMD   = 1
};

#ifndef NVCONF_CGIPATH
#ifdef DUAL_OS
#define NVCONF_CGIPATH "/misc/cgi_config.bin"
#else
#define NVCONF_CGIPATH "/customer/wifi/cgi_config.bin"
#endif
#endif

#ifndef NVCONF_NETPATH
#define NVCONF_NETPATH "/customer/wifi/net_config.bin"
#endif

typedef void (*Callback)(void *parg);
void     PrintHttpHdr_text(void);
void     PrintHttpHdr_image(void);
int      ProcessCgiCmd(CMDNODE *head, char *name, char *returnpage);
int      check_cgi_cmd_method(void);
int      check_equal_symbol(char *tmpstr);
CMDNODE *addCmdNode(char *strcgikey, char *strcginame, int cgitype, Callback cb,
                    CMDNODE *cur); //,void (* func)(void *parg)
// void add_cmd_list(char* strcgikey,char* strcginame, CMDNODE *cur);
void FreeCgiList(CMDNODE *head);
int  GetProcess(void *result);
int  SetProcess(void *result);
int  DelProcess(void *result);

#endif /* CGI_UTIL_H_ */
