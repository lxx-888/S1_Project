/*
 *	easy_cgi.c- Sigmastar
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
#include <string.h>
#include <ctype.h>
#include "easy_cgi.h"
/* convert hex string to int */
#define UNUSED(x) (x) = (x)
ENTRY entry;

int check_only_key(char *tmpstr)
{
    if (strchr(tmpstr, '=') == 0)
    {
        return 1;
    }
    else
        return 0;
}
int hex_string_dec(char *str, unsigned int *result)
{
    unsigned char ch, len, i;
    unsigned int  val;

    len = strlen(str);

    if (len > 8)
    {
        return -1;
    }

    for (val = 0, i = 0; i < len; i++)
    {
        if ((str[i] >= '0') && (str[i] <= '9'))
        {
            ch = str[i] - '0';
        }
        else if ((str[i] >= 'a') && (str[i] <= 'f'))
        {
            ch = (str[i] - 'a') + 10;
        }
        else if ((str[i] >= 'A') && (str[i] <= 'F'))
        {
            ch = (str[i] - 'A') + 10;
        }
        else
        {
            return -1;
        }

        val <<= 4;
        val += ch;
    }

    *result = val;

    return 0;
}

int htoi(char *s)
{
    char *digits = "0123456789ABCDEF";
    if (islower(s[0]))
        s[0] = toupper(s[0]);
    if (islower(s[1]))
        s[1] = toupper(s[1]);
    return 16 * (strchr(digits, s[0]) - strchr(digits, '0')) + (strchr(digits, s[1]) - strchr(digits, '0'));
}

int query_env_var(int argc, char *argv[], char *envp[])
{
    UNUSED(argc);
    UNUSED(argv);
    printf("Content-type: text/html\n\n");
    int i;
    for (i = 0; envp[i] != NULL; ++i)
    {
        printf("%s<br/>", envp[i]);
    }
    return 0;
}

int decode(char *str)
{
    int  i, n, p = 0, q = 0, iskey = 1, isval = 0;
    char c;

    memset(entry.pkey, '\0', sizeof(entry.pkey));
    memset(entry.pval, '\0', sizeof(entry.pval));
    n = strlen(str);
    for (i = 0; i < n; i++, str++)
    {
        int iseq = 0;
        c        = *str;
        switch (c)
        {
        case '+':
            c = ' ';
            break;
        case '%': {
            char s[3];
            s[0] = *(str + 1);
            s[1] = *(str + 2);
            s[2] = 0;
            c    = htoi(s);
            if (iskey)
            {
                entry.pkey[p] = c;
                p++;
            }
            else if (isval)
            {
                entry.pval[q] = c;
                q++;
            }
            i += 2;
            str += 2;
        }
        break;
        case '=':
            c    = ':';
            iseq = 1;
            break;
        };
        // putchar(c);
        if (iseq)
        {
            // putchar(' ');
            iskey = 0;
            // if(is_only_key==0)
            isval = 1;
        }
        else if (iskey)
        {
            entry.pkey[p] = c;
            p++;
        }
        else if (isval)
        {
            entry.pval[q] = c;
            q++;
        }
        // if (iseq) putchar(' ');
    }
    // putchar('\n');
    return 0;
}

int set_ok(void)
{
    // printf("Content-type: text/html\n\n");
    printf("<head>\n");
    printf("<title>OK</title>\n");
    printf("</head>\n");

    printf("<body bgcolor=white>\n");
    printf("<p>OK</p>\n");
    // printf("<input type=button name=\"okbutton\" value=\"OK\" style=\"width:100px\"
    // OnClick='window.location.replace(\"../%s\")'>\n",url);
    printf("</body>\n");

    return 0;
}

int ok_page(char *title, char *url, char *str)
{
    // printf("Content-type: text/html\n\n");
    printf("<head>\n");
    printf("<title>%s</title>\n", title);
    printf("</head>\n");

    printf("<body bgcolor=white>\n");
    printf("<p>%s</p>\n", str);
    printf(
        "<input type=button name=\"okbutton\" value=\"OK\" style=\"width:100px\" "
        "OnClick='window.location.replace(\"../%s\")'>\n",
        url);
    printf("</body>\n");

    return 0;
}

int set_complete(char *title, char *url)
{
    // printf("Content-type: text/html\n\n");
    printf("<head>\n");
    printf("<title>%s</title>\n", title);
    printf("</head>\n");

    printf("<body bgcolor=white>\n");
    printf("Setup is complete<p>");
    printf(
        "<input type=button name=\"okbutton\" value=\"OK\" style=\"width:100px\" "
        "OnClick='window.location.replace(\"../%s\")'>\n",
        url);
    printf("</body>\n");

    return 0;
}

int upgrade_headerfail(char *title, char *url)
{
    // printf("Content-type: text/html\n\n");
    printf("<head>\n");
    printf("<title>%s</title>\n", title);
    printf("</head>\n");

    printf("<body bgcolor=white>\n");
    printf("The file is wrong,please check the file again!!<p>");
    printf(
        "<input type=button name=\"okbutton\" value=\"OK\" style=\"width:100px\" "
        "OnClick='window.location.replace(\"../%s\")'>\n",
        url);
    printf("</body>\n");

    return 0;
}

int upgrade_crcfail(char *title, char *url)
{
    // printf("Content-type: text/html\n\n");
    printf("<head>\n");
    printf("<title>%s</title>\n", title);
    printf("</head>\n");

    printf("<body bgcolor=white>\n");
    printf("The crc16 of file is not the same, please retry firmware upload again!!<p>");
    printf(
        "<input type=button name=\"okbutton\" value=\"OK\" style=\"width:100px\" "
        "OnClick='window.location.replace(\"../%s\")'>\n",
        url);
    printf("</body>\n");

    return 0;
}

int error_alert(char *title, char *url, char *str)
{
    UNUSED(url);
    UNUSED(str);
    // printf("Content-type: text/html\n\n");
    printf("<head>\n");
    printf("<title>%s</title>\n", title);
    printf("</head>\n");

    printf("<body bgcolor=white>\n");
    printf("Setup is complete<p>");
    printf("reboot...<p>");
    // printf("<input type=button name=\"okbutton\" value=\"OK\" style=\"width:100px\"
    // OnClick='window.location.replace(\"../%s\")'>\n",url);
    printf("</body>\n");

    return 0;
}

int system_reboot(char *title, char *url)
{
    UNUSED(url);
    // printf("Content-type: text/html\n\n");
    printf("<head>\n");
    printf("<title>%s</title>\n", title);
    printf("</head>\n");

    printf("<body bgcolor=white>\n");
    printf("Setup is complete<p>");
    printf("reboot...<p>");
    // printf("<input type=button name=\"okbutton\" value=\"OK\" style=\"width:100px\"
    // OnClick='window.location.replace(\"../%s\")'>\n",url);
    printf("</body>\n");

    return 0;
}

int gotopage(char *title, char *url)
{
    fprintf(stderr, "[ in gotopage func ]\n\n");
    printf("<head>\n");

    printf("<title>%s</title>\n", title);
    printf("</head>\n");

    printf("<body bgcolor=white onload=window.location.replace(\"../%s\") >\n", url);
    // printf("Setup is complete<p>");
    // printf("reboot...<p>");
    // printf("<input type=button name=\"okbutton\" value=\"OK\" style=\"width:100px\"
    // OnClick='window.location.replace(\"../%s\")'>\n",url);
    printf("</body>\n");

    return 0;
}

int streamer_status(int alive)
{
    // printf("Content-type: text/html\n\n");
    printf("<head>\n");
    printf("<title>streamer status</title>\n");
    printf("</head>\n");
    printf("<body bgcolor=white>\n");
    if (alive == 0)
    {
        printf("<p>0</p>");
        // printf("<p>");
        // printf("<input type=button name=\"okbutton\" value=\"OK\" style=\"width:100px\"
        // OnClick='window.location.replace(\"../%s\")'>\n",url);
    }
    else if (alive == 1)
    {
        printf("<p>1</p>");
    }
    printf("</body>\n");

    return 0;
}

int goto_parent_page(char *title, char *url)
{
    // printf("Content-type: text/html\n\n");
    printf("<head>\n");

    printf("<title>%s</title>\n", title);
    printf("</head>\n");

    printf("<body bgcolor=white onload=parent.location.replace(\"../%s\") >\n", url);
    // printf("Setup is complete<p>");
    // printf("reboot...<p>");
    // printf("<input type=button name=\"okbutton\" value=\"OK\" style=\"width:100px\"
    // OnClick='window.location.replace(\"../%s\")'>\n",url);
    printf("</body>\n");

    return 0;
}
