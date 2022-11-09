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
 * @brief Calculates an perspective transform from three pairs of the corresponding points.
 * the function first resize the src to the size of dst in the y channel and uv channel, and then convert YUV pixel formats to bgr pixel formats
 * @param[in] src Coordinates of triangle vertices in the source image.
 * @param[out] dst Coordinates of the corresponding triangle vertices in the destination image.
*/
FCV_API Mat get_perspective_transform(const Point2f src[], const Point2f dst[]);

/**
 * @brief the implementation of perspective transformation, supported element types: u8 and f32
 * when the flag #WARP_INVERSE_MAP is set. Otherwise, the transformation is first inverted
 * with #invertAffineTransform and then put in the formula above instead of M. The function cannot operate in-place.
 * @param[in] src source image, supported image type:Mat, the number of channel: 1 ,3.
 * @param[out] dst dst image, supported image type:Mat,the number of channel: 1 ,3
 * @param[in] m affine matrix, the size of matrix should be 3x3.
 * @param[in] flags combination of interpolation methods (#INTER_LINEAR or #INTER_NEAREST or #INTER_AREA) and the
 * optional flag #WARP_INVERSE_MAP, that sets M as the inverse transformation.
 * @param[in] border_method pixel extrapolation method (#BORDER_CONSTANT).
 * @param[in] borderValue value used in case of a constant border; by default, it equals 0.
 */
FCV_API int warp_perspective(
        const Mat& src,
        Mat& dst,
        Mat& m,
        InterpolationType flag = InterpolationType::INTER_LINEAR,
        BorderTypes border_method = BorderTypes::BORDER_CONSTANT,
        const Scalar borderValue = {0});

G_FCV_NAMESPACE1_END()
