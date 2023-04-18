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

#include "modules/img_transform/add_weighted/interface/add_weighted.h"
#include "modules/img_transform/add_weighted/include/add_weighted_common.h"

#ifdef HAVE_NEON
#include "modules/img_transform/add_weighted/include/add_weighted_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int add_weighted(
        Mat& src1,
        double alpha,
        Mat& src2,
        double beta,
        double gamma,
        Mat& dst) {
    if (src1.empty()) {
        LOG_ERR("The first input mat is empty!");
        return -1;
    }

    if (src2.empty()) {
        LOG_ERR("The second input mat is empty!");
        return -1;
    }
    if ((src1.type() != FCVImageType::PKG_BGR_U8) && (src1.type() != FCVImageType::PKG_RGB_U8)) {
        LOG_ERR("The src1 mat type is not supported!");
        return -1;
    }
    if ((src2.type() != FCVImageType::PKG_BGR_U8) && (src2.type() != FCVImageType::PKG_RGB_U8)) {
        LOG_ERR("The src2 mat type is not supported!");
        return -1;
    }

    if (src1.type() != src2.type() || 
        (src1.type() != FCVImageType::PLA_BGR_U8 &&
        src1.type() != FCVImageType::PLA_RGB_U8 &&
        src1.type() != FCVImageType::PKG_BGR_U8 &&
        src1.type() != FCVImageType::PKG_RGB_U8)) {
        LOG_ERR("The input type is not surpport, which is %d \n", (int)src1.type());
        return -1;
    }

#ifdef HAVE_NEON
    auto status = add_weighted_neon(src1, alpha, src2, beta, gamma, dst);
    if (status != 0) {
        LOG_ERR("Add weighted arm failed!");
        return status;
    }
#else
    auto status = add_weighted_common(src1, alpha, src2, beta, gamma, dst);
    if (status != 0) {
        LOG_ERR("Add weighted common failed!");
        return status;
    }
#endif

    return 0;
}

G_FCV_NAMESPACE1_END()
