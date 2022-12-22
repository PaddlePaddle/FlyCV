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

#include "modules/img_transform/extract_channel/include/extract_channel_common.h"

#include <cstdint>

#include "modules/core/parallel/interface/parallel.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template <typename T>
class ExtractWithMemcpyTaskCommon : public ParallelTask {
public:
    ExtractWithMemcpyTaskCommon(const T* src, int width, int height, int stride, int channel, int index, T* dst)
            : _src(src), _dst(dst), _width(width), _height(height), _stride(stride), _channel(channel), _index(index) {
        _stride /= sizeof(T);
        _width = FCV_MAX(_width, _stride / _channel);
    }

    void operator()(const Range& range) const {
        const T* src_data = _src;
        T* dst_data = _dst;

        if (_channel == 3) {
            // the first address of each channel
            for (int i = range.start(); i < range.end(); i++) {
                for (int j = 0; j < _width; j++) {
                    dst_data[i * _width + j] = src_data[(i * _width + j) * _channel + _index];
                }
            }
        } else {
            LOG_ERR("the channel extract is not supported!\n");
        }
        return;
    }

private:
    const T* _src;
    T* _dst;
    int _width;
    int _height;
    int _channel;
    int _stride;
    int _index;
};

int extract_channel_common(Mat& _src, Mat& _dst, int _index) {
    ExtractWithMemcpyTaskCommon<unsigned char> task((const unsigned char*)_src.data(),
            _src.width(),
            _src.height(),
            _src.stride(),
            _src.channels(),
            _index,
            (unsigned char*)_dst.data());
    parallel_run(Range(0, _src.height()), task);
    return 0;
}

G_FCV_NAMESPACE1_END()
