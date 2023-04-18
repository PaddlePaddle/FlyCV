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
 * @brief combine resize and convert bgra to bgr, which means the src's
 *        data type is bgra, and the data type of dst is bgr.
 *        the function, firstly, resize the src to the size of dst
 *        and then convert bgra pixel formats to bgr pixel formats
 * @param[in] src input data CMat, bgra package, u8 data
 * @param[out] dst ouput data CMat, bgr package, u8 data
 * @param[in] interpolation Interpolation method, currently only support INTER_LINEAR
*/
EXTERN_C FCV_API int fcvBgraToResizeToBgr(
        CMat* src,
        CMat* dst,
        CInterpolationType interpolation);

G_FCV_NAMESPACE1_END()
