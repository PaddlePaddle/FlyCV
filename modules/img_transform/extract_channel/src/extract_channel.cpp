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

#include "modules/img_transform/extract_channel/interface/extract_channel.h"

#include <float.h>
#include <math.h>
#include <cmath>

#include <algorithm>
#include <cstdint>
#include <iostream>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

typedef unsigned char uchar;

template<typename T>
static void extract_with_memcpy_common(
        const T * src,
        int width,
        int height,
        int stride,
        int channel,
        int index,
        T* dst) {
    stride /= sizeof(T);
    width = FCV_MAX(width, stride / channel);

    const T* src_data = src;
    T* dst_data = dst;

    if (channel == 3) {
        // the first address of each channel
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                dst_data[i * width + j] = src_data[(i * width + j) * channel + index];
            }
        }
    } else {
        LOG_ERR("the channel extract is not supported!\n");
        return;
    }

    return;
}

void extract_channel(Mat& _src, Mat& _dst, int _index) {
    if (_src.type() != FCVImageType::PKG_BGR_U8
            && _src.type() != FCVImageType::PLA_BGR_U8) {
        LOG_ERR("src type is not support");
        return;
    }

    _dst = Mat(_src.size(), FCVImageType::GRAY_U8, _src.stride());

    return extract_with_memcpy_common((const uchar *)_src.data(),
            _src.width(), _src.height(), _src.stride(), _src.channels(),
            _index, (uchar *)_dst.data());
}

G_FCV_NAMESPACE1_END()
