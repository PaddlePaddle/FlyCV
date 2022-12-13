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
 * @brief Finds the minimum and maximum pixel values with its positions.
 * @param[in] src src Input matrix data, only support single channel type!
 * @param[out] min_val min_val Pointer to store minimum value, set it to nullptr if it is not needed.
 * @param[out] max_val max_val Pointer to store maximum value, set it to nullptr if it is not needed.
 * @param[out] min_loc min_loc Pointer to store the position of minimum position, set it to nullptr if it is not needed.
 * @param[out] max_loc max_loc Pointer to store the position of maximum position, set it to nullptr if it is not needed.
 * @param[in] mask mask Mask matrix to select the valid area of src, only support the same size with src , and single channle U8 data.
 * @return FCV_API zero : success; non-zero : fail.
 */
EXTERN_C FCV_API int fcvMinMaxLoc(
        CMat* src,
        double* min_val,
        double* max_val,
        CPoint* min_loc,
        CPoint* max_loc,
        CMat* mask);

G_FCV_NAMESPACE1_END()