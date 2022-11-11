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

#include "modules/fusion_api/y420sp_to_resize_to_bgr/interface/y420sp_to_resize_to_bgr.h"
#include "modules/fusion_api/y420sp_to_resize_to_bgr/include/y420sp_to_resize_to_bgr_common.h"

#ifdef HAVE_NEON
#include "modules/fusion_api/y420sp_to_resize_to_bgr/include/y420sp_to_resize_to_bgr_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int nv12_to_resize_to_bgr(
        Mat& src,
        Mat& dst,
        Size size,
        InterpolationType interpolation) {
    int ret = -1;

    if (src.empty()) {
        LOG_ERR("The src mat is empty!");
        return -1;
    }

    if (dst.empty()) {
        if (size.width() != 0 && size.height() != 0) {
            dst = Mat(size, FCVImageType::PKG_BGR_U8);
        } else {
            LOG_ERR("The dst size is illegal!");
            return -1;
        }
    }

    if (src.type() != FCVImageType::NV12
            || dst.type() != FCVImageType::PKG_BGR_U8) {
        LOG_ERR("The src or dst mat type is wrong!");
        return -1;
    }

    switch(interpolation) {
    case InterpolationType::INTER_LINEAR:
#ifdef HAVE_NEON
        ret = y420sp_to_resize_bilinear_to_bgr_neon(src, dst, true);
#else
        ret = y420sp_to_resize_bilinear_to_bgr_common(src, dst, true);
#endif
        break;
    case InterpolationType::INTER_NEAREST:
#ifdef HAVE_NEON
        ret = y420sp_to_resize_nearest_to_bgr_neon(src, dst, true);
#else
        ret = y420sp_to_resize_nearest_to_bgr_common(src, dst, true);
#endif
        break;
    case InterpolationType::INTER_CUBIC:
    default:
        LOG_ERR("Unsupport interpolation type!");
        break;
    };

    return ret;
}

int nv21_to_resize_to_bgr(
        Mat& src,
        Mat& dst,
        Size size,
        InterpolationType interpolation) {
    int ret = -1;
    if (src.empty()) {
        LOG_ERR("The src mat is empty!");
        return -1;
    }

    if (dst.empty()) {
        if (size.width() != 0 && size.height() != 0) {
            dst = Mat(size, FCVImageType::PKG_BGR_U8);
        } else {
            LOG_ERR("The dst size is illegal!");
            return -1;
        }
    }

    if (src.type() != FCVImageType::NV21
            || dst.type() != FCVImageType::PKG_BGR_U8) {
        LOG_ERR("The src or dst mat type is wrong!");
        return -1;
    }

    switch(interpolation) {
    case InterpolationType::INTER_LINEAR:
#ifdef HAVE_NEON
        ret = y420sp_to_resize_bilinear_to_bgr_neon(src, dst, false);
#else
        ret = y420sp_to_resize_bilinear_to_bgr_common(src, dst, false);
#endif
        break;
    case InterpolationType::INTER_NEAREST:
#ifdef HAVE_NEON
        ret = y420sp_to_resize_nearest_to_bgr_neon(src, dst, false);
#else
        ret = y420sp_to_resize_nearest_to_bgr_common(src, dst, false);
#endif
        break;
    case InterpolationType::INTER_CUBIC:
    default:
        LOG_ERR("Unsupport interpolation type!");
        break;
    };

    return ret;
}

G_FCV_NAMESPACE1_END()
