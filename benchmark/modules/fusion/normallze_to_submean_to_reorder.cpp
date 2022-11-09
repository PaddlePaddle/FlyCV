/***************************************************************************
 *
 * Copyright (c) 2022 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @contributor     taotianran
 * @created         2022-07-22 10:09
 */

#include "benchmark/benchmark.h"
#include "common/utils.h"
#include "flycv.h"
// #include "modules/core/opencl/interface/opencl.h"

using namespace g_fcv_ns;

class NormalizeSubmeanReorder : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(NormalizeSubmeanReorder, BGR2RGBSubmead_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    std::vector<float> mean_param = {23.55,33.66,77.99};
    std::vector<float> std_param = {10.55,4.66,7.99};

    set_thread_num(1);
    for (auto _state : state) {  
        normalize_to_submean_to_reorder(src,mean_param,std_param,{0,1,2},dst);
    }
}

BENCHMARK_DEFINE_F(NormalizeSubmeanReorder, BGR2RGBSubmead_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    std::vector<float> mean_param = {23.55,33.66,77.99};
    std::vector<float> std_param = {10.55,4.66,7.99};

    set_thread_num(1);
    for (auto _state : state) {  
        normalize_to_submean_to_reorder(src, mean_param, std_param, {0, 1, 2}, dst);
    }
}

BENCHMARK_DEFINE_F(NormalizeSubmeanReorder, BGR2RGBSubmead_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst;
    std::vector<float> mean_param = {23.55,33.66,77.99};
    std::vector<float> std_param = {10.55,4.66,7.99};

    set_thread_num(1);
    for (auto _state : state) {  
        normalize_to_submean_to_reorder(src,mean_param,std_param,{0,1,2},dst);
    }
}

BENCHMARK_REGISTER_F(NormalizeSubmeanReorder, BGR2RGBSubmead_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(NormalizeSubmeanReorder, BGR2RGBSubmead_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(NormalizeSubmeanReorder, BGR2RGBSubmead_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);


