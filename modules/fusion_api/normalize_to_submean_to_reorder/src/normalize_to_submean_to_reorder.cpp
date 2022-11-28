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

#include "modules/fusion_api/normalize_to_submean_to_reorder/interface/normalize_to_submean_to_reorder.h"

#include "modules/fusion_api/normalize_to_submean_to_reorder/include/normalize_to_submean_to_reorder_common.h"

#ifdef HAVE_NEON
#include "modules/fusion_api/normalize_to_submean_to_reorder/include/normalize_to_submean_to_reorder_arm.h"
#endif
#include "modules/fusion_api/normalize_to_submean_to_reorder/include/normalize_to_submean_to_reorder_avx.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int normalize_to_submean_to_reorder(
        const Mat& src,
        const std::vector<float>& mean_params,
        const std::vector<float>& std_params,
        const std::vector<uint32_t>& channel_reorder_index,
        Mat& dst,
        bool output_package) {
    // Common step: check params legal
    if (src.empty()) {
        LOG_ERR("The src data is empty!");
        return -1;
    }
    constexpr const int CHANNELS_NUM = 3;
    if ((src.channels() != CHANNELS_NUM)
            || (std_params.size() != CHANNELS_NUM)
            || (mean_params.size() != CHANNELS_NUM)) {
        LOG_ERR("Input mat channel is not equal to param size");
        return -1;
    }

    if ((dst.channels() != src.channels())
            || (dst.width() != src.width())
            || (dst.height() != src.height())
            || (dst.type() < FCVImageType::PLA_BGR_F32)
            || (dst.type() > FCVImageType::PKG_RGB_F32)) {
        FCVImageType dst_type = output_package ? FCVImageType::PKG_RGB_F32 : FCVImageType::PLA_BGR_F32;
        dst = Mat(src.width(), src.height(), dst_type);
    }

#ifdef HAVE_NEON
    return normalize_to_submean_to_reorder_neon(src, mean_params,
            std_params, channel_reorder_index, dst, output_package);
#elif defined(HAVE_AVX)
    return normalize_to_submean_to_reorder_avx(src, mean_params,
            std_params, channel_reorder_index, dst, output_package);
#else
    return normalize_to_submean_to_reorder_common(src, mean_params,
            std_params, channel_reorder_index, dst, output_package);
#endif
}

G_FCV_NAMESPACE1_END()