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

#include "modules/core/cmat/interface/cmat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief Calculates an perspective transform from three pairs of the corresponding points.
 * @param[in] src Coordinates of triangle vertices in the source image.
 * @param[out] dst Coordinates of the corresponding triangle vertices in the destination image.
*/
EXTERN_C FCV_API CMat* fcvGetPerspectiveTransform(CPoint2f src[], CPoint2f dst[]);

/**
 * @brief the implementation of perspective transformation, supported element types: u8 and f32
 * when the flag #WARP_INVERSE_MAP is set. Otherwise, the transformation is first inverted
 * with #invertAffineTransform and then put in the formula above instead of M. The function cannot operate in-place.
 * @param[in] src source image, supported image type:CMat, the number of channel: 1 ,3.
 * @param[out] dst dst image, supported image type:CMat,the number of channel: 1 ,3
 * @param[in] m affine matrix, the size of matrix should be 3x3.
 * @param[in] flags combination of interpolation methods (#INTER_LINEAR or #INTER_NEAREST or #INTER_AREA) and the
 * optional flag #WARP_INVERSE_MAP, that sets M as the inverse transformation.
 * @param[in] border_method pixel extrapolation method (#BORDER_CONSTANT).
 * @param[in] borderValue value used in case of a constant border
 */
EXTERN_C FCV_API int fcvWarpPerspective(
        CMat* src,
        CMat* dst,
        CMat* m,
        CInterpolationType flag,
        CBorderType border_method,
        CScalar* border_value);

G_FCV_NAMESPACE1_END()
