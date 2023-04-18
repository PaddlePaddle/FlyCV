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

#include "modules/img_calculation/matrix_mul/interface/matrix_mul_c.h"
#include "modules/img_calculation/matrix_mul/interface/matrix_mul.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

CMat* fcvMatrixMul(CMat* src0, CMat* src1) {
    if (!check_cmat(src0)) {
         LOG_ERR("The src0 is illegal, please check whether the attribute values ​​of src0 are correct");
         return nullptr;
    }

    if (!check_cmat(src1)) {
         LOG_ERR("The src1 is illegal, please check whether the attribute values ​​of src1 are correct");
         return nullptr;
    }

    Mat src0_tmp;
    Mat src1_tmp;
    cmat_to_mat(src0, src0_tmp);
    cmat_to_mat(src1, src1_tmp);

    Mat res_tmp = matrix_mul(src0_tmp, src1_tmp);
    CMat* res = mat_to_cmat(res_tmp);
    return res;
}

G_FCV_NAMESPACE1_END()