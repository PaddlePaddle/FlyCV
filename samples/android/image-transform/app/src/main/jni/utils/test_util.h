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

#include <chrono>
#include <random>
#include <vector>
#include <string>

const int IMG_720P_WIDTH = 1280;
const int IMG_720P_HEIGHT = 720;
const int IMG_480P_WIDTH = 640;
const int IMG_480P_HEIGHT = 480;
const int IMG_180P_WIDTH = 320;
const int IMG_180P_HEIGHT = 180;

const std::string JPG_320X180("/sdcard/flycv/data/BGR_320x180.jpg");
const std::string PNG_320X180("/sdcard/flycv/data/BGR_320x180.png");

const std::string GRAY_1280X720_U8_BIN("/sdcard/flycv/data/GRAY_1280x720_U8.bin");
const std::string GRAY_1280X720_U16_BIN("/sdcard/flycv/data/GRAY_1280x720_U16.bin");
const std::string GRAY_1280X720_F32_BIN("/sdcard/flycv/data/GRAY_1280x720_F32.bin");
const std::string BGR_320X180_U8_BIN("/sdcard/flycv/data/BGR_320x180_U8.bin");
const std::string BGR_1280X720_U8_BIN("/sdcard/flycv/data/BGR_1280x720_U8.bin");
const std::string RGBA_1280X720_U8_BIN("/sdcard/flycv/data/RGBA_1280x720_U8.bin");
const std::string BGRA_1280X720_U8_BIN("/sdcard/flycv/data/BGRA_1280x720_U8.bin");
const std::string NV12_1280X720_U8_BIN("/sdcard/flycv/data/NV12_1280x720_U8.bin");
const std::string NV21_1280X720_U8_BIN("/sdcard/flycv/data/NV21_1280x720_U8.bin");
const std::string I420_1280X720_U8_BIN("/sdcard/flycv/data/I420_1280x720_U8.bin");
const std::string BGR_1280X720_F32_BIN("/sdcard/flycv/data/BGR_1280x720_F32.bin");
const std::string RGB_1280X720_U8_BIN("/sdcard/flycv/data/RGB_1280x720_U8.bin");

const std::string RESULTS_PATH("./");

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
