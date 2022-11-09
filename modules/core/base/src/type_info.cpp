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

#include "modules/core/base/include/type_info.h"

#include <map>
#include <utility>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int get_type_info(FCVImageType type, TypeInfo& type_info) {
    using KeyVal = std::pair<FCVImageType, TypeInfo>;
    static std::map<FCVImageType, TypeInfo> s_type_info;

    if (s_type_info.empty()) {
        s_type_info.insert(KeyVal(FCVImageType::GRAY_U8,
                {DataType::UINT8,
                StorageFormat::PACKAGE,
                1,
                sizeof(char),
                sizeof(char),
                sizeof(char), 
                LayoutType::SINGLE}));

        s_type_info.insert(KeyVal(FCVImageType::GRAY_U16,
                {DataType::UINT16,
                StorageFormat::PACKAGE,
                1,
                sizeof(unsigned short),
                sizeof(unsigned short),
                sizeof(unsigned short),
                LayoutType::SINGLE}));

        s_type_info.insert(KeyVal(FCVImageType::GRAY_S16,
                {DataType::SINT16,
                StorageFormat::PACKAGE,
                1,
                sizeof(short),
                sizeof(short),
                sizeof(short),
                LayoutType::SINGLE}));

        s_type_info.insert(KeyVal(FCVImageType::GRAY_S32,
                {DataType::SINT32,
                StorageFormat::PACKAGE,
                1,
                sizeof(int),
                sizeof(int),
                sizeof(int),
                LayoutType::SINGLE}));

        s_type_info.insert(KeyVal(FCVImageType::GRAY_F32,
                {DataType::F32,
                StorageFormat::PACKAGE,
                1,
                sizeof(float),
                sizeof(float),
                sizeof(float),
                LayoutType::SINGLE}));

        s_type_info.insert(KeyVal(FCVImageType::GRAY_F64,
                {DataType::F64,
                StorageFormat::PACKAGE,
                1,
                sizeof(double),
                sizeof(double),
                sizeof(double),
                LayoutType::SINGLE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_BGR_U8,
                {DataType::UINT8,
                StorageFormat::PACKAGE,
                3,
                sizeof(char) * 3,
                sizeof(char),
                sizeof(char) * 3,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_RGB_U8,
                {DataType::UINT8,
                StorageFormat::PACKAGE,
                3,
                sizeof(char) * 3,
                sizeof(char),
                sizeof(char) * 3,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_BGRA_U8,
                {DataType::UINT8,
                StorageFormat::PACKAGE,
                4,
                sizeof(char) * 4,
                sizeof(char),
                sizeof(char) * 4,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_RGBA_U8,
                {DataType::UINT8,
                StorageFormat::PACKAGE,
                4,
                sizeof(char) * 4,
                sizeof(char),
                sizeof(char) * 4,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_BGR_F32,
                {DataType::F32,
                StorageFormat::PACKAGE,
                3,
                sizeof(float) * 3,
                sizeof(float),
                sizeof(float) * 3,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_RGB_F32,
                {DataType::F32,
                StorageFormat::PACKAGE,
                3,
                sizeof(float) * 3,
                sizeof(float),
                sizeof(float) * 3,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_BGRA_F32,
                {DataType::F32,
                StorageFormat::PACKAGE,
                4,
                sizeof(float) * 4,
                sizeof(float),
                sizeof(float) * 4,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_RGBA_F32,
                {DataType::F32,
                StorageFormat::PACKAGE,
                4,
                sizeof(float) * 4,
                sizeof(float),
                sizeof(float) * 4,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_BGR_F64,
                {DataType::F64,
                StorageFormat::PACKAGE,
                3,
                sizeof(double) * 3,
                sizeof(double),
                sizeof(double) * 3,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_RGB_F64,
                {DataType::F64,
                StorageFormat::PACKAGE,
                3,
                sizeof(double) * 3,
                sizeof(double),
                sizeof(double) * 3,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_BGRA_F64,
                {DataType::F64,
                StorageFormat::PACKAGE,
                4,
                sizeof(double) * 4,
                sizeof(double),
                sizeof(double) * 4,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_RGBA_F64,
                {DataType::F64,
                StorageFormat::PACKAGE,
                4,
                sizeof(double) * 4,
                sizeof(double),
                sizeof(double) * 4,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_BGR565_U8,
                {DataType::UINT8,
                StorageFormat::PACKAGE,
                1,
                sizeof(char) * 2,
                sizeof(short),
                sizeof(char) * 2,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PKG_RGB565_U8,
                {DataType::UINT8,
                StorageFormat::PACKAGE,
                1,
                sizeof(char) * 2,
                sizeof(short),
                sizeof(char) * 2,
                LayoutType::PACKAGE}));

        s_type_info.insert(KeyVal(FCVImageType::PLA_BGR_U8,
                {DataType::UINT8,
                StorageFormat::PLANAR,
                3,
                sizeof(char) * 3,
                sizeof(char),
                sizeof(char),
                LayoutType::PLANAR}));

        s_type_info.insert(KeyVal(FCVImageType::PLA_RGB_F32,
                {DataType::F32,
                StorageFormat::PLANAR,
                3,
                sizeof(float) * 3,
                sizeof(float),
                sizeof(float),
                LayoutType::PLANAR}));

        s_type_info.insert(KeyVal(FCVImageType::PLA_RGB_U8,
                {DataType::UINT8,
                StorageFormat::PLANAR,
                3,
                sizeof(char) * 3,
                sizeof(char),
                sizeof(char),
                LayoutType::PLANAR}));

        s_type_info.insert(KeyVal(FCVImageType::PLA_BGRA_U8,
                {DataType::UINT8,
                StorageFormat::PLANAR,
                4,
                sizeof(char) * 4,
                sizeof(char),
                sizeof(char),
                LayoutType::PLANAR}));

        s_type_info.insert(KeyVal(FCVImageType::PLA_RGBA_U8,
                {DataType::UINT8,
                StorageFormat::PLANAR,
                4,
                sizeof(char) * 4,
                sizeof(char),
                sizeof(char),
                LayoutType::PLANAR}));

        s_type_info.insert(KeyVal(FCVImageType::PLA_BGR_F32,
                {DataType::F32,
                StorageFormat::PLANAR,
                3,
                sizeof(float) * 3,
                sizeof(float),
                sizeof(float),
                LayoutType::PLANAR}));

        s_type_info.insert(KeyVal(FCVImageType::PLA_BGRA_F32,
                {DataType::F32,
                StorageFormat::PLANAR,
                4,
                sizeof(float) * 4,
                sizeof(float),
                sizeof(float),
                LayoutType::PLANAR}));

        s_type_info.insert(KeyVal(FCVImageType::PLA_RGBA_F32,
                {DataType::F32,
                StorageFormat::PLANAR,
                4,
                sizeof(float) * 4,
                sizeof(float),
                sizeof(float),
                LayoutType::PLANAR}));

        s_type_info.insert(KeyVal(FCVImageType::NV12,
                {DataType::UINT8,
                StorageFormat::PACKAGE,
                3,
                sizeof(char) * 2,
                sizeof(char),
                sizeof(char),
                LayoutType::YUV}));

        s_type_info.insert(KeyVal(FCVImageType::NV21,
                {DataType::UINT8,
                StorageFormat::PACKAGE,
                3,
                sizeof(char) * 2,
                sizeof(char),
                sizeof(char),
                LayoutType::YUV}));

        s_type_info.insert(KeyVal(FCVImageType::I420,
                {DataType::UINT8,
                StorageFormat::PLANAR,
                3,
                sizeof(char) * 2,
                sizeof(char),
                sizeof(char),
                LayoutType::YUV}));
    }

    if (s_type_info.find(type) != s_type_info.end()) {
        type_info = s_type_info[type];
        return 0;
    } else {
        return -1;
    }
}

G_FCV_NAMESPACE1_END()
