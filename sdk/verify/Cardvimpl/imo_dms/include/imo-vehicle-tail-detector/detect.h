#ifndef _IMO_VEHICLE_TAIL_DETECTOR_API_H
#define _IMO_VEHICLE_TAIL_DETECTOR_API_H

#include "imo-core/imo_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************IMO-DETECT-DEFINE***********************************/

// 车尾检测结果的点位数量
#define IMO_VEHICLE_TAIL_DETECTOR_VEHICLE_TAIL_INFO_POINTS_COUNT    5

/**
* 车尾检测-handle
*/
typedef struct {
    /**
     * 句柄
     */
    void *handle;
} imo_vehicle_tail_detector_handle;

typedef struct {
    /**
     * 最小车尾尺寸，未来可能会舍弃此参数
     */
    int min_vehicle_tail_size;
#if defined(__ANDROID__) || defined(__APPLE__)
    /**
     * 检测车尾点位时是否开启高质量检测，默认true
     */
    bool predict_high_quality_points;
#endif

    /**
     * 底层跑检测网络时的并发线程数，默认4，主要针对嵌入式，手机等场景，调节此参数，推理速度可能会有变化
     */
    int num_threads;
  
    /**
     * 最大GPU限制，默认为0.3, 此参数主要用于nvidia GPU.
     */
    float max_gpu;
    /**
     * 阈值，用于过滤框，默认是0.45
     */
    float thresh;

} imo_vehicle_tail_detector_config;

/**
 * 车尾检测-单个车尾信息
 */
typedef struct {
    /**
     * 车尾点位[X1,Y1,X2,Y2,...,X5, Y5](左眼/右眼/鼻尖/左嘴角/右嘴角)
     */
    imo_point points[IMO_VEHICLE_TAIL_DETECTOR_VEHICLE_TAIL_INFO_POINTS_COUNT];

    /**
     * 车尾框
     */
    imo_rect rect;

    /**
     * confidence of rect
     */
    float  score;
} imo_vechile_tail_detector_tail_info;

/**
 * 车尾检测-多车尾信息
 */
typedef struct {

    imo_image img;                   // image 图片数据

    imo_image_orientation ori;       //orientation 图片朝向 IMO_IMAGE_UP

    /**
     * 车尾信息数组 (in & out)
     */
    imo_vechile_tail_detector_tail_info *vehicle_tail_infos;

    // 客户端定义最大检测的车尾数，vehicle_tail_infos数组的长度 (in)
    unsigned int input_size;

    // 检测到的车尾个数 (out)
    unsigned int output_size;
} imo_vehicle_tail_detector_io;



/***********************************IMO-API***********************************/

/**
 * 车尾检测初始化
 * @param handle 句柄（out）
 * @param config 参数配置，默认填nullptr即可
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_vehicle_tail_detector_create(imo_vehicle_tail_detector_handle *handle, const imo_vehicle_tail_detector_config *config);

/**
 * 车尾检测-检测图片
 * @param handle 句柄（由imo_vehicle_tail_detector_create接口创建）
 * @param ios  输入输出数组。　一个io 也就是一个待检测图片和图片相关的车尾信息结构体（一个图片上可能有多个车尾）
 * @param size  ios数组大小
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_vehicle_tail_detector_exec(const imo_vehicle_tail_detector_handle *handle, imo_vehicle_tail_detector_io * ios , int size);


/**
 * 车尾检测-销毁句柄
 * @param handle 句柄（由imo_vehicle_tail_detector_create接口创建）
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_vehicle_tail_detector_destroy(imo_vehicle_tail_detector_handle *handle);

#ifdef __cplusplus
}
#endif

#endif //_IMO_VEHICLE_TAIL_DETECTOR_API_H
