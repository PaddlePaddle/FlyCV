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

class MeanBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(MeanBench, Mean_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
     
    for (auto _state : state) {
        mean(src);
    }
}

BENCHMARK_DEFINE_F(MeanBench, Mean_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
     
    for (auto _state : state) {
        mean(src);
    }
}

BENCHMARK_DEFINE_F(MeanBench, Mean_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
         
    for (auto _state : state) {
        mean(src);
    }
}

BENCHMARK_DEFINE_F(MeanBench, Mean_Mask_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mask = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
     
    for (auto _state : state) {
        mean(src, mask);
    }
}

BENCHMARK_DEFINE_F(MeanBench, Mean_Mask_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mask = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
     
    for (auto _state : state) {
        mean(src, mask);
    }
}

BENCHMARK_DEFINE_F(MeanBench, Mean_Mask_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mask = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
     
    for (auto _state : state) {
        mean(src, mask);
    }
}

BENCHMARK_DEFINE_F(MeanBench, Mean_Rect_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Rect rect = Rect(160, 120, 320, 240);
     
    for (auto _state : state) {
        mean(src, rect);
    }
}

BENCHMARK_DEFINE_F(MeanBench, Mean_Rect_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Rect rect = Rect(320, 180, 640, 360);
     
    for (auto _state : state) {
        mean(src, rect);
    }
}

BENCHMARK_DEFINE_F(MeanBench, Mean_Rect_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Rect rect = Rect(320, 180, 640, 360);
     
    for (auto _state : state) {
        mean(src, rect);
    }
}

BENCHMARK_DEFINE_F(MeanBench, Mean_Stddev_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mean;
    Mat stddev;
     
    for (auto _state : state) {
        mean_stddev(src, mean, stddev);
    }
}

BENCHMARK_DEFINE_F(MeanBench, Mean_Stddev_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mean;
    Mat stddev;
     
    for (auto _state : state) {
        mean_stddev(src, mean, stddev);
    }
}

BENCHMARK_DEFINE_F(MeanBench, Mean_Stddev_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mean;
    Mat stddev;
     
    for (auto _state : state) {
        mean_stddev(src, mean, stddev);
    }
}

BENCHMARK_REGISTER_F(MeanBench, Mean_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MeanBench, Mean_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MeanBench, Mean_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MeanBench, Mean_Mask_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MeanBench, Mean_Mask_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MeanBench, Mean_Mask_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MeanBench, Mean_Rect_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MeanBench, Mean_Rect_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MeanBench, Mean_Rect_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MeanBench, Mean_Stddev_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MeanBench, Mean_Stddev_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MeanBench, Mean_Stddev_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);
