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

CRotatedRect fcvMinAreaRect(CPoint* pts, int pts_num) {
    if (pts == nullptr) {
        return CRotatedRect();
    }

    std::vector<Point> src_pts;

    for (int i = 0; i < pts_num; ++i) {
        src_pts.push_back(Point(pts[i].x, pts[i].y));
    }

    RotatedRect r = min_area_rect(src_pts);
    CRotatedRect res;

    res.angle = r.angle();
    res.center.x = r.center().x();
    res.center.y = r.center().y();
    res.size.width = r.size().width();
    res.size.height = r.size().height();

    return res;
}

G_FCV_NAMESPACE1_END()
