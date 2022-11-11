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

class ConnectedComponentsBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(ConnectedComponentsBench, ConnctedComponents_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Mat label;
     
    for (auto _state : state) {
        connected_components(src, label, 8, FCVImageType::GRAY_S32);
    }
}

BENCHMARK_DEFINE_F(ConnectedComponentsBench, ConnctedComponents_720p)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Mat label;
     
    for (auto _state : state) {
        connected_components(src, label, 8, FCVImageType::GRAY_S32);
    }
}

BENCHMARK_DEFINE_F(ConnectedComponentsBench, ConnctedComponents_1080p)(benchmark::State& state) {
     Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Mat label;
     
    for (auto _state : state) {
        connected_components(src, label, 8, FCVImageType::GRAY_S32);
    }
}

BENCHMARK_REGISTER_F(ConnectedComponentsBench, ConnctedComponents_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ConnectedComponentsBench, ConnctedComponents_720p)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ConnectedComponentsBench, ConnctedComponents_1080p)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);
