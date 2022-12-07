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

#include "modules/core/cmat/interface/cmat.h"

#include <map>

#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

static std::map<CFCVImageType, g_fcv_ns::FCVImageType> type_map {
    {CFCVImageType::GRAY_U8, g_fcv_ns::FCVImageType::GRAY_U8},
    {CFCVImageType::GRAY_U16, g_fcv_ns::FCVImageType::GRAY_U16},
    {CFCVImageType::GRAY_S32, g_fcv_ns::FCVImageType::GRAY_S32},
    {CFCVImageType::GRAY_F32, g_fcv_ns::FCVImageType::GRAY_F32},
    {CFCVImageType::GRAY_F64, g_fcv_ns::FCVImageType::GRAY_F64},
    {CFCVImageType::PLA_BGR_U8, g_fcv_ns::FCVImageType::PLA_BGR_U8}, 
    {CFCVImageType::PLA_RGB_U8, g_fcv_ns::FCVImageType::PLA_RGB_U8},
    {CFCVImageType::PKG_BGR_U8, g_fcv_ns::FCVImageType::PKG_BGR_U8},
    {CFCVImageType::PKG_RGB_U8, g_fcv_ns::FCVImageType::PKG_RGB_U8},
    {CFCVImageType::PLA_BGRA_U8, g_fcv_ns::FCVImageType::PLA_BGRA_U8},
    {CFCVImageType::PLA_RGBA_U8, g_fcv_ns::FCVImageType::PLA_RGBA_U8},
    {CFCVImageType::PKG_BGRA_U8, g_fcv_ns::FCVImageType::PKG_BGRA_U8},
    {CFCVImageType::PKG_RGBA_U8, g_fcv_ns::FCVImageType::PKG_RGBA_U8},
    {CFCVImageType::PLA_BGR_F32, g_fcv_ns::FCVImageType::PLA_BGR_F32},
    {CFCVImageType::PLA_RGB_F32, g_fcv_ns::FCVImageType::PLA_RGB_F32},
    {CFCVImageType::PKG_BGR_F32, g_fcv_ns::FCVImageType::PKG_BGR_F32},
    {CFCVImageType::PKG_RGB_F32, g_fcv_ns::FCVImageType::PKG_RGB_F32},
    {CFCVImageType::PLA_BGRA_F32, g_fcv_ns::FCVImageType::PLA_BGRA_F32},
    {CFCVImageType::PLA_RGBA_F32, g_fcv_ns::FCVImageType::PLA_RGBA_F32},
    {CFCVImageType::PKG_BGRA_F32, g_fcv_ns::FCVImageType::PKG_BGRA_F32},
    {CFCVImageType::PKG_RGBA_F32, g_fcv_ns::FCVImageType::PKG_RGBA_F32},
    {CFCVImageType::PKG_BGR_F64, g_fcv_ns::FCVImageType::PKG_BGR_F64},
    {CFCVImageType::PKG_RGB_F64, g_fcv_ns::FCVImageType::PKG_RGB_F64},
    {CFCVImageType::PKG_BGRA_F64, g_fcv_ns::FCVImageType::PKG_BGRA_F64},
    {CFCVImageType::PKG_RGBA_F64, g_fcv_ns::FCVImageType::PKG_RGBA_F64},
    {CFCVImageType::PKG_BGR565_U8, g_fcv_ns::FCVImageType::PKG_BGR565_U8},
    {CFCVImageType::PKG_RGB565_U8, g_fcv_ns::FCVImageType::PKG_RGB565_U8},
    {CFCVImageType::NV12, g_fcv_ns::FCVImageType::NV12},
    {CFCVImageType::NV21, g_fcv_ns::FCVImageType::NV21},
    {CFCVImageType::I420, g_fcv_ns::FCVImageType::I420},
};

int cmat_to_mat(CMat* src, g_fcv_ns::Mat& dst) {
    if (src == nullptr) {
        return -1;
    }

    auto iter = type_map.find(src->type);

    if (iter == type_map.end()) {
        LOG_ERR("The src type is not supported!");
        return -1;
    }

    dst = g_fcv_ns::Mat(src->width, src->height, iter->second, src->data);

    return 0;
}

int mat_to_cmat(g_fcv_ns::Mat& src, CMat* dst) {
    if (src.empty() || dst == nullptr) {
        return -1;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()

CMat* create_cmat(int width, int height, CFCVImageType type) {
    return 0;
}