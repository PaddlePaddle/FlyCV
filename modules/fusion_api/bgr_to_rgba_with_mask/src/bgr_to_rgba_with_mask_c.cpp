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

#include "modules/fusion_api/bgr_to_rgba_with_mask/interface/bgr_to_rgba_with_mask_c.h"
#include "modules/fusion_api/bgr_to_rgba_with_mask/interface/bgr_to_rgba_with_mask.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fcvBgrToRgbaWithMask(CMat* src, CMat* mask, CMat* dst) {
    if (!checkCMat(src)) {
        LOG_ERR("The src is not illegal!");
        return -1;
    }

    if (!checkCMat(mask)) {
        LOG_ERR("The mask is not illegal!");
        return -1;
    }

    if (!checkCMat(dst)) {
        LOG_ERR("The dst is not illegal!");
        return -1;
    }

    Mat src_tmp;
    Mat mask_tmp;
    Mat dst_tmp;
    cmatToMat(src, src_tmp);
    cmatToMat(mask, mask_tmp);
    cmatToMat(dst, dst_tmp);

    return bgr_to_rgba_with_mask(src_tmp, mask_tmp, dst_tmp);;

}

G_FCV_NAMESPACE1_END()