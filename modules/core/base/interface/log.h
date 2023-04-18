// Copyright (c) 2021 FlyCV Authors. All Rights Reserved.
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

#pragma once

#ifdef __ANDROID__
#include <android/log.h>
#else
#include <cstdio>
#endif

#include "flycv_namespace.h"
#include "macro_export.h"
#include "macro_ns.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief set log status
 * @param[in] status 0: disable log output; 1: enable log output
 */
FCV_API void set_log_status(int status);

/**
 * @brief get log status
 * @param[in] status 0: disable log output; 1: enable log output
 */
FCV_API int get_log_status();

G_FCV_NAMESPACE1_END()

#ifdef __ANDROID__

#define LOG_ERR(fmt, ...) \
    if (g_fcv_ns::get_log_status()) { \
        do { \
            __android_log_print(ANDROID_LOG_ERROR, "flycv --error-- ", "<line %d: %s> " \
            fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
        } while (0); \
    }

#else

#define LOG_ERR(fmt, ...) \
    if (g_fcv_ns::get_log_status()) { \
        do { \
            printf("flycv --error-- <line %d: %s> " fmt "\n", __LINE__, __FUNCTION__, ##__VA_ARGS__); \
        } while (0); \
    }

#endif