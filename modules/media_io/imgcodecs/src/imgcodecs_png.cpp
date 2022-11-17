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

#include "modules/media_io/imgcodecs/include/imgcodecs_png.h"

#ifdef WITH_LIB_PNG
#include "third_party/libpng/png.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#ifdef WITH_LIB_PNG
Mat imdecode_png(const uint8_t* buf, const size_t size) {
    if (!buf || !size) {
        LOG_ERR("The input data is empty for imdecode!");
        return Mat();
    }

    png_image image;
    memset(&image, 0, sizeof image);
    image.version = PNG_IMAGE_VERSION;
    int res = png_image_begin_read_from_memory(&image, buf, size);
    if (res == 0) {
        LOG_ERR("Imdecode_png failed from buf!");
        return Mat();
    }

    FCVImageType fcv_mat_type = FCVImageType::PKG_BGR_U8;
    switch (image.format) {
    case PNG_FORMAT_RGB:
        fcv_mat_type = FCVImageType::PKG_RGB_U8;
        break;
    case PNG_FORMAT_BGR:
        fcv_mat_type = FCVImageType::PKG_BGR_U8;
        break;
    case PNG_FORMAT_GRAY:
        fcv_mat_type = FCVImageType::GRAY_U8;
        break;
    case PNG_FORMAT_RGBA:
        fcv_mat_type = FCVImageType::PKG_RGBA_U8;
        break;
    case PNG_FORMAT_BGRA:
        fcv_mat_type = FCVImageType::PKG_BGRA_U8;
        break;
    default:
        LOG_ERR("Unsupport png format: %d", image.format);
        fcv_mat_type = FCVImageType::PKG_BGRA_U8;;
        break;
    }

    Mat img(image.width, image.height, fcv_mat_type);
    png_image_finish_read(&image, nullptr, img.data(), 0, nullptr);
    png_image_free(&image);
    return img;
}

bool imencode_png(const Mat& img, std::vector<uint8_t>& buf) {
    if (img.empty()) {
        LOG_ERR("The img for encode_png is empty!");
        return false;
    }
    png_image image;
    memset(&image, 0, sizeof image);
    image.version = PNG_IMAGE_VERSION;
    switch (img.type()) {
    case FCVImageType::GRAY_U8:
        image.format = PNG_FORMAT_GRAY;
        break;
    case FCVImageType::PKG_RGB_U8:
        image.format = PNG_FORMAT_RGB;
        break;
    case FCVImageType::PKG_BGR_U8:
        image.format = PNG_FORMAT_BGR;
        break;
    case FCVImageType::PKG_RGBA_U8:
        image.format = PNG_FORMAT_RGBA;
        break;
    case FCVImageType::PKG_BGRA_U8:
        image.format = PNG_FORMAT_BGRA;
        break;
    default:
        LOG_ERR("Unsupport fcv Mat type for imencode_png: %d", int(img.type()));
        return false;
    }

    image.width = img.width();
    image.height = img.height();
    size_t file_size = 0;
    png_image_write_get_memory_size(image, file_size, 0, img.data(), 0, 0);
    buf.resize(file_size);
    png_image_write_to_memory(&image, buf.data(), &file_size, 0, img.data(), 0, 0);
    png_image_free(&image);
    return true;
}
#endif

G_FCV_NAMESPACE1_END()
