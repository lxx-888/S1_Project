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
    TXZAS_WAIT_USB_MOUNT,           //�ȴ�usb����
    TXZAS_CREATE_PUSH,              //����push�ļ�
    TXZAS_WAIT_POP,                 //�ȴ�pop�ļ�
    TXZAS_SUCCESS,                  //����ɹ�
    TXZAS_FAILED,                   //����ʧ��
    TXZAS_GET_UUID_ERROR,           //��ȡΨһ�����
    TXZAS_ACTIVATE_INFO_ERROR,      //������Ϣ���� ��Ҫ��־����TXZ����
    TXZAS_POP_DATA_PARSE_ERROR,     //pop���ݽ�������
    TXZAS_MAX,
};
/**
 * ͬ����֪ͨ״̬�ص�
 *  @param 
 *      status:@see TXZActivateStatus
 * */
typedef void (*TXZ_ACTIVATE_STATUS_NOTIFY_FUNC)(
        cmd_int32_t status);

/**
 * ͬ���߻�ȡusb����״̬
 * @return
 *      0 ���سɹ�������Ϊ����ʧ��
 * */
typedef int (*TXZ_USB_MOUNT_STATUS_FUNC)(
        void);

/**
 * ���ü�����Ϣ�洢�ص��������Զ��洢����
 * @param
 *  load_callback����ȡ������Ϣ�ص���ͬ����ͨ���ý�ڶ�ȡ������Ϣ������Ȩ��
 *  save_callback�����漤����Ϣ�ص���ͬ����ͨ���ý�ڱ��漤����Ϣ��������һ����Ȩ
 * @return
 *  ����Ϊ�ɹ�
 * */
int txzEnineSetStorageFunc(
        FuncLoad load_callback,
        FuncSave save_callback);

/**
 * ����״̬֪ͨ�ص�����txzEngineCheckLicense����ǰ����
 * @param
 *      func:@see TXZ_ACTIVATE_STATUS_NOTIFY_FUNC
 * @return
 *      0 Ϊ���óɹ�������Ϊ����ʧ��
 * */
int txzEnineSetActivateStatusNotifyFunc(
        TXZ_ACTIVATE_STATUS_NOTIFY_FUNC func);

/**
 * ����usb����״̬������ͬ�������ڻ�ȡ����״̬����txzEngineCheckLicense����ǰ����
 * @param
 *      func:@see TXZ_USB_MOUNT_STATUS_FUNC
 * @return
 *      0 ���óɹ�������Ϊ����ʧ��
 * */
int txzEnineSetUsbMountStatusFunc(
        TXZ_USB_MOUNT_STATUS_FUNC func);

/**
 * ͬ���߼���ӿ�
 * channel���������� ��NULL
 * token�������̵�Ψһ�룬��ͬ�����ṩ
 * dir_txz_save��������Ϣ�洢��·����Ĭ�ϼ�����Ϣ�����ļ�����ʹ��txzEngineSetStorageFunc�޸Ĵ洢��ʽ���ò�����Ч
 * dir_usb_push��ע�⣬�����ԡ�/����β
 *              1��usb/sd��ģʽ��Ϊusb/sd��·����������pc������Ϣ�����ļ�����
 *              2��uartģʽ�����ڼ���뼤��߽���������Ϣ����·����Ҫ��д
 * nIsBlock:�ýӿ��Ƿ�������
 *          0������������Ҫ�û��Լ���ѯ��ֱ������ɹ�Ϊֹ��
 *          1������������ɹ���Ÿýӿڲ��˳����ڲ�����ѯ���ƣ���ʹ��txzEngineStopCheckLicenseֹͣ������
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
 * ���õ��������������ȣ���ͬ���ߵ�ͬ�½�����ʹ��
 *
 * */
cmd_int32_t txzEngineSetCmdThr(
        void *cmdInst,
        uint32_t cmdIndex,
        float thr);

/**
 * ����ֹͣCheckLicense��ѯ�����ú�txzEngineCheckLicense�����˳�
 * */
void txzEngineStopCheckLicense();

/**
 * ������־�ȼ���[3~8];
 * level����־�ȼ�����Χ[3~8];
 * */
void txzEngineSetLogLevel(
        int level);

#ifdef __cplusplus
}
#endif

#endif
