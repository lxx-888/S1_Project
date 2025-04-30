/*************************************************
 *
 * Copyright (C) 2018 Sigmastar Technology Corp.
 * All rights reserved.
 *
 **************************************************
 * File name:  module_gps.c
 * Author:     jeff.li@sigmastar.com.cn
 * Version:    Initial Draft
 * Date:       2019/3/22
 * Description: gps module source file
 *
 *
 *
 * History:
 *
 *    1. Date  :        2019/3/22
 *       Author:        jeff.li@sigmastar.com.cn
 *       Modification:  Created file
 *
 **************************************************/

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <linux/serial.h>
#include <nmea/nmea.h>
#include <errno.h>

#include "module_common.h"
#include "module_gps.h"
#include "module_mux.h"
#include "mid_mux.h"

#if (CARDV_EDOG_ENABLE)
extern "C" {
#include "edog_ctl.h"
}

//==============================================================================
//
//                              MACRO DEFINES
//
//==============================================================================

#define SPEEM_CAM_MANUAL_DB     ("SpeedCam_Data_Manual.bin") // TBD. Should be configed by API.
#define MAPLAYOUT_HEADERNAME    ("SpeedCam_FileHeader.bin")
#define MAPLAYOUT_BASENAME      ("DB")
#define MAPLAYOUT_EXTNAME       (".bin")
#define EDOG_DB_DYNAMIC_SEGMENT (0)
#endif

#define GPS_CHUNK_MAX_CNT (32)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

#if (CARDV_GPS_ENABLE)
pthread_t g_gpsThread = 0;
BOOL      g_gpsThread_exit;

const char *_port         = "/dev/ttyS1";
int         _baud         = 115200;
int         _rts_cts      = 0;
int         _2_stop_bit   = 0;
int         _parity       = 0;
int         _odd_parity   = 0;
int         _stick_parity = 0;
int         _no_rx        = 0;
int         _no_tx        = 1;
int         _fd           = -1;
#endif

GPSINFOCHUCK     gstGpsInfoChuck    = {0};
GPSINFOCHUCKTIME gstGpsLastUtc      = {0};
MI_U8 *          gpu8GpsCompBuf     = NULL;
MI_U32           gu32GpsChunkWrIdx  = 0;
MI_BOOL          gbGpsChunkStartRec = FALSE;
#if (CARDV_GPS_ENABLE)
static pthread_mutex_t _gstGpsMutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#if (CARDV_EDOG_ENABLE)
EDOG_HANDLE *pEdogHdl = NULL;
#endif

extern MI_S32    g_s32SocId;
extern MI_Muxer *g_muxerArray[];

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

#if (CARDV_GPS_ENABLE)
void set_baud_divisor(int speed)
{
    // default baud was not found, so try to set a custom divisor
    struct serial_struct ss;
    if (ioctl(_fd, TIOCGSERIAL, &ss) != 0)
    {
        printf("TIOCGSERIAL failed\n");
        return;
    }

    ss.flags          = (ss.flags & ~ASYNC_SPD_MASK) | ASYNC_SPD_CUST;
    ss.custom_divisor = (ss.baud_base + (speed / 2)) / speed;
    int closest_speed = ss.baud_base / ss.custom_divisor;

    if (closest_speed < speed * 98 / 100 || closest_speed > speed * 102 / 100)
    {
        printf("Cannot set speed to %d, closest is %d\n", speed, closest_speed);
        return;
    }

    printf("closest baud = %i, base = %i, divisor = %i\n", closest_speed, ss.baud_base, ss.custom_divisor);

    if (ioctl(_fd, TIOCSSERIAL, &ss) < 0)
    {
        printf("TIOCSSERIAL failed\n");
        return;
    }
}

// converts integer baud to Linux define
int get_baud(int baud)
{
    switch (baud)
    {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    case 460800:
        return B460800;
    case 500000:
        return B500000;
    case 576000:
        return B576000;
    case 921600:
        return B921600;
    case 1000000:
        return B1000000;
    case 1152000:
        return B1152000;
    case 1500000:
        return B1500000;
    case 2000000:
        return B2000000;
    case 2500000:
        return B2500000;
    case 3000000:
        return B3000000;
    case 3500000:
        return B3500000;
    case 4000000:
        return B4000000;
    default:
        return -1;
    }
}

void setup_serial_port(int baud)
{
    struct termios newtio;

    _fd = open(_port, O_RDWR); // | O_NONBLOCK

    if (_fd < 0)
    {
        printf("Error opening serial port \n");
        return;
    }

    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

    /* man termios get more info on below settings */
    newtio.c_cflag = baud | CS8 | CLOCAL | CREAD;

    if (_rts_cts)
    {
        newtio.c_cflag |= CRTSCTS;
    }

    if (_2_stop_bit)
    {
        newtio.c_cflag |= CSTOPB;
    }

    if (_parity)
    {
        newtio.c_cflag |= PARENB;
        if (_odd_parity)
        {
            newtio.c_cflag |= PARODD;
        }
        if (_stick_parity)
        {
            newtio.c_cflag |= CMSPAR;
        }
    }

    newtio.c_iflag = 0;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;

    // block for up till 128 characters
    newtio.c_cc[VMIN] = 128;

    // 0.5 seconds read timeout
    newtio.c_cc[VTIME] = 5;

    /* now clean the modem line and activate the settings for the port */
    tcflush(_fd, TCIOFLUSH);
    tcsetattr(_fd, TCSANOW, &newtio);
}

void GpsSetInfo2Chunk(nmeaINFO *pInfo)
{
    /* GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive) */
    if (pInfo->sig == 0)
    {
        time_t     timep;
        struct tm *p;
        time(&timep);
        p                           = localtime(&timep);
        gstGpsInfoChuck.dwLat       = GPS_NOSIGNAL_MARK_DOUBLE;
        gstGpsInfoChuck.dwLon       = GPS_NOSIGNAL_MARK_DOUBLE;
        gstGpsInfoChuck.ubDirection = GPS_NOSIGNAL_MARK_BYTE;
        gstGpsInfoChuck.lAlt        = GPS_NOSIGNAL_MARK_LONG;
        gstGpsInfoChuck.usSpeed     = GPS_NOSIGNAL_MARK_SHORT;
        gstGpsInfoChuck.sUTC.byYear = p->tm_year;
        gstGpsInfoChuck.sUTC.byMon  = p->tm_mon;
        gstGpsInfoChuck.sUTC.byDay  = p->tm_mday;
        gstGpsInfoChuck.sUTC.byHour = p->tm_hour;
        gstGpsInfoChuck.sUTC.byMin  = p->tm_min;
        gstGpsInfoChuck.sUTC.bySec  = p->tm_sec;
        gstGpsInfoChuck.ubFlag      = FALSE;
        gstGpsInfoChuck.ubVersion   = GPS_NOSIGNAL_MARK_BYTE;
        gstGpsInfoChuck.ubReserved  = GPS_NOSIGNAL_MARK_BYTE;
    }
    else
    {
        gstGpsInfoChuck.dwLat       = pInfo->lat;
        gstGpsInfoChuck.dwLon       = pInfo->lon;
        gstGpsInfoChuck.ubDirection = (unsigned char)(pInfo->direction / 2);
        gstGpsInfoChuck.lAlt        = (long)pInfo->elv;
        gstGpsInfoChuck.usSpeed     = (unsigned short)pInfo->speed;
        gstGpsInfoChuck.sUTC.byYear = (unsigned char)pInfo->utc.year;
        gstGpsInfoChuck.sUTC.byMon  = (unsigned char)pInfo->utc.mon;
        gstGpsInfoChuck.sUTC.byDay  = (unsigned char)pInfo->utc.day;
        gstGpsInfoChuck.sUTC.byHour = (unsigned char)pInfo->utc.hour;
        gstGpsInfoChuck.sUTC.byMin  = (unsigned char)pInfo->utc.min;
        gstGpsInfoChuck.sUTC.bySec  = (unsigned char)pInfo->utc.sec;
        gstGpsInfoChuck.ubFlag      = TRUE;
        gstGpsInfoChuck.ubVersion   = GPS_CONFIG_INFOSTRUC_VER;
        gstGpsInfoChuck.ubReserved  = 0;
    }
}
#endif

unsigned char *GpsGetChunk(int *pSize)
{
    if (pSize)
        *pSize = sizeof(GPSINFOCHUCK);
    return (unsigned char *)&gstGpsInfoChuck;
}

#if (CARDV_EDOG_ENABLE)
EDOG_ERR EDOGIO_FS_OPEN_Wrap(char *pFileName, char *pMode, unsigned int *ulFileID)
{
    *ulFileID = (unsigned int)fopen(pFileName, pMode);
    if (*ulFileID == NULL)
    {
        printf("%s is %s: %s\n", __FUNCTION__, pFileName, strerror(errno));
        return EDOG_ERR_FS_FAIL;
    }
    return EDOG_ERR_NONE;
}

EDOG_ERR EDOGIO_FS_CLOSE_Wrap(unsigned int ulFileID)
{
    int nRet = 0;
    nRet     = fclose((FILE *)ulFileID);
    if (nRet != 0)
    {
        printf("%s is: %s\n", __FUNCTION__, strerror(errno));
        return EDOG_ERR_FS_FAIL;
    }
    return EDOG_ERR_NONE;
}

EDOG_ERR EDOGIO_FS_READ_Wrap(unsigned int ulFileID, unsigned char *pData, unsigned int NumBytes,
                             unsigned int *read_count)
{
    int nRet    = 0;
    nRet        = fread(pData, 1, NumBytes, (FILE *)ulFileID);
    *read_count = nRet;
    if (nRet < 0)
    {
        printf("%s is: %s\n", __FUNCTION__, strerror(errno));
        return EDOG_ERR_FS_FAIL;
    }
    return EDOG_ERR_NONE;
}

EDOG_ERR EDOGIO_FS_WRITE_Wrap(unsigned int ulFileID, unsigned char *pData, unsigned int NumBytes,
                              unsigned int *write_count)
{
    int nRet     = 0;
    nRet         = fwrite(pData, 1, NumBytes, (FILE *)ulFileID);
    *write_count = nRet;
    if (nRet < 0)
    {
        printf("%s is: %s\n", __FUNCTION__, strerror(errno));
        return EDOG_ERR_FS_FAIL;
    }
    return EDOG_ERR_NONE;
}

EDOG_ERR EDOGIO_FS_SEEK_Wrap(unsigned int ulFileID, long long llOffset, int lOrigin)
{
    int nRet = 0;
    nRet     = fseek((FILE *)ulFileID, llOffset, lOrigin);
    if (nRet != 0)
    {
        printf("%s is: %s\n", __FUNCTION__, strerror(errno));
        return EDOG_ERR_FS_FAIL;
    }
    return EDOG_ERR_NONE;
}

EDOG_ERR EDOGIO_FS_GetFileSize_Wrap(unsigned int ulFileID, unsigned int *ulFileSize)
{
    int         nRet = 0;
    FILE *      file;
    int         fd;
    char        path[1024];
    char        pFileName[1024];
    struct stat Fstat;

    file = (FILE *)ulFileID;
    fd   = fileno(file);
    sprintf(path, "/proc/self/fd/%d", fd);
    memset(pFileName, 0, sizeof(pFileName));
    readlink(path, pFileName, sizeof(pFileName) - 1);
    nRet = stat(pFileName, &Fstat);
    if (nRet < 0)
    {
        printf("%s is %s: %s\n", __FUNCTION__, pFileName, strerror(errno));
        return EDOG_ERR_FS_FAIL;
    }
    *ulFileSize = Fstat.st_size;
    return EDOG_ERR_NONE;
}

EDOG_BOOL EDOGIO_GPS_IsAttached_Wrap(void)
{
    return EDOG_TRUE;
}

void EDOGIO_LED_LcdBacklight_Wrap(EDOG_BOOL bOn) {}

EDOG_BOOL EDOGIO_LED_GetBacklightStatus_Wrap(void)
{
    return EDOG_FALSE;
}

EDOG_BOOL EDOGIO_GPS_IsValidValue_Wrap(void)
{
    return EDOG_TRUE;
}

void EDOGIO_RTC_GetTime_Wrap(EDOG_RTC_TIME *sEdogRTC_Time)
{
    time_t     now;
    struct tm *timenow;
    time(&now);
    timenow                     = localtime(&now);
    sEdogRTC_Time->uwYear       = timenow->tm_year;
    sEdogRTC_Time->uwMonth      = timenow->tm_mon;
    sEdogRTC_Time->uwDay        = timenow->tm_mday;
    sEdogRTC_Time->uwDayInWeek  = timenow->tm_wday;
    sEdogRTC_Time->uwHour       = timenow->tm_hour;
    sEdogRTC_Time->uwMinute     = timenow->tm_min;
    sEdogRTC_Time->uwSecond     = timenow->tm_sec;
    sEdogRTC_Time->ubAmOrPm     = timenow->tm_hour < 12 ? 0 : 1;
    sEdogRTC_Time->b_12FormatEn = 0;
}

double EDOGIO_GPS_Dmm2Degree_DBL_Wrap(double dblLatLonDmm)
{
    return nmea_ndeg2degree(dblLatLonDmm);
}

double EDOGIO_GPS_Degree2Dmm_DBL_Wrap(double dblLatLonDmm)
{
    return nmea_degree2ndeg(dblLatLonDmm);
}

float EDOGIO_GPS_Dmm2Degree_Float_Wrap(float dflLatLonDmm)
{
    return (float)nmea_ndeg2degree(dflLatLonDmm);
}

float EDOGIO_GPS_Degree2Dmm_Float_Wrap(float dflLatLonDmm)
{
    return (float)nmea_degree2ndeg(dflLatLonDmm);
}

EDOG_BOOL EDOGIO_GPS_IsEof(void)
{
    return EDOG_FALSE;
}

void EDOGIO_GPS_ResetEof(void) {}

void EDOGCtrl_Operation(int *param, int paramLen)
{
    if (g_gpsThread_exit == TRUE)
    {
        printf("start GPS first!\n");
        return;
    }
    printf("EDOGCtrl_Operation: %d\n", param[0]);
    switch (param[0])
    {
    case 0:
        EDOGCtrl_Initial();
        break;
    case 1:
        EDOGCtrl_ListAllCameraInfo();
        break;
    case 2: // add_manual
    {
        MANUALSPEEDCAMERA *Temp;
        unsigned int       ulTotalIndex;

        if (param[1] == 0 || param[1] > 5)
            param[1] = 1;
        while (param[1])
        {
            if (EDOGCtrl_GetAddSpeedCamPOI())
            {
                if (EDOGCtrl_AddPOI_Manual(pEdogHdl))
                {
                    EDOGCtrl_GetTotalIndex(&ulTotalIndex);
                    Temp = (MANUALSPEEDCAMERA *)EDOGCtrl_GetPOI_Manual(ulTotalIndex - 1);
                    printf("%d: dwLat = %f  dwLon = %f  uwYear = %d \r\n", ulTotalIndex - 1, Temp->dwLat, Temp->dwLon,
                           Temp->sRTC.uwYear);
                }
            }
            else
            {
                printf("Please check Manual file exit first!\r\n");
            }
            param[1]--;
        }
    }
    break;
    case 3:
        EDOGCtrl_RestoesPOItoFile_Manual();
        break;
    case 4:
        EDOGCtrl_ClearAllManualSetting();
        break;
    case 5:
        EDOGCtrl_ParseHeader(pEdogHdl);
        break;
    case 6: // delete_manual
        EDOGCtrl_DeletPOI_Manual(param[1]);
        break;
    case 7:
        EDOGCtrl_UpdateSpeedCameraArray_ALL();
        break;
    case 8:
        EDOGCtrl_SetEnEdogLog(param[1]);
        EDOGCtrl_GetEdogVersion();
        break;
    default:
        break;
    }
}
#else
void EDOGCtrl_Operation(int *param, int paramLen)
{
    return;
}
#endif

#if (CARDV_GPS_ENABLE)

void GpsStartRec(MI_BOOL bStart)
{
    pthread_mutex_lock(&_gstGpsMutex);
    gbGpsChunkStartRec = bStart;
    gu32GpsChunkWrIdx  = 0;
    pthread_mutex_unlock(&_gstGpsMutex);
}

MI_U32 GpsMove2CompBuf(MI_U8 *pu8SrcBuf)
{
    MI_U32 u32DstAddr = 0;

    if (gu32GpsChunkWrIdx >= GPS_CHUNK_MAX_CNT)
        return 0;

    for (int i = 0; i < MAX_MUXER_NUMBER; i++)
    {
        if (g_muxerArray[i] == NULL)
            continue;
        if (g_muxerArray[i]->IsMuxerEnable() && g_muxerArray[i]->IsGpsFrameQFull())
            return 0;
    }

    u32DstAddr = (MI_U32)(gpu8GpsCompBuf + gu32GpsChunkWrIdx * sizeof(GPSINFOCHUCK));
    memcpy((MI_U8 *)u32DstAddr, pu8SrcBuf, sizeof(GPSINFOCHUCK));
    gu32GpsChunkWrIdx = (gu32GpsChunkWrIdx + 1) % GPS_CHUNK_MAX_CNT;
    return u32DstAddr;
}

void *GpsTask(void *argv)
{
    int           baud = B9600;
    struct pollfd serial_poll;

    nmeaINFO   info;
    nmeaPARSER parser;
    nmeaPOS    dpos;

    CARDV_THREAD();

    if (_baud)
        baud = get_baud(_baud);

    if (baud <= 0)
    {
        printf("NOTE: non standard baud rate, trying custom divisor\n");
        baud = B38400;
        setup_serial_port(B38400);
        set_baud_divisor(_baud);
    }
    else
    {
        setup_serial_port(baud);
    }

    memset(&serial_poll, 0, sizeof(struct pollfd));
    serial_poll.fd = _fd;
    if (!_no_rx)
    {
        serial_poll.events |= POLLIN;
    }
    else
    {
        serial_poll.events &= ~POLLIN;
    }

    if (!_no_tx)
    {
        serial_poll.events |= POLLOUT;
    }
    else
    {
        serial_poll.events &= ~POLLOUT;
    }

    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    gpu8GpsCompBuf = (MI_U8 *)MALLOC(sizeof(GPSINFOCHUCK) * GPS_CHUNK_MAX_CNT);
    MuxerSetGpsChunkSize(sizeof(GPSINFOCHUCK));

#if (CARDV_EDOG_ENABLE)
    unsigned int SpeedCamMFile = 0;
    EDOGCtrl_SetEnEdogLog(0); // EDOG DEBUG
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_PRINTF, (void *)printf);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_OPEN, (void *)EDOGIO_FS_OPEN_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_CLOSE, (void *)EDOGIO_FS_CLOSE_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_READ, (void *)EDOGIO_FS_READ_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_WRITE, (void *)EDOGIO_FS_WRITE_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_SEEK, (void *)EDOGIO_FS_SEEK_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_FS_GET_FILESIZE, (void *)EDOGIO_FS_GetFileSize_Wrap);
    // EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_GETINFORMATION, (void *)EDOGIO_GPS_GetInfor_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_ISATTACHED, (void *)EDOGIO_GPS_IsAttached_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_LED_CTRL_LCDBACKLIGHT, (void *)EDOGIO_LED_LcdBacklight_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_LED_CTRL_BACKLIGHT_STATUS,
                                  (void *)EDOGIO_LED_GetBacklightStatus_Wrap);

    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_ISVALIDVALUE, (void *)EDOGIO_GPS_IsValidValue_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_DMM2DEGREE_DBL, (void *)EDOGIO_GPS_Dmm2Degree_DBL_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_DEGREE2DMM_DBL, (void *)EDOGIO_GPS_Degree2Dmm_DBL_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_DMM2DEGREE_FLOAT, (void *)EDOGIO_GPS_Dmm2Degree_Float_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_DEGREE2DMM_FLOAT, (void *)EDOGIO_GPS_Degree2Dmm_Float_Wrap);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_ISEOF, (void *)EDOGIO_GPS_IsEof);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_GPS_RESETEOF, (void *)EDOGIO_GPS_ResetEof);
    EDOGCtrl_Register_IOFunctions(EDOGIO_FUNC_ENUM_RTC_GETTIME, (void *)EDOGIO_RTC_GetTime_Wrap);
#if 1
    EDOGCtrl_SetPoiDataType(1); // for China
    EDOGCtrl_SetBaseName((char *)MAPLAYOUT_BASENAME);
    EDOGCtrl_SetExtName((char *)MAPLAYOUT_EXTNAME);
    EDOGCtrl_SetMapHeaderName((char *)MAPLAYOUT_HEADERNAME);
#else
    EDOGCtrl_SetPoiDataType(0); // 0:for other
    EDOGCtrl_SetSpCamDBFName(SPEED_CAM_DB_FILE_NAME);
#endif
    EDOGCtrl_SetSpCamManuDBFName((char *)SPEEM_CAM_MANUAL_DB);
    if (EDOG_ERR_NONE == EDOGIO_FS_OPEN_Wrap((char *)SPEEM_CAM_MANUAL_DB, (char *)"rb", &SpeedCamMFile))
    {
        EDOGCtrl_EnAddSpeedCamPOI(1);
        EDOGIO_FS_CLOSE_Wrap(SpeedCamMFile);
    }
    else
        EDOGCtrl_EnAddSpeedCamPOI(0);

    EDOGCtrl_SetDistMethod(DIST_METHOD_GBL_CIRCLE_DIST);
    EDOGCtrl_SetSpeedLvl(0);
    EDOGCtrl_SetSpeedDist(1000);
#endif

    while (g_gpsThread_exit == FALSE)
    {
        int retval = poll(&serial_poll, 1, 1000);
        if (retval == -1)
        {
            printf("poll error.\n");
            continue;
        }
        else if (retval)
        {
            char rb[1024];
            int  c = read(_fd, rb, sizeof(rb));
            if (c > 0)
            {
                nmea_parse(&parser, rb, c, &info);
                nmea_info2pos(&info, &dpos);
                // printf("NMEA time= %d-%d-%d %d:%d:%d\n", info.utc.year, info.utc.mon, info.utc.day, info.utc.hour,
                // info.utc.min, info.utc.sec);
                // printf("degree  lat:%+010.6lf, lon:%+011.6lf\n", nmea_radian2degree(dpos.lat),
                // nmea_radian2degree(dpos.lon));
                GpsSetInfo2Chunk(&info);
#if (CARDV_EDOG_ENABLE)
                {
                    POSITIONINFO *pEDOGInfo = (POSITIONINFO *)EDOGCtrl_Information();
                    GPS_DATA_BUF  gpsInfo;

                    gpsInfo.h_dec       = (EDOG_HANDLE *)&info;
                    gpsInfo.dwLat       = info.lat;
                    gpsInfo.dwLon       = info.lon;
                    gpsInfo.dwSpeed     = info.speed;
                    gpsInfo.dwDirection = info.direction;
                    pEdogHdl            = (EDOG_HANDLE *)&gpsInfo;

                    EDOGCtrl_Handler((EDOG_HANDLE *)&gpsInfo);
                    EDOGCtrl_ParseHeader((EDOG_HANDLE *)&gpsInfo);
                    if (EDOGCtrl_IsSPCAMFound())
                    {
                        CARDV_INFO("#EDOG: Lat:%f,Lon:%f\r\n", pEDOGInfo->fLockLat, pEDOGInfo->fLockLon);
                        EDOGCtrl_CurrentDist();
                    }
                }
#endif
            }
        }
        else
        {
            nmea_zero_INFO(&info);
            GpsSetInfo2Chunk(&info);
            // printf("No data within one second.\n");
        }

        if (memcmp(&gstGpsLastUtc, &gstGpsInfoChuck.sUTC, sizeof(GPSINFOCHUCKTIME)))
        {
            memcpy(&gstGpsLastUtc, &gstGpsInfoChuck.sUTC, sizeof(GPSINFOCHUCKTIME));

            pthread_mutex_lock(&_gstGpsMutex);
            if (gbGpsChunkStartRec)
            {
                MI_U32 u32FrameBuf = GpsMove2CompBuf((MI_U8 *)&gstGpsInfoChuck);
                if (u32FrameBuf)
                {
                    MI_U64 u64Pts;
                    MI_SYS_GetCurPts(g_s32SocId, &u64Pts);
                    for (int i = 0; i < MAX_MUXER_NUMBER; i++)
                    {
                        if (g_muxerArray[i] == NULL)
                            continue;
                        if (g_muxerArray[i]->IsMuxerEnable())
                            g_muxerArray[i]->PushInfoToGpsFrameQ(u32FrameBuf, u64Pts);
                    }
                }
            }
            pthread_mutex_unlock(&_gstGpsMutex);
        }
    }

    FREEIF(gpu8GpsCompBuf);
    nmea_parser_destroy(&parser);

    tcdrain(_fd);
    tcflush(_fd, TCIOFLUSH);
    close(_fd);
    _fd = -1;
    pthread_exit(NULL);
}
#endif

void GpsStartThread(void)
{
#if (CARDV_GPS_ENABLE)
    int ret          = 0;
    g_gpsThread_exit = FALSE;

    if (0 != g_gpsThread)
    {
        CARDV_ERR("%s alread start\n", __func__);
        return;
    }

    ret = pthread_create(&g_gpsThread, NULL, GpsTask, NULL);
    if (0 == ret)
    {
        pthread_setname_np(g_gpsThread, "gps_task");
    }
    else
    {
        CARDV_ERR("%s pthread_create failed\n", __func__);
    }
#endif
}

void GpsStopThread(void)
{
#if (CARDV_GPS_ENABLE)
    g_gpsThread_exit = TRUE;
    pthread_join(g_gpsThread, NULL);
    g_gpsThread = 0;
#endif
}