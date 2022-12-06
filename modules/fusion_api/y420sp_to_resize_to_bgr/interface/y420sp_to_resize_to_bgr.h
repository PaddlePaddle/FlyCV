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
 * @brief combine resize and convert nv12 to bgr, which means the src's
 *        data type is nv12, and the data type of dst is bgr the function
 *        first resize the src to the size of dst in the y channel and uv
 *        channel, and then convert YUV pixel formats to bgr pixel formats
 * @param[in] src input data Mat, nv12, u8 data
 * @param[out] dst ouput data Mat, bgr package, u8 data  and INTER_NEAREST
 * @param[in] interpolation Interpolation method, currently only support INTER_LINEAR and INTER_NEAREST
*/
EXTERN_C FCV_API int nv12_to_resize_to_bgr(
        Mat& src,
        Mat& dst,
        Size size = Size(0, 0),
        InterpolationType interpolation = InterpolationType::INTER_LINEAR);

/**
 * @brief combine resize and convert nv21 to bgr, which means the src's data
 *        type is nv21, and the data type of dst is bgr the function first
 *        resize the src to the size of dst in the y channel and uv channel,
 *        and then convert YUV pixel formats to bgr pixel formats
 * @param[in] src input data Mat, nv21, u8 data
 * @param[out] dst ouput data Mat, bgr package, u8 data
 * @param[in] interpolation Interpolation method, currently only support INTER_LINEAR and INTER_NEAREST
*/
EXTERN_C FCV_API int nv21_to_resize_to_bgr(
        Mat& src,
        Mat& dst,
        Size size = Size(0, 0),
        InterpolationType interpolation = InterpolationType::INTER_LINEAR);

G_FCV_NAMESPACE1_END()
