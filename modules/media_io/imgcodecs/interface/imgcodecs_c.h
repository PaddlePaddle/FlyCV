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

#include "modules/core/cmat/interface/cmat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief Read image data from JPEG file or PNG file.
 * @param[in] file_name Image file path, only support suffix is '.jpg' or 'jpeg' or 'png'(ignoring case).
 * @param[in] flags Mode of reading, presevred key word, unused now.
 * @return CMat type will be PKG_BGR_U8 or GRAY_U8 if success, or the cmat is empty if failed.
 */
EXTERN_C FCV_API CMat* fcvImread(const char* file_name, int flags);

/**
 * @brief Write CMat data to JPEG file
 * @param[in] file_name Image file name, only support suffix is '.jpg' or 'jpeg' or 'png'(ignoring case).
 * @param[in] img The Mat data for writing, Only support image_type of ['GRAY_U8', 'PKG_RGB_U8', 'PKG_BGR_U8',
 *            'PKG_RGBA_U8', 'PKG_RGBA_U8'].
 * @param[in] quality The quality of output JPEG file, range [1~100].
 * @return int
 */
EXTERN_C FCV_API int fcvImwrite(
        const char* file_name,
        CMat* img,
        int quality);

/**
 * @brief Decode image data from memory, only support JPEG or PNG now!
 * @param[in] buf Image data address.  
 * @param[in] size Data size(unit: Byte).
 * @param[in] flags Mode of reading, presevred key word, unused now.
 * @return CMat type will be PKG_BGR_U8 or GRAY_U8 if success, or the cmat is empty if failed.
 */
EXTERN_C FCV_API CMat* fcvImdecode(
        unsigned char* buf,
        uint64_t size,
        int flags);

/**
 * @brief Encode image data to memory as standard jpeg/jpg/png format.
 * @param[in] ext Lable to decide the format of output data(only support:'.jpg' 'jpeg' or 'png').
 * @param[in] img Input image data to compress.
 * @param[out] buf Output result data with resize to the proper value, remember to use 'free' to release the memory outside.
 * @param[out] buf_size The total byte size of encode result
 * @param[in] quality quality Jpeg compress quality ,the same meaning of imwrite, no effect when ext is .png.
 * @return zero : success; non-zero: failed.  
 */
EXTERN_C FCV_API int fcvImencode(
        const char* ext,
        CMat* img,
        unsigned char** buf,
        uint64_t* buf_size,
        int quality);

G_FCV_NAMESPACE1_END()
