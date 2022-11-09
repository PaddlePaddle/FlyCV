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
#include "modules/img_transform/resize/interface/resize.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int resize_rv1109(
        Mat& src,
        Mat& dst,
        const Size& dsize,
        double fx = 0,
        double fy = 0,
        InterpolationType interpolation = InterpolationType::INTER_LINEAR);

G_FCV_NAMESPACE1_END()
