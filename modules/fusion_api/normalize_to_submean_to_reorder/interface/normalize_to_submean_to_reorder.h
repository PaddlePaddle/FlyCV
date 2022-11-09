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

#include "modules/core/mat/interface/mat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief Combine normalize, hwc to chw, reorder channels three steps to one.
 * @note Caclute value formula: dst_pixel_value = (src_pixel_value - mean_params) / std_params,
 * and then reorder channels if the param [channel_reorder_index] is not empty.
 * @param[in] src           input data Mat, only support package bgr/rgb u8/f32 format.
 * @param[in] mean_params   mean values, 3 float data.
 * @param[in] std_params    std_values, 3 float data.
 * @param[in] channel_reorder_index    Reorder the channel at last step,
 * eg: bgr_package_u8 -> rgb_planar_f32(normailized) need to set this to {2, 0, 1},
 * or set this to {} or {0, 1, 2} will do nothing.
 * @param[out] dst           ouput data Mat, 3 channel, float32 type.
 * @param[in] output_package false(default value) : output planar Mat
 *                           true : output package Mat.
*/
FCV_API int normalize_to_submean_to_reorder(
        const Mat& src,
        const std::vector<float>& mean_params,
        const std::vector<float>& std_params,
        const std::vector<uint32_t>& channel_reorder_index,
        Mat& dst,
        bool output_package = false);

G_FCV_NAMESPACE1_END()
