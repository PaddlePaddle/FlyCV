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

#include "modules/img_calculation/min_max_loc/interface/min_max_loc.h"
#include "modules/img_calculation/min_max_loc/include/min_max_loc_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int min_max_loc(
        const Mat& src,
        double* min_val,
        double* max_val,
        Point* min_loc,
        Point* max_loc,
        const Mat& mask) {
    if (src.empty()) {
        LOG_ERR("The src mat is empty for min_max_loc!");
        return -1;
    }

    if (src.channels() != 1) {
        LOG_ERR("The num of src mat channles is %d, but min_max_loc "
                "only support single channle input!", src.channels());
        return -1;
    }

    if (mask.channels() > 1) {
        LOG_ERR("The num of mask mat channles is %d, but min_max_loc "
                "only support single channle input!", mask.channels());
        return -1;
    }

    return min_max_loc_common(src, min_val, max_val, min_loc, max_loc, mask);
}

G_FCV_NAMESPACE1_END()
