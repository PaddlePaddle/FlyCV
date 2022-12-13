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

#include "modules/img_calculation/min_area_rect/interface/min_area_rect_c.h"
#include "modules/img_calculation/min_area_rect/interface/min_area_rect.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

CRotatedRect fcvMinAreaRect(CMat* src) {
    CRotatedRect res;

    if (!check_cmat(src)) {
        LOG_ERR("The src is illegal, please check whether "
                "the attribute values ​​of src are correct");
        return res;
    }

    std::vector<Point> src_pts;
    int* src_data = reinterpret_cast<int*>(src->data);

    for (int i = 0; i < src->width * src->height / 2; ++i) {
        src_pts.push_back(Point(src_data[2 * i], src_data[2 * i + 1]));
    }

    RotatedRect r = min_area_rect(src_pts);
    res.angle = r.angle();
    res.center.x = r.center().x();
    res.center.y = r.center().y();
    res.size.width = r.size().width();
    res.size.height = r.size().height();

    return res;
}

G_FCV_NAMESPACE1_END()
