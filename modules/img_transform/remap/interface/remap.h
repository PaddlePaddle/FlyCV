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
 * @brief the implementation of a generic geometrical transformation to an image, supported element types: u8 and f32
 * @param[in] src source image, supported image type:Mat, the number of channel: 1 ,3.
 * @param[out] dst dst image, supported image type:Mat,the number of channel: 1 ,3, 
 *            It has the same height and the half width as map and the same type as src
 * @param[in] map1 The first map of (x,y) points having the type S16, Mat map(dst_width * 2, dst_height, FCVImageType::GRAY_S16).
 *             or just x values having the type F32C1
 * @param[in] map2 The second map of coeff values having the type S16C1 or y values having the type F32C1.
 * @param[in] inter_type Interpolation method, see @InterpolationType, currently only support INTER_LINEAR.
 * @param[in] border_method Make border method, see @BorderType, currently only support BORDER_CONSTANT.
 * @param[in] border_value Value used in case of a constant border, by default, set it to 0.
 */

FCV_API int remap(
        const Mat& src,
        Mat& dst,
        const Mat& map1,
        const Mat& map2,
        InterpolationType inter_type = InterpolationType::INTER_LINEAR,
        BorderType border_method = BorderType::BORDER_CONSTANT,
        const Scalar border_value = {0});

G_FCV_NAMESPACE1_END()
