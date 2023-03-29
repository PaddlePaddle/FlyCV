// Copyright (c) 2023 FlyCV Authors. All Rights Reserved.
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

#include "modules/img_calculation/box_points/interface/box_points_c.h"
#include "modules/img_calculation/box_points/interface/box_points.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fcvBoxPoints(CRotatedRect crect, CMat* cpoints);
    if (!checkCMat(cpoints)) {
        LOG_ERR("The points is illegal, please check whether "
                "the attribute values ​​of points are correct");
        return -1;
    }

    RotatedRect rect(crect.center.x, crect.center.y,
            crect.size.width, crect.size.height, crect.angle);

    Mat points;
    cmatToMat(cpoints, points);

    return box_points(rect, points);
} 

G_FCV_NAMESPACE1_END()