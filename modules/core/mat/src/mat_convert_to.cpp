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

#include "modules/core/mat/interface/mat.h"

#include "modules/core/basic_math/interface/basic_math.h"
#include "modules/core/mat/include/mat_convert_to_common.h"
#include "modules/core/base/include/type_info.h"

#ifdef HAVE_NEON
#include "modules/core/mat/include/mat_convert_to_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int Mat::convert_to(
        Mat& dst,
        FCVImageType dst_type,
        double scale,
        double shift) const {
    if (dst.empty()) {
        dst = Mat(_width, _height, dst_type);
    }

    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_c = dst.channels();

    if ((dst_w != _width) || (dst_h != _height) || (dst_c != _channels)) {
        LOG_ERR("The size of dst and src should be the same, "
                "width: %d -> %d, height: %d -> %d, channels: "
                "%d -> %d", dst_w, _width, dst_h, _height, dst_c, _channels);
        return -1;
    }

    TypeInfo type_info;
    int status = get_type_info(dst_type, type_info);

    if (status != 0 || (type_info.data_type != DataType::F32 &&
            type_info.data_type != DataType::F64)) {
        LOG_ERR("The dst_type is not supported!");
        return -1;
    }

#ifdef HAVE_NEON
    status = convert_to_neon(_type, _data, dst_type,
            dst_w * dst_h * dst_c, dst.data(), scale, shift);
    if (status == 0) {
        return 0;
    }
#endif

    convert_to_common(_type, _stride, _data,
            dst_w, dst_h, dst.data(), scale, shift);

    return 0;
}

G_FCV_NAMESPACE1_END()
