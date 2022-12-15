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

#include "modules/img_transform/warp_perspective/interface/warp_perspective_c.h"
#include "modules/img_transform/warp_perspective/interface/warp_perspective.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

CMat* fcvGetPerspectiveTransform(CPoint2f src[], CPoint2f dst[]) {
    Point2f src_tmp[4];
    Point2f dst_tmp[4];

    for (int i = 0; i < 4; ++i) {
        src_tmp[i] = Point2f(src[i].x, src[i].y);
        dst_tmp[i] = Point2f(dst[i].x, dst[i].y);
    }

    Mat tmp = get_perspective_transform(src_tmp, dst_tmp);
    return mat_to_cmat(tmp);
}

int fcvWarpPerspective(
        CMat* src,
        CMat* dst,
        CMat* m,
        CInterpolationType flag,
        CBorderType border_method,
        CScalar* border_value) {
    if (!check_cmat(src)) {
        LOG_ERR("The src is illegal, please check whether "
                "the attribute values ​​of src are correct");
        return -1;
    }

    if (!check_cmat(dst)) {
        LOG_ERR("The dst is illegal, please check whether "
                "the attribute values ​​of dst are correct");
        return -1;
    }

    if (!check_cmat(m)) {
        LOG_ERR("The m is illegal, please check whether "
                "the attribute values ​​of m are correct");
        return -1;
    }

    Scalar s = {0};

    if (border_value != nullptr) {
        for (int i = 0; i < src->channels; ++i) {
            s[i] = border_value->val[i];
        }
    }

    Mat src_tmp;
    Mat dst_tmp;
    Mat m_tmp;
    cmat_to_mat(src, src_tmp);
    cmat_to_mat(dst, dst_tmp);
    cmat_to_mat(m, m_tmp);

    return warp_perspective(src_tmp, dst_tmp, m_tmp,
            static_cast<InterpolationType>(flag),
            static_cast<BorderType>(border_method), s);
}

G_FCV_NAMESPACE1_END()
