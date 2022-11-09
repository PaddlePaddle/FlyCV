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

class MatrixMulBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(MatrixMulBench, Mul2Matrix_720p)(benchmark::State& state) {
    Mat src0 = Mat(1280, 720, FCVImageType::GRAY_F32);
    construct_data<float>(src0.total_byte_size() / src0.type_byte_size(), feed_num, src0.data());
    Mat src1 = Mat(720, 1280, FCVImageType::GRAY_F32);
    construct_data<float>(src1.total_byte_size() / src0.type_byte_size(), feed_num, src1.data());

    for (auto _state : state) {
        matrix_mul(src0,src1);
    }
}

BENCHMARK_DEFINE_F(MatrixMulBench, Mul2Matrix_1080p)(benchmark::State& state) {
    Mat src0 = Mat(1920, 1080, FCVImageType::GRAY_F32);
    construct_data<float>(src0.total_byte_size() / src0.type_byte_size(), feed_num, src0.data());
    Mat src1 = Mat(1080, 1920, FCVImageType::GRAY_F32);
    construct_data<float>(src1.total_byte_size() / src1.type_byte_size(), feed_num, src1.data());

    for (auto _state : state) {
        matrix_mul(src0,src1);
    }
}

// BENCHMARK_DEFINE_F(MatrixMulBench, Mul2Matrix_4K)(benchmark::State& state) {
//     Mat src0 = Mat(4032, 3024, FCVImageType::GRAY_F32);
//     construct_data<float>(src0.total_byte_size() / src0.type_byte_size(), feed_num, src0.data());
//     Mat src1 = Mat(3024, 4032, FCVImageType::GRAY_F32);
//     construct_data<float>(src1.total_byte_size() / src1.type_byte_size(), feed_num, src1.data());

//     for (auto _state : state) {
//         matrix_mul(src0,src1);
//     }
// }

BENCHMARK_REGISTER_F(MatrixMulBench, Mul2Matrix_720p)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(10)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MatrixMulBench, Mul2Matrix_1080p)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(10)
        ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(MatrixMulBench, Mul2Matrix_4K)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(10)
//         ->DenseRange(55, 255, 200);