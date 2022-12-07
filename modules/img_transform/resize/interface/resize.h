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

#include <vector>

#include "modules/core/mat/interface/mat.h"
#include "modules/core/cmat/interface/cmat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/** 
 * @brief Resizes an image.
 * The function resize resizes the image src down to or up to the specified size. Note that the
 * initial dst type or size are not taken into account. Instead, the size and type are derived from
 * the `src`,`dsize`,`fx`, and `fy`. If you want to resize src so that it fits the pre-created dst,
 * you may call the function as follows:
 * @param[in] src input image, supported image type:Mat, the number of channel: 1, 3, 4.
 * @param[out] dst output image; it has the size dsize (when it is non-zero) or the size computed from
 * src.size(), fx, and fy; the type of dst is the same as of src.
 * @param[in] dsize output image size; if it equals zero, it is computed as: {dsize = Size(round(fx*src.cols), round(fy*src.rows))}.
 * Either dsize or both fx and fy must be non-zero.
 * @param[in] fx scale factor along the horizontal axis; when it equals 0, it is computed as {(double)dsize.width/src.cols}.
 * @param[in] fy scale factor along the vertical axis; when it equals 0, it is computed as {(double)dsize.height/src.rows}.
 * @param[in] interpolation interpolation method, see #InterpolationType.
 */
FCV_API int resize(
        Mat& src,
        Mat& dst,
        const Size& dsize,
        double fx = 0,
        double fy = 0,
        InterpolationType interpolation = InterpolationType::INTER_LINEAR);

int resize_c(
        CMat* src,
        CMat* dst,
        CSize size,
        double fx,
        double fy,
        CInterpolationType interpolation);

G_FCV_NAMESPACE1_END()
