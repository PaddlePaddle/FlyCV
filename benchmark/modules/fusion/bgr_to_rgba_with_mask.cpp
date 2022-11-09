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
//#include "modules/core/opencl/interface/opencl.h"

using namespace g_fcv_ns;

class BGR2RGBAWithMask : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(BGR2RGBAWithMask, 720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mask = Mat(1280,720,FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
    Mat dst;
    for (auto _state : state) {  
        bgr_to_rgba_with_mask(src,mask,dst);
    }
}

BENCHMARK_DEFINE_F(BGR2RGBAWithMask, 1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mask = Mat(1920, 1080,FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
    Mat dst;
    for (auto _state : state) {  
        bgr_to_rgba_with_mask(src,mask,dst);
    }
}

BENCHMARK_DEFINE_F(BGR2RGBAWithMask, 4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat mask = Mat(4032,3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(mask.total_byte_size(), feed_num, mask.data());
    Mat dst;
    for (auto _state : state) {  
        bgr_to_rgba_with_mask(src,mask,dst);
    }
}

BENCHMARK_REGISTER_F(BGR2RGBAWithMask, 720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(BGR2RGBAWithMask, 1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(BGR2RGBAWithMask, 4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);
