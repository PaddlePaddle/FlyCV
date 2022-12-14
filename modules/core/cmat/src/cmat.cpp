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

int cmat_to_mat(CMat* src, Mat& dst) {
    if (src == nullptr) {
        LOG_ERR("The src is nullptr!");
        return -1;
    }

    FCVImageType type = static_cast<FCVImageType>(src->type); 

    dst = Mat(src->width, src->height, type, src->data);

    return 0;
}

CMat* mat_to_cmat(Mat& src) {
    if (src.empty()) {
        LOG_ERR("The src is empty!");
        return nullptr;
    }

    CMat* dst = create_cmat(src.width(), src.height(), CFCVImageType((int)src.type()));
    memcpy(dst->data, src.data(), src.total_byte_size());

    return dst;
}

bool check_cmat(CMat* src) {
    if (src->data == nullptr
            || src->height <= 0
            || src->width <= 0
            || src->total_byte_size <= 0
            || src->stride <= 0) {
        return false;
    }

    return true;
}

CMat* create_cmat(int width, int height, CFCVImageType type) {
    FCVImageType image_type = static_cast<FCVImageType>(type); 
    TypeInfo type_info;

    if (get_type_info(image_type, type_info)) {
        LOG_ERR("The type is not supported!");
        return nullptr;
    }

    CMat* mat = (CMat*)malloc(sizeof(CMat));
    int channel_offset = 0;
    int stride = 0;

    parse_type_info(type_info, width, height,
            channel_offset, stride, mat->total_byte_size);

    mat->data = malloc(sizeof(unsigned char) * mat->total_byte_size);
    mat->channels = type_info.channels;
    mat->width = width;
    mat->height = height;
    mat->stride = stride;
    mat->type = type;
    mat->type_byte_size = type_info.type_byte_size;

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

G_FCV_NAMESPACE1_END()
