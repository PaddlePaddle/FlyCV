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

#include "modules/img_transform/add_weighted/interface/add_weighted_c.h"
#include "modules/img_transform/add_weighted/interface/add_weighted.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fcvAddWeighted(
        CMat* src1,
        double alpha,
        CMat* src2,
        double beta,
        double gamma,
        CMat* dst) {
    if (!check_cmat(src1)) {
        LOG_ERR("The src1 is illegal, please check whether "
                "the attribute values ​​of src1 are correct");
        return -1;
    }

    if (!check_cmat(src2)) {
        LOG_ERR("The src2 is illegal, please check whether "
                "the attribute values ​​of src2 are correct");
        return -1;
    }

    if (!check_cmat(dst)) {
        LOG_ERR("The dst is illegal, please check whether "
                "the attribute values ​​of dst are correct");
        return -1;
    }

    Mat src1_tmp;
    Mat src2_tmp;
    Mat dst_tmp;
    cmat_to_mat(src1, src1_tmp);
    cmat_to_mat(src2, src2_tmp);
    cmat_to_mat(dst, dst_tmp);

    return add_weighted(src1_tmp, alpha, src2_tmp, beta, gamma, dst_tmp);
}

G_FCV_NAMESPACE1_END()
