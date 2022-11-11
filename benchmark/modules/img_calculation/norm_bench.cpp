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

class NormBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(NormBench, Norm720pC1INF)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_INF);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm720pC3INF)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_INF);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm720pC4INF)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_INF);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm720pC1L1)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L1);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm720pC3L1)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L1);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm720pC4L1)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L1);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm720pC1L2)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L2);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm720pC3L2)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L2);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm720pC4L2)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L2);
    }
}

//4k
BENCHMARK_DEFINE_F(NormBench, Norm4KC1INF)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_INF);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm4KC3INF)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_INF);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm4KC4INF)(benchmark::State& state) {
    Mat src = Mat(4032, 3024,  FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_INF);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm4KC1L1)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L1);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm4KC3L1)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L1);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm4KC4L1)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
        
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L1);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm4KC1L2)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L2);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm4KC3L2)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L2);
    }
}

BENCHMARK_DEFINE_F(NormBench, Norm4KC4L2)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    for (auto _state : state) {
        norm(src, NormTypes::NORM_L2);
    }
}

BENCHMARK_REGISTER_F(NormBench, Norm720pC1INF)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(NormBench, Norm720pC3INF)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm720pC4INF)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   
       
BENCHMARK_REGISTER_F(NormBench, Norm720pC1L1)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm720pC3L1)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm720pC4L1)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm720pC1L2)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm720pC3L2)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm720pC4L2)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

//4k
BENCHMARK_REGISTER_F(NormBench, Norm4KC1INF)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm4KC3INF)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm4KC4INF)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm4KC1L1)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm4KC3L1)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm4KC4L1)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm4KC1L2)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm4KC3L2)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   

BENCHMARK_REGISTER_F(NormBench, Norm4KC4L2)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);   
