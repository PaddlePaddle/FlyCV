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
 * @brief two imgs fusion to one img, only support PKG_BGR_U8 format of input image now.
 * @param[in] src1  input first img
 * @param[in] alpha first img proportion when fusion
 * @param[in] src2  input second img
 * @param[in] beta  second img proportion when fusion
 * @param[in] gamma offset value when fusion
 * @param[out] dst   output img when fusion
 */
EXTERN_C FCV_API int add_weighted(
        Mat& src1,
        double alpha,
        Mat& src2,
        double beta,
        double gamma,
        Mat& dst);

G_FCV_NAMESPACE1_END()
