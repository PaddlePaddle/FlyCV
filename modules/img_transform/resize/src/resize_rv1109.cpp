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

#include "modules/img_transform/resize/include/resize_rv1109.h"

#include "RockchipRga.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int resize_rv1109(
        Mat& src,
        Mat& dst,
        const Size& dsize,
        double fx,
        double fy,
        int interpolation) {
    // Check input mat type
    int rga_format = RK_FORMAT_UNKNOWN;
    switch (src.type()) {
    case PKG_BGR_U8:
        rga_format = RK_FORMAT_BGR_888;
        break;
    case PKG_BGRA_U8:
        rga_format = RK_FORMAT_BGRA_8888;
        break;
    case PKG_RGB_U8:
        rga_format = RK_FORMAT_RGB_888;
        break;
    case PKG_RGBA_U8:
        rga_format = RK_FORMAT_RGBA_8888;
        break;
    case NV21:
        rga_format = RK_FORMAT_YCrCb_420_SP;
        break;
    case NV12:
        rga_format = RK_FORMAT_YCbCr_420_SP;
        break;
    default:
        break;
    };

    if (rga_format == RK_FORMAT_UNKNOWN) {
        FCV_ERR("The type of input mat is not support now for rga resize!");
        return -1;
    }
    if (dst.width() != dsize._width || dst.height() != dsize._height || dst.type() != src.type()) {
        dst = Mat(dsize._width, dsize._height, src.type());
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
    rga_set_rect(&src_rga_info.rect, 0, 0, src.width(), src.height(), src.width(), src.height(), rga_format);
    rga_set_rect(&dst_rga_info.rect, 0, 0, dst.width(), dst.height(), dst.width(), dst.height(), rga_format);
    // do rga resize
    int ret = rkRga.RkRgaBlit(&src_rga_info, &dst_rga_info, nullptr);
    if (ret < 0) {
        FCV_ERR("RGA image resize failed!");
        return -1;
    }
    return 0;
}

G_FCV_NAMESPACE1_END()
