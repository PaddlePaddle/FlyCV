/***************************************************************************
 *
 * Copyright (c) 2022 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @contributor     liuyinghan
 * @created         2022-10-20 10:09
 */

#include "benchmark/benchmark.h"
#include "common/utils.h"
#include "flycv.h"

using namespace g_fcv_ns;

class ConnectedComponentsBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(ConnectedComponentsBench, ConnctedComponents_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Mat label;
     
    for (auto _state : state) {
        connected_components(
        src,
        label,
        8,
        FCVImageType::GRAY_S32);
    }
}

BENCHMARK_DEFINE_F(ConnectedComponentsBench, ConnctedComponents_720p)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Mat label;
     
    for (auto _state : state) {
        connected_components(
        src,
        label,
        8,
        FCVImageType::GRAY_S32);
    }
}

BENCHMARK_DEFINE_F(ConnectedComponentsBench, ConnctedComponents_1080p)(benchmark::State& state) {
     Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    
    Mat label;
     
    for (auto _state : state) {
        connected_components(
        src,
        label,
        8,
        FCVImageType::GRAY_S32);
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