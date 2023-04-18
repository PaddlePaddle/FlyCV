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

#include "modules/img_draw/line/interface/line_c.h"
#include "modules/img_draw/line/interface/line.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fcvLine(
        CMat* img,
        CPoint pt1,
        CPoint pt2,
        CScalar* color,
        int thickness,
        CLineType line_type,
        int shift) {
    if (!check_cmat(img)) {
         LOG_ERR("The img is illegal, please check whether "
                "the attribute values ​​of img are correct");
         return -1;
    }

    Scalar s = {0};

    if (color != nullptr) {
        for (int i = 0; i < img->channels; ++i) {
            s[i] = color->val[i];
        }
    }

    Mat img_tmp;
    cmat_to_mat(img, img_tmp);

    return line(img_tmp, Point(pt1.x, pt1.y), Point(pt2.x, pt2.y),
            s, thickness, static_cast<LineType>(line_type), shift);
}

G_FCV_NAMESPACE1_END()