// Copyright (c) 2023 FlyCV Authors. All Rights Reserved.
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

#pragma once

#include "modules/core/mat/interface/mat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief Calculate the four vertex coordinates of rotated rect.
 * @param[in] rect Input rotated rect.
 * @param[out] points Output four vertex coordinates
 * @return no-zero: success; -1 : fail.
 */
FCV_API int box_points(RotatedRect rect, Mat& points);

G_FCV_NAMESPACE1_END()