// Copyright (c) 2023 FlyCV Authors. All Rights Reserved.
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

#include <emscripten/bind.h>

#include "modules/core/base/interface/basic_types.h"

using g_fcv_ns::FCVImageType;
using g_fcv_ns::InterpolationType;
using g_fcv_ns::BorderType;
using g_fcv_ns::NormType;
using g_fcv_ns::Scalar;
using g_fcv_ns::Rect;
using g_fcv_ns::Size;

using emscripten::enum_;
using emscripten::val;

EMSCRIPTEN_BINDINGS(basic_types) {
    enum_<FCVImageType>("FCVImageType")
        .value("GRAY_U8", FCVImageType::GRAY_U8)
        .value("GRAY_U16", FCVImageType::GRAY_U16)
        .value("GRAY_S16", FCVImageType::GRAY_S16)
        .value("GRAY_S32", FCVImageType::GRAY_S32)
        .value("GRAY_F32", FCVImageType::GRAY_F32)
        .value("GRAY_F64", FCVImageType::GRAY_F64)
        .value("PLA_BGR_U8", FCVImageType::PLA_BGR_U8)
        .value("PLA_RGB_U8", FCVImageType::PLA_RGB_U8)
        .value("PKG_BGR_U8", FCVImageType::PKG_BGR_U8)
        .value("PKG_RGB_U8", FCVImageType::PKG_RGB_U8)
        .value("PLA_BGRA_U8", FCVImageType::PLA_BGRA_U8)
        .value("PLA_RGBA_U8", FCVImageType::PLA_RGBA_U8)
        .value("PKG_BGRA_U8", FCVImageType::PKG_BGRA_U8)
        .value("PKG_RGBA_U8", FCVImageType::PKG_RGBA_U8)
        .value("PLA_BGR_F32", FCVImageType::PLA_BGR_F32)
        .value("PLA_RGB_F32", FCVImageType::PLA_RGB_F32)
        .value("PKG_BGR_F32", FCVImageType::PKG_BGR_F32)
        .value("PKG_RGB_F32", FCVImageType::PKG_RGB_F32)
        .value("PLA_BGRA_F32", FCVImageType::PLA_BGRA_F32)
        .value("PLA_RGBA_F32", FCVImageType::PLA_RGBA_F32)
        .value("PKG_BGRA_F32", FCVImageType::PKG_BGRA_F32)
        .value("PKG_RGBA_F32", FCVImageType::PKG_RGBA_F32)
        .value("PKG_BGR_F64", FCVImageType::PKG_BGR_F64)
        .value("PKG_RGB_F64", FCVImageType::PKG_RGB_F64)
        .value("PKG_BGRA_F64", FCVImageType::PKG_BGRA_F64)
        .value("PKG_RGBA_F64", FCVImageType::PKG_RGBA_F64)
        .value("PKG_BGR565_U8", FCVImageType::PKG_BGR565_U8)
        .value("PKG_RGB565_U8", FCVImageType::PKG_RGB565_U8)
        .value("NV12", FCVImageType::NV12)
        .value("NV21", FCVImageType::NV21)
        .value("I420", FCVImageType::I420)
        ;

    enum_<InterpolationType>("InterpolationType")
        .value("INTER_NEAREST", InterpolationType::INTER_NEAREST)
        .value("INTER_LINEAR", InterpolationType::INTER_LINEAR)
        .value("INTER_CUBIC", InterpolationType::INTER_CUBIC)
        .value("INTER_AREA", InterpolationType::INTER_AREA)
        .value("WARP_INVERSE_MAP", InterpolationType::WARP_INVERSE_MAP)
        ;

    enum_<BorderType>("BorderType")
        .value("BORDER_CONSTANT", BorderType::BORDER_CONSTANT)
        .value("BORDER_REPLICATE", BorderType::BORDER_REPLICATE)
        .value("BORDER_REFLECT", BorderType::BORDER_REFLECT)
        .value("BORDER_WRAP", BorderType::BORDER_WRAP)
        .value("BORDER_REFLECT_101", BorderType::BORDER_REFLECT_101)
        .value("BORDER_TRANSPARENT", BorderType::BORDER_TRANSPARENT)
        ;

    enum_<NormType>("NormType")
        .value("NORM_INF", NormType::NORM_INF)
        .value("NORM_L1", NormType::NORM_L1)
        .value("NORM_L2", NormType::NORM_L2)
        ;

    emscripten::value_array<Scalar>("Scalar")
        .element(emscripten::index<0>())
        .element(emscripten::index<1>())
        .element(emscripten::index<2>())
        .element(emscripten::index<3>())
        ;

    emscripten::class_<Rect>("Rect")
        .constructor<>()
        .constructor<int, int, int, int>()
        .function("setX", &Rect::set_x)
        .function("setY", &Rect::set_y)
        .function("setWidth", &Rect::set_width)
        .function("setHeight", &Rect::set_height)
        .function("x", &Rect::x)
        .function("y", &Rect::y)
        .function("width", &Rect::width)
        .function("height", &Rect::height)
        ;

    emscripten::class_<Size>("Size")
        .constructor<>()
        .constructor<int, int>()
        .function("setWidth", &Size::set_width)
        .function("setHeight", &Size::set_height)
        .function("width", &Size::width)
        .function("height", &Size::height)
        ;
}
