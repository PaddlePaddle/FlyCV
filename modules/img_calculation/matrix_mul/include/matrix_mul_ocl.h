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

#ifdef HAVE_OPENCL

#pragma once

#include "modules/core/mat/interface/mat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief the implementation of perspective matrix_mul_opencl, supported element types: f32
 * The method returns a temporary object encoding per-element array multiplication,
   Computed with OpenCL
 * @param src0 source image, supported image type:Mat, the number of channel: 3.
 * @param src1 another source image, supported image type:Mat, the number of channel: 3.
 * @param dst dst image, supported image type:Mat,the number of channel: 3
 */
int matrix_mul_opencl(const Mat& src0, const Mat& src1, Mat& dst);

G_FCV_NAMESPACE1_END()

#endif