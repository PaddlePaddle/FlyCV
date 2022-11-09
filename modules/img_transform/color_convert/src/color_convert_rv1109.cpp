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

#include "modules/img_transform/color_convert/include/color_convert_rv1109.h"

#include "RockchipRga.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int cvt_color_rv1109(Mat& src, Mat& dst, FCVImageType type) {
    if (src.empty()) {
        LOG_ERR("Input Mat to cvtColor is empty !\n");
        return -1;
    }

    // Check input mat type
    int src_rga_format = RK_FORMAT_UNKNOWN;
    switch (src.type()) {
    case NV21:
        src_rga_format = RK_FORMAT_YCrCb_420_SP;
        break;
    case NV12:
        src_rga_format = RK_FORMAT_YCbCr_420_SP;
        break;
    case PKG_BGR_U8:
        src_rga_format = RK_FORMAT_BGR_888;
        break;
    case PKG_RGB_U8:
        src_rga_format = RK_FORMAT_RGB_888;
        break;
    default:
        break;
    }

    if (src_rga_format == RK_FORMAT_UNKNOWN) {
        LOG_ERR("The color type of input mat is not support now for rga convert!\n");
        return -1;
    }

    // Check output mat type
    int dst_rga_format = RK_FORMAT_UNKNOWN;
    switch (type) {
    case PKG_BGR_U8:
        dst_rga_format = RK_FORMAT_BGR_888;
        break;
    case PKG_RGB_U8:
        dst_rga_format = RK_FORMAT_RGB_888;
        break;
    case NV21:
        dst_rga_format = RK_FORMAT_YCrCb_420_SP;
        break;
    case NV12:
        dst_rga_format = RK_FORMAT_YCbCr_420_SP;
        break;
    default:
        break;
    };

    if (dst_rga_format == RK_FORMAT_UNKNOWN) {
        LOG_ERR("The target color_type is not support now for rga convert!\n");
        return -1;
    }

    if (dst.width() != src.width()
            || dst.height() != src.height() || dst.type() != type) {
        dst = Mat(src.width(), src.height(), type);
    }

    // get rga ability instance
    RockchipRga& rkRga(RockchipRga::get());

    // config rga src and dst info
    rga_info_t src_rga_info;
    rga_info_t dst_rga_info;
    memset(&src_rga_info, 0, sizeof(rga_info_t));
    src_rga_info.fd = -1;
    src_rga_info.mmuFlag = 1;
    src_rga_info.virAddr = src.data();
    memset(&dst_rga_info, 0, sizeof(rga_info_t));
    dst_rga_info.fd = -1;
    dst_rga_info.mmuFlag = 1;
    dst_rga_info.virAddr = dst.data();

    // set input and output valid aera
    rga_set_rect(&src_rga_info.rect, 0, 0, src.width(), src.height(), src.width(), src.height(), src_rga_format);
    rga_set_rect(&dst_rga_info.rect, 0, 0, dst.width(), dst.height(), dst.width(), dst.height(), dst_rga_format);

    // do rga resize
    int ret = rkRga.RkRgaBlit(&src_rga_info, &dst_rga_info, nullptr);
    if (ret < 0) {
        LOG_ERR("RGA image color convert failed!\n");
        return -1;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
