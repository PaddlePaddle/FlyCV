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

#include "modules/core/mat/interface/mat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief Read image data from JPEG file or PNG file.
 * @param[in] file_name file_name Image file path, only support suffix is '.jpg' or 'jpeg' or 'png'(ignoring case).
 * @param[in] flags flags Mode of reading, presevred key word, unused now.
 * @return Mat type will be PKG_BGR_U8 or GRAY_U8 if success, or the mat is empty if failed.
 */
FCV_API Mat imread(const std::string& file_name, int flags = 0);

/**
 * @brief Write Mat data to JPEG file
 * @param[in] file_name file_name Image file name, only support suffix is '.jpg' or 'jpeg' or 'png'(ignoring case).
 * @param[in] img img The Mat data for writing, Only support image_type of ['GRAY_U8', 'PKG_RGB_U8', 'PKG_BGR_U8',
 *            'PKG_RGBA_U8', 'PKG_RGBA_U8'].
 * @param[in] quality quality The quality of output JPEG file, range [1~100].
 * @return FCV_API
 */
FCV_API bool imwrite(
        const std::string& file_name,
        const Mat& img,
        int quality = 95);

/**
 * @brief Decode image data from memory, only support JPEG or PNG now!
 * @param[in] buf buf Image data address.  
 * @param[in] size size Data size(unit: Byte).
 * @param[in] flags flags Mode of reading, presevred key word, unused now.
 * @return FCV_API Mat type will be PKG_BGR_U8 or GRAY_U8 if success, or the mat is empty if failed.
 */
FCV_API Mat imdecode(
        const uint8_t* buf,
        const size_t size,
        int flags = 0);

/**
 * @brief Encode image data to memory as standard jpeg/jpg/png format.
 * @param[in] ext ext Lable to decide the format of output data(only support:'.jpg' 'jpeg' or 'png').
 * @param[in] img img Input image data to compress.
 * @param[inout] buf buf Output result data with resize to the proper value. 
 * @param[in] quality quality Jpeg compress quality ,the same meaning of imwrite, no effect when ext is .png.
 * @return True : success; False: failed.  
 */
FCV_API bool imencode(
        const std::string& ext,
        const Mat& img,
        std::vector<uint8_t>& buf,
        int quality = 95);

G_FCV_NAMESPACE1_END()
