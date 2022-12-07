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
// #include "modules/core/opencl/interface/opencl.h"

using namespace g_fcv_ns;


class Y420SPResize2BGRBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Linear_NV21_1080P_To_480P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv21_to_resize_to_bgr(src, dst, Size(640, 480), InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Nearest_NV21_1080P_To_480P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv21_to_resize_to_bgr(src, dst, Size(640, 480), InterpolationType::INTER_NEAREST);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Linear_NV21_1080P_To_720P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv21_to_resize_to_bgr(src, dst, Size(1280, 720), InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Nearest_NV21_1080P_To_720P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv21_to_resize_to_bgr(src, dst, Size(1280, 720), InterpolationType::INTER_NEAREST);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Linear_NV21_4K_To_720P)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv21_to_resize_to_bgr(src, dst, Size(1280, 720), InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Nearest_NV21_4K_To_720P)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv21_to_resize_to_bgr(src, dst, Size(1280, 720), InterpolationType::INTER_NEAREST);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Linear_NV21_4K_To_1080P)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv21_to_resize_to_bgr(src, dst, Size(1920, 1080), InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Nearest_NV21_4K_To_1080P)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv21_to_resize_to_bgr(src, dst, Size(1920, 1080), InterpolationType::INTER_NEAREST);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Linear_NV12_1080P_To_480P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::NV12);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst(640, 480, FCVImageType::PKG_BGR_U8);
    for (auto _state : state) {
        nv12_to_resize_to_bgr(src, dst, Size(640, 480), InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Nearest_NV12_1080P_To_480P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::NV12);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst(640, 480, FCVImageType::PKG_BGR_U8);
    for (auto _state : state) {
        nv12_to_resize_to_bgr(src, dst, Size(640, 480), InterpolationType::INTER_NEAREST);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Linear_NV12_1080P_To_720P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::NV12);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv12_to_resize_to_bgr(src, dst, Size(1280, 720), InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Nearest_NV12_1080P_To_720P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::NV12);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv12_to_resize_to_bgr(src, dst, Size(1280, 720), InterpolationType::INTER_NEAREST);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Linear_NV12_4K_To_720P)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::NV12);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv12_to_resize_to_bgr(src, dst, Size(1280, 720), InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Nearest_NV12_4K_To_720P)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::NV12);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv12_to_resize_to_bgr(src, dst, Size(1280, 720), InterpolationType::INTER_NEAREST);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Linear_NV12_4K_To_1080P)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::NV12);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv12_to_resize_to_bgr(src, dst, Size(1920, 1080), InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(Y420SPResize2BGRBench, Nearest_NV12_4K_To_1080P)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::NV12);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    for (auto _state : state) {
        nv12_to_resize_to_bgr(src, dst, Size(1920, 1080), InterpolationType::INTER_NEAREST);
    }
}

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Linear_NV21_1080P_To_480P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Linear_NV21_1080P_To_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Linear_NV21_4K_To_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Linear_NV21_4K_To_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Linear_NV12_1080P_To_480P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Linear_NV12_1080P_To_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Linear_NV12_4K_To_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Linear_NV12_4K_To_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Nearest_NV21_1080P_To_480P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Nearest_NV21_1080P_To_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Nearest_NV21_4K_To_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Nearest_NV21_4K_To_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Nearest_NV12_1080P_To_480P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Nearest_NV12_1080P_To_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Nearest_NV12_4K_To_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(Y420SPResize2BGRBench, Nearest_NV12_4K_To_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);
