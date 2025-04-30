#ifndef _IMO_LANE_DETECTOR_API_H
#define _IMO_LANE_DETECTOR_API_H

#include "imo-core/imo_define.h"
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************IMO-DETECT-DEFINE***********************************/

#define DETECT_MODEL_WIDTH (512)
#define DETECT_MODEL_HEIGHT (288)

/**
* 道路线检测-handle
*/
typedef struct {
    /**
     * 句柄
     */
    void *handle;
} imo_lane_detector_handle;

typedef struct {

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

} imo_lane_detector_config;


/**
 * 道路线检测-道路线信息
 */
typedef struct {

    imo_image img;                   // image 图片数据

    imo_image_orientation ori;       //orientation 图片朝向 IMO_IMAGE_UP

    // 检测道路线
    unsigned char *output_lane_masks[5];
} imo_lane_detector_io;



/***********************************IMO-API***********************************/

/**
 * 道路线检测初始化
 * @param handle 句柄（out）
 * @param config 参数配置，默认填nullptr即可
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_lane_detector_create(imo_lane_detector_handle *handle, const imo_lane_detector_config *config);

/**
 * 道路线检测-检测图片
 * @param handle 句柄（由imo_lane_detector_create接口创建）
 * @param ios  输入输出数组
 * @param size  ios数组大小
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_lane_detector_exec(const imo_lane_detector_handle *handle, imo_lane_detector_io * ios , int size);


/**
 * 道路线检测-销毁句柄
 * @param handle 句柄（由imo_lane_detector_create接口创建）
 * @return >=0:成功 <0:失败
 */
int DLL_PUBLIC imo_lane_detector_destroy(imo_lane_detector_handle *handle);

#ifdef __cplusplus
}
#endif

#endif //_IMO_LANE_DETECTOR_API_H
