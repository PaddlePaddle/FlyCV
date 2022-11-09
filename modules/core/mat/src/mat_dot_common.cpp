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

#include "modules/core/mat/include/mat_dot_common.h"

#include "modules/core/mat/interface/mat.h"
#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

double dot_common(const Mat& ma, const Mat& mb) {
    double result = 0.;

    TypeInfo type_info;
    int status = get_type_info(ma.type(), type_info);

    if (status != 0) {
        LOG_ERR("The mat type is not supported!");
        return result;
    }

    int len = ma.width() * ma.height() * ma.channels();
    void* ma_data = ma.data();
    void* mb_data = mb.data();

    switch (type_info.data_type) {
    case DataType::UINT8:
        result = dot_product<unsigned char>(reinterpret_cast<unsigned char*>(ma_data),
                reinterpret_cast<unsigned char*>(mb_data), len);
        break;
    case DataType::UINT16:
        result = dot_product<unsigned short>(reinterpret_cast<unsigned short*>(ma_data),
                reinterpret_cast<unsigned short*>(mb_data), len);
        break;
    case DataType::SINT32:
        result = dot_product<int>(reinterpret_cast<int*>(ma_data),
                reinterpret_cast<int*>(mb_data), len);
        break;
    case DataType::F32:
        result = dot_product<float>(reinterpret_cast<float*>(ma_data),
                reinterpret_cast<float*>(mb_data), len);
        break;
    default:
        LOG_ERR("The src type is not supported!");
        break;
    };

    return result;
}

G_FCV_NAMESPACE1_END()