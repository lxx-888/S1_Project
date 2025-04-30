#pragma once

#include <stdint.h>
#include <memory.h>
#include "imo-core/imo_define.h"


#ifdef __cplusplus
extern "C" {
#endif

/***********************************IMO-DEFINE***********************************/
#define IMO_SMART_DESK_LAMP_FACE_POINTS_COUNT 66

typedef enum{
    Sitting_Status_Unknown = -1,        //-1:未知
    Sitting_Status_Not_Standard = 0,    //0:不规范坐姿
    Sitting_Status_Standard = 1,        //1:正确坐姿
}imo_sitting_status;

typedef enum{
    CALIBRATION_STATUS_UNINITIAL       = 0,         //0:未标定/无人脸标定失败
    CALIBRATION_STATUS_CALIBRATING     = 1,         //1:正在标定
    CALIBRATION_STATUS_DONE            = 2          //2:已标定
}imo_calibration_status;


typedef enum{
    Monitoring_Status_Unknown                   = -1,     //未知
    Monitoring_Status_Outof_Region              =  0,     //完全在监控区域外
    Monitoring_Status_Partin_Region             =  1,     //部分在监控区域内
    Monitoring_Status_Completely_In_Region      =  2,     //完全在监控区域内
}imo_monitoring_status;

typedef enum{
    Monitoring_Event_None           = 0,    //无事件
    Monitoring_Event_Enter_Region   = 1,    //进入监控区域
    Monitoring_Event_Leave_Region   = 2,    //离开监控区域
}imo_monitoring_event;

typedef enum{
    Face_Detect_Nobody      = 0,        //未检测到人脸
    Face_Detect_Detected    = 1,        //检测到人脸
}imo_face_detect_result;

typedef struct 
{
    int64_t anybody_duration;                                       //有人持续时间（毫秒）
    int64_t nobody_duration;                                        //无人持续时间（毫秒）
    imo_monitoring_status current_monitoring_status;                //当前帧是否在监控区域内
    imo_monitoring_status pre_monitoring_status;                    //上一帧是否在监控区域内
    imo_monitoring_event monitoring_event;                          //监控区域内有无进入、离开事件发生。
    float head_position_in_camera[3];                               //头部相对相机的3D坐标（厘米）(x, y, z)
    float head_height_to_desktop;                                   //头部相对桌面的高度（厘米）
    imo_sitting_status sitting_status;                              //当前帧坐姿是否合规
    int64_t abnormal_duration;                                      //不规范坐姿持续时间（毫秒）
    imo_rect monitoring_region;                                     //监控区域范围
    imo_face_detect_result face_detect_result;                      //当前帧是否检测到人脸
    imo_rect detect_face_rect;                                      //检测到的人脸最小外接框
    imo_point landmark[IMO_SMART_DESK_LAMP_FACE_POINTS_COUNT];      //检测到的人脸关键点
}imo_smart_desk_lamp_info;

typedef struct 
{
    imo_face_detect_result faceDetectResult;                    //当前帧是否检测到人脸
    float fPitch;                                               //人脸pitch俯仰角度, 即抬头低头角度
    float fYaw;                                                 //人脸yaw偏航角度，即左右转动角度
    float fRoll;                                                //人脸roll滚转角度，即歪头角度
    imo_point pts2d[IMO_SMART_DESK_LAMP_FACE_POINTS_COUNT];     //人脸2d点
    imo_point_3d pts3d[IMO_SMART_DESK_LAMP_FACE_POINTS_COUNT];  //人脸3d点
    imo_rect bbox;                                              //检测到的人脸最小外接框
}imo_face_info;

typedef struct _timo_smart_desk_lamp_param
{
    //监控区域相对人脸框大小的倍数系数
    float monitoring_region_scale_width;
    float monitoring_region_scale_height;

    //头部位置偏移量阈值（x左右，y上下，z前后，厘米）
    float offset_x_cm;
    float offset_y_cm;
    float offset_z_cm;

    //头部姿态角度变化量阈值（pitch俯仰，yaw偏航，roll滚转，角度）
    float pitch_diff_angle;
    float yaw_diff_angle;
    float roll_diff_angle;

    //初始化参数时间阈值(秒)
    float anybody_stats_init;
    float anybody_stats_age;
    float sitting_stats_reset;
    float sitting_stats_age;

    //人脸大小缩放系数（计算的距离是相对一般人脸大小的，所以会出现用户脸小计算距离偏大、用户脸大计算距离偏小的情况，可以通过该系数调节）
    float face_model_scale;

    //相机内参
    float camera_matrix[9];
    //相机相对桌面的安装高度
    float camera_height_cm;
    //相机光轴方向相对水平方向的安装角度
    float camera_pitch_angle;
    
    imo_calibration_status calib_status;
	_timo_smart_desk_lamp_param()
	{
        //监控区域相对人脸框大小的倍数系数
        monitoring_region_scale_width = 2.0;
        monitoring_region_scale_height = 1.5;

        //头部位置偏移量阈值（x左右，y上下，z前后）
        offset_x_cm = 2.0;
        offset_y_cm = 4.0;
        offset_z_cm = 15.0;

        //头部姿态角度变化量阈值（pitch俯仰，yaw偏航，roll滚转，角度）
        pitch_diff_angle = 20.0;
        yaw_diff_angle = 30.0;
        roll_diff_angle = 12.0;

        //初始化参数时间阈值(秒)
        anybody_stats_init = 2.0;
        anybody_stats_age = 5.0;
        sitting_stats_reset = 5.0;
        sitting_stats_age = 5.0;

        //人脸大小缩放系数（计算的距离是相对一般人脸大小的，所以会出现用户脸小计算距离偏大、用户脸大计算距离偏小的情况，可以通过该系数调节）
        face_model_scale = 1.0;

        //相机内参
        float inner_matrix[] = {640, 0, 320, 0, 640, 240, 0, 0, 1};
        memcpy(camera_matrix, inner_matrix, sizeof(camera_matrix));
        //相机相对桌面的安装高度
        camera_height_cm = 10.0;
        //相机光轴方向相对水平方向的安装角度
        camera_pitch_angle = 25;

        calib_status = CALIBRATION_STATUS_UNINITIAL;
	}
} imo_smart_desk_lamp_param;

/**
 * 智能台灯算法-handle
 */
typedef struct {
    /**
     * 句柄
     */
    void *handle;
} imo_smart_desk_lamp_handle;


/**
 * 智能台灯算法配置参数
 */
typedef struct {
    imo_smart_desk_lamp_param config_param;
    imo_face_info calib_face_data;
} imo_smart_desk_lamp_config;


/**
* 智能台灯算法版本信息
* version 版本信息
*/
typedef struct
{
    char version[128]; //版本信息
} imo_smart_desk_lamp_version_io;
/***********************************IMO-API***********************************/

/**
 * 智能台灯算法初始化
 * @param handle 句柄（out）
 * @param config 参数配置
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_smart_desk_lamp_create(imo_smart_desk_lamp_handle *handle, const imo_smart_desk_lamp_config &config);

/**
 * 智能台灯算法执行
 * @param handle 句柄（由imo_smart_desk_lamp_create接口创建）
 * @param image 图片数据
 * @param output 算法输出结果
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_smart_desk_lamp_exec(const imo_smart_desk_lamp_handle &handle, const imo_image &image, imo_smart_desk_lamp_info *output);
                               
/**
 * 智能台灯算法-销毁句柄
 * @param handle 句柄（由imo_smart_desk_lamp_create接口创建）
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_smart_desk_lamp_destroy(const imo_smart_desk_lamp_handle &handle);

/**
 * 智能台灯算法标定开始
 * @param handle 句柄（由imo_smart_desk_lamp_create接口创建）
 * @param config 算法参数
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_smart_desk_lamp_calib_start(const imo_smart_desk_lamp_handle &handle, imo_smart_desk_lamp_config *config);

/**
 * 智能台灯算法标定执行
 * @param handle 句柄（由imo_smart_desk_lamp_create接口创建）
 * @param image 图片数据
 * @param output 算法输出结果
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_smart_desk_lamp_calib_exec(const imo_smart_desk_lamp_handle &handle, const imo_image &image, imo_smart_desk_lamp_info *output);

/**
 * 智能台灯算法标定结束
 * @param handle 句柄（由imo_smart_desk_lamp_create接口创建）
 * @param config 算法参数
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_smart_desk_lamp_calib_stop(const imo_smart_desk_lamp_handle &handle, imo_smart_desk_lamp_config *config);

/**
 * 智能台灯算法-重设算法参数
 * @param handle 句柄（由imo_smart_desk_lamp_create接口创建）
 * @param config 已保存需要直接设定到算法内部的参数
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_smart_desk_lamp_reset(const imo_smart_desk_lamp_handle &handle, const imo_smart_desk_lamp_config &config);

/**
 * 获取智能台灯算法版本号;
 * @return 0:success
 */
int DLL_PUBLIC imo_smart_desk_lamp_get_version(imo_smart_desk_lamp_version_io *version_io);

#ifdef __cplusplus
}
#endif

