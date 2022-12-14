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
 * @brief the implementation of perspective transformation, supported element types: only f32
 * The function calculates the per-element difference between two arrays or array and a scalar.
 * @param[in] src input image, supported image type:CMat, the number of channel: 1 ,3
 * @param[out] dst output image, supported image type:CMat, the number of channel: 1 ,3
   Destination matrix. 
*/
EXTERN_C FCV_API int fcvSubtract(CMat* src, CScalar* scalar, CMat* dst);

G_FCV_NAMESPACE1_END()
