/*
 *	nvconf.h- Sigmastar
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

#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#define NVRAM_MAGIC  0x12345678

#define CTBLSIZE 257

#define NVCONF_CGIPATH "/customer/wifi/cgi_config.bin"
#define NVCONF_NETPATH "/customer/wifi/net_config.bin"

typedef unsigned int __u32;
typedef __u32        uint32_t;

typedef struct entry
{
    char          pkey[50];
    char          pval[200];
    struct entry *pnext;
} ENTRY, *PENTRY;

typedef struct
{
    char *pkey;
    char *pval;
    int   len;
    // CALLBACK
    void *priv;
} NVPARAM, *PNVPARAM;

typedef struct nvconfhdr
{
    uint32_t magicnum;
    uint32_t crcnum;
    uint32_t len;
    uint32_t hdrtail;
} NVHDR, *PNVHDR;

int  semaphore_open();
int  semaphore_lock();
void semaphore_unlock();
void semaphore_close();

int   insert_hashtbl(PENTRY pentry, int idx);
int   load_nvconf(char *nvconf_path);
int   printctbl();
int   search_entry(char *pkey);
int   load_nvconf_without_header(char *nvconf_path);
int   nvconf_add_entry(PENTRY pentry, int idx);
int   nvconf_del_entry(char *pkey);
int   nvconf_set(PENTRY pentry);
int   nvconf_get(char *pkey);
char *nvconf_get_val(char *pkey);
int   nvconf_commit(char *nvconf_path);
int   nvconf_all();
int   nvconf_update(char *nvconf_path);