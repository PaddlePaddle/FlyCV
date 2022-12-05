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
 * @brief Calculates an affine transform from three pairs of the corresponding points,
 * supported element types: u8 and f32
 * the function first resize the src to the size of dst in the y channel and uv channel,
 * and then convert YUV pixel formats to bgr pixel formats
 * @param[in] src Coordinates of triangle vertices in the source image.
 * @param[out] dst Coordinates of the corresponding triangle vertices in the destination image.
*/
FCV_API Mat get_affine_transform(const Point2f src[], const Point2f dst[]);

/**
 * @brief Calculates an affine matrix of 2D rotation, supported element types: u8 and f32
 * @param[in] center Center of the rotation in the source image.
 * @param[in] angle  Rotation angle in degrees.)Positive values mean counter-clockwise rotation (the
 * coordinate origin is assumed to be the top-left corner
 * @param[in] scale isotropic scale factor.
 */
FCV_API Mat get_rotation_matrix_2D(Point2f center, double angle, double scale);

/**
 * @brief the implementation of affine transformation, supported element types: u8 and f32
 * @param[in] src source image, supported image type:Mat, the number of channel: 1 ,3.
 * @param[out] dst dst image, supported image type:Mat,the number of channel: 1 ,3
 * @param[in] m affine matrix, the size of matrix should be 2x3.
 * @param[in] flag Interpolation method, see @InterpolationType, currently only support INTER_LINEAR.
 * @param[in] border_method Make border method, see @BorderType, currently only support BORDER_CONSTANT.
 * @param[in] border_value Value used in case of a constant border, by default, set it to 0.
 */
EXTERN_C FCV_API int warp_affine(
        const Mat& src,
        Mat& dst,
        Mat& m,
        InterpolationType flag = InterpolationType::INTER_LINEAR,
        BorderType border_method = BorderType::BORDER_CONSTANT,
        const Scalar borderValue = {0});
G_FCV_NAMESPACE1_END()
