/***************************************************************************
 *
 * Copyright (c) 2022 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @contributor     litonghui.huweidong01
 * @created         2022-08-31 16:44:00
 * @brief
 */

#ifndef MARLIN_INCLUDE_SRC_UTILS_TIME_UTILS_H_
#define MARLIN_INCLUDE_SRC_UTILS_TIME_UTILS_H_

#include "modules/core/opencl/include/utils/log_utils.h"

namespace ocl {

class MarlinTime {
public:
    static double get_current_time();
};

#define MARLIN_TIME_LOGD(tag, time) LOGI("elapse time < %lf ms > at [ %s ] section",time,#tag);

#ifdef DEBUG
#define MARLIN_TIME_START(tag) \
double start_##tag##_time = ocl::MarlinTime::get_current_time();

#define MARLIN_TIME_END(tag) \
            double end_##tag##_time = ocl::MarlinTime::get_current_time(); \
            MARLIN_TIME_LOGD(tag,end_##tag##_time-start_##tag##_time)
#else
#define MARLIN_TIME_START(tag)
#define MARLIN_TIME_END(tag)
#define MARLIN_TIME_LOGD(tag,time)
#endif
}

#endif //MARLIN_INCLUDE_SRC_UTILS_TIME_UTILS_H_
