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

#include "modules/core/mat/include/mat_convert_to_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template <typename ST, typename DT>
static inline void convert_type(
        ST* src,
        DT* dst,
        int count,
        double scale,
        double shift) {
    ST* src_data = src;
    DT* dst_data = dst;

    for (int i = 0; i < count; ++i) {
        dst_data[i] = static_cast<DT>(src_data[i] * scale + shift);
    }
}

int convert_to_common(
        FCVImageType src_type,
        int src_stride,
        void* src_data,
        int dst_w,
        int dst_h,
        void* dst_data,
        double scale,
        double shift) {
    switch (src_type) {
    case FCVImageType::GRAY_U8:
    case FCVImageType::PLA_BGR_U8:
    case FCVImageType::PLA_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PLA_BGRA_U8:
    case FCVImageType::PLA_RGBA_U8:
    case FCVImageType::PKG_BGRA_U8:
    case FCVImageType::PKG_RGBA_U8:
        dst_w = FCV_MAX(dst_w, (int)src_stride);
        convert_type<unsigned char, float>(static_cast<unsigned char*>(src_data),
                static_cast<float*>(dst_data), dst_w * dst_h, scale, shift);
        break;
    case FCVImageType::GRAY_U16:
        dst_w = FCV_MAX(dst_w, (int)(src_stride / sizeof(unsigned short)));
        convert_type<unsigned short, float>(static_cast<unsigned short*>(src_data),
                static_cast<float*>(dst_data), dst_w * dst_h, scale, shift);
        break;
    case FCVImageType::GRAY_S32:
        dst_w = FCV_MAX(dst_w, (int)(src_stride / sizeof(int)));
        convert_type<int, float>(static_cast<int*>(src_data),
                static_cast<float*>(dst_data), dst_w * dst_h, scale, shift);
        break;
    case FCVImageType::GRAY_F64:
        dst_w = FCV_MAX(dst_w, (int)(src_stride / sizeof(double)));
        convert_type<double, double>(static_cast<double*>(src_data),
                static_cast<double*>(dst_data), dst_w * dst_h, scale, shift);
        break;
    default:
        LOG_ERR("The src type is not supported!");
        return -1;
    };

    return 0;
}

G_FCV_NAMESPACE1_END()