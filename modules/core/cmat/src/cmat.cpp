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

#include "modules/core/mat/interface/mat.h"
#include "modules/core/cmat/include/cmat_common.h"
#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

static std::map<CFCVImageType, FCVImageType> c_img_type_map {
    {CFCVImageType::GRAY_U8, FCVImageType::GRAY_U8},
    {CFCVImageType::GRAY_U16, FCVImageType::GRAY_U16},
    {CFCVImageType::GRAY_S32, FCVImageType::GRAY_S32},
    {CFCVImageType::GRAY_F32, FCVImageType::GRAY_F32},
    {CFCVImageType::GRAY_F64, FCVImageType::GRAY_F64},
    {CFCVImageType::PLA_BGR_U8, FCVImageType::PLA_BGR_U8},
    {CFCVImageType::PLA_RGB_U8, FCVImageType::PLA_RGB_U8},
    {CFCVImageType::PKG_BGR_U8, FCVImageType::PKG_BGR_U8},
    {CFCVImageType::PKG_RGB_U8, FCVImageType::PKG_RGB_U8},
    {CFCVImageType::PLA_BGRA_U8, FCVImageType::PLA_BGRA_U8},
    {CFCVImageType::PLA_RGBA_U8, FCVImageType::PLA_RGBA_U8},
    {CFCVImageType::PKG_BGRA_U8, FCVImageType::PKG_BGRA_U8},
    {CFCVImageType::PKG_RGBA_U8, FCVImageType::PKG_RGBA_U8},
    {CFCVImageType::PLA_BGR_F32, FCVImageType::PLA_BGR_F32},
    {CFCVImageType::PLA_RGB_F32, FCVImageType::PLA_RGB_F32},
    {CFCVImageType::PKG_BGR_F32, FCVImageType::PKG_BGR_F32},
    {CFCVImageType::PKG_RGB_F32, FCVImageType::PKG_RGB_F32},
    {CFCVImageType::PLA_BGRA_F32, FCVImageType::PLA_BGRA_F32},
    {CFCVImageType::PLA_RGBA_F32, FCVImageType::PLA_RGBA_F32},
    {CFCVImageType::PKG_BGRA_F32, FCVImageType::PKG_BGRA_F32},
    {CFCVImageType::PKG_RGBA_F32, FCVImageType::PKG_RGBA_F32},
    {CFCVImageType::PKG_BGR_F64, FCVImageType::PKG_BGR_F64},
    {CFCVImageType::PKG_RGB_F64, FCVImageType::PKG_RGB_F64},
    {CFCVImageType::PKG_BGRA_F64, FCVImageType::PKG_BGRA_F64},
    {CFCVImageType::PKG_RGBA_F64, FCVImageType::PKG_RGBA_F64},
    {CFCVImageType::PKG_BGR565_U8, FCVImageType::PKG_BGR565_U8},
    {CFCVImageType::PKG_RGB565_U8, FCVImageType::PKG_RGB565_U8},
    {CFCVImageType::NV12, FCVImageType::NV12},
    {CFCVImageType::NV21, FCVImageType::NV21},
    {CFCVImageType::I420, FCVImageType::I420},
};

int cmat_to_mat(CMat* src, Mat& dst) {
    if (src == nullptr) {
        return -1;
    }

    auto iter = c_img_type_map.find(src->type);

    if (iter == c_img_type_map.end()) {
        LOG_ERR("The src type is not supported!");
        return -1;
    }

    dst = Mat(src->width, src->height, iter->second, src->data);

    return 0;
}

int mat_to_cmat(Mat& src, CMat* dst) {
    if (src.empty() || dst == nullptr) {
        return -1;
    }

    return 0;
}

CMat* create_cmat(int width, int height, CFCVImageType type) {
    auto iter = c_img_type_map.find(type);

    if (iter == c_img_type_map.end()) {
        LOG_ERR("The type is not supported!");
        return nullptr;
    }

    TypeInfo type_info;

    if (get_type_info(iter->second, type_info)) {
        LOG_ERR("The type is not supported!");
        return nullptr;
    }

    CMat* mat = (CMat*)malloc(sizeof(CMat));
    int channel_offset = 0;
    int stride = 0;
    uint64_t total_byte_size = 0;

    parse_type_info(type_info, width, height,
            channel_offset, stride, total_byte_size);

    mat->data = malloc(sizeof(unsigned char) * total_byte_size);
    mat->channels = type_info.channels;
    mat->stride = stride;

    return mat;
}

int release_cmat(CMat* mat) {
    if (mat == nullptr) return -1;

    if (mat->data != nullptr) {
        free(mat->data);
        mat->data = nullptr;
    }

    free(mat);
    mat = nullptr;
    return 0;
}

void csize_to_size(CSize& csize, Size& size) {
    size.set_width(csize.width);
    size.set_height(csize.height);
}

InterpolationType cinterpolation_to_interpolation(CInterpolationType ctype) {
    static std::map<CInterpolationType, InterpolationType> type_map {
        {CInterpolationType::INTER_NEAREST, InterpolationType::INTER_NEAREST},
        {CInterpolationType::INTER_LINEAR, InterpolationType::INTER_LINEAR},
        {CInterpolationType::INTER_CUBIC, InterpolationType::INTER_CUBIC},
        {CInterpolationType::INTER_AREA, InterpolationType::INTER_AREA},
        {CInterpolationType::WARP_INVERSE_MAP, InterpolationType::WARP_INVERSE_MAP},
    };

    auto iter = type_map.find(ctype);

    if (iter != type_map.end()) {
        return InterpolationType::INTER_NEAREST;
    }

    return type_map[ctype];
}

G_FCV_NAMESPACE1_END()
