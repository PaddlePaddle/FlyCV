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

#pragma once

/**
 * @brief Image types.
 * U8  - unsigned 1 byte integer
 * U16 - unsigned 2 byte integer
 * S16 - signed 2 byte integer
 * S32 - signed 4 byte integer
 * F32 - 4 byte float
 * F64 - 8 byte float
 */
enum CFCVImageType {
    GRAY_U8 = 0,
    GRAY_U16,
    GRAY_S16,
    GRAY_S32,
    GRAY_F32,
    GRAY_F64,
    PLA_BGR_U8 = 20,
    PLA_RGB_U8,
    PKG_BGR_U8,
    PKG_RGB_U8,
    PLA_BGRA_U8,
    PLA_RGBA_U8,
    PKG_BGRA_U8,
    PKG_RGBA_U8,
    PLA_BGR_F32 = 40,
    PLA_RGB_F32,
    PKG_BGR_F32,
    PKG_RGB_F32,
    PLA_BGRA_F32,
    PLA_RGBA_F32,
    PKG_BGRA_F32,
    PKG_RGBA_F32,
    PKG_BGR_F64,
    PKG_RGB_F64,
    PKG_BGRA_F64,
    PKG_RGBA_F64,
    PKG_BGR565_U8,
    PKG_RGB565_U8,
    NV12 = 60,
    NV21,
    I420,
};