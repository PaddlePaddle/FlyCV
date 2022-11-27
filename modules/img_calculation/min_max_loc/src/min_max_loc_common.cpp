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

#include "modules/img_calculation/min_max_loc/include/min_max_loc_common.h"

#include <deque>
#include <thread>
#include <mutex>

#include "modules/core/parallel/interface/parallel.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template<typename T>
static int min_max_loc_c(
        const T* data,
        size_t len,
        double* min_val,
        double* max_val,
        size_t* min_index,
        size_t* max_index,
        uint8_t* mask) {
    T tmp_min = data[0];
    size_t index_min = 0;
    T tmp_max = data[0];
    size_t index_max = 0;
    if (mask) {
        for (size_t i = 0; i < len; i++) {
            if (mask[i] == 0) {
                continue;
            }
            if (data[i] < tmp_min) {
                tmp_min = data[i];
                index_min = i;
            }
            if (data[i] > tmp_max) {
                tmp_max = data[i];
                index_max = i;
            }
        }
    } else {
        for (size_t i = 0; i < len; i++) {
            if (data[i] < tmp_min) {
                tmp_min = data[i];
                index_min = i;
            }
            if (data[i] > tmp_max) {
                tmp_max = data[i];
                index_max = i;
            }
        }
    }
    *min_val = double(tmp_min);
    *max_val = double(tmp_max);
    *min_index = index_min;
    *max_index = index_max;
    return 0;
}

template<typename T>
struct MinMaxInfo {
    T min_val;
    size_t min_idx;
    T max_val;
    size_t max_idx;
};

template<typename T>
class Min_Max_Loc_ParallelTask : public ParallelTask {
public:
    Min_Max_Loc_ParallelTask(
            const T* data,
            size_t len, 
            std::mutex* ptr_mtx, 
            std::deque<MinMaxInfo<T>>* ptr_result)
            : _data(data),
            _len(len),
            _ptr_mtx(ptr_mtx),
            _ptr_result(ptr_result) {}

    void operator() (const Range& range) const override {
        T cur_min_val = _data[range.start()];
        size_t cur_min_idx = range.start();
        T cur_max_val = cur_min_val;
        size_t cur_max_idx = cur_min_idx;
        for (int i = (range.start() + 1); i < range.end(); ++i) {
            if (_data[i] < cur_min_val) {
                cur_min_val = _data[i];
                cur_min_idx = i;
            }
            if (_data[i] > cur_max_val) {
                cur_max_val = _data[i];
                cur_max_idx = i;
            }
        }
        MinMaxInfo<T> tmp;
        tmp.min_val = cur_min_val;
        tmp.min_idx = cur_min_idx;
        tmp.max_val = cur_max_val;
        tmp.max_idx = cur_max_idx;

        _ptr_mtx->lock();
        _ptr_result->push_back(tmp);
        _ptr_mtx->unlock();
    }
private:
    const T* _data;
    size_t _len;

    std::mutex* _ptr_mtx;
    std::deque<MinMaxInfo<T>>* _ptr_result;
};

template<typename T>
void min_max_loc_multi_thread(const T* data, 
        size_t len, 
        double* min_val, 
        size_t* min_idx, 
        double* max_val, 
        size_t* max_idx, 
        uint8_t* mask) {
    if (len == 0) {
        return;
    }
    if (mask) {
        min_max_loc_c<T>(data, len, min_val, max_val, min_idx, max_idx, mask);
        return;
    }
    std::mutex mtx;
    std::deque<MinMaxInfo<T>> result;
    Min_Max_Loc_ParallelTask<T> task(data, len, &mtx, &result);
    parallel_run(Range(0, len), task);

    T tmp_min_val = result[0].min_val;
    size_t tmp_min_idx = result[0].min_idx;
    T tmp_max_val = result[0].max_val;
    size_t tmp_max_idx = result[0].max_idx;
    for (size_t i = 1; i < result.size(); i++) {
        if (result[i].min_val < tmp_min_val) {
            tmp_min_val = result[i].min_val;
            tmp_min_idx = result[i].min_idx;
        }
        if (result[i].min_val == tmp_min_val) {
            tmp_min_idx = (result[i].min_idx < tmp_min_idx) ? result[i].min_idx : tmp_min_idx;
        }

        if (result[i].max_val > tmp_max_val) {
            tmp_max_val = result[i].max_val;
            tmp_max_idx = result[i].max_idx;
        }
        if (result[i].max_val == tmp_max_val) {
            tmp_max_idx = (result[i].max_idx < tmp_max_idx) ? result[i].max_idx : tmp_max_idx;
        }
    }

    *min_val = double(tmp_min_val);
    *min_idx = tmp_min_idx;
    *max_val = double(tmp_max_val);
    *max_idx = tmp_max_idx;
}

int min_max_loc_common(
        const Mat& src,
        double* min_val,
        double* max_val,
        Point* min_loc,
        Point* max_loc,
        const Mat& mask) {
    int res = 0;
    if (src.width() * src.type_byte_size() != src.stride()) {
        // TODO: non-continous image data
        LOG_ERR("The src mat in not continous for min_max_loc!");
        return -1;
    }
    uint8_t* mask_data = nullptr;
    if (!mask.empty()) {
        if ((mask.width() != src.width()) || (mask.height() != src.height())) {
            LOG_ERR("The size of mask is not equal to the src for min_max_loc!");
            return -1;
        }
        if ((mask.type() != FCVImageType::GRAY_U8)) {
            LOG_ERR("The type of mask is not FCVImageType::GRAY_U8!");
            return -1;
        }
        mask_data = reinterpret_cast<uint8_t*>(mask.data());
    }
    double min_value = 0.;
    double max_value = 0.;
    size_t min_index = 0;
    size_t max_index = 0;

    size_t pixel_num = src.width() * src.height();
    switch (src.type()) {
    case FCVImageType::GRAY_U8:
        min_max_loc_multi_thread<uint8_t>(reinterpret_cast<const uint8_t*>(src.data()),
                pixel_num, 
                &min_value, 
                &min_index, 
                &max_value,
                &max_index, 
                mask_data);
        break;
    case FCVImageType::GRAY_U16:
        min_max_loc_multi_thread<uint16_t>(reinterpret_cast<const uint16_t*>(src.data()),
                pixel_num,
                &min_value,
                &min_index,
                &max_value,
                &max_index,
                mask_data);
        break;
    case FCVImageType::GRAY_S32:
        min_max_loc_multi_thread<int32_t>(reinterpret_cast<const int32_t*>(src.data()),
                pixel_num,
                &min_value,
                &min_index,
                &max_value,
                &max_index,
                mask_data);
        break;
    case FCVImageType::GRAY_F32:
        min_max_loc_multi_thread<float>(reinterpret_cast<const float*>(src.data()),
                pixel_num,
                &min_value,
                &min_index,
                &max_value,
                &max_index,
                mask_data);
        break;
    case FCVImageType::GRAY_F64:
        min_max_loc_multi_thread<double>(reinterpret_cast<const double*>(src.data()),
                pixel_num,
                &min_value,
                &min_index,
                &max_value,
                &max_index,
                mask_data);
        break;
    default:
        LOG_ERR("Unsupport type for min_max_loc_common!");
        res = -1;
        break;
    }

    if (res) {
        return -1;
    }
    if (min_val) {
        *min_val = min_value;
    }
    if (max_val) {
        *max_val = max_value;
    }
    if (min_loc) {
        *min_loc = Point(static_cast<int>(min_index % src.width()),
                static_cast<int>(min_index / src.width()));
    }
    if (max_loc) {
        *max_loc = Point(static_cast<int>(max_index % src.width()),
                static_cast<int>(max_index / src.width()));
    }

    return res;
}

G_FCV_NAMESPACE1_END()
