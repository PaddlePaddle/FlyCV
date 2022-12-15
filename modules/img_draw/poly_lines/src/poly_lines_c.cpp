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

#include "modules/img_draw/poly_lines/interface/poly_lines_c.h"
#include "modules/img_draw/poly_lines/interface/poly_lines.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fcvPolyLines(
        CMat* img,
        const CPoint2l* v,
        int count,
        bool is_closed,
        const void* color,
        int thickness,
        CLineType line_type,
        int shift) {
     if (!check_cmat(img)) {
         LOG_ERR("The img is illegal, please check whether "
                "the attribute values ​​of img are correct");
         return -1;
    }

    Mat img_tmp;
    cmat_to_mat(img, img_tmp);

    Point2l* v_tmp = new Point2l[count];

    for (int i = 0; i < count; ++i) {
        v_tmp[i] = Point2l(v[i].x, v[i].y);
    }

    poly_lines(img_tmp, v_tmp, count, is_closed, color,
            thickness, static_cast<LineType>(line_type), shift);

    delete[] v_tmp;
    v_tmp = nullptr;

    return 0;
}

G_FCV_NAMESPACE1_END()
