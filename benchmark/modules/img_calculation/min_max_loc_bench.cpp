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

class MinMaxLocBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(MinMaxLocBench, MinMaxLoc_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    double min_val;
    double max_val;
    Point min_loc;
    Point max_loc;
    Mat mask = Mat(4032,3024,FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
    
    for (auto _state : state) {
        min_max_loc(
            src,
            &min_val,&max_val,
            &min_loc,&max_loc,
            mask);
    }
}

BENCHMARK_DEFINE_F(MinMaxLocBench, MinMaxLoc_720p)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    double min_val;
    double max_val;
    Point min_loc;
    Point max_loc;
    Mat mask = Mat(1280,720,FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
    
    for (auto _state : state) {
        min_max_loc(
            src,
            &min_val,&max_val,
            &min_loc,&max_loc,
            mask);
    }
}

BENCHMARK_DEFINE_F(MinMaxLocBench, MinMaxLoc_1080p)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    double min_val;
    double max_val;
    Point min_loc;
    Point max_loc;
    Mat mask = Mat(1920,1080,FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
    
    for (auto _state : state) {
        min_max_loc(
            src,
            &min_val,&max_val,
            &min_loc,&max_loc,
            mask);
    }
}

BENCHMARK_REGISTER_F(MinMaxLocBench, MinMaxLoc_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MinMaxLocBench, MinMaxLoc_720p)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MinMaxLocBench, MinMaxLoc_1080p)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);