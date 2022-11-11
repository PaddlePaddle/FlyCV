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

class SplitToMemBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(SplitToMemBench, SplitToMemC3_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
    float* dst = new float[1280*720*3];
    
    for (auto _state : state) {  
        split_to_memcpy(src,dst);
    }
}

BENCHMARK_DEFINE_F(SplitToMemBench, SplitToMemC4_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGRA_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
    float* dst = new float[1280*720*4];
    
    for (auto _state : state) {  
        split_to_memcpy(src,dst);
    }
}

BENCHMARK_DEFINE_F(SplitToMemBench, SplitToMemC3_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
    float* dst = new float[1920*1080*3];
    
    for (auto _state : state) {  
        split_to_memcpy(src,dst);
    }
}

BENCHMARK_DEFINE_F(SplitToMemBench, SplitToMemC4_1080P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGRA_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
    float* dst = new float[1920*1080*4];
    
    for (auto _state : state) {  
        split_to_memcpy(src,dst);
    }
}

BENCHMARK_DEFINE_F(SplitToMemBench, SplitToMemC3_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
    float* dst = new float[4032 * 3024 * 3];
    
    for (auto _state : state) {  
        split_to_memcpy(src,dst);
    }
}

BENCHMARK_DEFINE_F(SplitToMemBench, SplitToMemC4_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGRA_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
    float* dst = new float[4032 * 3024 * 4];
    
    for (auto _state : state) {  
        split_to_memcpy(src,dst);
    }
}

BENCHMARK_REGISTER_F(SplitToMemBench, SplitToMemC3_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SplitToMemBench, SplitToMemC4_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SplitToMemBench, SplitToMemC3_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SplitToMemBench, SplitToMemC4_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SplitToMemBench, SplitToMemC3_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SplitToMemBench, SplitToMemC4_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);
