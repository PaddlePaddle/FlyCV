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

#include "modules/img_transform/subtract/interface/subtract.h"

#include "modules/core/base/include/type_info.h"
#include "modules/img_transform/subtract/include/subtract_common.h"
#ifdef HAVE_NEON
#include "modules/img_transform/subtract/include/subtract_arm.h"
#endif

#ifdef HAVE_SVE2
#include "modules/img_transform/subtract/include/subtract_sve.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int subtract(const Mat& src, Scalar scalar, Mat& dst) {
    if (src.empty()) {
        LOG_ERR("the src is empty!");
        return -1;
    }
    TypeInfo cur_type_info;
    if (get_type_info(src.type(), cur_type_info)) {
        LOG_ERR("failed to get type info from src mat while get_type_info");
        return -1;
    }
    if (cur_type_info.data_type != DataType::F32) {
        LOG_ERR("subtract only support f32 data, the current src element data type is %d", 
                int(cur_type_info.data_type));
        return -1;
    }

    const int width = src.width();
    const int height = src.height();
    const int stride = src.stride() >> 2;
    const int channel = src.channels();

    if (dst.empty()) {
        dst = Mat(src.width(), src.height(), src.type());
    }

    if (dst.width() != src.width() 
            || dst.height() != src.height() 
            || dst.channels() != src.channels() 
            || dst.type() != src.type()) {
        LOG_ERR("illegal format of dst mat to subtract, which should be same size and type with src");
    }

    const float *src_data = (const float*)src.data();
    float *dst_data = (float*)dst.data();

#ifdef HAVE_SVE2
    subtract_sve(src_data, width, height, stride, channel, scalar, dst_data);
#elif defined(HAVE_NEON)
    subtract_neon(src_data, width, height, stride, channel, scalar, dst_data);
#else
    subtract_common(src_data, width, height, stride, channel, scalar, dst_data);
#endif

    return 0;
}

G_FCV_NAMESPACE1_END()