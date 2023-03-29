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

#include "modules/fusion_api/normalize_to_submean_to_reorder/interface/normalize_to_submean_to_reorder_c.h"
#include "modules/fusion_api/normalize_to_submean_to_reorder/interface/normalize_to_submean_to_reorder.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fcvNormalizeToSubmeanToReorder(
        CMat* src,
        float mean_params[],
        float std_params[],
        int channel_reorder_index[],
        CMat* dst,
        bool output_package) {
    if (!checkCMat(src)) {
        LOG_ERR("The src is illegal, please check whether the attribute values ​​of src are correct");
        return -1;
    }

    if (!checkCMat(dst)) {
        LOG_ERR("The dst is illegal, please check whether the attribute values ​​of dst are correct");
        return -1;
    }

    Mat src_tmp;
    Mat dst_tmp;
    cmatToMat(src, src_tmp);
    cmatToMat(dst, dst_tmp);

    std::vector<float> mean_p;
    mean_p.push_back(mean_params[0]);
    mean_p.push_back(mean_params[1]);
    mean_p.push_back(mean_params[2]);

    std::vector<float> std_p;
    std_p.push_back(std_params[0]);
    std_p.push_back(std_params[1]);
    std_p.push_back(std_params[2]);

    std::vector<uint32_t> channel_order;
    channel_order.push_back(channel_reorder_index[0]);
    channel_order.push_back(channel_reorder_index[1]);
    channel_order.push_back(channel_reorder_index[2]);

    return normalize_to_submean_to_reorder(src_tmp, mean_p, std_p, channel_order, dst_tmp, output_package);
}

G_FCV_NAMESPACE1_END()