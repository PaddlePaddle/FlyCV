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

int min_max_loc_common(
        const Mat& src,
        double* min_val,
        double* max_val,
        Point* min_loc,
        Point* max_loc,
        const Mat& mask) {
    int res = 0;
    if (src.width() * src.type_byte_size() == src.stride()) {
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
            res = min_max_loc_c<uint8_t>(reinterpret_cast<const uint8_t*>(src.data()),
                    pixel_num, &min_value, &max_value, &min_index,
                    &max_index, mask_data);
            break;
        case FCVImageType::GRAY_U16:
            res = min_max_loc_c<uint16_t>(reinterpret_cast<const uint16_t*>(src.data()),
                    pixel_num, &min_value, &max_value, &min_index,
                    &max_index, mask_data);
            break;
        case FCVImageType::GRAY_S32:
            res = min_max_loc_c<int32_t>(reinterpret_cast<const int32_t*>(src.data()),
                    pixel_num, &min_value, &max_value, &min_index,
                    &max_index, mask_data);
            break;
        case FCVImageType::GRAY_F32:
            res = min_max_loc_c<float>(reinterpret_cast<const float*>(src.data()),
                    pixel_num, &min_value, &max_value, &min_index,
                    &max_index, mask_data);
            break;
        case FCVImageType::GRAY_F64:
            res = min_max_loc_c<double>(reinterpret_cast<const double*>(src.data()),
                    pixel_num, &min_value, &max_value, &min_index,
                    &max_index, mask_data);
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
    } else {
        // TODO: non-continous image data
        LOG_ERR("The src mat in not continous for min_max_loc!");
        res = -1;
    }

    return res;
}

G_FCV_NAMESPACE1_END()
