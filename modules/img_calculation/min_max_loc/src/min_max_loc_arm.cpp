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

#include "modules/img_calculation/min_max_loc/include/min_max_loc_arm.h"

#include <cstdint>
#include <deque>
#include <thread>
#include <mutex>

#include <arm_neon.h>

#include "modules/core/parallel/interface/parallel.h"
#include "modules/img_calculation/min_max_loc/include/min_max_loc_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#if __aarch64__
#else
inline uint8_t vminvq_u8(uint8x16_t a) {
    uint8_t buf[16] = {0};
    vst1q_u8(buf, a);
    uint8_t tmp = buf[0];
    for (int i = 1; i < 16; i++) {
        tmp = buf[i] < tmp ? buf[i] : tmp;
    }
    return tmp;
}

inline uint8_t vmaxvq_u8(uint8x16_t a) {
    uint8_t buf[16] = {0};
    vst1q_u8(buf, a);
    uint8_t tmp = buf[0];
    for (int i = 1; i < 16; i++) {
        tmp = buf[i] > tmp ? buf[i] : tmp;
    }
    return tmp;
}
#endif

inline uint8_t get_min_idx(uint8_t val, uint8x16_t& data, uint8x16_t& index) {
    uint8x16_t mask = vceqq_u8(vdupq_n_u8(val), data);
    uint8x16_t background = vdupq_n_u8(255);
    return vminvq_u8(vbslq_u8(mask, index, background));
}

static void min_max_loc_neon_u8(const uint8_t* data,
        size_t len,
        double* min_val,
        size_t* min_idx,
        double* max_val,
        size_t* max_idx,
        uint8_t* mask) {
    if (len < 32) {
        min_max_loc_c(data, len, min_val, max_val, min_idx, max_idx, mask);
        return;
    }

    uint8_t tmp_min_val = data[0];
    uint8_t tmp_max_val = tmp_min_val;
    size_t tmp_min_idx = 0;
    size_t tmp_max_idx = 0;
    size_t base_idx = 0;

    size_t len_align16 = len & (~15);
    size_t remain = len - len_align16;

    const uint8_t* ptr_cur_data = data;
    uint8x16_t vec_min_val = vld1q_u8(ptr_cur_data);
    uint8x16_t vec_max_val = vec_min_val;

    uint8x16_t vec_zero = vdupq_n_u8(0);
    if (mask) {
        uint8x16_t vec_mask_1st = vcgtq_u8(vld1q_u8(mask), vec_zero);
        vec_min_val = vbslq_u8(vec_mask_1st, vec_min_val, vdupq_n_u8(255));
        vec_max_val = vbslq_u8(vec_mask_1st, vec_max_val, vdupq_n_u8(0));
    }
    ptr_cur_data += 16;

    const uint8_t INIT_INDEX[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint8x16_t vec_cur_idx = vld1q_u8(INIT_INDEX); 
    uint8x16_t vec_max_idx = vec_cur_idx;
    uint8x16_t vec_min_idx = vec_cur_idx;

    uint8x16_t vec_step = vdupq_n_u8(16); 

    int over_flow = 0;
    if (mask == nullptr) {
        for (size_t i = 16; i < len_align16; i += 16) {
            uint8x16_t vec_cur_val = vld1q_u8(ptr_cur_data);

            uint8x16_t vec_cur_min_flag = vcltq_u8(vec_cur_val, vec_min_val);
            uint8x16_t vec_cur_max_flag = vcgtq_u8(vec_cur_val, vec_max_val);
            // update val
            vec_min_val = vbslq_u8(vec_cur_min_flag, vec_cur_val, vec_min_val);
            vec_max_val = vbslq_u8(vec_cur_max_flag, vec_cur_val, vec_max_val);
            // current vec index
            vec_cur_idx = vaddq_u8(vec_cur_idx, vec_step);
            // update idx
            vec_min_idx = vbslq_u8(vec_cur_min_flag, vec_cur_idx, vec_min_idx);
            vec_max_idx = vbslq_u8(vec_cur_max_flag, vec_cur_idx, vec_max_idx);

            ptr_cur_data += 16;
            over_flow += 1;

            if ((over_flow >= 15) || (i == len_align16)) {
                uint8_t cur_min_val = vminvq_u8(vec_min_val);
                if (cur_min_val < tmp_min_val) {
                    tmp_min_val = cur_min_val;
                    tmp_min_idx = base_idx + get_min_idx(tmp_min_val, vec_min_val, vec_min_idx);
                }

                uint8_t cur_max_val = vmaxvq_u8(vec_max_val);
                if (cur_max_val > tmp_max_val) {
                    tmp_max_val = cur_max_val;
                    tmp_max_idx = base_idx + get_min_idx(tmp_max_val, vec_max_val, vec_max_idx);
                }

                base_idx += (15 * 16);
                over_flow = 0;
                vec_cur_idx = vld1q_u8(INIT_INDEX);
                vec_max_idx = vec_cur_idx;
                vec_min_idx = vec_cur_idx;
            }
        }
    } else {
        for (size_t i = 16; i < len_align16; i += 16) {
            uint8x16_t vec_cur_val = vld1q_u8(ptr_cur_data);

            uint8x16_t vec_cur_mask = vcgtq_u8(vld1q_u8(mask + i), vec_zero);

            uint8x16_t vec_cur_min_flag = vandq_u8(vcltq_u8(vec_cur_val, vec_min_val), vec_cur_mask);
            uint8x16_t vec_cur_max_flag = vandq_u8(vcgtq_u8(vec_cur_val, vec_max_val), vec_cur_mask);
            // update val
            vec_min_val = vbslq_u8(vec_cur_min_flag, vec_cur_val, vec_min_val);
            vec_max_val = vbslq_u8(vec_cur_max_flag, vec_cur_val, vec_max_val);
            // current vec index
            vec_cur_idx = vaddq_u8(vec_cur_idx, vec_step);
            // update idx
            vec_min_idx = vbslq_u8(vec_cur_min_flag, vec_cur_idx, vec_min_idx);
            vec_max_idx = vbslq_u8(vec_cur_max_flag, vec_cur_idx, vec_max_idx);

            ptr_cur_data += 16;
            over_flow += 1;

            if ((over_flow >= 15) || (i == len_align16)) {
                uint8_t cur_min_val = vminvq_u8(vec_min_val);
                if (cur_min_val < tmp_min_val) {
                    tmp_min_val = cur_min_val;
                    tmp_min_idx = base_idx + get_min_idx(tmp_min_val, vec_min_val, vec_min_idx);
                }

                uint8_t cur_max_val = vmaxvq_u8(vec_max_val);
                if (cur_max_val > tmp_max_val) {
                    tmp_max_val = cur_max_val;
                    tmp_max_idx = base_idx + get_min_idx(tmp_max_val, vec_max_val, vec_max_idx);
                }

                base_idx += (15 * 16);
                over_flow = 0;
                vec_cur_idx = vld1q_u8(INIT_INDEX);
                vec_max_idx = vec_cur_idx;
                vec_min_idx = vec_cur_idx;
            }
        }
    }

    double remain_min_val = double(ptr_cur_data[0]);
    double remain_max_val = remain_min_val;
    size_t remain_min_idx = 0;
    size_t remain_max_idx = 0;
    if (remain) {
        // do remain data
        min_max_loc_c<uint8_t>(data + len_align16,
                      remain,
                      &remain_min_val,
                      &remain_max_val,
                      &remain_min_idx,
                      &remain_max_idx,
                      nullptr);
        remain_min_idx += len_align16;
        remain_max_idx += len_align16;

        if (remain_min_val < tmp_min_val) {
            tmp_min_val = uint8_t(remain_min_val);
            tmp_min_idx = remain_min_idx;
        }
        if (remain_max_val > tmp_max_val) {
            tmp_max_val = uint8_t(remain_max_val);
            tmp_max_idx = remain_max_idx;
        }
    }
    *min_val = double(tmp_min_val);
    *min_idx = tmp_min_idx;

    *max_val = double(tmp_max_val);
    *max_idx = tmp_max_idx;
    return;
}

int min_max_loc_arm(
        const Mat& src,
        double* min_val,
        double* max_val,
        Point* min_loc,
        Point* max_loc,
        const Mat& mask) {
    int res = 0;
    if (src.width() * src.type_byte_size() != src.stride()) {
        // TODO: non-continous image data
        LOG_ERR("The src mat in not continous!");
        return -1;
    }
    uint8_t* mask_data = nullptr;
    if (!mask.empty()) {
        if ((mask.width() != src.width()) || (mask.height() != src.height())) {
            LOG_ERR("The size of mask is not equal to the src!");
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
        min_max_loc_neon_u8(reinterpret_cast<const uint8_t*>(src.data()),
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
