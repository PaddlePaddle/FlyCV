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

#include "modules/fusion_api/bgra_to_resize_to_bgr/interface/bgra_to_resize_to_bgr.h"

#include "modules/fusion_api/bgra_to_resize_to_bgr/include/bgra_to_resize_to_bgr_common.h"

#ifdef HAVE_NEON
#include "modules/fusion_api/bgra_to_resize_to_bgr/include/bgra_to_resize_to_bgr_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int bgra_to_resize_to_bgr(
        Mat& src,
        Mat& dst,
        const Size& dsize,
        InterpolationType interpolation){
    if (src.empty()) {
        LOG_ERR("Input Mat of resize is empty!");
        return -1;
    }
    if ((src.type() != FCVImageType::PKG_BGRA_U8) && (src.type() != FCVImageType::PKG_RGBA_U8)) {
        LOG_ERR("Wrong type of input mat type when bgra_to_resize_to_bgr");
        return -1;
    }

    if (dst.empty()) {
        if (dsize.width() == 0 || dsize.height() == 0) {
            LOG_ERR("Dst Mat width or height is zero which is illegal!");
            return -1;
        } else {
            dst = Mat(dsize.width(), dsize.height(), FCVImageType::PKG_BGR_U8);
        }
    }

#ifdef HAVE_NEON
    if (interpolation == InterpolationType::INTER_LINEAR) {
        return bgra_to_resize_bilinear_to_bgr_neon(src, dst);
    } else if (interpolation == InterpolationType::INTER_NEAREST) {
        return bgra_to_resize_nearest_to_bgr_neon(src, dst);
    } else {
        LOG_ERR("The resize interpolation type is not supported!");
        return -1;
    }
#else
    if (interpolation == InterpolationType::INTER_LINEAR) {
        return bgra_to_resize_bilinear_to_bgr_common(src, dst);
    } else if (interpolation == InterpolationType::INTER_NEAREST) {
        return bgra_to_resize_nearest_to_bgr_common(src, dst);
    } else {
        LOG_ERR("The resize interpolation type is not supported!");
        return -1;
    }
#endif
    return 0;
}

G_FCV_NAMESPACE1_END()