// Copyright (c) 2022 FlyCV Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
