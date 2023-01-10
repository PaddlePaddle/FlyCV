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
 * @brief Finds a perspective transformation between two planes. 
 * @param[in] src_points, points in the original plane, only 4 points are supported currently.
 * @param[in] dst_points, points in the target plane, only 4 points are supported currently.
 */
EXTERN_C FCV_API CMat* fcvFindHomography(
        CPoint2f* src_pts,
        CPoint2f* dst_pts,
        int pts_num,
        int method);

G_FCV_NAMESPACE1_END()
