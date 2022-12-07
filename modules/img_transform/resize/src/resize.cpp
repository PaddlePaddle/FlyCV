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

#include "modules/img_transform/resize/interface/resize.h"
#include "modules/img_transform/resize/include/resize_common.h"

#include <cmath>

#ifdef __RV1109_OPTIMIZATION__
#include "modules/img_transform/resize/include/resize_rv1109.h"
#endif

#ifdef HAVE_NEON
#include "modules/img_transform/resize/include/resize_arm.h"
#endif

#ifdef HAVE_SSE
#include "modules/img_transform/resize/include/resize_sse.h"
#endif

#ifdef HAVE_AVX
#include "modules/img_transform/resize/include/resize_avx.h"
#endif

#ifdef HAVE_OPENCL
#include "modules/core/opencl/interface/opencl.h"
#include "modules/img_transform/resize/include/resize_ocl.h"
#endif

#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int resize(
        Mat& src,
        Mat& dst,
        const Size& dsize,
        double fx,
        double fy,
        InterpolationType interpolation) {
    if (src.empty()) {
        LOG_ERR("Input Mat of resize is empty!");
        return -1;
    }

    if (dst.empty()) {
        if (dsize.width() > 0 && dsize.height() > 0) {
            dst = Mat(dsize.width(), dsize.height(), src.type());
        } else if (fx > 0 && fy > 0) {
            dst = Mat(std::round(src.width() * fx), std::round(src.height() * fy), src.type());
        } else {
            LOG_ERR("Dst Mat width or height is zero which is illegal!");
            return -1;
        }
    }

    switch (interpolation) {
    case InterpolationType::INTER_LINEAR:
    case InterpolationType::INTER_CUBIC:
    case InterpolationType::INTER_AREA:
        break;
    default:
        LOG_ERR("The resize interpolation %d is unsupported now", int(interpolation));
        return -1;
    }

#ifdef __RV1109_OPTIMIZATION__
    return resize_rv1109(src, dst, dsize, fx, fy, interpolation);
#endif

#ifdef HAVE_OPENCL
    if (ocl::isUseOpenCL()) {
        int res = resize_opencl(src, dst, dsize);
        if (res == 0) {
            return res;
        }
    }
#endif

#ifdef HAVE_NEON
    if (interpolation == InterpolationType::INTER_LINEAR) {
        return resize_bilinear_neon(src, dst);
    } else if (interpolation == InterpolationType::INTER_CUBIC) {
        return resize_cubic_neon(src, dst);
    } else if (interpolation == InterpolationType::INTER_AREA) {
        return resize_area_neon(src, dst);
    }
//#elif defined(HAVE_AVX)
//    if (interpolation == InterpolationType::INTER_LINEAR) {
//        return resize_bilinear_avx(src, dst);
//    }
//    else if (interpolation == InterpolationType::INTER_CUBIC) {
//        return resize_cubic_avx(src, dst);
//    }
// #elif defined(HAVE_SSE)
//     if (interpolation == InterpolationType::INTER_LINEAR) {
//         return resize_bilinear_sse(src, dst);
//     } else if (interpolation == InterpolationType::INTER_CUBIC) {
//         return resize_cubic_sse(src, dst);
//     }
#else
    if (interpolation == InterpolationType::INTER_LINEAR) {
        return resize_bilinear_common(src, dst);
    } else if (interpolation == InterpolationType::INTER_CUBIC) {
        return resize_cubic_common(src, dst);
    } else if (interpolation == InterpolationType::INTER_AREA) {
        return resize_area_common(src, dst);
    }
#endif

    return 0;
}

G_FCV_NAMESPACE1_END()
