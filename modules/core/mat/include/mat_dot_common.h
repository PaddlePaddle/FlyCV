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

#include "modules/core/mat/interface/mat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template<typename T>
inline double dot_product(
        const T* src1,
        const T* src2,
        int len) {
    double result = 0;
    double result0 = 0;
    double result1 = 0;

    int i = 0;
    for (; i <= len - 8; i += 8) {
        result0 += (double)src1[i] * src2[i] +
                (double)src1[i + 1] * src2[i + 1] +
                (double)src1[i + 2] * src2[i + 2] +
                (double)src1[i + 3] * src2[i + 3];

        result1 += (double)src1[i + 4] * src2[i + 4] +
                (double)src1[i + 5] * src2[i + 5] +
                (double)src1[i + 6] * src2[i + 6] +
                (double)src1[i + 7] * src2[i + 7];
    }

    for(; i < len; ++i) {
        result += (double)src1[i] * src2[i];
    }

    return (result + result0 + result1);
}

double dot_common(const Mat& ma, const Mat& mb);

G_FCV_NAMESPACE1_END()
