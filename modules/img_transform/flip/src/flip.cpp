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

#include "modules/img_transform/flip/interface/flip.h"
#include "modules/img_transform/flip/include/flip_common.h"

#ifdef HAVE_NEON
#include <arm_neon.h>
#include "modules/img_transform/flip/include/flip_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int flip(const Mat& src, Mat& dst, FlipType type) {
    if (src.empty()) {
        LOG_ERR("Input Mat of flip is empty!");
        return -1;
    }

    if (dst.empty()) {
        dst = Mat(src.width(), src.height(), src.type());
    }
    if (dst.width() != src.width() 
        || dst.height() != src.height() 
        || dst.type() != src.type()) {
        LOG_ERR("illegal size or type of dst mat to flip");
    }
    int res = -1;
#ifdef HAVE_NEON
    res = flip_neon(src, dst, type);
#else
    res = flip_common(src, dst, type);
#endif

    return res;
}

G_FCV_NAMESPACE1_END()
