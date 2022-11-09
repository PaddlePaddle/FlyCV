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

#include "modules/img_transform/copy_make_border/interface/copy_make_border.h"
#include "modules/img_transform/copy_make_border/include/copy_make_border_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int copy_make_border(
        Mat& src,
        Mat& dst,
        int top,
        int bottom,
        int left,
        int right,
        BorderTypes border_type,
        const Scalar& value) {
    // Check whether the parameter is legal
    if (src.empty()) {
        LOG_ERR("The src image is empty for copy_make_border!");
        return -1;
    }

    switch (src.type()) {
    case FCVImageType::PLA_BGR_U8:
    case FCVImageType::PLA_RGB_U8:
    case FCVImageType::PLA_BGRA_U8:
    case FCVImageType::PLA_RGBA_U8:
    case FCVImageType::PLA_BGR_F32:
    case FCVImageType::PLA_RGB_F32:
    case FCVImageType::PLA_BGRA_F32:
    case FCVImageType::PLA_RGBA_F32:
    case FCVImageType::NV21:
    case FCVImageType::NV12:
    case FCVImageType::I420:
        LOG_ERR("Unsupported src image type for copy_make_border : %d", int(src.type()));
        return -1;
    default:
        break;
    };

    if (top < 0 || bottom < 0 || left < 0 || right < 0) {
        LOG_ERR("The top : %d, bottom : %d, left : %d, right : %d has "
                "negative value for copy_make_border!",  top, bottom, left, right);
        return -1;
    }

    int expected_width = src.width() + left + right;
    int expected_height = src.height() + top + bottom;
    // Check whether dst image mat is legal
    if (dst.empty()
            || dst.width() != expected_width
            || dst.height() != expected_height
            || dst.type() != src.type()) {
        dst = Mat(expected_width, expected_height, src.type());
    }
    // TODO(chenlong22) : add other BorderTypes later
    if (border_type != BorderTypes::BORDER_CONSTANT) {
        LOG_ERR("Falconcv only support copy_make_border type BORDER_CONSTANT now!");
        return -1;
    }
    int status = copy_make_border_common(src, dst, top, bottom, left, right, border_type, value);
    return status;
}

G_FCV_NAMESPACE1_END()
