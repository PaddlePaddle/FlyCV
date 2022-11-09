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
 * @brief interpolation types.
 */
enum class FlipType {
    X = 0,  // flip along the X axis
    Y,      // flip along the Y axis
    XY,     // flip along the XY axis
};

/**
 * @brief the implementation of perspective transformation, supported element types: u8 and f32
 * The function flips an input image fromvertical, horizontal, or both axes to another
 * @param[in] src input image, supported image type:Mat, the number of channel: 1 ,3
 * @param[out] dst output image, supported image type:Mat, the number of channel: 1 ,3
 * @param[in] type a flag to specify how to flip the array;
 * 0 means flipping around the x-axis and positive value (for example, 1) means
 * flipping around y-axis. (for example, 2) means flipping around both axes.
 */
FCV_API int flip(const Mat& src, Mat& dst, FlipType type);

G_FCV_NAMESPACE1_END()
