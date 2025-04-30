/*
 *   nv.c- Sigmastar
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "nvconf.h"

#include <semaphore.h>
#include <errno.h>
#include <time.h>

extern uint8_t crc8(uint8_t *pdata, uint32_t nbytes, uint8_t crc);

#define CONFIG_SIZE 4096
#define TIMEOUT_S   2
#define UNUSED(x)   (x) = (x)

ENTRY ctbl[CTBLSIZE];
int   hash_tbl[CTBLSIZE];

sem_t *sem_nvconf = SEM_FAILED;

int semaphore_open()
{
    if (sem_nvconf != SEM_FAILED)
        return 0;
    sem_nvconf = sem_open("sem_nvconf", O_CREAT, 0666, 1);
    if (sem_nvconf == SEM_FAILED)
    {
        perror("sem_open:");
        return -1;
    }

    return 0;
}

int semaphore_lock()
{
    struct timespec t;
    int             ret = -1;
    t.tv_sec            = time(NULL) + TIMEOUT_S;
    t.tv_nsec           = 0;

    if (sem_nvconf == SEM_FAILED)
        return -1;
    /* Restart if interrupted by handler */
    while ((ret = sem_timedwait(sem_nvconf, &t)) == -1 && errno == EINTR)
        continue;

    /* Check what happened */
    if (ret == -1)
    {
        perror("sem_lock:");
    }

    return ret;
}

void semaphore_unlock()
{
    if (sem_nvconf == SEM_FAILED)
        return;
    sem_post(sem_nvconf);
}

void semaphore_close()
{
    if (sem_nvconf == SEM_FAILED)
        return;
    sem_close(sem_nvconf);
    sem_nvconf = SEM_FAILED;
}

static int check_crc(PNVHDR pheader, uint8_t *buf)
{
    uint8_t crc;
    crc = crc8(buf, pheader->len, 0xff);
    return (((uint8_t)pheader->crcnum == crc) ? 0 : -1);
}

int hash_key(char *pkey)
{
    int i = 0;
    for (i = 0; *pkey; pkey++)
    {
        i += *pkey;
    }
    return (i % ARRAYSIZE(ctbl));
}

int search_entry(char *pkey)
{
    unsigned int i   = 0;
    int          ret = -1;

    for (i = 0; i < ARRAYSIZE(ctbl); i++)
    {
        if (ctbl[i].pkey != NULL && strcmp(pkey, ctbl[i].pkey) == 0)
        {
            return i;
        }
    }

    return ret;
}

int key_update(PENTRY pentry, int idx)
{
    memcpy(ctbl[idx].pval, pentry->pval, sizeof(pentry->pval));
    return 0;
}

int nvconf_add_entry(PENTRY pentry, int idx)
{
    UNUSED(idx);
    int hkey;
    if (semaphore_lock() < 0)
    {
        return -1;
    }

    if ((hkey = search_entry(pentry->pkey)) != -1)
    {
        key_update(pentry, hkey);
    }
    else
    {
        hkey = hash_key(pentry->pkey);
        insert_hashtbl(pentry, hkey);
    }

    semaphore_unlock();
    return 0;
}

int nvconf_del_entry(char *pkey)
{
    int hkey;
    if (!pkey)
        return -1;
    hkey = search_entry(pkey);
    if (hkey < 0)
    {
        printf("no key found!\n");
        return -1;
    }
    ctbl[hkey].pkey[0] = '\0';
    ctbl[hkey].pval[0] = '\0';
    return 0;
}

int nvconf_update_entry(ENTRY entry, int idx)
{
    UNUSED(entry);
    UNUSED(idx);
    return 0;
}

int nvconf_set(PENTRY pentry)
{
    int hkey;
    int ret = -1;
    hkey    = hash_key(pentry->pkey);
    ret     = nvconf_add_entry(pentry, hkey);

    return ret;
}

int nvconf_all()
{
    unsigned int i = 0, j = 0, entry_count = 0;
    ENTRY        show_tbl[CTBLSIZE];
    ENTRY        tmpentry;

    if (semaphore_lock() < 0)
    {
        return -1;
    }

    memset(tmpentry.pkey, '\0', sizeof(tmpentry.pkey));
    memset(tmpentry.pval, '\0', sizeof(tmpentry.pval));
    memset(show_tbl, '\0', sizeof(show_tbl));

    // append
    for (i = 0; i < ARRAYSIZE(show_tbl); i++)
        if (ctbl[i].pkey[0])
        {
            show_tbl[entry_count] = ctbl[i];
            entry_count++;
        }

    // sort
    for (i = 1; i < entry_count; i++)
        for (j = 0; j < entry_count - 1; j++)
            if (strcmp(show_tbl[j].pkey, show_tbl[j + 1].pkey) > 0)
            {
                tmpentry        = show_tbl[j];
                show_tbl[j]     = show_tbl[j + 1];
                show_tbl[j + 1] = tmpentry;
            }

    for (i = 0; i < entry_count; i++)
    {
        printf("%s=%s;;\n", show_tbl[i].pkey, show_tbl[i].pval);
    }

    semaphore_unlock();
    return 0;
}

int nvconf_get(char *pkey)
{
    int hkey;

    if (semaphore_lock() < 0)
    {
        return -1;
    }

    if ((hkey = search_entry(pkey)) >= 0)
    {
        printf("%s\n", ctbl[hkey].pval);
        semaphore_unlock();
        return hkey;
    }
    else
    {
        semaphore_unlock();
        return -1;
    }
}

char *nvconf_get_val(char *pkey)
{
    int hkey;

    if (semaphore_lock() < 0)
    {
        return NULL;
    }

    if ((hkey = search_entry(pkey)) >= 0)
    {
        semaphore_unlock();
        return ctbl[hkey].pval;
    }
    else
    {
        semaphore_unlock();
        return NULL;
    }
}

int nvconf_commit(char *nvconf_path)
{
    char         buf[100], tmpbuf[100];
    int          len;
    int          ret = -1;
    char         key[50], val[50];
    NVHDR        header;
    uint8_t      crc = 0xff;
    int          fd;
    unsigned int i;

    if (semaphore_lock() < 0)
    {
        return -1;
    }

    fd = open(nvconf_path, O_RDWR);
    if (fd < 0)
        return -1;

    memset(key, '\0', sizeof(key));
    memset(val, '\0', sizeof(val));

    flock(fd, LOCK_EX);
    /* wtite back to file */

    // clean file
    snprintf(tmpbuf, sizeof(tmpbuf), "cat /dev/null > %s", nvconf_path);
    system(tmpbuf);

    memset(&header, 0, sizeof(header));
    header.magicnum = NVRAM_MAGIC;
    header.len      = 0;
    write(fd, &header, sizeof(header));

    for (i = 0; i < ARRAYSIZE(ctbl); i++)
    {
        if (ctbl[i].pkey[0])
        {
            len = snprintf(buf, sizeof(buf), "%s=%s;;\n", ctbl[i].pkey, ctbl[i].pval);
            write(fd, buf, len);
            header.len += len;
            crc = crc8((uint8_t *)buf, len, crc);
        }
    }

    header.crcnum  = crc;
    header.hdrtail = 0x0A524448;
    lseek(fd, 0, SEEK_SET);
    if (write(fd, &header, sizeof(header)) != -1)
        ret = 0;

    semaphore_unlock();
    flock(fd, LOCK_UN);
    close(fd);
    fprintf(stderr, "CRC update\n");
    sync();

    return ret;
}

int insert_hashtbl(PENTRY pentry, int idx)
{
    UNUSED(idx);
    /*
        if (ctbl[idx].pkey[0]) {
            fprintf(stderr, "the hash key of [%s] exist, please set another name.\n", ctbl[idx].pkey);
            return 1;
        }

        memcpy(ctbl[idx].pkey, pentry->pkey, sizeof(pentry->pkey));
        memcpy(ctbl[idx].pval, pentry->pval, sizeof(pentry->pval));
    */
    static int index = 0;
    if (index >= 256)
    {
        fprintf(stderr, "index overflow, %d\n", index);
        return -1;
    }
    memcpy(ctbl[index].pkey, pentry->pkey, sizeof(pentry->pkey));
    memcpy(ctbl[index].pval, pentry->pval, sizeof(pentry->pval));
    index++;
    return 0;
}

int set_ctbl(char *tmpstr)
{
    int    ret = -1;
    char * tmpkey, *tmpval;
    int    hkey;
    PENTRY pentry;
    ENTRY  entry;

    memset(entry.pkey, '\0', sizeof(entry.pkey));
    memset(entry.pval, '\0', sizeof(entry.pval));
    pentry  = &entry;
    tmpval  = index(tmpstr, '=');
    *tmpval = '\0';
    tmpkey  = tmpstr;
    hkey    = hash_key(tmpkey);
    memcpy(pentry->pkey, tmpkey, strlen(tmpkey));
    memcpy(pentry->pval, tmpval + 1, strlen(tmpval + 1));
    ret = insert_hashtbl(pentry, hkey);
    return ret;
}

int load_nvconf(char *nvconf_path)
{
    int   fd   = -1;
    int   ret  = -1;
    int   size = -1;
    char *p;
    char *buf = NULL;
    NVHDR header;

    fd = open(nvconf_path, O_RDONLY);
    if (fd < 0)
    {
        perror("error occurred: load_nvconf");
        return -1;
    }

    if (semaphore_lock() < 0)
    {
        close(fd);
        return -1;
    }

    if (read(fd, &header, sizeof(NVHDR)) != sizeof(NVHDR))
    {
        fprintf(stderr, "error occurred: read:%s \n", strerror(errno));
        goto Exit;
    }

    if (header.magicnum != NVRAM_MAGIC)
    {
        fprintf(stderr, "Invalid magic number!\n");
        goto Exit;
    }

    if (lseek(fd, sizeof(NVHDR), SEEK_SET) == -1)
        goto Exit;

    buf = malloc(CONFIG_SIZE);
    if (buf == NULL)
        goto Exit;

    memset(buf, 0, CONFIG_SIZE);
    size = read(fd, buf, CONFIG_SIZE);
    if (size == -1)
        goto Exit;

    if (check_crc(&header, (uint8_t *)buf) < 0)
    {
        fprintf(stderr, "CRC error\n");
        goto Exit;
    }

    buf[CONFIG_SIZE - 1] = '\0';
    p                    = index(buf, '=');
    p++;

    char *tmpstr;
    char *delim = ";;\n";
    tmpstr      = strtok(buf, delim);
    ret         = set_ctbl(tmpstr);

    while ((tmpstr = strtok(NULL, delim)))
    {
        if (tmpstr[0])
            ret = set_ctbl(tmpstr);
    }

Exit:
    semaphore_unlock();
    if (fd >= 0)
        close(fd);
    if (buf)
        free(buf);
    return ret;
}

int cal_crc()
{
    return 0;
}

int printctbl(int idx)
{
    printf("%s:: ctbl[%d]: key:%s, val:%s \n", __func__, idx, ctbl[idx].pkey, ctbl[idx].pval);
    return 0;
}

int clean_ctbl()
{
    memset(ctbl, '\0', sizeof(ctbl));
    // PENTRY pentry;
    // int idx = 0;
    // pentry = &ctbl[idx];
    // while (pentry != NULL && idx <= 10) {
    //     printf("ctbl[%d]: key :%s, val :%s\n", idx, pentry->pkey, pentry->pval);
    //     idx++;
    //     pentry++;
    // }

    return 0;
}

int nvconf_update(char *nvconf_path)
{
    char *buf     = NULL;
    int   fd      = -1;
    int   size    = -1;
    int   ret     = -1;
    char *tmpstr  = NULL;
    char  delim[] = ";;\n";

    fd = open(nvconf_path, O_RDONLY);
    if (fd < 0)
        return -1;

    if (semaphore_lock() < 0)
        goto L_EXIT;

    if (lseek(fd, sizeof(NVHDR), SEEK_SET) == -1)
        goto L_EXIT;

    buf = malloc(CONFIG_SIZE);
    if (buf == NULL)
        goto L_EXIT;

    size = read(fd, buf, CONFIG_SIZE);
    if (size == -1)
        goto L_EXIT;

    buf[CONFIG_SIZE - 1] = '\0';
    tmpstr               = strtok(buf, delim);
    set_ctbl(tmpstr);

    while ((tmpstr = strtok(NULL, delim)))
    {
        if (tmpstr[0])
            set_ctbl(tmpstr);
    }

    semaphore_unlock();
    nvconf_commit(nvconf_path);
    ret = 0;
L_EXIT:
    if (fd >= 0)
        close(fd);
    if (buf)
        free(buf);
    return ret;
}