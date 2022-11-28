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

#include "modules/img_transform/add_weighted/include/add_weighted_arm.h"
#include "modules/img_transform/add_weighted/include/add_weighted_common.h"
#include <arm_neon.h>

#include "modules/core/parallel/interface/parallel.h"

#include <iostream>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int add_weighted_neon(
        Mat& src1,
        double alpha,
        Mat& src2,
        double beta,
        double gamma,
        Mat& dst) {

    if (src1.empty() || src2.empty()) {
        LOG_ERR("The src is empty");
        return -1;
    }

    if (src1.width() != src2.width()
            || src1.height() != src2.height()
            || src1.type() != src2.type()) {
        LOG_ERR("The information of src do not match.");
        return -1;
    }

    if (dst.empty()) {
        dst = Mat(src1.width(), src1.height(), src1.type());
    }

#if __aarch64__
    unsigned char * src1_ptr = (unsigned char *)src1.data();
    unsigned char * src2_ptr = (unsigned char *)src2.data();
    unsigned char * dst_ptr = (unsigned char *)dst.data();

    uint8_t alpha_u8 = alpha * 256;
    uint8_t beta_u8 = beta * 256;
    uint16_t gamma_u16 = gamma;

    int nn = src1.height() * src1.width();

    int nn_neon = nn >> 4;
    int nn_remain = nn - (nn_neon << 4);

    asm volatile(
        "0:                                                      \n"
        "prfm   pldl1keep, [%0, #128]                            \n"
        "prfm   pldl1keep, [%1, #128]                            \n"

        "ld1    {v0.16b, v1.16b, v2.16b}, [%0], #48              \n"
        "ld1    {v3.16b, v4.16b, v5.16b}, [%1], #48              \n"
        "dup    v6.16b, %w3                                      \n"
        "dup    v7.16b, %w4                                      \n"
        "dup    v8.8h, %w5                                       \n"

        "subs   %[nn_neon], %[nn_neon], #1                       \n"

        "umull   v9.8h, v0.8b, v6.8b                             \n"
        "umull2  v10.8h, v0.16b, v6.16b                          \n"
        "umlal   v9.8h, v3.8b, v7.8b                             \n"
        "umlal2  v10.8h, v3.16b, v7.16b                          \n"

        "umull   v11.8h, v1.8b, v6.8b                            \n"
        "umull2  v12.8h, v1.16b, v6.16b                          \n"
        "umlal   v11.8h, v4.8b, v7.8b                            \n"
        "umlal2  v12.8h, v4.16b, v7.16b                          \n"

        "umull   v13.8h, v2.8b, v6.8b                            \n"
        "umull2  v14.8h, v2.16b, v6.16b                          \n"
        "umlal   v13.8h, v5.8b, v7.8b                            \n"
        "umlal2  v14.8h, v5.16b, v7.16b                          \n"

        "uqshrn  v9.8b, v9.8h, #8                                \n"
        "uqshrn  v10.8b, v10.8h, #8                              \n"
        "uqshrn  v11.8b, v11.8h, #8                              \n"
        "uqshrn  v12.8b, v12.8h, #8                              \n"
        "uqshrn  v13.8b, v13.8h, #8                              \n"
        "uqshrn  v14.8b, v14.8h, #8                              \n"

        "uaddw   v15.8h, v8.8h, v9.8b                            \n"
        "uaddw  v16.8h, v8.8h, v10.8b                            \n"

        "uaddw   v17.8h, v8.8h, v11.8b                           \n"
        "uaddw  v18.8h, v8.8h, v12.8b                            \n"

        "uaddw   v19.8h, v8.8h, v13.8b                           \n"
        "uaddw  v20.8h, v8.8h, v14.8b                            \n"

        "uqxtn v15.8b, v15.8h                                    \n"
        "uqxtn v16.8b, v16.8h                                    \n"
        "uqxtn v17.8b, v17.8h                                    \n"
        "uqxtn v18.8b, v18.8h                                    \n"
        "uqxtn v19.8b, v19.8h                                    \n"
        "uqxtn v20.8b, v20.8h                                    \n"

        "st1 {v15.8b, v16.8b, v17.8b, v18.8b}, [%2], #32         \n"
        "st1 {v19.8b, v20.8b}, [%2], #16                         \n"
        "b.gt 0b                                                 \n"
        : "+r"(src1_ptr),
          "+r"(src2_ptr),
          "+r"(dst_ptr),
          "+r"(alpha_u8),
          "+r"(beta_u8),
          "+r"(gamma_u16),
          [nn_neon]"+r"(nn_neon)
        :
        : "cc", "memory", "v0", "v1", "v2","v3","v4","v5","v6","v7","v8","v9","v10","v11",
                        "v12","v13","v14","v15","v16","v17","v18","v19","v20"
    );

    int nn_remain_byte = nn_remain * 3;
    for (int i = 0; i < nn_remain_byte; i++) {
        *(dst_ptr + i) = static_cast<unsigned char>(*(src1_ptr + i) * alpha + *(src2_ptr + i) * beta + gamma);
    }
#else
    auto status = add_weighted_common(src1, alpha, src2, beta, gamma, dst);
    if (status != 0) {
        LOG_ERR("Add weighted failed in arm32!");
        return status;
    }
#endif

    return 0;
}

G_FCV_NAMESPACE1_END()
