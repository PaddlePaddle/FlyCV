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

#ifndef MARLIN_INCLUDE_CORE_MACRO_H
#define MARLIN_INCLUDE_CORE_MACRO_H
#include <stdio.h>
#include <stdlib.h>

// MARLIN profile
#ifndef MARLIN_PROFILE
#define MARLIN_PROFILE 0
#endif

// Interface visibility
#if defined _WIN32 || defined __CYGWIN__
#ifdef BUILDING_DLL
#ifdef __GNUC__
#define PUBLIC __attribute__((dllexport))
#else
#define PUBLIC __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PUBLIC __attribute__((dllimport))
#else
#define PUBLIC __declspec(dllimport)
#endif
#endif
#define LOCAL
#else
#if __GNUC__ >= 4
#define PUBLIC __attribute__((visibility("default")))
#define LOCAL __attribute__((visibility("hidden")))
#else
#define PUBLIC
#define LOCAL
#endif
#endif

// DATAPRECISION
// float IEEE 754
#ifndef FLT_MIN
#define FLT_MIN 1.175494351e-38F
#define FLT_MAX 3.402823466e+38F
#define FLT_EPSILON 1.192092896e-07F
#endif
// int8
#ifndef INT8_MIN
#define INT8_MIN ((int8_t)-128)
#endif
#ifndef INT8_MAX
#define INT8_MAX ((int8_t)127)
#endif

// BREAK_IF
#define BREAK_IF(cond)                                                                                                 \
    if (cond)                                                                                                          \
    break
#ifdef __OPTIMIZE__
#define BREAK_IF_MSG(cond, msg)                                                                                        \
    if (cond)                                                                                                          \
    break
#else
#define BREAK_IF_MSG(cond, msg)                                                                                        \
    if (cond)                                                                                                          \
        LOGD(msg);                                                                                                     \
    if (cond)                                                                                                          \
    break
#endif

// Math
#ifndef UP_DIV
#define UP_DIV(x, y) (((int)(x) + (int)(y) - (1)) / (int)(y))
#endif
#ifndef ROUND_UP
#define ROUND_UP(x, y) (((int)(x) + (int)(y) - (1)) / (int)(y) * (int)(y))
#endif
#ifndef ALIGN_UP4
#define ALIGN_UP4(x) ROUND_UP((x), 4)
#endif
#ifndef ALIGN_UP8
#define ALIGN_UP8(x) ROUND_UP((x), 8)
#endif
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#ifndef ABS
#define ABS(x) ((x) > (0) ? (x) : (-(x)))
#endif

#if (__arm__ || __aarch64__) && (defined(__ARM_NEON__) || defined(__ARM_NEON))
#define MARLIN_USE_NEON
#endif

#define RETURN_VALUE_ON_NEQ(status, expected, value)                  \
    do {                                                                                                         \
        auto _status = (status);                                                                         \
        if (_status != (expected)) {                                                                     \
            return (value);                                                                                 \
        }                                                                                                          \
    } while (0)

#define RETURN_ON_NEQ(status, expected)                                         \
    do {                                                                                                        \
        auto _status = (status);                                                                        \
        if (_status != (expected)) {                                                                    \
            return _status;                                                                               \
        }                                                                                                         \
    } while (0)

#define CHECK_PARAM_NULL(param)                                                   \
    do {                                                                                                         \
        if (!param) {                                                                                        \
            return Status(MARLINERR_PARAM_ERR, "Error: param is nil");                                                    \
        }                                                                                                          \
    } while (0)

#if defined(__GNUC__) || defined(__clang__)
#define DEPRECATED(msg) __attribute__((deprecated (msg)))
#elif defined(_MSC_VER)
#define DEPRECATED(msg) __declspec(deprecated (msg))
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED
#endif

#endif //MARLIN_INCLUDE_BASE_MACRO_H
