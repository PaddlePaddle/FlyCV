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

class MinMaxLocBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(MinMaxLocBench, MinMaxLoc_720P_NoMask)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    double min_val;
    double max_val;
    Point min_loc;
    Point max_loc;
    
    for (auto _state : state) {
        min_max_loc(src, &min_val, &max_val, &min_loc, &max_loc);
    }
}

BENCHMARK_DEFINE_F(MinMaxLocBench, MinMaxLoc_1080P_NoMask)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    double min_val;
    double max_val;
    Point min_loc;
    Point max_loc;
    
    for (auto _state : state) {
        min_max_loc(src, &min_val, &max_val, &min_loc, &max_loc);
    }
}

BENCHMARK_DEFINE_F(MinMaxLocBench, MinMaxLoc_4K_NoMask)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    double min_val;
    double max_val;
    Point min_loc;
    Point max_loc;
    
    for (auto _state : state) {
        min_max_loc(src, &min_val, &max_val, &min_loc, &max_loc);
    }
}

BENCHMARK_DEFINE_F(MinMaxLocBench, MinMaxLoc_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mask = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
    double min_val;
    double max_val;
    Point min_loc;
    Point max_loc;
    
    for (auto _state : state) {
        min_max_loc(src, &min_val, &max_val, &min_loc, &max_loc, mask);
    }
}

BENCHMARK_DEFINE_F(MinMaxLocBench, MinMaxLoc_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mask = Mat(1280,720,FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
    double min_val;
    double max_val;
    Point min_loc;
    Point max_loc;
    
    for (auto _state : state) {
        min_max_loc(src, &min_val, &max_val, &min_loc, &max_loc, mask);
    }
}

BENCHMARK_DEFINE_F(MinMaxLocBench, MinMaxLoc_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mask = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
    double min_val;
    double max_val;
    Point min_loc;
    Point max_loc;
    
    for (auto _state : state) {
        min_max_loc(src, &min_val, &max_val, &min_loc, &max_loc, mask);
    }
}

BENCHMARK_REGISTER_F(MinMaxLocBench, MinMaxLoc_720P_NoMask)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MinMaxLocBench, MinMaxLoc_1080P_NoMask)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MinMaxLocBench, MinMaxLoc_4K_NoMask)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MinMaxLocBench, MinMaxLoc_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MinMaxLocBench, MinMaxLoc_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MinMaxLocBench, MinMaxLoc_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);
