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
 * @brief Extended image with border around
 * @param[in] src Source image, only support PACKAGE format data, the planar or YUV data is not support now.
 * @param[out] dst Destination image of the same type as src and the size(src.width() + left + right, src.height() + top + bottom)
 * @param[in] top Number of rows expanded up
 * @param[in] bottom Number of rows expanded down
 * @param[in] left Number of columns to expand left
 * @param[in] right Number of columns to expand right
 * @param[in] border_type The rule about how to fill the extended border pixel, see BorderType declaration for details
 * @param[in] value The value of border only worked if the border_type is BORDER_CONSTANT
 * @return FCV_API
 */
FCV_API int copy_make_border(
        Mat& src,
        Mat& dst,
        int top,
        int bottom,
        int left,
        int right,
        BorderType border_type,
        const Scalar& value = Scalar());

G_FCV_NAMESPACE1_END()
