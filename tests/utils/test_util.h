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

#include <chrono>
#include <random>
#include <vector>
#include <string>

#include "flycv.h"

const int IMG_720P_WIDTH = 1280;
const int IMG_720P_HEIGHT = 720;
const int IMG_480P_WIDTH = 640;
const int IMG_480P_HEIGHT = 480;

const std::string JPG_1280X720("data/BGR_1280x720.jpg");
const std::string PNG_1280X720("data/BGR_1280x720.png");

const std::string GRAY_1280X720_U8_BIN("data/GRAY_1280x720_U8.bin");
const std::string GRAY_1280X720_U16_BIN("data/GRAY_1280x720_U16.bin");
const std::string GRAY_1280X720_F32_BIN("data/GRAY_1280x720_F32.bin");
const std::string BGR_320X180_U8_BIN("data/BGR_320x180_U8.bin");
const std::string BGR_1280X720_U8_BIN("data/BGR_1280x720_U8.bin");
const std::string BGR_1280X720_U8_2_BIN("data/BGR_1280x720_U8_2.bin");
const std::string RGBA_1280X720_U8_BIN("data/RGBA_1280x720_U8.bin");
const std::string BGRA_1280X720_U8_BIN("data/BGRA_1280x720_U8.bin");
const std::string NV12_1280X720_U8_BIN("data/NV12_1280x720_U8.bin");
const std::string NV21_1280X720_U8_BIN("data/NV21_1280x720_U8.bin");
const std::string I420_1280X720_U8_BIN("data/I420_1280x720_U8.bin");
const std::string BGR_1280X720_F32_BIN("data/BGR_1280x720_F32.bin");
const std::string RGB_1280X720_U8_BIN("data/RGB_1280x720_U8.bin");

const std::string RESULTS_PATH("./");

const std::vector<int> C1_640X360_IDX = {0, 1, 2, 102398, 102400, 102402, 230395, 230397, 230399};
const std::vector<int> C1_1280X720_IDX = {0, 1, 2, 460798, 460800, 460802, 921595, 921597, 921599};
const std::vector<int> C3_640X360_IDX = {0, 1, 2, 345598, 345600, 345602, 691195, 691197, 691199};
const std::vector<int> C4_640X360_IDX = {0, 1, 2, 460798, 460800, 460802, 921595, 921597, 921599};
const std::vector<int> C3_1280X720_IDX = {0, 1, 2, 1382398, 1382400, 1382402, 2764695, 2764797, 2764799};
const std::vector<int> C4_1280X720_IDX = {0, 1, 2, 1843198, 1843200, 1843202, 3686395, 3686397, 3686399};
const std::vector<int> YUV_1280X720_IDX = {0, 1, 2, 691198, 691200, 691202, 1382395, 1382397, 1382399};

class Timer {
public:
    void tic() {
        start = std::chrono::high_resolution_clock::now();
    }

    double toc() {
        stop = std::chrono::high_resolution_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
        return diff / 1000.0f; //ms
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::time_point<std::chrono::high_resolution_clock> stop;
};

//init random data
template <typename T>
int init_random(T* host_data, int n, T range_min, T range_max) {
    static std::mt19937 g(42);
    std::uniform_real_distribution<> rnd(range_min, range_max);

    for (int i = 0; i < n; i++) {
        host_data[i] = static_cast<T>(rnd(g));
    }

    return 0;
}

int read_binary_file(
        const std::string& file_path,
        void* buffer,
        long length);

int write_binary_file(
        const std::string& file_path,
        void* buffer,
        long length);

int prepare_gray_u8_720p(g_fcv_ns::Mat& src);
int prepare_gray_u16_720p(g_fcv_ns::Mat& src);
int prepare_gray_f32_720p(g_fcv_ns::Mat& src);
int prepare_pkg_rgb_u8_720p(g_fcv_ns::Mat& src);
int prepare_pkg_bgr_u8_720p(g_fcv_ns::Mat& src);
int prepare_pkg_bgr_u8_720p_2(g_fcv_ns::Mat& src);
int prepare_pkg_bgr_f32_720p(g_fcv_ns::Mat& src);
int prepare_pkg_bgra_u8_720p(g_fcv_ns::Mat& src);
int prepare_nv12_720p(g_fcv_ns::Mat& src);
int prepare_nv21_720p(g_fcv_ns::Mat& src);
int prepare_i420_720p(g_fcv_ns::Mat& src);