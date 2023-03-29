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

#include "modules/media_io/imgcodecs/interface/imgcodecs_c.h"
#include "modules/media_io/imgcodecs/interface/imgcodecs.h"
#include "modules/core/cmat/include/cmat_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

CMat* fcvImread(const char* file_name, int flags) {
    Mat tmp = imread(file_name, flags);
    return matToCMat(tmp);
}

int fcvImwrite(
        const char* file_name,
        CMat* img,
        int quality) {
    if (!checkCMat(img)) {
        LOG_ERR("The img is illegal, please check whether "
                "the attribute values ​​of img are correct");
        return false;
    }

    Mat tmp;
    cmatToMat(img, tmp);

    return imwrite(file_name, tmp, quality);
}

CMat* fcvImdecode(
        unsigned char* buf,
        uint64_t size,
        int flags) {
    Mat tmp = imdecode(buf, size, flags);
    return matToCMat(tmp);
}

int fcvImencode(
        const char* ext,
        CMat* img,
        unsigned char** buf,
        uint64_t* buf_size,
        int quality) {
    if (!checkCMat(img)) {
        LOG_ERR("The img is illegal, please check whether "
                "the attribute values ​​of img are correct");
        return -1;
    }

    if (buf == nullptr) {
        LOG_ERR("The buf is nullptr which is not allowed!");
        return -1;
    }
    
    Mat tmp;
    cmatToMat(img, tmp);

    std::vector<uint8_t> res;
    imencode(ext, tmp, res, quality);

    *buf_size = res.size();
    *buf = (unsigned char*)malloc(*buf_size);
    memcpy(*buf, res.data(), *buf_size);

    return 0;
}

G_FCV_NAMESPACE1_END()