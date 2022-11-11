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

class WarpAffineBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(WarpAffineBench, PKG_BGR_U8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
        Mat matrix(3, 2, FCVImageType::GRAY_F32, m);
        Mat dst;
        warp_affine(src, dst, matrix);
    }
}

BENCHMARK_DEFINE_F(WarpAffineBench, PKG_BGR_F32_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
        Mat matrix(3, 2, FCVImageType::GRAY_F32, m);
        Mat dst;
        warp_affine(src, dst, matrix);
    }
}

BENCHMARK_REGISTER_F(WarpAffineBench, PKG_BGR_U8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(WarpAffineBench, PKG_BGR_F32_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_DEFINE_F(WarpAffineBench, PKG_BGR_U8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
        Mat matrix(3, 2, FCVImageType::GRAY_F32, m);
        Mat dst;
        warp_affine(src, dst, matrix);
    }
}

BENCHMARK_DEFINE_F(WarpAffineBench, PKG_BGR_F32_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
        Mat matrix(3, 2, FCVImageType::GRAY_F32, m);
        Mat dst;
        warp_affine(src, dst, matrix);
    }
}

BENCHMARK_REGISTER_F(WarpAffineBench, PKG_BGR_U8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(WarpAffineBench, PKG_BGR_F32_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_DEFINE_F(WarpAffineBench, PKG_BGR_U8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
        Mat matrix(3, 2, FCVImageType::GRAY_F32, m);
        Mat dst;
        warp_affine(src, dst, matrix);
    }
}

BENCHMARK_DEFINE_F(WarpAffineBench, PKG_BGR_F32_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
        Mat matrix(3, 2, FCVImageType::GRAY_F32, m);
        Mat dst;
        warp_affine(src, dst, matrix);
    }
}

BENCHMARK_REGISTER_F(WarpAffineBench, PKG_BGR_U8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(WarpAffineBench, PKG_BGR_F32_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);        
