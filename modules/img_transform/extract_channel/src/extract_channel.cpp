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

#include "modules/img_transform/extract_channel/interface/extract_channel.h"

#include "modules/core/base/include/type_info.h"
#include "modules/img_transform/extract_channel/include/extract_channel_common.h"
#ifdef HAVE_NEON
#include "modules/img_transform/extract_channel/include/extract_channel_arm.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int extract_channel(Mat& _src, Mat& _dst, int _index) {
    if (_src.type() != FCVImageType::PKG_BGR_U8) {
        LOG_ERR("src type is not support");
        return -1;
    }

    if (_index >= _src.channels()) {
        LOG_ERR("Input index must less than mat channel count");
        return -1;
    }

    TypeInfo cur_type_info;
    if (get_type_info(_src.type(), cur_type_info)) {
        LOG_ERR("failed to get type info from src mat while get_type_info");
        return -1;
    }

    if (cur_type_info.data_type != DataType::UINT8) {
        LOG_ERR("extract_channel only support u8 data, the current src element "
                "data type is %d",
                int(cur_type_info.data_type));
        return -1;
    }

    if (_src.channels() != 3) {
        LOG_ERR("extract_channel only support 3 or 4 channels, current src "
                "channels is %d",
                _src.channels());
        return -1;
    }

    if (_dst.empty() || _dst.width() != _src.width() || _dst.height() != _src.height()) {
        _dst = Mat(_src.size(), FCVImageType::GRAY_U8);
    }

    int ret = 0;
#ifdef HAVE_NEON
    ret = extract_channel_neon(_src, _dst, _index);
#else
    ret = extract_channel_common(_src, _dst, _index);
#endif
    return ret;
}

G_FCV_NAMESPACE1_END()
