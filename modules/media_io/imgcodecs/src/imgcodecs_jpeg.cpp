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

#include "modules/media_io/imgcodecs/include/imgcodecs_jpeg.h"
#include "modules/core/base/include/macro_utils.h"

#ifdef WITH_LIB_JPEG_TURBO
#include "turbojpeg.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#ifdef WITH_LIB_JPEG_TURBO

Mat imdecode_jpeg(const uint8_t* buf, const size_t size, int flags) {
    UN_USED(flags); /// Preserve flags for future.
    if (!buf || !size) {
        LOG_ERR("The input data is empty for imdecode_jpeg!");
        return Mat();
    }

    /// Init jpeg-turbo decompress handle.
    tjhandle tj_decompress_instance = tjInitDecompress();
    if (!tj_decompress_instance) {
        LOG_ERR("Failed to init jpeg-turbo decompress engine!");
        return Mat();
    }

    /// Read jpeg head info.
    int width = 0;
    int height = 0;
    int subsampling_type = 0;
    int color_space = 0;

    if (tjDecompressHeader3(tj_decompress_instance, buf, size,
            &width, &height, &subsampling_type, &color_space) < 0) {
        LOG_ERR("Failed to read header info from file as jpeg format when imdecode_jpeg!");
        tjDestroy(tj_decompress_instance);
        return Mat();
    }

    /// Decide image type : gray or bgr.
    FCVImageType fcv_mat_type = FCVImageType::PKG_BGR_U8;
    int pixel_format = TJPF_BGR;
    if (color_space == TJCS_GRAY) {
        fcv_mat_type = FCVImageType::GRAY_U8;
        pixel_format = TJPF_GRAY;
    }

    Mat img(width, height, fcv_mat_type);
    int bit_flags = 0;
    /// Decompress data to Mat.
    if (tjDecompress2(tj_decompress_instance, buf, size,
            reinterpret_cast<uint8_t*>(img.data()), width,
            0, height, pixel_format, bit_flags) < 0) {
        LOG_ERR("Failed to decompress image data from file as jpeg format when imread!");
        tjDestroy(tj_decompress_instance);
        return Mat();
    }
    /// release handle resources.
    tjDestroy(tj_decompress_instance);
    return img;
}

bool imencode_jpeg(const Mat& img,
        std::vector<uint8_t>& buf,
        int quality) {
    /// Check input Mat is legal.
    if (img.empty()) {
        LOG_ERR("img is empty for imencode_jpeg!");
        return false;
    }
    int pixel_format = TJPF_UNKNOWN;
    switch (img.type()) {
    case FCVImageType::GRAY_U8:
        pixel_format = TJPF_GRAY;
        break;
    case FCVImageType::PKG_RGB_U8:
        pixel_format = TJPF_RGB;
        break;
    case FCVImageType::PKG_BGR_U8:
        pixel_format = TJPF_BGR;
        break;
    case FCVImageType::PKG_RGBA_U8:
        pixel_format = TJPF_RGBA;
        break;
    case FCVImageType::PKG_BGRA_U8:
        pixel_format = TJPF_BGRA;
        break;
    default:
        break;
    }
    if (pixel_format == TJPF_UNKNOWN) {
        LOG_ERR("imwrite params img's type is not support!");
        return false;
    }
    /// Init jpeg-turbo compress handle.
    tjhandle jpeg_turbo_instance = tjInitCompress();
    if (!jpeg_turbo_instance) {
        LOG_ERR("Failed to init jpeg-turbo compress engine!");
        tjDestroy(jpeg_turbo_instance);
        return false;
    }
    /// Compress Mat data to jpeg type.
    int subsampling_type = (pixel_format == TJPF_GRAY) ? TJSAMP_GRAY : TJSAMP_444;
    unsigned long file_size = tjBufSize(img.width(), img.height(), subsampling_type);
    buf.resize(file_size);
    uint8_t* file_buf = buf.data();
    int bit_flags = 0;
    if (tjCompress2(jpeg_turbo_instance, reinterpret_cast<uint8_t*>(img.data()),
            img.width(), 0, img.height(), pixel_format, &file_buf,
            &file_size, subsampling_type, quality, bit_flags) < 0) {
        LOG_ERR("Failed to compress image data to jpeg data when imencode!");
        tjDestroy(jpeg_turbo_instance);
        return false;
    }
    tjDestroy(jpeg_turbo_instance);
    return true;
}

#endif

G_FCV_NAMESPACE1_END()
