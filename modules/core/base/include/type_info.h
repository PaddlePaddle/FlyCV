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

#pragma once

#include "modules/core/base/interface/basic_types.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

enum class DataType {
    UINT8 = 0,
    UINT16,
    SINT16,
    SINT32,
    F32,
    F64
};

enum class StorageFormat {
    PACKAGE = 0,
    PLANAR
};

enum class LayoutType {
    SINGLE = 0,
    PACKAGE,
    PLANAR,
    YUV,
    NONE
};

/**
 * @brief details of specific image type
 */
struct TypeInfo {
    DataType data_type;
    StorageFormat format;
    int channels;
    int pixel_size;
    int type_byte_size;
    int pixel_offset;
    LayoutType layout;
};

int get_type_info(FCVImageType type, TypeInfo& type_info);

G_FCV_NAMESPACE1_END()
