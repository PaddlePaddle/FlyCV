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

#include "modules/img_transform/crop/interface/crop.h"

#include <cstring>

#include "modules/core/base/include/type_info.h"

#ifdef __RV1109_OPTIMIZATION__
#include "modules/img_transform/crop/include/crop_rv1109.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

bool check_rect_in_area(const Size& image_size, const Rect& drect) {
    if (drect.x() < 0 || drect.y() < 0) {
        return false;
    }
    if ((drect.x() + drect.width()) > image_size.width()
            || (drect.y() + drect.height()) > image_size.height()) {
        return false;
    }
    return true;
}

int crop_common(Mat& src, Mat& dst, Rect& drect) {
    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);
    if (status) {
        LOG_ERR("Unsupport image type!");
        return -1;
    }
    int step = type_info.pixel_size;
    if (step <= 0) {
        LOG_ERR("invalid Mat type for crop!");
        return -1;
    }
    // check image type
    switch (type_info.layout) {
    case LayoutType::SINGLE:
    case LayoutType::PACKAGE:
        break;
    default:
        LOG_ERR("crop not support yuv or planar image_type now!");
        return -1;
    }
    Size dst_size(drect.width(), drect.height());
    if (dst.empty()
            || dst.type() != src.type()
            || dst.width() != dst_size.width()
            || dst.height() != dst_size.height()) {
        dst = Mat(dst_size.width(), dst_size.height(), src.type());
    }
    uint8_t* dst_data = reinterpret_cast<uint8_t*>(dst.data());
    uint8_t* valid_src_addr = reinterpret_cast<uint8_t*>(src.data()) + 
            step * src.width() * drect.y() + drect.x() * step;

    for (int i = 0; i < dst_size.height(); ++i) {
        memcpy(dst_data, valid_src_addr, drect.width() * step);
        valid_src_addr += (src.width() * step);
        dst_data += (dst.width() * step);
    }
    return 0;
}

int crop(Mat& src, Mat& dst, Rect& drect) {
    if (src.empty()) {
        LOG_ERR("The src Mat to crop from is empty!");
        return -1;
    }
    if (!check_rect_in_area(Size(src.width(), src.height()), drect)) {
        LOG_ERR("The crop area size is beyond the src image range!");
        return -1;
    }
    int result = -1;
#ifdef __RV1109_OPTIMIZATION__
    result = crop_rv1109(src, dst, drect);
#else
    result = crop_common(src, dst, drect);
#endif
    return result;
}

G_FCV_NAMESPACE1_END()
