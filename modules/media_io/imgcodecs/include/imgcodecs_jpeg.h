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

#ifdef WITH_LIB_JPEG_TURBO

/**
 * @brief Decode image data from memory, only support jpeg format.
 * @param[in] buf Image data address.  
 * @param[in] size Data size(unit: Byte).
 * @param[in] flags Mode of reading, presevred key word, unused now.
 * @return Mat type will be PKG_BGR_U8 or GRAY_U8 if success, or the mat is empty if failed.
 */
Mat imdecode_jpeg(
        const uint8_t* buf,
        const size_t size,
        int flags = 0);

/**
 * @brief Encode Mat data to memory as standard jpeg format.
 * @param[in] img Image mat to be compressed.
 * @param[inout] buf Output buffer resized to fit the compressed result.
 * @param[in] quality The quality of compressed image data.
 * @return true Success.
 * @return false Fail.
 */
bool imencode_jpeg(
        const Mat& img,
        std::vector<uint8_t>& buf,
        int quality = 95);

#endif /// WITH_LIB_JPEG_TURBO

G_FCV_NAMESPACE1_END()
