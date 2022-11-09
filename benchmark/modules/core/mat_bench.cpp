/***************************************************************************
 *
 * Copyright (c) 2021 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @contributor     taotianran
 * @created         2022-05-23 12:44
 * @brief
 */

#include "benchmark/benchmark.h"
#include "flycv.h"

using namespace g_fcv_ns;

class MatBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        pkg_bgr_u8_720 = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
        unsigned char* pkg_bgr_u8_720_data = (unsigned char*)pkg_bgr_u8_720.data();
        
        for (size_t i = 0; i < pkg_bgr_u8_720.total_byte_size(); ++i) {
            pkg_bgr_u8_720_data[i] = i * state.range(0) % 256;
        }

        pkg_bgr_u8_1080 = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
        unsigned char* pkg_bgr_u8_1080_data = (unsigned char*)pkg_bgr_u8_1080.data();
        
        for (size_t i = 0; i < pkg_bgr_u8_1080.total_byte_size(); ++i) {
            pkg_bgr_u8_1080_data[i] = i * state.range(0) % 256;
        }

        pkg_bgr_u8_4K = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
        unsigned char* pkg_bgr_u8_4K_data = (unsigned char*)pkg_bgr_u8_4K.data();
        
        for (size_t i = 0; i < pkg_bgr_u8_4K.total_byte_size(); ++i) {
            pkg_bgr_u8_4K_data[i] = i * state.range(0) % 256;
        }
    }

public:
    Mat pkg_bgr_u8_720;
    Mat pkg_bgr_u8_1080;
    Mat pkg_bgr_u8_4K;
};

BENCHMARK_DEFINE_F(MatBench, ConstructorPkgBGRU8_720P)(benchmark::State& state) {
    for (auto _state : state) {
        Mat dst(1280, 720, FCVImageType::PKG_BGR_U8);
    }
}

BENCHMARK_DEFINE_F(MatBench, ConstructorPkgBGRU8_1080P)(benchmark::State& state) {
    for (auto _state : state) {
        Mat dst(1920, 1080, FCVImageType::PKG_BGR_U8);
    }
}

BENCHMARK_DEFINE_F(MatBench, ConstructorPkgBGRU8_4K)(benchmark::State& state) {
    for (auto _state : state) {
        Mat dst(4032, 3024, FCVImageType::PKG_BGR_U8);
    }
}

BENCHMARK_DEFINE_F(MatBench, ConvertTo_PkgBGRU8ToPkgBGRF32_720P)(
        benchmark::State& state) {
    for (auto _state : state) {
        Mat dst;
        pkg_bgr_u8_720.convert_to(dst, FCVImageType::PKG_BGR_F32, 0.5f, 10);
    }
}

BENCHMARK_DEFINE_F(MatBench, ConvertTo_PkgBGRU8ToPkgBGRF32_1080P)(
        benchmark::State& state) {
    for (auto _state : state) {
        Mat dst;
        pkg_bgr_u8_1080.convert_to(dst, FCVImageType::PKG_BGR_F32, 0.5f, 10);
    }
}

BENCHMARK_DEFINE_F(MatBench, ConvertTo_PkgBGRU8ToPkgBGRF32_4K)(
        benchmark::State& state) {
    for (auto _state : state) {
        Mat dst;
        pkg_bgr_u8_4K.convert_to(dst, FCVImageType::PKG_BGR_F32, 0.5f, 10);
    }
}

BENCHMARK_REGISTER_F(MatBench, ConstructorPkgBGRU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MatBench, ConstructorPkgBGRU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MatBench, ConstructorPkgBGRU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MatBench, ConvertTo_PkgBGRU8ToPkgBGRF32_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MatBench, ConvertTo_PkgBGRU8ToPkgBGRF32_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MatBench, ConvertTo_PkgBGRU8ToPkgBGRF32_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);
