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

#include "modules/fusion_api/normalize_to_submean_to_reorder/include/normalize_to_submean_to_reorder_common.h"
#include "modules/core/parallel/interface/parallel.h"
G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template<typename T, typename D>
class NormalizePermute3cCommonParallelTask : public ParallelTask {
public:
    NormalizePermute3cCommonParallelTask(
        const T* ptr_src,
        D* ptr_dst_b,
        D* ptr_dst_g,
        D* ptr_dst_r,
        const D* mean_param,
        const D* std_param,
        const uint32_t* index) : 
        _ptr_src(ptr_src),
        _ptr_dst_b(ptr_dst_b),
        _ptr_dst_g(ptr_dst_g),
        _ptr_dst_r(ptr_dst_r),
        _mean_param(mean_param),
        _std_param(std_param),
        _index(index) {}

    void operator()(const Range& range) const override {
        for (int i = range.start(); i < range.end(); i++) {
            D tmp_data[3];
            tmp_data[0] = (static_cast<D>((_ptr_src + i * 3)[0]) - _mean_param[0]) / _std_param[0];
            tmp_data[1] = (static_cast<D>((_ptr_src + i * 3)[1]) - _mean_param[1]) / _std_param[1];
            tmp_data[2] = (static_cast<D>((_ptr_src + i * 3)[2]) - _mean_param[2]) / _std_param[2];

            *(_ptr_dst_b + i) = tmp_data[_index[0]];
            *(_ptr_dst_g + i) = tmp_data[_index[1]];
            *(_ptr_dst_r + i) = tmp_data[_index[2]];
        }
    }

private:
    const T* _ptr_src;
    D* _ptr_dst_b;
    D* _ptr_dst_g;
    D* _ptr_dst_r;
    const D* _mean_param;
    const D* _std_param;
    const uint32_t* _index;
};

/**
 * @Note: Must allocate enough memory for the following parameters before call the function
 * @param src_bgr_hwc_data   :   w * h * 3 * sizeof(T)
 * @param w                     :   > 0
 * @param h                     :   > 0
 * @param mean_param            : 3 D-type data array
 * @param std_param             : 3 D-type data array
 * @param channel_reorder_index : 3 uint32-type data array
 * @param dst_bgr_chw_data  : w * h * 3 * sizeof(float)
 */
template<typename T, typename D>
int normalize_permute_3c(
        const T* src_bgr_hwc_data,
        const int w,
        const int h,
        const D mean_param[],
        const D std_param[],
        const uint32_t channel_reorder_index[],
        D dst_bgr_chw_data[]) {
    int total_pixel_num = w * h;
    // Reorder channel finally if needed.
    const uint32_t NATURAL_ORDER[3] = {0, 1, 2};
    const uint32_t* index = nullptr;
    if (channel_reorder_index == nullptr) {
        index = NATURAL_ORDER;
    } else {
        index = channel_reorder_index;
    }
    // caculate the start address
    const T* ptr_src = src_bgr_hwc_data;

    D* ptr_dst_b = dst_bgr_chw_data;
    D* ptr_dst_g = ptr_dst_b + total_pixel_num;
    D* ptr_dst_r = ptr_dst_g + total_pixel_num;
    NormalizePermute3cCommonParallelTask<T, D> task(
        ptr_src,
        ptr_dst_b,
        ptr_dst_g,
        ptr_dst_r,
        mean_param,
        std_param,
        index);
    parallel_run(Range(0, total_pixel_num), task);
    return 0;
}

template<typename T, typename D>
class Normalize3cCommonParallelTask : public ParallelTask {
public:
    Normalize3cCommonParallelTask(
        const T* ptr_src,
        D* ptr_dst,
        const D* mean_param,
        const D* std_param,
        const uint32_t* index) : 
        _ptr_src(ptr_src),
        _ptr_dst(ptr_dst),
        _mean_param(mean_param),
        _std_param(std_param),
        _index(index) {}

    void operator()(const Range& range) const override {
        for (int i = range.start(); i < range.end(); i++) {
            D tmp_data[3];
            tmp_data[0] = (static_cast<D>((_ptr_src + i * 3)[0]) - _mean_param[0]) / _std_param[0];
            tmp_data[1] = (static_cast<D>((_ptr_src + i * 3)[1]) - _mean_param[1]) / _std_param[1];
            tmp_data[2] = (static_cast<D>((_ptr_src + i * 3)[2]) - _mean_param[2]) / _std_param[2];

            (_ptr_dst + i * 3)[0] = tmp_data[_index[0]];
            (_ptr_dst + i * 3)[1] = tmp_data[_index[1]];
            (_ptr_dst + i * 3)[2] = tmp_data[_index[2]];
        }
    }

private:
    const T* _ptr_src;
    D* _ptr_dst;
    const D* _mean_param;
    const D* _std_param;
    const uint32_t* _index;
};

/**
 * @note: Must allocate enough memory for the following parameters before call the function
 * @param src_bgr_hwc_data   :   w * h * 3 * sizeof(T)
 * @param w                     :   > 0
 * @param h                     :   > 0
 * @param mean_param            : 3 D-type data array
 * @param std_param             : 3 D-type data array
 * @param channel_reorder_index : 3 uint32-type data array
 * @param dst_bgr_chw_data  : w * h * 3 * sizeof(float)
 */
template<typename T, typename D>
int normalize_3c(
        const T* src_hwc_data,
        const int w,
        const int h,
        const D mean_param[],
        const D std_param[],
        const uint32_t channel_reorder_index[],
        D dst_hwc_data[]) {
    int total_pixel_num = w * h;
    // Reorder channel finally if needed.
    const uint32_t NATURAL_ORDER[3] = {0, 1, 2};
    const uint32_t* index = nullptr;
    if (channel_reorder_index == nullptr) {
        index = NATURAL_ORDER;
    } else {
        index = channel_reorder_index;
    }
    // caculate the start address
    const T* ptr_src = src_hwc_data;
    D* ptr_dst = dst_hwc_data;
    Normalize3cCommonParallelTask<T, D> task(
        ptr_src,
        ptr_dst,
        mean_param,
        std_param,
        index);
    parallel_run(Range(0, total_pixel_num), task);
    return 0;
}

int normalize_to_submean_to_reorder_common(
        const Mat& src,
        const std::vector<float>& mean_params,
        const std::vector<float>& std_params,
        const std::vector<uint32_t>& channel_reorder_index,
        Mat& dst,
        bool output_package) {
    int res = -1;
    const uint32_t* ptr_order = nullptr;
    if (!channel_reorder_index.empty()
            && channel_reorder_index.size() == 3
            && channel_reorder_index[0] < 3
            && channel_reorder_index[1] < 3
            && channel_reorder_index[2] < 3) {
        ptr_order = channel_reorder_index.data();
    }
    switch (src.type()) {
    case FCVImageType::PKG_BGR_U8:
    case FCVImageType::PKG_RGB_U8:
        if (output_package) {
            res = normalize_3c(reinterpret_cast<const unsigned char*>(src.data()),
                    src.width(), src.height(), mean_params.data(), std_params.data(),
                    ptr_order, reinterpret_cast<float*>(dst.data()));
        } else {
            res = normalize_permute_3c(reinterpret_cast<const unsigned char*>(src.data()),
                    src.width(), src.height(), mean_params.data(), std_params.data(),
                    ptr_order, reinterpret_cast<float*>(dst.data()));
        }
        break;
    case FCVImageType::PKG_BGR_F32:
    case FCVImageType::PKG_RGB_F32:
        if (output_package) {
            res = normalize_3c(reinterpret_cast<const float*>(src.data()),
                    src.width(), src.height(), mean_params.data(), std_params.data(),
                    ptr_order, reinterpret_cast<float*>(dst.data()));

        } else {
            res = normalize_permute_3c(reinterpret_cast<const float*>(src.data()),
                    src.width(), src.height(), mean_params.data(), std_params.data(),
                    ptr_order, reinterpret_cast<float*>(dst.data()));
        }
        break;
    default:
        LOG_ERR("Unsupported src type for normalize_permute_3c\n");
        res = -1;
        break;
    }

    return res;
}

G_FCV_NAMESPACE1_END()