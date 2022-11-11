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

#include "modules/img_draw/circle/interface/circle.h"

#include "modules/img_draw/circle/include/circle_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

FCV_API int circle(
        Mat& img,
        Point center,
        int radius,
        const Scalar& color,
        int thickness,
        LineTypes line_type,
        int shift) {
    if (img.empty()) {
        LOG_ERR("The input img is empty!");
        return -1;
    }
    if (img.type() != FCVImageType::PKG_BGR_U8 && img.type() != FCVImageType::PKG_RGB_U8) {
        LOG_ERR("circle only support PKG_BGR_U8 or PKG_RGB_U8 input format now");
        return -1;
    }

    circle_common(img, center, radius, color, thickness, line_type, shift);
    return 0;
}

G_FCV_NAMESPACE1_END()
