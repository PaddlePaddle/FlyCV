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

#include "modules/img_transform/rotation/include/rotation_common.h"
#include "modules/img_transform/rotation/include/rotation_arm.h"

#include <cstring>

#ifdef HAVE_NEON
#include <arm_neon.h>
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int transpose(const Mat& src, Mat& dst) {
    if (src.empty()) {
        LOG_ERR("Input Mat of transpose is empty!");
        return -1;
    }

    if (dst.empty()) {
        dst = Mat(src.height(), src.width(), src.type());
    }
    if (dst.width() != src.height() 
            || dst.height() != src.width() 
            || dst.type() != src.type()) {
        LOG_ERR("illegal size of dst mat");
        return -1;
    }

    int res = -1;
#ifdef HAVE_NEON
    res = transpose_neon(src, dst);
#else
    res = transpose_common(src, dst);
#endif

    return res;
}

G_FCV_NAMESPACE1_END()
