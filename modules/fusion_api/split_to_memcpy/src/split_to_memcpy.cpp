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

#include "modules/fusion_api/split_to_memcpy/interface/split_to_memcpy.h"

#include "modules/core/base/include/type_info.h"
#include "modules/fusion_api/split_to_memcpy/include/split_to_memcpy_common.h"

#ifdef HAVE_NEON
#include "modules/fusion_api/split_to_memcpy/include/split_to_memcpy_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int split_to_memcpy(const Mat& src, float* dst) {
    if (src.empty()) {
        LOG_ERR("The input src is empty!");
        return -1;
    }

    if (dst == nullptr) {
        LOG_ERR("The dst need to be allocated outside!");
        return -1;
    }

    TypeInfo cur_type_info;

    if (get_type_info(src.type(), cur_type_info)) {
        LOG_ERR("The src type is not supported!");
        return -1;
    }

    if (cur_type_info.data_type != DataType::F32) {
        LOG_ERR("Only support f32, the current src element data type is %d",
                int(cur_type_info.data_type));
        return -1;
    }

    if (src.channels() != 3 && src.channels() != 4) {
        LOG_ERR("Only support 3 or 4 channels, current src channels is %d", src.channels());
        return -1;
    }

    const int width = src.width();
    const int height = src.height();
    const int channel = src.channels();
    const float *src_ptr = (const float *)src.data();

    int ret = -1;

#ifdef HAVE_NEON
    ret = split_to_memcpy_neon(src_ptr, width, height, channel, dst);
#else
    ret = split_to_memcpy_common(src_ptr, width, height, channel, dst);
#endif

    return ret;
}

G_FCV_NAMESPACE1_END()
