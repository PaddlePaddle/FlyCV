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

#include "modules/img_calculation/mean/interface/mean_c.h"
#include "modules/img_calculation/mean/interface/mean.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

CScalar fcvMean(CMat* src) {
    CScalar res;
    memset(res.val, 0, 4 * sizeof(double));

    if (!checkCMat(src)) {
         LOG_ERR("The src is illegal, please check whether "
                "the attribute values ​​of src are correct");
         return res;
    }

    Mat src_tmp;
    cmatToMat(src, src_tmp);

    Scalar s = mean(src_tmp);


    for (int i = 0; i < src_tmp.channels(); ++i) {
        res.val[i] = s[i];
    }

    return res;
}

int fcvMeanStddev(CMat* src, CMat* mean, CMat* stddev) {
    if (!checkCMat(src)) {
         LOG_ERR("The src is illegal, please check whether "
                "the attribute values ​​of src are correct");
         return -1;
    }

    if (!checkCMat(mean)) {
         LOG_ERR("The mean is illegal, please check whether "
                "the attribute values ​​of mean are correct");
         return -1;
    }

    if (!checkCMat(stddev)) {
         LOG_ERR("The stddev is illegal, please check whether "
                "the attribute values ​​of stddev are correct");
         return -1;
    }

    Mat src_tmp;
    Mat mean_tmp;
    Mat stddev_tmp;
    cmatToMat(src, src_tmp);

    mean_stddev(src_tmp, mean_tmp, stddev_tmp);
    memcpy(mean->data, mean_tmp.data(), mean_tmp.total_byte_size());
    memcpy(stddev->data, stddev_tmp.data(), stddev_tmp.total_byte_size());

    return 0;
}

G_FCV_NAMESPACE1_END()