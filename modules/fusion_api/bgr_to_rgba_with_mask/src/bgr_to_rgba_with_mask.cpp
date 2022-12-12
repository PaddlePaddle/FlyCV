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

#include "modules/fusion_api/bgr_to_rgba_with_mask/interface/bgr_to_rgba_with_mask.h"
#include "modules/fusion_api/bgr_to_rgba_with_mask/include/bgr_to_rgba_with_mask_common.h"

#ifdef HAVE_NEON
#include "modules/fusion_api/bgr_to_rgba_with_mask/include/bgr_to_rgba_with_mask_arm.h"
#endif

#ifdef HAVE_SSE
#include "modules/fusion_api/bgr_to_rgba_with_mask/include/bgr_to_rgba_with_mask_sse.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int bgr_to_rgba_with_mask(Mat& src, Mat& mask, Mat& dst) {
    if (src.empty() || mask.empty()) {
        LOG_ERR("Input mat or the mask is empty!");
        return -1;
    }
    if (src.type() != FCVImageType::PKG_BGR_U8 && src.type() != FCVImageType::PKG_RGB_U8) {
        LOG_ERR("The src type is not supported!");
        return -1;
    }

    if (dst.empty()) {
       dst = Mat(src.width(), src.height(), FCVImageType::PKG_RGBA_U8);
    }

    int ret = -1;

#ifdef HAVE_NEON
    ret = bgr_to_rgba_with_mask_neon(src, mask, dst);
#elif defined(HAVE_SSE)
    ret = bgr_to_rgba_with_mask_sse(src, mask, dst);
#else
    ret = bgr_to_rgba_with_mask_common(src, mask, dst);
#endif

    return ret;
}

G_FCV_NAMESPACE1_END()