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
 * @brief convert bgr u8 3-channel src image with 1-channel u8 alpha mask to 4-channel rgba u8 image, all package format.
 * @param[in] src input data Mat, bgr package, u8 data
 * @param[in] mask mask data Mat, gray, u8 data
 * @param[out] dst ouput data Mat, rgba package, u8 data
 */
FCV_API int bgr_to_rgba_with_mask(Mat& src, Mat& mask, Mat& dst);

G_FCV_NAMESPACE1_END()
