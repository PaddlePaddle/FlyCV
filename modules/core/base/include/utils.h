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

#pragma once

#include "modules/core/base/interface/basic_types.h"
#include "modules/core/base/include/macro_platforms.h"
#include "modules/core/base/include/macro_utils.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template<typename T>
int32_t sign(T val) {
    return (val > T(0)) - (val < T(0));
}

template<typename T>
inline T dot_product(Point_<T> first, Point_<T> second) {
    return first.x() * second.x() + first.y() * second.y();
}

inline void prefetch(const void *ptr, size_t offset = 32 * 10) {
#if !defined _MSC_VER
    __builtin_prefetch(reinterpret_cast<const char *>(ptr) + offset);
#endif
}

inline void prefetch_l1(const void *ptr, size_t offset) {
#if defined FCV_OS_ANDROID || defined FCV_OS_APPLE
#if __aarch64__
    asm volatile(
        "prfm pldl1keep, [%0, %1]\n\t"
        :
        : "r"(ptr), "r"(offset)
        : "cc", "memory");
#else
    asm volatile(
        "pld   [%0, %1]       \n\t"
        :
        : "r"(ptr), "r"(offset)
        : "cc", "memory");
#endif
#else
    UN_USED(ptr);
    UN_USED(offset);
#endif
}

G_FCV_NAMESPACE1_END()
