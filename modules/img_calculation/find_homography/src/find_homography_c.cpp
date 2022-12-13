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
        CMat* src_points,
        CMat* dst_points,
        int method) {
    if (!check_cmat(src_points)) {
         LOG_ERR("The src is illegal, please check whether the attribute values ​​of src are correct");
         return nullptr;
    }

    if (!check_cmat(dst_points)) {
         LOG_ERR("The dst is illegal, please check whether the attribute values ​​of dst are correct");
         return nullptr;
    }

    float* src_data = reinterpret_cast<float*>(src_points->data);
    float* dst_data = reinterpret_cast<float*>(dst_points->data);
    std::vector<Point2f> src_pts;
    std::vector<Point2f> dst_pts;

    for (int i = 0; i < src_points->width * src_points->height / 2; ++i) {
        src_pts.push_back(Point2f(src_data[2 * i], src_data[2 * i + 1]));
        dst_pts.push_back(Point2f(dst_data[2 * i], dst_data[2 * i + 1]));
    }

    Mat res_tmp = find_homography(src_pts, dst_pts, method);

    if (res_tmp.empty()) {
        LOG_ERR("The result is empty, pelease check wether the input params are correct!");
        return nullptr;
    }

    CMat* res = create_cmat(res_tmp.width(), res_tmp.height(), CFCVImageType::GRAY_F64);
    memcpy(res->data, res_tmp.data(), res_tmp.total_byte_size());
    return res;
}

G_FCV_NAMESPACE1_END()
