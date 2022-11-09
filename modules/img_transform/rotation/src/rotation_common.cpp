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

#include "modules/img_transform/rotation/include/rotation_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template<typename T>
void transpose_c(const T* src,
        int src_h,
        int src_w,
        int sc,
        int sstep,
        T* dst,
        int dstep) {
    T *dst_col = nullptr;
    int i = 0, j = 0, k = 0;
    for (; i < src_h; i++) {
        dst_col = dst;
        for (j = 0; j < src_w; j++) {
            for (k = 0; k < sc; k++) {
                dst_col[k] = src[j * sc + k];
            }
            dst_col += dstep;
        }

        dst += sc;
        src += sstep;
    }
}

int transpose_common(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    const void *src_ptr = (const void *)src.data();
    void*dst_ptr = (void *)dst.data();
    if ((src_w != dst_h) || (src_h != dst_w)) {
        LOG_ERR("size of input or output is not match!");
        return -1;
    }

    int sc = src.channels();
    const int s_stride = src_w * sc;
    const int d_stride = dst_w * sc;

    switch (src.type()) {
    case FCVImageType::GRAY_U8:
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
    case FCVImageType::PKG_RGBA_U8:
    case FCVImageType::PKG_BGRA_U8:
        transpose_c((unsigned char*)src_ptr, src_h, src_w,
                sc, s_stride, (unsigned char*)dst_ptr, d_stride);
        break;
    case FCVImageType::GRAY_F32:
    case FCVImageType::PKG_RGB_F32:
    case FCVImageType::PKG_BGR_F32:
    case FCVImageType::PKG_RGBA_F32:
    case FCVImageType::PKG_BGRA_F32:
        transpose_c((float*)src_ptr, src_h, src_w, sc,
                s_stride, (float*)dst_ptr, d_stride);
        break;
    default:
        LOG_ERR("transpose type not support yet!");
        break;
    };

    return 0;
}

G_FCV_NAMESPACE1_END()
