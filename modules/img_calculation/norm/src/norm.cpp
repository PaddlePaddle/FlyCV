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

#include "modules/img_calculation/norm/interface/norm.h"

#include <cmath>

#include "modules/img_calculation/norm/include/norm_common.h"

#ifdef HAVE_NEON
#include "modules/img_calculation/norm/include/norm_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

double norm(Mat& src, NormType norm_type) {
    if (src.empty()) {
        LOG_ERR("Input Mat of resize is empty!");
        return -1;
    }

    double result = 0.f;

#ifdef HAVE_NEON
    int status = 0;
    status = norm_neon(src, norm_type, result);
    if (status == 0) return result;
#endif

    norm_common(src, norm_type, result);
    return result;
}

G_FCV_NAMESPACE1_END()
