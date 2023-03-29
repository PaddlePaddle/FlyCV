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

#include "modules/img_calculation/min_max_loc/interface/min_max_loc_c.h"
#include "modules/img_calculation/min_max_loc/interface/min_max_loc.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fcvMinMaxLoc(
        CMat* src,
        double* min_val,
        double* max_val,
        CPoint* min_loc,
        CPoint* max_loc,
        CMat* mask) {
    if (!checkCMat(src)) {
        LOG_ERR("The src is illegal, please check whether "
                "the attribute values ​​of src are correct");
        return -1;
    }

    Mat src_tmp;
    Mat mask_tmp;
    cmatToMat(src, src_tmp);

    if (mask) {
        cmatToMat(mask, mask_tmp);
    }

    Point min_loc_tmp;
    Point max_loc_tmp;

    min_max_loc(src_tmp, min_val, max_val,
            min_loc == nullptr ? nullptr : &min_loc_tmp,
            max_loc == nullptr ? nullptr : &max_loc_tmp, mask_tmp);
    
    if (min_loc != nullptr) {
        min_loc->x = min_loc_tmp.x();
        min_loc->y = min_loc_tmp.y();
    }

    if (max_loc != nullptr) {
        max_loc->x = max_loc_tmp.x();
        max_loc->y = max_loc_tmp.y();
    }

    return 0;
}

G_FCV_NAMESPACE1_END()