#ifndef __TXZ_ENGINE_H__
#define __TXZ_ENGINE_H__
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#ifdef __cplusplus
extern "C"
{
#endif

/*-----------------------------------------------------*/
/*   Error codes                                       */
/*-----------------------------------------------------*/
#define CMD_CODE_NOVALID_LICENSE -5
#define CMD_CODE_ERROR_PARAMETER -4
#define CMD_CODE_ERROR_NULLPTR   -3
#define CMD_CODE_ERROR_GENER     -2
#define CMD_CODE_ERROR_UNINIT    -1
#define CMD_CODE_NORMAL           0
#define CMD_CODE_PROCESSEND       1

typedef unsigned char cmd_uint8_t;
typedef signed char cmd_int8_t;

typedef unsigned short cmd_uint16_t;
typedef signed short cmd_int16_t;

typedef unsigned int cmd_uint32_t;
typedef signed int cmd_int32_t;

typedef float cmd_float32_t;

typedef enum cmd_paramterId_e {
    cmd_shiftFrames,   /* cmd_uint16_t  : 1 (0~10) */
    cmd_smoothFrames,  /* cmd_uint16_t  : 30 (0~30) */
    cmd_lockFrames,    /* cmd_uint16_t  : 60 (0~100) */
    cmd_postMaxFrames, /* cmd_uint16_t  : 100 (0~100) */
    cmd_thresHold,     /* cmd_float32_t : 0.60f (0.0f~1.0f) */
} cmd_paramterId;

typedef int (*FuncLoad)(
        void *,
        cmd_int32_t);
typedef int (*FuncSave)(
        void *,
        cmd_int32_t);

enum TXZActivateStatus
{
    TXZAS_NONE = 0, //
    TXZAS_WAIT_USB_MOUNT,           //等待usb挂载
    TXZAS_CREATE_PUSH,              //生成push文件
    TXZAS_WAIT_POP,                 //等待pop文件
    TXZAS_SUCCESS,                  //激活成功
    TXZAS_FAILED,                   //激活失败
    TXZAS_GET_UUID_ERROR,           //获取唯一吗错误
    TXZAS_ACTIVATE_INFO_ERROR,      //激活信息错误 需要日志给到TXZ分析
    TXZAS_POP_DATA_PARSE_ERROR,     //pop数据解析错误
    TXZAS_MAX,
};
/**
 * 同行者通知状态回调
 *  @param 
 *      status:@see TXZActivateStatus
 * */
typedef void (*TXZ_ACTIVATE_STATUS_NOTIFY_FUNC)(
        cmd_int32_t status);

/**
 * 同行者获取usb挂载状态
 * @return
 *      0 挂载成功。其他为挂载失败
 * */
typedef int (*TXZ_USB_MOUNT_STATUS_FUNC)(
        void);

/**
 * 设置激活信息存储回调，用于自定存储区。
 * @param
 *  load_callback：读取激活信息回调，同行者通过该借口读取激活信息进行授权。
 *  save_callback：保存激活信息回调，同行者通过该借口保存激活信息，用于下一次授权
 * @return
 *  非零为成功
 * */
int txzEnineSetStorageFunc(
        FuncLoad load_callback,
        FuncSave save_callback);

/**
 * 设置状态通知回调。在txzEngineCheckLicense调用前设置
 * @param
 *      func:@see TXZ_ACTIVATE_STATUS_NOTIFY_FUNC
 * @return
 *      0 为设置成功，其他为设置失败
 * */
int txzEnineSetActivateStatusNotifyFunc(
        TXZ_ACTIVATE_STATUS_NOTIFY_FUNC func);

/**
 * 设置usb挂载状态函数，同行者用于获取挂载状态。在txzEngineCheckLicense调用前设置
 * @param
 *      func:@see TXZ_USB_MOUNT_STATUS_FUNC
 * @return
 *      0 设置成功，其他为设置失败
 * */
int txzEnineSetUsbMountStatusFunc(
        TXZ_USB_MOUNT_STATUS_FUNC func);

/**
 * 同行者激活接口
 * channel：暂无意义 填NULL
 * token：方案商的唯一码，有同行者提供
 * dir_txz_save：激活信息存储的路径，默认激活信息存在文件。如使用txzEngineSetStorageFunc修改存储方式，该参数无效
 * dir_usb_push：注意，必须以“/”结尾
 *              1、usb/sd卡模式：为usb/sd卡路径，用于与pc激活信息交换文件区域。
 *              2、uart模式：用于间接与激活工具交换数据信息，该路径需要可写
 * nIsBlock:该接口是否阻塞，
 *          0：非阻塞，需要用户自己轮询，直到激活成功为止。
 *          1：阻塞，激活成功后才该接口才退出，内部做轮询机制，可使用txzEngineStopCheckLicense停止阻塞。
 * */
cmd_int32_t txzEngineCheckLicense(
        void *channel,
        const char *token,
        const char *dir_txz_save,
        const char *dir_usb_push,
        int nIsBlock);
/**
 *  This function get keyword name.
 *
 * @param
 *      index      -I  : Keyword index.
 *      name       -IO : Pointer pointer to keyword name (not has copy operation) or NULL.
 * @return
 *      Returns CMD_CODE_NORMAL if success or other error code.
 */

cmd_int32_t txzEngineGetName(
        cmd_uint16_t index,
        const char** name);

/**
 * Allocates the memory needed by the CMD.
 *
 * @param
 *      cmdInst -IO: Pointer to the created object or NULL with error.
 *
 */
void txzEngineCreate(
        void** cmdInst);

/**
 *  Initializes the CMD instance.
 *
 * @param
 *      cmdInst    -IO : Pointer to the CMD instance.
 * @return
 *      Returns CMD_CODE_NORMAL if success or other error code.
 */
cmd_int32_t txzEngineCmdInit(
        void* cmdInst);

/**
 *  Reset the CMD instance.
 *
 * @param
 *      cmdInst    -IO : Pointer to the CMD instance.
 * @return
 *      Returns CMD_CODE_NORMAL if success or other error code.
 */
cmd_int32_t txzEngineReset(
        void* cmdInst);

/**
 * Search keyword on one frame of data.
 *
 * @param
 *      cmdInst        -IO : Pointer to the CMD instance.
 *      audio          -I  : In buffer containing one frame of recording signal (Number of samples must be 160).
 *                           If "audio" set NULL means end of process.
 *      samples        -I  : Number of samples in audio buffer, must be 160 now.
 *      keyIndex       -IO : Index greater than 0 means keyword has been searched, call txzEngineGetName() to review the keyword name.
 * @return
 *      Returns CMD_CODE_NORMAL or CMD_CODE_PROCESSEND if success or other error code
 */
cmd_int32_t txzEngineProcess(
        void* cmdInst,
        const cmd_int16_t* audio,
        cmd_uint16_t samples,
        cmd_uint16_t* keyIndex,
        float* confidence);

/**
 * This function releases the memory allocated by txzEngineCreate().
 *
 * @param
 *     uvoiceCMD -IO: Pointer to the CMD instance.
 */
void txzEngineDesrtoy(
        void** cmdInst);

/**
 * 设置单个词条的灵敏度，在同行者的同事建议下使用
 *
 * */
cmd_int32_t txzEngineSetCmdThr(
        void *cmdInst,
        uint32_t cmdIndex,
        float thr);

/**
 * 用于停止CheckLicense轮询，调用后txzEngineCheckLicense马上退出
 * */
void txzEngineStopCheckLicense();

/**
 * 设置日志等级。[3~8];
 * level：日志等级，范围[3~8];
 * */
void txzEngineSetLogLevel(
        int level);

#ifdef __cplusplus
}
#endif

#endif
