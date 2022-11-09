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

#include "modules/img_transform/add_weighted/include/add_weighted_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int add_weighted_common(
        Mat& src1,
        double alpha,
        Mat& src2,
        double beta,
        double gamma,
        Mat& dst) {
    if (src1.empty() || src2.empty()) {
        LOG_ERR("the src is empty");
        return -1;
    }

    if (src1.width() != src2.width()
            || src1.height() != src2.height()
            || src1.type() != src2.type()) {
        LOG_ERR("src1 and src2 is not match, they width height type must be same.");
        return -1;
    }

    if (dst.empty()) {
        dst = Mat(src1.width(), src1.height(), src1.type());
    }

    const unsigned char * src1_ptr = (const unsigned char *)src1.data();
    const unsigned char * src2_ptr = (const unsigned char *)src2.data();
    unsigned char * dst_ptr = (unsigned char *)dst.data();
    const int src1_stride = src1.stride();
    const int src2_stride = src2.stride();
    const int dst_stride = dst.stride();

    // 两个图象执行融合
    for (int i = 0; i < src1.height(); i++) {
        const unsigned char * src1_row_start = src1_ptr;
        const unsigned char * src2_row_start = src2_ptr;
        unsigned char * dst_row_start = dst_ptr;

        for (int j = 0; j < src1.width(); j++) {
            *(dst_row_start + j * 0) = *(src1_row_start + j * 0) * alpha +
                    *(src2_row_start + j * 0) * beta + gamma;
            *(dst_row_start + j * 1) = *(src1_row_start + j * 1) * alpha +
                    *(src2_row_start + j * 1) * beta + gamma;
            *(dst_row_start + j * 2) = *(src1_row_start + j * 2) * alpha +
                    *(src2_row_start + j * 2) * beta + gamma;
        }

        src1_ptr += src1_stride;
        src2_ptr += src2_stride;
        dst_ptr += dst_stride;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
