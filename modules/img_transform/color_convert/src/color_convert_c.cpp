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

#include "modules/img_transform/color_convert/interface/color_convert_c.h"
#include "modules/img_transform/color_convert/interface/color_convert.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fcvCvtColor(
        CMat* src,
        CMat* dst,
        CColorConvertType cvt_type) {
    if (!checkCMat(src)) {
        LOG_ERR("The src is illegal, please check whether "
                "the attribute values ​​of src are correct");
        return -1;
    }

    if (!checkCMat(dst)) {
        LOG_ERR("The dst is illegal, please check whether "
                "the attribute values ​​of dst are correct");
        return -1;
    }

    Mat src_tmp;
    Mat dst_tmp;
    cmatToMat(src, src_tmp);
    cmatToMat(dst, dst_tmp);

    ColorConvertType type = static_cast<ColorConvertType>(cvt_type);

    return cvt_color(src_tmp, dst_tmp, type);
}

G_FCV_NAMESPACE1_END()
