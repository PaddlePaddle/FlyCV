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
 * @brief Computes the connected components labeled image.
 * @param[in] src Input matrix data, only support single channel type!
 * @param[out] labels Output labeled matrix data.
 * @param[out] connectivity 8 or 4 for 8-way or 4-way connectivity respectively, Currently only support 8.
 * @param[out] ltype output image label type, Currently only support int32.
 * @return FCV_API no-zero: the num of connected components labels; -1 : fail.
 */
EXTERN_C FCV_API int fcvConnectedComponents(
        CMat* src,
        CMat* labels,
        int connectivity,
        CFCVImageType type);

G_FCV_NAMESPACE1_END()