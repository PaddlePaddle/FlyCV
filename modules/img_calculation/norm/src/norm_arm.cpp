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

#include "modules/img_calculation/norm/include/norm_arm.h"
#include "modules/core/parallel/interface/parallel.h"
#include <iostream>

#include <cmath>
#include <arm_neon.h>
#include <map>
#include <mutex>
#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class NormL1NeonTask : public ParallelTask {

public:
    NormL1NeonTask(const unsigned char* src_ptr, 
            double& sum) : 
            _sum(sum), 
            _src_ptr(src_ptr)
            {}

    void operator() (const Range & range) const {
        uint32x4_t v_sum = vdupq_n_u32(0);
        uint8x8_t v0_u8, v1_u8, v2_u8, v3_u8;
        const unsigned char* tmp_src_ptr = _src_ptr + range.start() * 32;
        for (unsigned int i = range.start(); i < range.end(); ++i) {
            v0_u8 = vld1_u8(tmp_src_ptr);
            v1_u8 = vld1_u8(tmp_src_ptr + 8);
            uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8, v1_u8);
            v2_u8 = vld1_u8(tmp_src_ptr + 16);
            v3_u8 = vld1_u8(tmp_src_ptr + 24);
            uint16x8_t vtmp1_u16 = vaddl_u8(v2_u8, v3_u8);
            v_sum = vaddw_u16(v_sum,
                    vpadd_u16(vget_low_u16(vtmp0_u16), vget_high_u16(vtmp0_u16)));
            v_sum = vaddw_u16(v_sum,
                     vpadd_u16(vget_low_u16(vtmp1_u16), vget_high_u16(vtmp1_u16)));
            tmp_src_ptr += 32;
        }
        sum_lock.lock();
        _sum = _sum + v_sum[0] + v_sum[1] + v_sum[2] + v_sum[3];
        sum_lock.unlock();
    }

private:
    const unsigned char* _src_ptr;
    double& _sum;
    mutable std::mutex sum_lock;
};

static double norm_u8_l1_neon(const unsigned char* src_ptr, unsigned int len) {
    double sum = 0.f;

    // avoid numeric overflow (2^32 - 1) * 4 / 255
    unsigned int block_size = FCV_MIN(len, 67372036);
    unsigned int loop_cnt = len / block_size + 1;
    for (unsigned int i = 0; i < loop_cnt; ++i) {
        unsigned int step = (i + 1) * block_size > len ? len - i * block_size : block_size;
        unsigned int count = step / 32;

        NormL1NeonTask task(src_ptr, sum);
        parallel_run(Range(0, count), task);

        src_ptr += 32 * count;
        //remain pixels process
        for(unsigned int j = count * 32; j < step; ++j) {
            sum = sum + *src_ptr++;
        }
    }

    return sum;
}

class NormL2NeonTask : public ParallelTask {
public:
    NormL2NeonTask(const unsigned char* src_ptr, 
            double& sum) : 
            _sum(sum), 
            _src_ptr(src_ptr)
            {}

    void operator() (const Range & range) const {
        uint32x4_t v_sum = vdupq_n_u32(0);
        uint8x8_t v0_u8, v1_u8;
        const unsigned char* tmp_src_ptr = _src_ptr + range.start() * 16;
        for (unsigned int i = range.start(); i < range.end(); ++i) {
            v0_u8 = vld1_u8(tmp_src_ptr);
            v1_u8 = vld1_u8(tmp_src_ptr + 8);
            uint16x8_t vtmp0_u16 = vmull_u8(v0_u8, v0_u8);
            uint16x8_t vtmp1_u16 = vmull_u8(v1_u8, v1_u8);
            v_sum = vaddq_u32(v_sum,
                    vaddl_u16(vget_low_u16(vtmp0_u16), vget_high_u16(vtmp0_u16)));
            v_sum = vaddq_u32(v_sum,
                    vaddl_u16(vget_low_u16(vtmp1_u16), vget_high_u16(vtmp1_u16)));

            tmp_src_ptr += 16;
        }
        sum_lock.lock();
        _sum = _sum + v_sum[0] + v_sum[1] + v_sum[2] + v_sum[3];
        sum_lock.unlock();
    }

private:
    const unsigned char* _src_ptr;
    double& _sum;
    mutable std::mutex sum_lock;
};

double norm_u8_l2_neon(const unsigned char* src_ptr, unsigned int len) {
    double sum = 0.f;

    // avoid numeric overflow (2^32 - 1) * 4 / 255 / 255
    unsigned int block_size = FCV_MIN(len, 264204);
    unsigned int loop_cnt = len / block_size + 1;

    for (unsigned int i = 0; i < loop_cnt; ++i) {
        unsigned int step = (i + 1) * block_size > len ? len - i * block_size : block_size;
        unsigned int count = step / 16;

        NormL2NeonTask task(src_ptr, sum);
        parallel_run(Range(0, count), task);

        src_ptr += 16 * count;
        for(unsigned int j = count * 16; j < step; ++j) {
            sum = sum + src_ptr[0] * src_ptr[0];
            src_ptr++;
        }

        //sum = sum + v_sum[0] + v_sum[1] + v_sum[2] + v_sum[3];
    }

    return std::sqrt(sum);
}

class NormINFNeonTask : public ParallelTask {
public:
    NormINFNeonTask(const unsigned char* src_ptr, unsigned char& temp_max) : 
            _src_ptr(src_ptr),
            _temp_max(temp_max)
            {}

    void operator() (const Range & range) const {
        uint8x8_t v_res = vdup_n_u8(0);
        uint8x16_t v0_u8, v1_u8;
        const unsigned char* tmp_src_ptr = _src_ptr + range.start() * 32;
        for (unsigned int i = range.start(); i < range.end(); ++i) {
            v0_u8 = vld1q_u8(tmp_src_ptr);
            v1_u8 = vld1q_u8(tmp_src_ptr + 16);

            uint8x16_t vmax_v0v1 = vmaxq_u8(v0_u8, v1_u8);
            uint8x8_t vmax = vmax_u8(vget_low_u8(vmax_v0v1), vget_high_u8(vmax_v0v1));
            v_res = vmax_u8(v_res, vmax);
            tmp_src_ptr += 32;
        }
        unsigned char max0 = FCV_MAX(v_res[0], v_res[1]);
        unsigned char max1 = FCV_MAX(v_res[2], v_res[3]);
        unsigned char max2 = FCV_MAX(v_res[4], v_res[5]);
        unsigned char max3 = FCV_MAX(v_res[6], v_res[7]);

        max1 = FCV_MAX(max0, max1);
        max2 = FCV_MAX(max2, max3);
        unsigned char max4 = FCV_MAX(max1, max2);
        sum_lock.lock();
        _temp_max = FCV_MAX(max4, _temp_max);
        sum_lock.unlock();
    }

private:
    const unsigned char* _src_ptr;
    unsigned char& _temp_max;
    mutable std::mutex sum_lock;
};

double norm_u8_inf_neon(const unsigned char* src_ptr, unsigned int len) {
    unsigned char max = 0;
    int count = len / 32; //32 pixels each loop

    NormINFNeonTask task(src_ptr, max);
    parallel_run(Range(0, count), task);
    src_ptr = src_ptr + 32 * count;
    for (unsigned int i = 32 * count; i < len; ++i) {
        unsigned char v = *(src_ptr++);
        max = FCV_MAX(max, v);
    }

    return (double)max;
}

static double norm_u8_neon(
        const unsigned char* src,
        NormType norm_type,
        unsigned int len) {
    switch (norm_type) {
    case NormType::NORM_L1:
        return norm_u8_l1_neon(src, len);
    case NormType::NORM_L2:
        return norm_u8_l2_neon(src, len);
    case NormType::NORM_INF:
        return norm_u8_inf_neon(src, len);
    default:
        return 0.0f;
    };
}

int norm_neon(Mat& src, NormType norm_type, double& result) {
    TypeInfo type_info;
    if (get_type_info(src.type(), type_info) != 0) {
        LOG_ERR("The src type is not supported!");
        return -1;
    }

    unsigned int len = src.height() * src.stride() / src.type_byte_size();

    switch (type_info.data_type) {
    case DataType::UINT8:
        result = norm_u8_neon(reinterpret_cast<unsigned char*>(src.data()), norm_type, len);
        break;
    default:
        return -1;
    };

    return 0;
}

G_FCV_NAMESPACE1_END()
