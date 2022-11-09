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

#include "flycv_namespace.h"
#include "modules/core/base/interface/macro_ns.h"

#ifdef HAVE_NEON
#include <arm_neon.h>
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#define FCV_EPSILON         (1e-6)
#define FCV_DBL_EPSILON     (1e-9)
#define FCV_PI              (3.1415926535897932384626433832795)
#define FCV_MIN(x, y)       ((x) < (y) ? (x) : (y))
#define FCV_MAX(x, y)       ((x) > (y) ? (x) : (y))
#define FCV_ABS(x)          ((x) < 0 ? -(x) : (x))

#ifdef __aarch64__
#define VCVTAQ_S32_F32(a)   vcvtaq_s32_f32(a)
#else
#define VCVTAQ_S32_F32(a)   (vcvtq_s32_f32(vaddq_f32(a, vbslq_f32(vcgeq_f32(a, vdupq_n_f32(0.f)), \
        vdupq_n_f32(0.5f), vdupq_n_f32(-0.5f)))))
#endif

constexpr signed char S8_MIN_VAL = -128;
constexpr signed char S8_MAX_VAL = 127;
constexpr unsigned char U8_MIN_VAL = 0;
constexpr unsigned char U8_MAX_VAL = 255;
constexpr short S16_MIN_VAL = -32768;
constexpr short S16_MAX_VAL = 32767;
constexpr unsigned short U16_MIN_VAL = 0;
constexpr unsigned short U16_MAX_VAL = 65535;

/**
 * @brief Compute the round of double type data
 * @param[in] value，supported type:double,
 */
static inline int fcv_round(double value) {
    return static_cast<int>(value + (value >= 0 ? 0.5 : -0.5));
}

/**
 * @brief Compute the floor of double type data
 * @param[in] value，supported type:double,
 */
static inline int fcv_floor(double value) {
    int i = static_cast<int>(value);
    return (i - (i > value));
}

/**
 * @brief Compute the ceiling of double type data
 * @param[in] value，supported type:double,
 */
static inline int fcv_ceil(double value) {
    int i = static_cast<int>(value);
    return i + (i < value);
}

/**
 * @brief A value is bounded between an upper and lower bound
 * @param[in] val supported type:int,float,double...
 * @param[in] min supported type:int,float,double...
 * @param[in] max supported type:int,float,double...
 */
template<class T, class D>
static inline constexpr T fcv_clamp(
        const T& val,
        const D& min,
        const D& max) {
    return val < min ? min : (val > max ? max : val);
}

template<class T>
static inline constexpr short fcv_cast_s16(const T& val) {
    return static_cast<short>(fcv_clamp(val, S16_MIN_VAL, S16_MAX_VAL));
}

template<class T>
static inline constexpr unsigned short fcv_cast_u16(const T& val) {
    return static_cast<unsigned short>(fcv_clamp(val, U16_MIN_VAL, U16_MAX_VAL));
}

template<class T>
static inline constexpr signed char fcv_cast_s8(const T& val) {
    return static_cast<signed char>(fcv_clamp(val, S8_MIN_VAL, S8_MAX_VAL));
}

template<class T>
static inline constexpr unsigned char fcv_cast_u8(const T& val) {
    return static_cast<unsigned char>(fcv_clamp(val, U8_MIN_VAL, U8_MAX_VAL));
}

/**
 * @brief Check if two numbers are equal
 * @param[in] a supported type:int,float,double...
 * @param[in] b supported type:int,float,double...
 */
template<class T, class D>
bool is_almost_equal(T a, D b) {
    return FCV_ABS(a - b) <  FCV_EPSILON ? true : false;
}

G_FCV_NAMESPACE1_END()
