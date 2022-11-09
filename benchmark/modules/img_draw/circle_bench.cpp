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

class CircleBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(CircleBench, GRAYU8_720P)(benchmark::State& state) {

    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Point center = Point(100,100);
    set_thread_num(1);
    for (auto _state : state) {
        circle(src, center, 10, Scalar(0, 0, 255));
    }
}

BENCHMARK_DEFINE_F(CircleBench, RGBU8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Point center = Point(100,100);
    set_thread_num(1);
    for (auto _state : state) {
        circle(src, center, 10, Scalar(0, 0, 255));
    }
}

BENCHMARK_DEFINE_F(CircleBench, RGBAU8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Point center = Point(100,100);
    set_thread_num(1);
    for (auto _state : state) {
        circle(src, center, 10, Scalar(0, 0, 255));
    }
}

BENCHMARK_REGISTER_F(CircleBench, GRAYU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(CircleBench, RGBU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(CircleBench, RGBAU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_DEFINE_F(CircleBench, GRAYU8_1080P)(benchmark::State& state) {

    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Point center = Point(100,100);
    set_thread_num(1);
    for (auto _state : state) {
        circle(src, center, 10, Scalar(0, 0, 255));
    }
}

BENCHMARK_DEFINE_F(CircleBench, RGBU8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Point center = Point(100,100);
    set_thread_num(1);
    for (auto _state : state) {
        circle(src, center, 10, Scalar(0, 0, 255));
    }
}

BENCHMARK_DEFINE_F(CircleBench, RGBAU8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Point center = Point(100,100);
    set_thread_num(1);
    for (auto _state : state) {
        circle(src, center, 10, Scalar(0, 0, 255));
    }
}

BENCHMARK_REGISTER_F(CircleBench, GRAYU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(CircleBench, RGBU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(CircleBench, RGBAU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_DEFINE_F(CircleBench, GRAYU8_4K)(benchmark::State& state) {

    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Point center = Point(100,100);
    set_thread_num(1);
    for (auto _state : state) {
        circle(src, center, 10, Scalar(0, 0, 255));
    }
}

BENCHMARK_DEFINE_F(CircleBench, RGBU8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Point center = Point(100,100);
    set_thread_num(1);
    for (auto _state : state) {
        circle(src, center, 10, Scalar(0, 0, 255));
    }
}

BENCHMARK_DEFINE_F(CircleBench, RGBAU8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Point center = Point(100,100);
    set_thread_num(1);
    for (auto _state : state) {
        circle(src, center, 10, Scalar(0, 0, 255));
    }
}

BENCHMARK_REGISTER_F(CircleBench, GRAYU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(CircleBench, RGBU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(CircleBench, RGBAU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);