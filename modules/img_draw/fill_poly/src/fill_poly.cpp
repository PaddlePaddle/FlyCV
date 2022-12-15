
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

#include "modules/img_draw/fill_poly/interface/fill_poly.h"

#include "modules/img_draw/fill_poly/include/fill_poly_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fill_poly(
        Mat& img,
        const Point** pts,
        const int* npts,
        int ncontours,
        const Scalar& color,
        LineType line_type,
        int shift,
        Point offset) {
    if (img.empty()) {
        LOG_ERR("Input Mat of draw is empty!");
        return -1;
    }
    if (img.type() != FCVImageType::PKG_BGR_U8 && img.type() != FCVImageType::PKG_RGB_U8) {
        LOG_ERR("fill_poly only support PKG_BGR_U8 or PKG_RGB_U8 input format now");
        return -1;
    }

    if (line_type == LineType::LINE_AA) {
        LOG_ERR("Line type is not support yet");
        return - 1;
    }

    fill_poly_common(img, pts, npts, ncontours, color, line_type, shift, offset);

    return 0;
}

G_FCV_NAMESPACE1_END()
