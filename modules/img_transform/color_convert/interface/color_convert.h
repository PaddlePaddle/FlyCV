// Copyright (c) 2021 FlyCV Authors. All Rights Reserved.
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

#include <vector>

#include "modules/core/base/interface/basic_types.h"
#include "modules/core/base/interface/macro_export.h"
#include "modules/core/base/interface/macro_ns.h"
#include "modules/core/mat/interface/mat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief color convert types.
 */
enum class ColorConvertType {
    CVT_PA_BGR2GRAY = 0,
    CVT_PA_RGB2GRAY,

    CVT_PA_BGR2PA_RGB,
    CVT_PA_RGB2PA_BGR,
    CVT_PA_BGR2PA_BGRA, // add alpha channel to RGB or BGR image
    CVT_PA_RGB2PA_RGBA,
    CVT_PA_BGR2PA_RGBA, // convert between RGB and BGR color spaces (with or without alpha channel)
    CVT_PA_RGB2PA_BGRA,
    CVT_PA_BGRA2PA_BGR, // remove alpha channel from RGB or BGR image
    CVT_PA_RGBA2PA_RGB,
    CVT_PA_RGBA2PA_BGR,
    CVT_PA_BGRA2PA_RGB,
    CVT_PA_BGRA2PA_RGBA,
    CVT_PA_RGBA2PA_BGRA,

    CVT_GRAY2PA_RGB, // convert between RGB/BGR and grayscale
    CVT_GRAY2PA_BGR,
    CVT_GRAY2PA_BGRA,
    CVT_GRAY2PA_RGBA,

    CVT_PA_BGR2NV12, // convert between RGB/BGR and YUV(NV12...)
    CVT_PA_BGR2NV21,
    CVT_PA_RGB2NV12,
    CVT_PA_RGB2NV21,

    CVT_PA_BGRA2NV12,
    CVT_PA_BGRA2NV21,
    CVT_PA_RGBA2NV12,
    CVT_PA_RGBA2NV21,

    CVT_NV122PA_RGB,
    CVT_NV212PA_RGB,
    CVT_NV122PA_BGR,
    CVT_NV212PA_BGR,
    CVT_I4202PA_BGR,

    CVT_NV122PA_BGRA,
    CVT_NV212PA_BGRA,
    CVT_NV122PA_RGBA,
    CVT_NV212PA_RGBA,

    CVT_PA_BGR2PL_BGR,  // bgrbgrbgr... convert to bbb...ggg...rrr
    CVT_PL_BGR2PA_BGR,  // bb..gg..rr.. convert to bgrbgr..

    CVT_GRAY2PA_BGR565, // convert between RGB/BGR and BGR565 (16-bit images)
    CVT_PA_BGR2PA_BGR565,
    CVT_PA_RGB2PA_BGR565,
    CVT_PA_BGRA2PA_BGR565,
    CVT_PA_RGBA2PA_BGR565,

    CVT_PA_RGBA2PA_mRGBA,

};

/**
 * @brief the implementation of perspective transformation, supported element types: u8 and f32
 * The function converts an input image from one color space to another
 * @param[in] src input image, supported image type:Mat, the number of channel: 1 ,3
 * @param[out] dst output image, supported image type:Mat, the number of channel: 1 ,3
 * @param[in] cvt_type color space conversion code (see #ColorConvertType, for example #CVT_I4202PA_BGR or #CVT_NV212PA_BGR or #CVT_NV122PA_BGR)
 */
EXTERN_C FCV_API int cvt_color(
        const Mat& src,
        Mat& dst,
        ColorConvertType cvt_type);

/**
 * @brief the implementation of perspective transformation, supported element types: u8 and f32
 * This function Converts an image from one color space to another where the source image is stored in
   three planes.This function only supports YUV420 to RGB conversion as of now.
 * @param[in] src_y 8-bit image (#GRAY_U8) of the Y plane
 * @param[in] src_u 8-bit image (#GRAY_U8) of the U plane
 * @param[in] src_v 8-bit image (#GRAY_U8) of the V plane
 * @param[out] dst image, supported image type:Mat, the number of channel: 1 ,3
 * @param[in] cvt_type color space conversion code (see #ColorConvertType, only supports #CVT_I4202PA_BGR)
 */
FCV_API int cvt_color(
        const Mat& src_y,
        Mat& src_u,
        Mat& src_v,
        Mat& dst,
        ColorConvertType cvt_type);

G_FCV_NAMESPACE1_END()
