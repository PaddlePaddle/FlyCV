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

#include "benchmark/benchmark.h"
#include "common/utils.h"
#include "flycv.h"

using namespace g_fcv_ns;

class RemapBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(RemapBench, GRAYU8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    Mat map_x(1280, 720, FCVImageType::GRAY_F32);
    Mat map_y(1280, 720, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }

    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_DEFINE_F(RemapBench, GRAYF32_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
        
    Mat map_x(1280, 720, FCVImageType::GRAY_F32);
    Mat map_y(1280, 720, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }

    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_REGISTER_F(RemapBench, GRAYU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RemapBench, GRAYF32_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_DEFINE_F(RemapBench, GRAYU8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    Mat map_x(1920, 1080, FCVImageType::GRAY_F32);
    Mat map_y(1920, 1080, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }
    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_DEFINE_F(RemapBench, GRAYF32_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
        
    Mat map_x(1920, 1080, FCVImageType::GRAY_F32);
    Mat map_y(1920, 1080, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }
    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_REGISTER_F(RemapBench, GRAYU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RemapBench, GRAYF32_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200); 

//4K
BENCHMARK_DEFINE_F(RemapBench, GRAYU8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    Mat map_x(4032, 3024, FCVImageType::GRAY_F32);
    Mat map_y(4032, 3024, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }

    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_DEFINE_F(RemapBench, GRAYF32_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
        
    Mat map_x(4032, 3024, FCVImageType::GRAY_F32);
    Mat map_y(4032, 3024, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }
    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_REGISTER_F(RemapBench, GRAYU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RemapBench, GRAYF32_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);


BENCHMARK_DEFINE_F(RemapBench, BGRU8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    Mat map_x(1280, 720, FCVImageType::GRAY_F32);
    Mat map_y(1280, 720, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }

    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_DEFINE_F(RemapBench, BGRF32_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
        
    Mat map_x(1280, 720, FCVImageType::GRAY_F32);
    Mat map_y(1280, 720, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }

    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_REGISTER_F(RemapBench, BGRU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RemapBench, BGRF32_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_DEFINE_F(RemapBench, BGRU8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    Mat map_x(1920, 1080, FCVImageType::GRAY_F32);
    Mat map_y(1920, 1080, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }
    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_DEFINE_F(RemapBench, BGRF32_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
        
    Mat map_x(1920, 1080, FCVImageType::GRAY_F32);
    Mat map_y(1920, 1080, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }
    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_REGISTER_F(RemapBench, BGRU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RemapBench, BGRF32_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200); 

//4K

BENCHMARK_DEFINE_F(RemapBench, BGRU8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    Mat map_x(4032, 3024, FCVImageType::GRAY_F32);
    Mat map_y(4032, 3024, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }

    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_DEFINE_F(RemapBench, BGRF32_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
        
    Mat map_x(4032, 3024, FCVImageType::GRAY_F32);
    Mat map_y(4032, 3024, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }
    for (auto _state : state) {
        Mat dst;
        remap(src, dst, map_x, map_y);
    }
}

BENCHMARK_REGISTER_F(RemapBench, BGRU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RemapBench, BGRF32_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);
