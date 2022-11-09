/***************************************************************************
 *
 * Copyright (c) 2022 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @contributor     litonghui
 * @created         2022-10-27 15:37
 */

#include "benchmark/benchmark.h"
#include "common/utils.h"
#include "flycv.h"

using namespace g_fcv_ns;

class RotationBench : public benchmark::Fixture {
public :
    void SetUp(const ::benchmark::State& state){
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(RotationBench, Transpose_C1_U8_720P)(benchmark::State& state){
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C3_U8_720P)(benchmark::State& state){
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C4_U8_720P)(benchmark::State& state){
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C1_F32_720P)(benchmark::State& state){
    Mat src = Mat(1280, 720, FCVImageType::GRAY_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C3_F32_720P)(benchmark::State& state){
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C4_F32_720P)(benchmark::State& state){
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGRA_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_REGISTER_F(RotationBench, Transpose_C1_U8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C3_U8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C4_U8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C1_F32_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C3_F32_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C4_F32_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_DEFINE_F(RotationBench, Transpose_C1_U8_1080P)(benchmark::State& state){
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C3_U8_1080P)(benchmark::State& state){
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C4_U8_1080P)(benchmark::State& state){
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C1_F32_1080P)(benchmark::State& state){
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C3_F32_1080P)(benchmark::State& state){
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C4_F32_1080P)(benchmark::State& state){
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGRA_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_REGISTER_F(RotationBench, Transpose_C1_U8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C3_U8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C4_U8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C1_F32_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C3_F32_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C4_F32_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_DEFINE_F(RotationBench, Transpose_C1_U8_4K)(benchmark::State& state){
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C3_U8_4K)(benchmark::State& state){
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C4_U8_4K)(benchmark::State& state){
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C1_F32_4K)(benchmark::State& state){
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C3_F32_4K)(benchmark::State& state){
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_DEFINE_F(RotationBench, Transpose_C4_F32_4K)(benchmark::State& state){
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGRA_F32);
    construct_data<float>(src.total_byte_size() / src.type_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        transpose(src, dst);
    }
}

BENCHMARK_REGISTER_F(RotationBench, Transpose_C1_U8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C3_U8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C4_U8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C1_F32_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C3_F32_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(RotationBench, Transpose_C4_F32_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);                