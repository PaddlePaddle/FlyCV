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

#include "modules/media_io/imgcodecs/interface/imgcodecs.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fstream>

#include "modules/core/base/include/macro_utils.h"

#ifdef WITH_LIB_PNG
#include "modules/media_io/imgcodecs/include/imgcodecs_png.h"
#endif

#ifdef WITH_LIB_JPEG_TURBO
#include "modules/media_io/imgcodecs/include/imgcodecs_jpeg.h"
#endif

#ifdef _WIN32
#define strcasecmp  stricmp
#define strncasecmp  strnicmp
#endif

namespace {
/**
 * @brief Check wether the data is standard png format.
 * @param[in] buf Data address.
 * @return true 
 * @return false 
 */
bool check_png_format(const uint8_t* buf) {
    if (!buf) {
        return false;
    }
    uint8_t header[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    for (size_t i = 0; i < sizeof(header); ++i) {
        if (header[i] != buf[i]) {
            return false;
        }
    }
    return true;
}
/**
 * @brief Check wether the data is standard jpeg format.
 * @param[in] buf Data address.
 * @return true 
 * @return false 
 */
bool check_jpeg_format(const uint8_t* buf) {
    if (!buf) {
        return false;
    }
    uint8_t header[] = {0xFF, 0xD8, 0xFF, 0xE0};
    for (size_t i = 0; i < sizeof(header); ++i) {
        if (header[i] != buf[i]) {
            return false;
        }
    }
    return true;
}
} // namespace anonymous

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

Mat imread(const std::string& file_name, int flags) {
    UN_USED(flags); /// Preserve flags for future.
    // Check filename format correct.
    if (file_name.empty()) {
        LOG_ERR("The file_name for imread is empty!");
        return Mat();
    }

    const char* file_suffix = strrchr(file_name.c_str(), '.');

    if (strcasecmp(file_suffix, ".jpg")
            && strcasecmp(file_suffix, ".jpeg")
            && strcasecmp(file_suffix, ".png")) {
        LOG_ERR("imread only support file_name suffix is .jpg or .jpeg or .png now!");
        return Mat();
    }

    /// Check file is readable. 
    FILE* p_file = fopen(file_name.c_str(), "rb");
    if (!p_file) {
        LOG_ERR("Failed to open file during imread process!");
        return Mat();
    }

    /// Get file size
    int res = fseek(p_file, 0, SEEK_END);
    if (res) {
        LOG_ERR("Failed to get file size during imread process!");
        fclose(p_file);
        return Mat();
    }

    size_t file_size = static_cast<size_t>(ftell(p_file));
    res = fseek(p_file, 0, SEEK_SET);
    if (res) {
        LOG_ERR("Failed to get file size during imread process!");
        fclose(p_file);
        return Mat();
    }

    /// Read file content to memory.
    std::unique_ptr<uint8_t[]> file_buf(new uint8_t[file_size]);
    size_t cnt = fread(reinterpret_cast<char*>(file_buf.get()), 1, file_size, p_file);
    if (cnt != file_size) {
        LOG_ERR("Failed to read file data during imread process!");
        fclose(p_file);
        return Mat();
    }
    fclose(p_file);

    /// Deal with PNG file.
    if (strcasecmp(file_suffix, ".png") == 0) {
#ifdef WITH_LIB_PNG
        return imdecode_png(file_buf.get(), file_size);
#else
        LOG_ERR("Rebuild fcv with libpng!");
        return Mat();
#endif
    } 

#ifdef WITH_LIB_JPEG_TURBO
    return imdecode_jpeg(file_buf.get(), file_size);
#else
    LOG_ERR("Rebuild fcv with libjpeg-turbo!");
#endif
    return Mat();
}

bool imwrite(
        const std::string& file_name,
        const Mat& img,
        int quality) {
    /// Check filename format correct.
    if (file_name.empty()) {
        LOG_ERR("imwrite params file_name is empty!");
        return false;
    }

    if (img.empty()) {
        LOG_ERR("imwrite dst img is empty!");
        return false;
    }

    const char* file_suffix = strrchr(file_name.c_str(), '.');
    if (strcasecmp(file_suffix, ".jpg")
            && strcasecmp(file_suffix, ".jpeg")
            && strcasecmp(file_suffix, ".png")) {
        LOG_ERR("imwrite only support file_name suffix is .jpg or .jpeg or png now!");
        return false;
    }

    std::vector<uint8_t> file_data;
    /// PNG file.
    if (strcasecmp(file_suffix, ".png") == 0) {
#ifdef WITH_LIB_PNG
        bool png_status = imencode_png(img, file_data);
        if (!png_status) {
            LOG_ERR("Failed to imwrite data with png format!");
            return false;
        }
#else
        LOG_ERR("Rebuild fcv with libpng!");
        return false;
#endif
    }
    /// JPEG file.
    if (!strcasecmp(file_suffix, ".jpg")
            || !strcasecmp(file_suffix, ".jpeg")) {
#ifdef WITH_LIB_JPEG_TURBO
        bool jpg_status = imencode_jpeg(img, file_data, quality);
        if (!jpg_status) {
            LOG_ERR("Failed to imwrite data with jpeg format!");
            return false;
        }
#else
        UN_USED(quality);
        LOG_ERR("Rebuild fcv with libjpeg-turbo!");
        return false;
#endif
    }

    if (file_data.size() <= 0) {
        LOG_ERR("Failed to read img data!");
        return false;
    }

    /// Write data to file.
    FILE* p_file = fopen(file_name.c_str(), "wb");
    if (!p_file) {
        LOG_ERR("Failed to open file during imwrite process!");
        return false;
    }

    size_t cnt = fwrite(reinterpret_cast<char*>(file_data.data()), 1,
            file_data.size(), p_file);
    if (cnt != file_data.size()) {
        LOG_ERR("Failed to write file during imwrite process!");
        fclose(p_file);
        return false;
    }

    fclose(p_file);

    return true;
}

Mat imdecode(const uint8_t* buf, const size_t size, int flags) {
    if (!buf || !size) {
        LOG_ERR("The input data is empty for imdecode!");
        return Mat();
    }
    Mat img;
    /// PNG format data.
    if (check_png_format(buf)) {
#ifdef WITH_LIB_PNG
        img = imdecode_png((const uint8_t*)buf, size);
        if (!img.empty()) {
            return img;
        }
#else
        LOG_ERR("Rebuild fcv with libpng!");
        return Mat();
#endif
    }
    /// JPEG format data.
    if (check_jpeg_format(buf)) {
#ifdef WITH_LIB_JPEG_TURBO
        img = imdecode_jpeg((const uint8_t*)buf, size, flags);
        if (!img.empty()) {
            return img;
        }
#else
        UN_USED(flags);
        LOG_ERR("Rebuild fcv with libjpeg-turbo!");
        return Mat();
#endif
    }

    LOG_ERR("Not support data format, only supoort jpeg or png data now!");
    return Mat();
}

bool imencode(
        const std::string& ext,
        const Mat& img,
        std::vector<uint8_t>& buf,
        int quality) {
    if (ext.empty() || img.empty()) {
        LOG_ERR("The ext or input mat is empty!");
        return false;
    }

    /// PNG encode.
    if (!strcasecmp(ext.c_str(), ".png")) {
#ifdef WITH_LIB_PNG
        return imencode_png(img, buf);
#else
        LOG_ERR("Rebuild fcv with libpng!");
        return false;
#endif
    }
    /// JPEG encode.
    if (!strcasecmp(ext.c_str(), ".jpg")
            || !strcasecmp(ext.c_str(), "jpeg")) {
#ifdef WITH_LIB_JPEG_TURBO
        return imencode_jpeg(img, buf, quality);
#else
        UN_USED(quality);
        LOG_ERR("Rebuild fcv with libjpeg-turbo!");
        return false;
#endif
    }

#if (!defined WITH_LIB_PNG) && (!defined WITH_LIB_JPEG_TURBO)
    UN_USED(buf);
#endif

    /// Unsupport format.
    LOG_ERR("Unsupport ext format!");
    return false;
}

G_FCV_NAMESPACE1_END()
