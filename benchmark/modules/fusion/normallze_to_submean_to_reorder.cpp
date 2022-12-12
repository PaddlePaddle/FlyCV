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

class NormalizeSubmeanReorder : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(NormalizeSubmeanReorder, BGR2RGBSubmean_720P_U8)
        (benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    Mat dst;
    std::vector<float> mean_param = {23.55f, 33.66f, 77.99f};
    std::vector<float> std_param = {10.55f, 4.66f, 7.99f};

    for (auto _state : state) {  
        normalize_to_submean_to_reorder(src, mean_param, std_param, {0, 1, 2}, dst);
    }
}

BENCHMARK_DEFINE_F(NormalizeSubmeanReorder, BGR2RGBSubmean_1080P_U8)
        (benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    Mat dst;
    std::vector<float> mean_param = {23.55f, 33.66f, 77.99f};
    std::vector<float> std_param = {10.55f, 4.66f, 7.99f};

    for (auto _state : state) {  
        normalize_to_submean_to_reorder(src, mean_param, std_param, {0, 1, 2}, dst);
    }
}

BENCHMARK_DEFINE_F(NormalizeSubmeanReorder, BGR2RGBSubmean_4K_U8)
        (benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    Mat dst;
    std::vector<float> mean_param = {23.55f, 33.66f, 77.99f};
    std::vector<float> std_param = {10.55f, 4.66f, 7.99f};

    for (auto _state : state) { 
        normalize_to_submean_to_reorder(src, mean_param, std_param, {0, 1, 2}, dst);
    }
}

BENCHMARK_REGISTER_F(NormalizeSubmeanReorder, BGR2RGBSubmean_720P_U8)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(NormalizeSubmeanReorder, BGR2RGBSubmean_1080P_U8)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(NormalizeSubmeanReorder, BGR2RGBSubmean_4K_U8)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_DEFINE_F(NormalizeSubmeanReorder, BGR2RGBSubmean_720P_F32)
        (benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size()/src.type_byte_size(), feed_num, src.data());

    Mat dst;
    std::vector<float> mean_param = {23.55f, 33.66f, 77.99f};
    std::vector<float> std_param = {10.55f, 4.66f, 7.99f};

    for (auto _state : state) {  
        normalize_to_submean_to_reorder(src, mean_param, std_param, {0, 1, 2}, dst);
    }
}

BENCHMARK_DEFINE_F(NormalizeSubmeanReorder, BGR2RGBSubmean_1080P_F32)
        (benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size()/src.type_byte_size(), feed_num, src.data());

    Mat dst;
    std::vector<float> mean_param = {23.55f, 33.66f, 77.99f};
    std::vector<float> std_param = {10.55f, 4.66f, 7.99f};

    for (auto _state : state) {  
        normalize_to_submean_to_reorder(src, mean_param, std_param, {0, 1, 2}, dst);
    }
}

BENCHMARK_DEFINE_F(NormalizeSubmeanReorder, BGR2RGBSubmean_4K_F32)
        (benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    Mat dst;
    std::vector<float> mean_param = {23.55f, 33.66f, 77.99f};
    std::vector<float> std_param = {10.55f, 4.66f, 7.99f};

    for (auto _state : state) {  
        normalize_to_submean_to_reorder(src, mean_param, std_param, {0, 1, 2}, dst);
    }
}

BENCHMARK_REGISTER_F(NormalizeSubmeanReorder, BGR2RGBSubmean_720P_F32)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(NormalizeSubmeanReorder, BGR2RGBSubmean_1080P_F32)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(NormalizeSubmeanReorder, BGR2RGBSubmean_4K_F32)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);