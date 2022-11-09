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

#include "modules/img_calculation/connected_components/interface/connected_components.h"
#include "modules/img_calculation/connected_components/include/connected_components_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int connected_components(const Mat& src,
        Mat& labels,
        int connectivity,
        FCVImageType ltype) {
    if (src.empty()) {
        LOG_ERR("The src mat is empty for connected_components!");
        return -1;
    }

    if (src.channels() != 1) {
        LOG_ERR("The num of src mat channles is %d, but connected_components "
                "only support single channle input!",  src.channels());
        return -1;
    }

    return connected_components_common(src, labels, connectivity, ltype);
}

G_FCV_NAMESPACE1_END()
