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
 * @brief the implementation of perspective transformation, supported element types: u8 and f32
 * The function calculates the product of a matrix and its transposition.
 * @param[in] src input image, supported image type:Mat, the number of channel: 1 ,3
 * @param[out] dst output image, supported image type:Mat, the number of channel: 1 ,3
 */
FCV_API int transpose(const Mat& src, Mat& dst);

/**
 * @brief the implementation of image rotation, supported image types: u8 and f32
 * The function rotate the src image.
 * @param[in] src input image, supported image type:Mat, the number of channel: 1 ,3
 * @param[out] dst output image, supported image type:Mat, the number of channel: 1 ,3
 * @param[in] rotate type, 0 for clockwise 90 degree, 1 for clockwise 180 degree, 1 for clockwise 270 degree
 */
FCV_API int rotate(const Mat& src, Mat& dst, RotateType rotate_type);

G_FCV_NAMESPACE1_END()
