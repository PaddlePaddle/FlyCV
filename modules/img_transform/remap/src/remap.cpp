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

#include "modules/img_transform/remap/interface/remap.h"
#include "modules/img_transform/remap/include/remap_common.h"

#ifdef HAVE_NEON
#include "modules/img_transform/remap/include/remap_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int remap(
        const Mat& src,
        Mat& dst,
        const Mat& map1,
        const Mat& map2,
        InterpolationType inter_type,
        BorderTypes border_method,
        const Scalar border_value) {
     if ((src.empty()) || (map1.empty()) || (map2.empty())) {
        LOG_ERR("null ptr in remap!");
        return -1;
    }

    int res = -1;

#ifdef HAVE_NEON
    res = remap_neon(src, dst, map1, map2, inter_type, border_method, border_value);
#else
    res = remap_common(src, dst, map1, map2, inter_type, border_method, border_value);
#endif

    return res;
}

G_FCV_NAMESPACE1_END()
