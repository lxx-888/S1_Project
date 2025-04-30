/*
    file.c -- File handler

    This module serves static file documents
 */

/********************************* Includes ***********************************/

#include    "goahead.h"

/*********************************** Locals ***********************************/

static char   *websIndex;                   /* Default page name */
static char   *websDocuments;               /* Default Web page directory */

/**************************** Forward Declarations ****************************/

static long CurRangeOffset;
static long CurReadSendSize;
#define SECTOR_SIZE             0x0200 // the size should be 2^n
#define SECTOR_FRAGMENT(s)      (s & (SECTOR_SIZE - 1))
static void fileWriteEvent(Webs *wp);

/*********************************** Code *************************************/

const char THUMB_PATH[] = "/thumb";
const char THUMB_STORE_PATH[] = "/tmp";

int GetThumbFromCarDV(Webs *wp)
{
    FILE *in_fd = NULL;
    FILE *out_fd = NULL;
    char InputFilename[128] = {0,};
    char OutputFilename[128] = {0,};
    bool bThumbnailFound = 0;

    sprintf(InputFilename, "%s", strstr(wp->filename, THUMB_PATH) + sizeof(THUMB_PATH) -1);
    in_fd = fopen(InputFilename, "rb");
    if (!in_fd) {
        fprintf(stderr, "File is not exist. File name: %s\n", InputFilename);
        return -1;
    }

    if (1) {
        char *format = NULL;
        char cmd[128] = {0,};
        int count = 100;
        int ret = 0;

        // demux thumbnail
        sprintf(cmd, "echo thumb %s > /tmp/cardv_fifo", InputFilename);
        system(cmd);

        // check demux done flag
        sprintf(OutputFilename, "%s%s", THUMB_STORE_PATH, strrchr(InputFilename, '/'));
        if (strtok(OutputFilename, ".")) {
            strcat(OutputFilename, ".DONE");
            ret = access(OutputFilename, F_OK);
            while (ret != 0 && count-- > 0) {
                usleep(10000);
                ret = access(OutputFilename, F_OK);
            }

            // succeeded than remove done flag
            if (ret == 0) {
                bThumbnailFound = 1;
                unlink(OutputFilename);
            }
        }
    }

    if (in_fd) fclose(in_fd);
    if (bThumbnailFound)
        return 0;
    else
        return -1;
}

/*!
 * \brief Returns a range of integers from a string.
 *
 * \return Always returns 1.
 */
static int GetNextRange(
    /*! string containing the token / range. */
    char **SrcRangeStr,
    /*! gets the first byte of the token. */
    ulong *FirstByte,
    /*! gets the last byte of the token. */
    ulong *LastByte)
{
    char *Ptr;
    char *Tok;
    int i;
    long long F = -1;
    long long L = -1;
    int Is_Suffix_byte_Range = 1;

    if (*SrcRangeStr == NULL)
        return -1;
    Tok = strtok(*SrcRangeStr, ",");

    if ((Ptr = strstr(Tok, "-")) == NULL)
        return -1;
    *Ptr = ' ';

    sscanf(Tok, "%lld%lld", &F, &L);
    if (F == -1 || L == -1) {
        *Ptr = '-';
        for (i = 0; i < (int)strlen(Tok); i++) {
            if (Tok[i] == '-') {
                break;
            } else if (isdigit(Tok[i])) {
                Is_Suffix_byte_Range = 0;
                break;
            }
        }
        if (Is_Suffix_byte_Range) {
            *FirstByte = (ulong) L;
            *LastByte = (ulong) F;
            return 1;
        }
    }
    *FirstByte = (ulong) F;
    *LastByte = (ulong) L;

    return 1;
}

static bool fileHandler(Webs *wp)
{
    WebsFileInfo    info;
    char            *tmp, *date;
    ssize           nchars;
    int             code;

    assert(websValid(wp));
    assert(wp->method);
    assert(wp->filename && wp->filename[0]);

#if !ME_ROM
    if (smatch(wp->method, "DELETE")) {
        if (unlink(wp->filename) < 0) {
            websError(wp, HTTP_CODE_NOT_FOUND, "Cannot delete the URI");
        } else {
            /* No content */
            websResponse(wp, 204, 0);
        }
    } else if (smatch(wp->method, "PUT")) {
        /* Code is already set for us by processContent() */
        websResponse(wp, wp->code, 0);

    } else
#endif /* !ME_ROM */
    {
        if (strstr(wp->filename, THUMB_PATH)) {
            char inputfile[128] = {0};
            char* filename = NULL;

            sprintf(inputfile, "%s%s", THUMB_STORE_PATH, strrchr(wp->filename, '/'));
            /*
             *  "/thumb/path/filename.xxx" -> "/tmp/filename.JPG"
             */
            filename = strtok(inputfile, ".");
            strcat(filename, ".JPG");
            if (0 != GetThumbFromCarDV(wp)) {
                websError(wp, HTTP_CODE_NOT_FOUND, "%s Not Found thumbnail", wp->path);
                return 1;
            }

            wfree(wp->filename);
            wp->filename = sclone(filename);
            websSetVar(wp, "PATH_TRANSLATED", wp->filename);
            wfree(wp->ext);
            wp->ext = sclone(".jpg");
        }
        /*
            If the file is a directory, redirect using the nominated default page
         */
        if (websPageIsDirectory(wp)) {
            nchars = strlen(wp->path);
            if (wp->path[nchars - 1] == '/' || wp->path[nchars - 1] == '\\') {
                wp->path[--nchars] = '\0';
            }
            tmp = sfmt("%s/%s", wp->path, websIndex);
            websRedirect(wp, tmp);
            wfree(tmp);
            return 1;
        }
        if (websPageOpen(wp, O_RDONLY | O_BINARY, 0666) < 0) {
#if ME_DEBUG
            if (wp->referrer) {
                trace(1, "From %s", wp->referrer);
            }
#endif
            websError(wp, HTTP_CODE_NOT_FOUND, "Cannot open document for: %s", wp->path);
            return 1;
        }
        if (websPageStat(wp, &info) < 0) {
            websError(wp, HTTP_CODE_NOT_FOUND, "Cannot stat page for URL");
            return 1;
        }
        code = 200;
        if (wp->since && info.mtime <= wp->since) {
            code = 304;
            info.size = 0;
        }
        if (wp->IsRangeActive == 1)
        {
            ulong FirstByte = 0, LastByte = 0;
            char RangeHeader[200];
            int rc = 0;

            if (GetNextRange(&wp->contentRange, (ulong*)&FirstByte, (ulong*)&LastByte) != -1) {
                if (info.size < FirstByte) {
                    websError(wp, HTTP_CODE_RANGE_NOT_SATISFIABLE | WEBS_CLOSE, "Invalid range");
                    return 1;
                }
            }

            if (LastByte >= FirstByte) {
                if (strstr(wp->userAgent, "VLC") != NULL) {
                    websResponse(wp, 206, 0);
                    return 1;
                }
                if (LastByte >= info.size)
                    LastByte = info.size - 1;
                /* Data between two range. */
                CurRangeOffset = FirstByte;
                CurReadSendSize = LastByte - FirstByte + 1;
                rc = snprintf(RangeHeader,
                    sizeof(RangeHeader),
                    "bytes %" "lld"
                    "-%" "lld" "/%" "lld",
                    (long long)FirstByte,
                    (long long)LastByte,
                    (long long)info.size);
                if (rc < 0 || (unsigned int) rc >= sizeof(RangeHeader)) {
                    websError(wp, HTTP_CODE_RANGE_NOT_SATISFIABLE | WEBS_CLOSE, "Invalid range");
                    return 1;
                }
            }
            else {
                websError(wp, HTTP_CODE_RANGE_NOT_SATISFIABLE | WEBS_CLOSE, "Invalid range");
                return 1;
            }
            code = 206;
            wfree(wp->contentRange);
            wp->contentRange = sclone(RangeHeader);
            websSetVar(wp, "CONTENT_RANGE", RangeHeader);
        }
        else {
            CurRangeOffset = 0;
            CurReadSendSize = info.size;
        }
        websSetStatus(wp, code);
        if (wp->IsRangeActive == 1)
            websWriteHeaders(wp, CurReadSendSize, 0);
        else
            websWriteHeaders(wp, info.size, 0);
        if ((date = websGetDateString(&info)) != NULL) {
            websWriteHeader(wp, "Last-Modified", "%s", date);
            wfree(date);
        }
        websWriteEndHeaders(wp);

        /*
            All done if the browser did a HEAD request
         */
        if (smatch(wp->method, "HEAD")) {
            websDone(wp);
            return 1;
        }
        if (info.size > 0) {
            websSetBackgroundWriter(wp, fileWriteEvent);
        } else {
            websDone(wp);
        }
    }
    return 1;
}


/*
    Do output back to the browser in the background. This is a socket write handler.
    This bypasses the output buffer and writes directly to the socket.
 */
static void fileWriteEvent(Webs *wp)
{
    char    *buf;
    ssize   len, wrote;
    int     err;
    int n = ME_GOAHEAD_LIMIT_BUFFER;

    assert(wp);
    assert(websValid(wp));

    if ((buf = walloc(ME_GOAHEAD_LIMIT_BUFFER)) == NULL) {
        websError(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "Cannot get memory");
        return;
    }
    if (wp->docfd_written == 0) {
        websPageSeek(wp, CurRangeOffset, SEEK_SET);
        if (SECTOR_FRAGMENT(CurRangeOffset)) {
            // CurRangeOffset is not at alignment of sector, read the fragment to
            // escape the bug that data MAY BE wrong when reading file not at begin of a sector
            n = SECTOR_SIZE - SECTOR_FRAGMENT(CurRangeOffset);
            if (n > ME_GOAHEAD_LIMIT_BUFFER) n = ME_GOAHEAD_LIMIT_BUFFER;
        }
    }
    while ((len = websPageReadData(wp, buf, n)) > 0) {
        if ((wrote = websWriteSocket(wp, buf, len)) < 0) {
            if (wrote == -EWOULDBLOCK || wrote == -EAGAIN) {
                websPageSeek(wp, -len, SEEK_CUR);
            } else {
                /* Will call websDone below */
                wp->state = WEBS_COMPLETE;
            }
            break;
        }
        wp->docfd_written += wrote;
        if (wrote != len) {
            websPageSeek(wp, - (len - wrote), SEEK_CUR);
            break;
        }
        n = ME_GOAHEAD_LIMIT_BUFFER;
        bzero(buf, ME_GOAHEAD_LIMIT_BUFFER);
    }
    wfree(buf);
    if (len <= 0) {
        websDone(wp);
        if (wp->state == WEBS_COMPLETE && strstr(wp->path, THUMB_PATH))
            remove(wp->filename);
    }
}


#if !ME_ROM
PUBLIC bool websProcessPutData(Webs *wp)
{
    ssize   nbytes;

    assert(wp);
    assert(wp->putfd >= 0);
    assert(wp->input.buf);

    nbytes = bufLen(&wp->input);
    wp->putLen += nbytes;
    if (wp->putLen > ME_GOAHEAD_LIMIT_PUT) {
        websError(wp, HTTP_CODE_REQUEST_TOO_LARGE | WEBS_CLOSE, "Put file too large");

    } else if (write(wp->putfd, wp->input.servp, (int) nbytes) != nbytes) {
        websError(wp, HTTP_CODE_INTERNAL_SERVER_ERROR | WEBS_CLOSE, "Cannot write to file");
    }
    websConsumeInput(wp, nbytes);
    return 1;
}
#endif


static void fileClose()
{
    wfree(websIndex);
    websIndex = NULL;
    wfree(websDocuments);
    websDocuments = NULL;
}


PUBLIC void websFileOpen(void)
{
    websIndex = sclone("index.html");
    websDefineHandler("file", 0, fileHandler, fileClose, 0);
}


/*
    Get the default page for URL requests ending in "/"
 */
PUBLIC cchar *websGetIndex(void)
{
    return websIndex;
}


PUBLIC char *websGetDocuments(void)
{
    return websDocuments;
}


/*
    Set the default page for URL requests ending in "/"
 */
PUBLIC void websSetIndex(cchar *page)
{
    assert(page && *page);

    if (websIndex) {
        wfree(websIndex);
    }
    websIndex = sclone(page);
}


/*
    Set the default web directory
 */
PUBLIC void websSetDocuments(cchar *dir)
{
    assert(dir && *dir);
    if (websDocuments) {
        wfree(websDocuments);
    }
    websDocuments = sclone(dir);
}

/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under commercial and open source licenses.
    You may use the Embedthis GoAhead open source license or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.
 */
