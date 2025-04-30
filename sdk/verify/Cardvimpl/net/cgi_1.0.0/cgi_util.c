/*
 * cgi_util.c
 *Author: XXXX <XXXX@sigmastar.com.cn>
 */
/*
 *Common Gateway Interface environment variable
 *
 SERVER_PROTOCOL:        HTTP/version.
 SERVER_PORT:                   TCP port (decimal).
 REQUEST_METHOD:         name of HTTP method (see above).
 PATH_INFO:                        path suffix, if appended to URL after program name and a slash.
 PATH_TRANSLATED:         corresponding full path as supposed by server, if PATH_INFO is present.
 SCRIPT_NAME:                   relative path to the program, like /cgi-bin/script.cgi.
 QUERY_STRING:                the part of URL after ? character. The query string may be composed of *name=value pairs
 separated with ampersands (such as var1=val1&var2=val2...) when used to submit form data transferred via GET method as
 defined by HTML application/x-www-form-urlencoded.
 REMOTE_HOST:                host name of the client, unset if server did not perform such lookup.
 REMOTE_ADDR:                IP address of the client (dot-decimal).
 AUTH_TYPE:                       identification type, if applicable.
 REMOTE_USER                  used for certain AUTH_TYPEs.
 REMOTE_IDENT:               see ident, only if server performed such lookup.
 CONTENT_TYPE:                Internet media type of input data if PUT or POST method are used, as provided via HTTP
 header.
 CONTENT_LENGTH:          similarly, size of input data (decimal, in octets) if provided via HTTP header.
 Variables passed by user agent (HTTP_ACCEPT, HTTP_ACCEPT_LANGUAGE, HTTP_USER_AGENT, HTTP_COOKIE and possibly others)
 contain values of corresponding HTTP headers and therefore have the same sense.


 Key	                                      Value
 DOCUMENT_ROOT	          The root directory of your server
 HTTP_COOKIE					  The visitor's cookie, if one is set
 HTTP_HOST	                      The hostname of the page being attempted
 HTTP_REFERER	              The URL of the page that called your program
 HTTP_USER_AGENT	      The browser type of the visitor
 HTTPS	                              "on" if the program is being called through a secure server
 PATH	                                  The system path your server is running under
 QUERY_STRING	              The query string (see GET, below)
 REMOTE_ADDR	              The IP address of the visitor
 REMOTE_HOST	              The hostname of the visitor (if your server has reverse-name-lookups on; otherwise this is
 the IP address again)
 REMOTE_PORT	                  The port the visitor is connected to on the web server
 REMOTE_USER	                  The visitor's username (for .htaccess-protected pages)
 REQUEST_METHOD	      GET or POST
 REQUEST_URI	                  The interpreted pathname of the requested document or CGI (relative to the document
 root)
 SCRIPT_FILENAME	          The full pathname of the current CGI
 SCRIPT_NAME	                  The interpreted pathname of the current CGI (relative to the document root)
 SERVER_ADMIN	              The email address for your server's webmaster
 SERVER_NAME                	  Your server's fully qualified domain name (e.g. www.cgi101.com)
 SERVER_PORT	                  The port number your server is listening on
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cgi_util.h"
#include "nvconf.h"
CMDNODE *    head, *cur;
extern ENTRY entry;
PENTRY       pentry;
extern ENTRY ctbl[CTBLSIZE];
int          POSTCMD = 0;

extern int decode(char *str);
extern int set_complete(char *title, char *url);

void PrintHttpHdr_text(void) // Display the CGI Header
{
    printf("Content-Type: text/html\r\n");
    printf("\r\n");
    fprintf(stderr, "Content-Type: text/html\n\n");
}

void PrintHttpHdr_image(void)
{
    printf("Content-type: image/jpeg\r\n");
    printf("\r\n");
    fprintf(stderr, "Content-type: image/jpeg\n\n");
}

int check_http_method(void)
{
    if (atoi(getenv("CONTENT_LENGTH")) > 0)
    {
        return 1;
    }
    else if (getenv("QUERY_STRING") && atoi(getenv("CONTENT_LENGTH")) != -1)
    { // GET
        return 0;
    }
    else
        return 0;
}

int check_cgi_cmd_method(void)
{
    if (atoi(getenv("CONTENT_LENGTH")) > 0)
    {
        return 1;
    }
    else if (getenv("QUERY_STRING") && atoi(getenv("CONTENT_LENGTH")) != -1)
    { // GET
        return 0;
    }
    else
        return 0;
}

int check_equal_symbol(char *tmpstr)
{
    if (strchr(tmpstr, '=') == 0)
        return 1;
    else
        return 0;
}

CMDNODE *addCmdNode(char *strcgikey, char *strcginame, int cgitype, Callback cb, CMDNODE *cur)
{
    CMDNODE *cmdNode = (CMDNODE *)malloc(strlen(strcgikey) + strlen(strcginame) + 3);
    if (NULL == cmdNode)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }

    cmdNode->key      = strcgikey;
    cmdNode->name     = strcginame;
    cmdNode->cmd      = cgitype;
    cmdNode->callback = cb;
    cmdNode->next     = NULL; // Change 1
    if (cur == NULL)
    {
        cur       = cmdNode;
        cur->next = cmdNode->next;
    }
    else
    {
        // add to end
        cur->next = cmdNode;
        cur       = cmdNode;
    }

    return cur;
}

int GetProcess(void *result)
{
    int hkey;
    int keylen = strlen((char *)result) - 1;
    int i      = 0;
    if ('*' == ((char *)result)[keylen])
    {
        // fprintf(stderr, "last_char=%c \n\r", ((char *)result)[keylen]);
        // fprintf(stdout, "0\n");
        // fprintf(stdout, "OK\n");
        // fprintf(stderr, "0\n");
        // fprintf(stderr, "OK\n");
        for (i = 0; i < CTBLSIZE; i++)
        {
            if (0 == strncmp(ctbl[i].pkey, (char *)result, keylen))
            {
                // int sublen = strlen("Camera.Menu.");
                fprintf(stdout, "%s=%s\n\r", ctbl[i].pkey, ctbl[i].pval);
                fprintf(stderr, "%s=%s\n\r", ctbl[i].pkey, ctbl[i].pval);
            }
        }
        fprintf(stdout, "0\n");
        fprintf(stdout, "OK\n");
        fprintf(stderr, "0\n");
        fprintf(stderr, "OK\n");
    }
    else
    {
        hkey = nvconf_get((char *)result);
        fprintf(stdout, "0\n");
        fprintf(stdout, "OK\n");
        fprintf(stderr, "0\n");
        fprintf(stderr, "OK\n");
        if (hkey >= 0 && ctbl[hkey].pkey != NULL && strcmp((char *)result, ctbl[hkey].pkey) == 0)
        {
            fprintf(stdout, "%s=%s\n\r", ctbl[hkey].pkey, ctbl[hkey].pval);
            fprintf(stderr, "%s=%s\n\r", ctbl[hkey].pkey, ctbl[hkey].pval);
        }
        else
        {
            fprintf(stdout, "no this key\n\r");
            fprintf(stderr, "no this key\n\r");
        }
    }
    return 0;
}

int SetProcess(void *result)
{
    decode(result);
    pentry = &entry;
    nvconf_set(pentry);
    nvconf_commit(NVCONF_CGIPATH);
    fprintf(stdout, "0\n");
    fprintf(stdout, "OK\n");
    fprintf(stderr, "0\n");
    fprintf(stderr, "OK\n");
    return 0;
}

int DelProcess(void *result)
{
    nvconf_del_entry((char *)result);
    nvconf_commit(NVCONF_CGIPATH);
    fprintf(stdout, "0\n");
    fprintf(stdout, "OK\n");
    fprintf(stderr, "0\n");
    fprintf(stderr, "OK\n");
    return 0;
}

//=========================================================================
//=========================================================================

int ProcessGetCgiCmd(CMDNODE *head)
{
    char *   tmpstr, *tmpbuf;
    char *   delim = "&";
    int      is_only_key; // Get key value from GET method
    int      hkey;
    CMDNODE *current = head;
    tmpbuf           = getenv("QUERY_STRING");
    tmpstr           = (char *)strtok(tmpbuf, delim);
    is_only_key      = check_equal_symbol(tmpstr);
    if (is_only_key == 1)
    {
        decode(tmpstr);
        hkey = nvconf_get(tmpstr);
        while (current != NULL && hkey >= 0)
        {
            if (strcmp(entry.pkey, current->key) == 0)
            {
                if (current->cmd == EXECMD)
                {
                    memset(entry.pval, 0, sizeof(entry.pval));
                    (current->callback)(entry.pval);
                    if (entry.pval[0])
                        printf("%s=%s", entry.pkey, entry.pval);
                }
                else
                    printf("%s=%s", entry.pkey, ctbl[hkey].pval);
                break;
            }
            current = current->next;
        }
    }
    else
    {
        decode(tmpstr);
        while (current != NULL)
        {
            if (current->cmd == EXECMD)
            {
                if (strcmp(entry.pkey, current->key) == 0)
                {
                    (current->callback)(entry.pval);
                }
            }
            else if (strcmp(entry.pkey, current->key) == 0)
            {
                pentry = &entry;
                nvconf_set(pentry);
                nvconf_commit(NVCONF_CGIPATH);
                break;
            }
            current = current->next;
        }
    }

    while ((tmpstr = (char *)strtok(NULL, delim)))
    {
        current = head;
        if (is_only_key == 1)
        {
            decode(tmpstr);
            hkey = nvconf_get(tmpstr);
            while (current != NULL && hkey >= 0)
            {
                if (strcmp(entry.pkey, current->key) == 0)
                {
                    if (current->cmd == EXECMD)
                    {
                        memset(entry.pval, 0, sizeof(entry.pval));
                        (current->callback)(entry.pval);
                        if (entry.pval[0])
                            printf("&%s=%s", entry.pkey, entry.pval);
                    }
                    else
                        printf("&%s=%s", entry.pkey, ctbl[hkey].pval);
                }
                current = current->next;
            }
        }
        else
        {
            decode(tmpstr);
            while (current != NULL)
            {
                if (current->cmd == EXECMD)
                {
                    if (strcmp(entry.pkey, current->key) == 0)
                    {
                        (current->callback)(entry.pval);
                    }
                }
                else if (strcmp(entry.pkey, current->key) == 0)
                {
                    pentry = &entry;
                    nvconf_set(pentry);
                    nvconf_commit(NVCONF_CGIPATH);
                }
                current = current->next;
            }
        }
    }

    return 1;
}

int ProcessPostCgiCmd(CMDNODE *head)
{
    char *   tmpstr, *tmpbuf = NULL;
    char *   delim = "&";
    int      length;
    CMDNODE *current = head;
    length           = atoi(getenv("CONTENT_LENGTH"));
    tmpbuf           = malloc(length + 1);

    while ((read(0, tmpbuf, length)) > 0)
        ;
    tmpbuf[length] = '\0';
    tmpstr         = (char *)strtok(tmpbuf, delim);
    decode(tmpstr);
    fprintf(stderr, " entry.pkey  %s\n", entry.pkey);

    while (current != NULL)
    {
        if (strcmp(entry.pkey, current->name) == 0)
        {
            if (current->cmd == EXECMD)
            {
                memset(entry.pval, 0, sizeof(entry.pval));
                (current->callback)(entry.pval);
                POSTCMD = 1;
                break;
            }
            else
            {
                memset(entry.pkey, '\0', sizeof(entry.pkey));
                snprintf(entry.pkey, sizeof(entry.pkey), current->key);
                pentry = &entry;
                nvconf_set(pentry);
                nvconf_commit(NVCONF_CGIPATH);
                break;
            }
        }
        current = current->next;
    }
    while ((tmpstr = (char *)strtok(NULL, delim)))
    {
        current = head;
        decode(tmpstr);
        while (current != NULL && (strcmp(current->name, "") != 0))
        {
            if (strcmp(entry.pkey, current->name) == 0)
            {
                memset(entry.pkey, '\0', sizeof(entry.pkey));
                snprintf(entry.pkey, sizeof(entry.pkey), current->key);
                pentry = &entry;
                nvconf_set(pentry);
                nvconf_commit(NVCONF_CGIPATH);
                break;
            }
            current = current->next;
        }
    }
    free(tmpbuf);
    return 1;
}

void FreeCgiList(CMDNODE *head)
{
    CMDNODE *tmp;
    while (head != NULL)
    {
        tmp  = head;
        head = head->next;
        free(tmp);
    }
}

int ProcessCgiCmd(CMDNODE *head, char *name, char *returnpage)
{
    int method = 0; // method:0 GET ,1 POST
    load_nvconf(NVCONF_CGIPATH);
    // nvconf_all();
    method = check_http_method();
    if (method == 0)
    { // GET
        ProcessGetCgiCmd(head);
    }
    else
    { // POST
        ProcessPostCgiCmd(head);
        if (!POSTCMD)
            set_complete(name, returnpage);
    }
    FreeCgiList(head);
    return 0;
}