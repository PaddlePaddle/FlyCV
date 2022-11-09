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

#include "modules/img_transform/color_convert/interface/color_convert.h"
#include "modules/img_transform/color_convert/include/color_convert_common.h"

#ifdef __RV1109_OPTIMIZATION__
#include "modules/img_transform/color_convert/include/color_convert_rv1109.h"
#endif

#ifdef HAVE_NEON
#include "modules/img_transform/color_convert/include/color_convert_arm.h"
#endif

#ifdef HAVE_AVX
#include "modules/img_transform/color_convert/include/color_convert_avx.h"
#endif

#ifdef HAVE_SSE
#include "modules/img_transform/color_convert/include/color_convert_sse.h"
#endif

#ifdef HAVE_OPENCL
#include "modules/core/opencl/interface/opencl.h"
#include "modules/img_transform/color_convert/include/color_convert_ocl.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int cvt_color(
        const Mat& src,
        Mat& dst,
        ColorConvertType cvt_type) {
    if (src.empty()) {
        LOG_ERR("Input Mat to cvtColor is empty!");
        return -1;
    }
    int type = get_cvt_color_dst_mat_type(cvt_type);
    if (dst.empty()) {
        dst = Mat(src.width(), src.height(), static_cast<FCVImageType>(type));
    }
    if (src.width() != dst.width() || src.height() != dst.height() || int(dst.type()) != int(type)) {
        LOG_ERR("illegal size [%d, %d] or format %d of the dst object while cvt_color", 
                dst.width(), 
                dst.height(), 
                int(dst.type()));
        return -1;
    }

    int res = -1;

#ifdef __RV1109_OPTIMIZATION__
    res = cvt_color_rv1109(src, dst, type);
#endif

#ifdef HAVE_OPENCL
    if (ocl::isUseOpenCL()) {
        res = cvt_color_opencl(src, dst, cvt_type);
        if (res == 0) {
            return res;
        }
    }
#endif

#ifdef HAVE_NEON
    res = cvt_color_neon(src, dst, cvt_type);
#elif defined(HAVE_AVX)
    res = cvt_color_avx(src, dst, cvt_type);
#else
    res = cvt_color_c(src, dst, cvt_type);
#endif

    return res;
}

int cvt_color(
        const Mat& src_y,
        Mat& src_u,
        Mat& src_v,
        Mat& dst,
        ColorConvertType cvt_type) {
    if (src_y.empty() || src_u.empty() || src_v.empty()) {
        LOG_ERR("Input Mat to cvtColor is empty !");
        return -1;
    }

    if (dst.empty()) {
        int type = get_cvt_color_dst_mat_type(cvt_type);
        dst = Mat(src_y.width(), src_y.height(), static_cast<FCVImageType>(type));
    }

    int res = -1;

#ifdef HAVE_NEON
    res = cvt_color_neon(src_y, src_u, src_v, dst, cvt_type);
#elif defined(HAVE_AVX)
    res = cvt_color_avx(src_y, src_u, src_v, dst, cvt_type);
#elif defined(HAVE_SSE)
    res = cvt_color_sse(src_y, src_u, src_v, dst, cvt_type);
#else
    res = cvt_color_c(src_y, src_u, src_v, dst, cvt_type);
#endif

    return res;
}

G_FCV_NAMESPACE1_END()
