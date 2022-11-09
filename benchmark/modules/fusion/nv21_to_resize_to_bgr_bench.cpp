/***************************************************************************
 *
 * Copyright (c) 2022 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @contributor     huwenchao
 * @created         2022-10-26 13:28
 */

#include "benchmark/benchmark.h"
#include "common/utils.h"
#include "flycv.h"
// #include "modules/core/opencl/interface/opencl.h"

using namespace g_fcv_ns;

class NV212Resize2BGRBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(NV212Resize2BGRBench, 1080Pto480P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst = Mat(640, 480, FCVImageType::PKG_BGR_U8);
    set_thread_num(1);
    for (auto _state : state) {  
        nv21_to_resize_to_bgr(src, dst, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(NV212Resize2BGRBench, 1080Pto720P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    set_thread_num(1);
    for (auto _state : state) {  
        nv21_to_resize_to_bgr(src, dst, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(NV212Resize2BGRBench, 4Kto720P)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    set_thread_num(1);
    for (auto _state : state) {  
        nv21_to_resize_to_bgr(src, dst, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(NV212Resize2BGRBench, 4Kto1080P)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
    Mat dst = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    set_thread_num(1);
    for (auto _state : state) {  
        nv21_to_resize_to_bgr(src, dst, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_REGISTER_F(NV212Resize2BGRBench, 1080Pto480P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(NV212Resize2BGRBench, 1080Pto720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(NV212Resize2BGRBench, 4Kto720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(NV212Resize2BGRBench, 4Kto1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);
