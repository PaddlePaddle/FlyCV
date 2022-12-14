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
 * @brief the implementation of perspective transformation, supported element types: u8 and f32
 * The function calculates the product of a matrix and its transposition.
 * @param[in] src input image, supported image type:CMat, the number of channel: 1 ,3
 * @param[out] dst output image, supported image type:CMat, the number of channel: 1 ,3
 */
EXTERN_C FCV_API int fcvTranspose(CMat* src, CMat* dst);

G_FCV_NAMESPACE1_END()
