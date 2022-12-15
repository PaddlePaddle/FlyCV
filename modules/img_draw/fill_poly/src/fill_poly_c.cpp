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

#include "modules/img_draw/fill_poly/interface/fill_poly_c.h"
#include "modules/img_draw/fill_poly/interface/fill_poly.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fcvFillPoly(
        CMat* img,
        const CPoint** pts,
        const int* npts,
        int ncontours,
        CScalar* color,
        CLineType line_type,
        int shift,
        CPoint* offset) {
    if (!check_cmat(img)) {
         LOG_ERR("The img is illegal, please check whether "
                "the attribute values ​​of img are correct");
         return -1;
    }

    Mat img_tmp;
    cmat_to_mat(img, img_tmp);

    Scalar s = {0};

    if (color != nullptr) {
        for (int i = 0; i < img->channels; ++i) {
            s[i] = color->val[i];
        }
    }

    Point offset_tmp(0, 0);

    if (offset != nullptr) {
        offset_tmp = Point(offset->x, offset->y);
    }

    Point** p = (Point**)malloc(sizeof(Point*) * ncontours);

    for (int i = 0;  i < ncontours; ++i) {
        p[i] = (Point*)malloc(sizeof(Point) * npts[i]);
    }

    for (int i = 0; i < ncontours; ++i) {
        for (int j = 0; j < npts[i]; ++j) {
            p[i][j] = Point(pts[i][j].x, pts[i][j].y);
        }
    }

    fill_poly(img_tmp, (const Point**)p, npts, ncontours, s,
            static_cast<LineType>(line_type), shift, offset_tmp);
    
    for (int i = 0; i < ncontours; ++i) {
        free(p[i]);
    }

    free(p);
    p = nullptr;

    return 0;
}

G_FCV_NAMESPACE1_END()
