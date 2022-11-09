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

#include "modules/img_transform/warp_affine/interface/warp_affine.h"
#include "modules/img_transform/warp_affine/include/warp_affine_common.h"

#ifdef __RV1109_OPTIMIZATION__
#include "modules/img_transform/warp_affine/include/warp_affine_rv1109.h"
#endif

#ifdef HAVE_NEON
#include "modules/img_transform/warp_affine/include/warp_affine_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int warp_affine(
        const Mat& src,
        Mat& dst,
        Mat& m,
        InterpolationType flag,
        BorderTypes border_method,
        const Scalar border_value) {
    if (src.empty() || m.empty()) {
        LOG_ERR("the src or M of warp_affine is empty!");
        return -1;
    }

    if (dst.empty()) {
        dst = Mat(src.width(), src.height(), src.type());
    }

    if (src.type() != dst.type()) {
        LOG_ERR("The src and dst Mat type should be the same for warp_affine!");
        return -1;
    }

    int res = -1;

#ifdef HAVE_NEON
    res = warp_affine_neon(src, dst, m, flag, border_method, border_value);
#else
    res = warp_affine_commmon(src, dst, m, flag, border_method, border_value);
#endif

    return res;
}

G_FCV_NAMESPACE1_END()
