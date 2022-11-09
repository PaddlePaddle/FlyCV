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

#include "modules/img_calculation/mean/interface/mean.h"
#include "modules/img_calculation/mean/include/mean_common.h"

#ifdef HAVE_NEON
#include "modules/img_calculation/mean/include/mean_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

Scalar mean(const Mat& src) {
    if (src.empty()) {
        LOG_ERR("The input mat is empty!");
        return -1;
    }

#ifdef HAVE_NEON
    return mean_neon(src);
#else
    return mean_common(src);
#endif
}

Scalar mean(const Mat& src, const Mat& mask) {
    if (mask.empty()) {
        return mean(src);
    }

    if (src.empty()) {
        LOG_ERR("The input mat is empty!");
        return Scalar::all(-1);
    }

    int src_len = src.stride() * src.height() /
            (src.channels() * src.type_byte_size());
    int mask_len = mask.stride() * mask.height() /
            (mask.channels() * mask.type_byte_size());

    if (src_len != mask_len || mask.type() != FCVImageType::GRAY_U8) {
        LOG_ERR("The size of mask should be the same with src "
                "and the data type must be u8!");
        return Scalar::all(-1);
    }

#ifdef HAVE_NEON
    return mean_neon(src, mask);
#else
    return mean_common(src, mask);
#endif
}

Scalar mean(const Mat& src, const Rect& rect) {
    if (src.empty()) {
        LOG_ERR("The input mat is empty!");
        return Scalar::all(-1);
    }

    if (rect.width() < 0 || rect.height() < 0) {
        LOG_ERR("The width and height of the rect must be positive numbers!");
        return Scalar::all(-1);
    }

#ifdef HAVE_NEON
    return mean_neon(src, rect);
#else
    return mean_common(src, rect);
#endif
}

void mean_stddev(const Mat& src, Mat& mean, Mat& stddev) {
    if (src.empty()) {
        LOG_ERR("The input mat is empty!");
        return;
    }

#ifdef HAVE_NEON
    mean_stddev_neon(src, mean, stddev);
#else
    mean_stddev_common(src, mean, stddev);
#endif
}

G_FCV_NAMESPACE1_END()
