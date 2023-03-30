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

#include "modules/img_transform/extract_channel/interface/extract_channel_c.h"
#include "modules/img_transform/extract_channel/interface/extract_channel.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int fcvExtractChannel(
        CMat* src,
        CMat* dst,
        int index) {
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
   
    return extract_channel(src_tmp, dst_tmp, index);
}

G_FCV_NAMESPACE1_END()
