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

#include "modules/core/base/interface/macro_ns.h"

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

enum CInterpolationType {
    INTER_NEAREST = 0,      // nearest interpolation
    INTER_LINEAR,           // bilinear interpolation
    INTER_CUBIC,            // bicubic interpolation
    INTER_AREA,             // area interpolation
    WARP_INVERSE_MAP = 16   // inverse transformation interpolation, the type means that M matrix is the inverse transformation
};

typedef struct {
    double val[4];
} CScalar;

typedef struct {
    int x;
    int y;
} CPoint;

typedef struct {
    float x;
    float y;
} CPoint2f;

typedef struct {
    int width;
    int height;
} CSize;

typedef struct {
    float width;
    float height;
} CSize2f;

typedef struct {
    CSize2f size;
    CPoint2f center;
    float angle;
} CRotatedRect;