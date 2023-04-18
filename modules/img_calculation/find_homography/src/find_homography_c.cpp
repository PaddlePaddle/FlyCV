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

#include "modules/img_calculation/find_homography/interface/find_homography_c.h"
#include "modules/img_calculation/find_homography/interface/find_homography.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

CMat* fcvFindHomography(
        CPoint2f* src_pts,
        CPoint2f* dst_pts,
        int pts_num,
        int method) {
    if (src_pts == nullptr) {
        LOG_ERR("The src_pts is nullptr!");
        return nullptr;
    }

    if (dst_pts == nullptr) {
        LOG_ERR("The dst_pts is nullptr!");
        return nullptr;
    }

    std::vector<Point2f> src_pts_tmp;
    std::vector<Point2f> dst_pts_tmp;

    for (int i = 0; i < pts_num; ++i) {
        src_pts_tmp.push_back(Point2f(src_pts[i].x, src_pts[i].y));
        dst_pts_tmp.push_back(Point2f(dst_pts[i].x, dst_pts[i].y));
    }

    Mat res_tmp = find_homography(src_pts_tmp, dst_pts_tmp, method);

    if (res_tmp.empty()) {
        LOG_ERR("The result is empty, pelease check wether the input params are correct!");
        return nullptr;
    }

    CMat* res = fcvCreateCMat(res_tmp.width(), res_tmp.height(), CFCVImageType::GRAY_F64);
    memcpy(res->data, res_tmp.data(), res_tmp.total_byte_size());
    return res;
}

G_FCV_NAMESPACE1_END()
