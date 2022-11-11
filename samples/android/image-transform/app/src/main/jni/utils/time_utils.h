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

#ifndef FLYCV_INCLUDE_SRC_UTILS_TIME_UTILS_H_
#define FLYCV_INCLUDE_SRC_UTILS_TIME_UTILS_H_

#include "time.h"
#include "log_utils.h"

class Time {
public:
    static double get_current_time() {
        struct timespec res;
        clock_gettime(CLOCK_REALTIME, &res);
        return 1000 * res.tv_sec + res.tv_nsec / 1e6;
    }
};

#define TIME_LOGD(tag, time) LOGE("elapse time < %lf ms > at [ %s ] section",time,#tag);

//#ifdef DEBUG
#define TIME_START(tag) \
double start_##tag##_time = Time::get_current_time();

#define TIME_END(tag) \
            double end_##tag##_time = Time::get_current_time(); \
            TIME_LOGD(tag,end_##tag##_time-start_##tag##_time)

#endif //FLYCV_INCLUDE_SRC_UTILS_TIME_UTILS_H_
