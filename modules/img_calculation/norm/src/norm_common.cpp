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

#include "modules/img_calculation/norm/include/norm_common.h"
#include "modules/core/parallel/interface/parallel.h"
#include <iostream>
#include <cmath>
#include <mutex>

#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template<typename ST, typename DT>
class NormL1Task : public ParallelTask {
public:
    NormL1Task(const ST* data, DT& sum) :
            _data(data),
            _sum(sum) {}

    void operator() (const Range & range) const {
        DT temp_sum = 0;
        for(int i = range.start(); i < range.end(); i += 1) {
            DT v0 = (DT)FCV_ABS(_data[i * 4]);
            DT v1 = (DT)FCV_ABS(_data[i * 4 + 1]);
            DT v2 = (DT)FCV_ABS(_data[i * 4 + 2]);
            DT v3 = (DT)FCV_ABS(_data[i * 4 + 3]);
            temp_sum += v0 + v1 + v2 + v3;
        }

        sum_lock.lock();
        _sum += temp_sum;
        sum_lock.unlock();
    }

private:
    const ST* _data;
    DT& _sum;
    mutable std::mutex sum_lock;
};

/*L1: the sum of absolute value of src
for example: sum = |-1| + |2| = 3 */
template<typename ST, typename DT> static inline
double norm_L1(const ST* data, int n) {
    DT sum = 0;
    
    int n_align4 = n & (~3);
    int range_num = n_align4 >> 2;

    NormL1Task<ST, DT> task(data, sum);
    parallel_run(Range(0, range_num), task);

    for(int i = n_align4; i < n; i++) {
        sum += (DT)FCV_ABS(data[i]);
    }

    return (double)sum;
}

template<typename ST, typename DT>
class NormL2Task : public ParallelTask {
public:
    NormL2Task(const ST* data, DT& sum) :
            _data(data),
            _sum(sum) {}

    void operator() (const Range & range) const {
        
        DT temp_sum = 0;
        for(int i = range.start(); i < range.end(); i += 1) {
            DT v0 = (DT)FCV_ABS(_data[i * 4]);
            DT v1 = (DT)FCV_ABS(_data[i * 4 + 1]);
            DT v2 = (DT)FCV_ABS(_data[i * 4 + 2]);
            DT v3 = (DT)FCV_ABS(_data[i * 4 + 3]);
            temp_sum += v0 * v0 + v1 * v1 + v2 * v2 + v3 * v3;
        }

        sum_lock.lock();
        _sum += temp_sum;
        sum_lock.unlock();
    }

private:
    const ST* _data;
    DT& _sum;
    mutable std::mutex sum_lock;
};

/*L2: the Euclidean distance of src
for example: sum = sqrt((-1)^2 + (2)^2) = 5 */
template<typename ST, typename DT> static inline
double norm_L2(const ST* data, int n) {
    DT sum = 0;
    int n_align4 = n & (~3);
    int range_num = n_align4 >> 2;
    NormL2Task<ST, DT> task(data, sum);
    parallel_run(Range(0, range_num), task);

    for(int i = n_align4; i < n; i++) {
        DT v = data[i];
        sum += v * v;
    }
    return (double)sum;
}


template<typename ST, typename DT>
class NormINFTask : public ParallelTask {
public:
    NormINFTask(const ST* data, double& max) :
            _data(data),
            _max(max) {}

    void operator() (const Range & range) const {
        double temp_max = 0;
        for(int i = range.start(); i < range.end(); i += 1) {
           DT v = static_cast<DT>(_data[i]);
           temp_max = FCV_MAX(temp_max, v);
        }

        sum_lock.lock();
        _max = FCV_MAX(_max, temp_max);
        sum_lock.unlock();
    }

private:
    const ST* _data;
    double& _max;
    mutable std::mutex sum_lock;
};

/*Inf: the max value of absolute value of src
for example: max =(|-1|, |2|) = 2 */
template<typename ST, typename DT>
static inline double norm_Inf(const ST* data, int n) {
    double max = 0;
    
    NormINFTask<ST, DT> task(data, max);
    parallel_run(Range(0, n), task);

    return max;
}

static double norm_l1_u8(void* src, int n) {
    return norm_L1<unsigned char, double>(reinterpret_cast<unsigned char*>(src), n);
}

static double norm_l1_f32(void* src, int n) {
    n /= sizeof(float);
    return norm_L1<float, double>(reinterpret_cast<float*>(src), n);
}

static double norm_l2_u8(void* src, int n) {
    double result = norm_L2<unsigned char, double>(reinterpret_cast<unsigned char*>(src), n);
    return std::sqrt(result);
}

static double norm_l2_f32(void* src, int n) {
    n /= sizeof(float);
    double result = norm_L2<float, double>(reinterpret_cast<float*>(src), n);
    return std::sqrt(result);
}

static double norm_inf_u8(void* src, int n) {
    return norm_Inf<unsigned char, int>(reinterpret_cast<unsigned char*>(src), n);
}

static double norm_inf_f32(void* src, int n) {
    n /= sizeof(float);
    return norm_Inf<float, float>(reinterpret_cast<float*>(src), n);
}

typedef double (*NormFuncCommon)(void*, int);

static NormFuncCommon get_norm_func(NormType norm_type, DataType data_type) {
    static std::map<NormType, NormFuncCommon> norm_funcs_u8 = {
        {NormType::NORM_L1, norm_l1_u8},
        {NormType::NORM_L2, norm_l2_u8},
        {NormType::NORM_INF, norm_inf_u8}
    };

    static std::map<NormType, NormFuncCommon> norm_funcs_f32 = {
        {NormType::NORM_L1, norm_l1_f32},
        {NormType::NORM_L2, norm_l2_f32},
        {NormType::NORM_INF, norm_inf_f32}
    };

    if (data_type == DataType::UINT8) {
        if (norm_funcs_u8.find(norm_type) != norm_funcs_u8.end()) {
            return norm_funcs_u8[norm_type];
        } else {
            return nullptr;
        }
    } else if (data_type == DataType::F32) {
        if (norm_funcs_f32.find(norm_type) != norm_funcs_f32.end()) {
            return norm_funcs_f32[norm_type];
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
}

int norm_common(Mat& src, NormType norm_type, double& result) {
    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return -1;
    }

    NormFuncCommon norm_func = get_norm_func(norm_type, type_info.data_type);

    if (norm_func) {
        result = norm_func(src.data(), src.height() * src.stride());
        return 0;
    } else {
        LOG_ERR("There is no matching norm function!");
        return -1;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
