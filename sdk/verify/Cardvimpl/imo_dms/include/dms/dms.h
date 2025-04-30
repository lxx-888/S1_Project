#ifndef DMS_DMSIMPL_H
#define DMS_DMSIMPL_H

#include "imo-core/imo_define.h"

#define IMO_FACE_LANDMARK_POINTS 66

#ifdef __cplusplus
extern "C"
{
#endif
    /**
    * Dms-handle
    */
    typedef struct
    {
        /**
        * 句柄
        */
        void *handle;
    } imo_dms_handle;

    /**
    * Dms状态枚举
    * DmsStatusNone 未检测到状态
    * DmsStatusActivated 状态触发
    */
    typedef enum
    {
        DmsStatusNone = 0,
        DmsStatusActivated = 1,
    }eDmsStatus;

    /**
    * Dms 输入输出数据
    */
    typedef struct
    {
        /* 数据输入 */
        imo_image image;                                         //image 图片数据输入

        /* 数据输出 */
        eDmsStatus is_smoke;                                     //是否抽烟
        eDmsStatus is_phone;                                     //是否打电话
        eDmsStatus is_tired;                                     //是否疲倦, 咪眼
        eDmsStatus is_distracted;                                //是否分神
        eDmsStatus is_yawn;                                      //是否打哈欠
        float progress_rate[5];                                  //各状态预警比例, 范围0 ~ 1.0, 触发即为1.0, 枚举顺序ePROGRESS_INDEX定义
        int detect_face_count;                                   //检测到人脸个数，DMS最大为1, 未检测到人脸为0
        imo_rect face_rect;                                      //检测的人脸框位置
        float points[IMO_FACE_LANDMARK_POINTS * 2];              //人脸关键点位置
    } imo_dms_io;

    /**
    * Dms 标定结果数据
    */
    typedef struct 
    {
        float calib_pitch;  //pitch轴数据
        float calib_yaw;    //yaw轴数据
    }imo_dms_calibration_result;
    /**
    * Dms版本信息
    * version 版本信息
    */
    typedef struct
    {
        char version[128]; //版本信息
    } imo_version_io;

    /**
    * Dms状态预警比例的枚举顺序
    */
    typedef enum
    {
        PROGRESS_INDEX_SMOKE = 0,
        PROGRESS_INDEX_PHONE,
        PROGRESS_INDEX_TIRED,
        PROGRESS_INDEX_DISTRACTED,
        PROGRESS_INDEX_YAWN,
    } ePROGRESS_INDEX;

    typedef struct
    {
        /**
        * 底层跑检测网络时的并发线程数，默认4，主要针对嵌入式，手机等场景，调节此参数，推理速度可能会有变化, Sigmastar平台无需设置此选项
        */
        int num_threads; 
        /**
        * 阈值，用于过滤人脸检测，默认是0.45
        */
        float thresh;
    } imo_dms_config;

    /**
     * Dms初始化
     * @param handle 句柄（out）
     * @param config 配置信息，可为NULL(in)
     * @return >=0:成功 <0:失败
    */
    int DLL_PUBLIC imo_dms_create(imo_dms_handle *handle, imo_dms_config *config);
    /**
     * 更新视频帧，并得到结果
     * @param handle 句柄（由imo_dms_create接口创建）
     * @param  dms_io dms输入输出数据, 包括输入image, 输出各状态和数据， 函数返回值为0时该状态有效
     * @return IMO_API_RET_PERMISSION_DENIED：鉴权失败; IMO_API_RET_ALG_NOT_INIT：算法未初始化; 0:success
     */
    int DLL_PUBLIC imo_dms_exec(const imo_dms_handle *handle, imo_dms_io *dms_io);

    /**
     * Dms-销毁句柄
     * @param handle 句柄（由imo_dms_create接口创建）
     * @return >=0:成功 <0:失败
     */
    int DLL_PUBLIC imo_dms_destroy(imo_dms_handle *handle);

    /**
     * 开始标定, 根据当前驾驶员姿态确定正常驾驶时角度，因摄像头位置安装不同，需标定初始角度
     * @return 0:success
     */
    int DLL_PUBLIC imo_dms_start_calibration();
    /**
     * 标定过程，需驾驶员尽量保持正常驾驶姿态，持续更新视频到标定缓存数据中, 推荐100帧
     * @return 0:success
     */
    int DLL_PUBLIC imo_dms_calibrate_frame(imo_image *image);
    /**
     * 结束标定，计算标定结果
     * @param calibration_result Dms 标定结果数据, 包括pitch, yaw轴偏移数据
     * @return 0:success
     */
    int DLL_PUBLIC imo_dms_stop_calibration(imo_dms_calibration_result *calibration_result);
    /**
     * 设定用户保存的结果，如当前运行有标定过程可无需再进行设置;
     * @param calibration_result Dms 标定结果数据, 包括pitch, yaw轴偏移数据
     * @return 0:success
     */
    int DLL_PUBLIC imo_dms_set_calibration_result(imo_dms_calibration_result *calibration_result);
    /**
     * 获取dms版本号;
     * @return 0:success
     */
    int DLL_PUBLIC imo_dms_get_version(imo_version_io *version_io);

#ifdef __cplusplus
}
#endif
#endif //DMS_DMSIMPL_H
