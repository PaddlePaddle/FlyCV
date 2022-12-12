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

#include "modules/img_transform/resize/interface/resize_c.h"
#include "modules/img_transform/resize/interface/resize.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int FcvResize(
        CMat* src,
        CMat* dst,
        CInterpolationType interpolation) {
    if (src == nullptr || src->width <= 0 || src->height <= 0) {
        LOG_ERR("The src is illegal!");
        return -1;
    }

    if (dst == nullptr || dst->width <= 0 || dst->height <= 0) {
        LOG_ERR("The dst is illegal!");
        return -1;
    }

    Mat src_tmp;
    Mat dst_tmp;
    cmat_to_mat(src, src_tmp);
    cmat_to_mat(dst, dst_tmp);

    InterpolationType inter_type = cinterpolation_to_interpolation(interpolation);

    return resize(src_tmp, dst_tmp, {}, 0, 0, inter_type);
}

G_FCV_NAMESPACE1_END()