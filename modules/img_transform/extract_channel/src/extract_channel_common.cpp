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

#include <cstdint>

#include "modules/img_transform/extract_channel/include/extract_channel_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template<typename T>
static int extract_with_memcpy_common(
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
        return -1;
    }

    return 0;
}

int extract_channel_common(Mat& _src, Mat& _dst, int _index) {
    return extract_with_memcpy_common(
            (const unsigned char *)_src.data(),
            _src.width(),
            _src.height(),
            _src.stride(),
            _src.channels(),
            _index,
            (unsigned char *)_dst.data());
}

G_FCV_NAMESPACE1_END()
