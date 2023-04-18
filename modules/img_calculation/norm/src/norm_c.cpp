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

#include "modules/img_calculation/norm/interface/norm_c.h"
#include "modules/img_calculation/norm/interface/norm.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

double fcvNorm(CMat* src, CNormType norm_type) {
    if (!check_cmat(src)) {
         LOG_ERR("The src is illegal, please check whether "
                "the attribute values ​​of src are correct");
         return 0;
    }

    Mat src_tmp;
    cmat_to_mat(src, src_tmp);

    NormType type = static_cast<NormType>(norm_type);
    return norm(src_tmp, type);
}

G_FCV_NAMESPACE1_END()
