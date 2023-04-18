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

#include "modules/img_transform/remap/interface/remap_c.h"
#include "modules/img_transform/remap/interface/remap.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fcvRemap(
        CMat* src,
        CMat* dst,
        CMat* map1,
        CMat* map2,
        CInterpolationType inter_type,
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

    if (!check_cmat(map1)) {
        LOG_ERR("The map1 is illegal, please check whether "
                "the attribute values ​​of map1 are correct");
        return -1;
    }

    if (!check_cmat(map2)) {
        LOG_ERR("The map2 is illegal, please check whether "
                "the attribute values ​​of map2 are correct");
        return -1;
    }

    Mat src_tmp;
    Mat dst_tmp;
    Mat map1_tmp;
    Mat map2_tmp;
    cmat_to_mat(src, src_tmp);
    cmat_to_mat(dst, dst_tmp);
    cmat_to_mat(map1, map1_tmp);
    cmat_to_mat(map2, map2_tmp);

    InterpolationType interpolation = static_cast<InterpolationType>(inter_type);
    BorderType border_type = static_cast<BorderType>(border_method);

    Scalar s = {0};

    if (border_value != nullptr) {
        for (int i = 0; i < src->channels; ++i) {
            s[i] = border_value->val[i];
        }
    }

    return remap(src_tmp, dst_tmp, map1_tmp,
            map2_tmp, interpolation, border_type, s);
}

G_FCV_NAMESPACE1_END()
