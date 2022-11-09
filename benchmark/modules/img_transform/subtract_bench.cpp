/***************************************************************************
 *
 * Copyright (c) 2021 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @contributor     huwenchao
 * @created         2022-10-25 13:28
 * @brief
 */

#include "benchmark/benchmark.h"
#include "common/utils.h"
#include "flycv.h"

using namespace g_fcv_ns;

class SubtractBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(SubtractBench, GRAYU8_720P)(benchmark::State& state) {

    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst = Mat(1280, 720, FCVImageType::GRAY_U8);
    set_thread_num(1);
    for (auto _state : state) {
        
        subtract(src,Scalar(2),dst);
    }
}

BENCHMARK_DEFINE_F(SubtractBench, RGBU8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    
    set_thread_num(1);
    for (auto _state : state) {
        Mat dst;
        subtract(src,Scalar(2,3,5),dst);
    }
}

BENCHMARK_DEFINE_F(SubtractBench, RGBAU8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    
    set_thread_num(1);
    for (auto _state : state) {
        Mat dst;
        subtract(src,Scalar(2,3,5,10),dst);
    }
}

BENCHMARK_DEFINE_F(SubtractBench, RGBF32_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
    
    Mat dst;
    set_thread_num(1);
    for (auto _state : state) {
        subtract(src,Scalar(2,3,5),dst);
    }
}

BENCHMARK_REGISTER_F(SubtractBench, GRAYU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SubtractBench, RGBU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SubtractBench, RGBAU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SubtractBench, RGBF32_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_DEFINE_F(SubtractBench, GRAYU8_1080P)(benchmark::State& state) {

    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    set_thread_num(1);
    for (auto _state : state) {
        subtract(src,Scalar(2),dst);
    }
}

BENCHMARK_DEFINE_F(SubtractBench, RGBU8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Mat dst;
    set_thread_num(1);
    for (auto _state : state) {
        subtract(src,Scalar(2,3,5),dst);
    }
}

BENCHMARK_DEFINE_F(SubtractBench, RGBAU8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Mat dst;
    set_thread_num(1);
    for (auto _state : state) {
        subtract(src,Scalar(2,3,5,10),dst);
    }
}

BENCHMARK_DEFINE_F(SubtractBench, RGBF32_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
    
    Mat dst;
    set_thread_num(1);
    for (auto _state : state) {
        subtract(src,Scalar(2,3,5),dst);
    }
}

BENCHMARK_REGISTER_F(SubtractBench, GRAYU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SubtractBench, RGBU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SubtractBench, RGBAU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SubtractBench, RGBF32_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_DEFINE_F(SubtractBench, GRAYU8_4K)(benchmark::State& state) {

    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    set_thread_num(1);
    for (auto _state : state) {
        subtract(src,Scalar(2),dst);
    }
}

BENCHMARK_DEFINE_F(SubtractBench, RGBU8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Mat dst;
    set_thread_num(1);
    for (auto _state : state) {
        subtract(src,Scalar(2,3,5),dst);
    }
}

BENCHMARK_DEFINE_F(SubtractBench, RGBAU8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Mat dst;
    set_thread_num(1);
    for (auto _state : state) {
        subtract(src,Scalar(2,3,5,10),dst);
    }
}

BENCHMARK_DEFINE_F(SubtractBench, RGBF32_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());
    
    Mat dst;
    set_thread_num(1);
    for (auto _state : state) {
        subtract(src,Scalar(2,3,5),dst);
    }
}

BENCHMARK_REGISTER_F(SubtractBench, GRAYU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SubtractBench, RGBU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SubtractBench, RGBAU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(SubtractBench, RGBF32_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);        