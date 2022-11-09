/***************************************************************************
 *
 * Copyright (c) 2021 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @contributor     taotianran
 * @created         2022-05-23 13:28
 * @brief
 */

#include "benchmark/benchmark.h"
#include "common/utils.h"
#include "flycv.h"

using namespace g_fcv_ns;

class AddWeightedBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

// BENCHMARK_DEFINE_F(AddWeightedBench, GRAYU8)(benchmark::State& state) {
//     Mat src0 = Mat(1280, 720, FCVImageType::GRAY_U8);
//     construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
//     double alpha = 0.111;
//     Mat src1 = Mat(1280, 720, FCVImageType::GRAY_U8);
//     construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
//     double beta = 0.111;
//     double gamma = 0.111;
//     //Mat dst = Mat(1280, 720, FCVImageType::GRAY_U8);
//     set_thread_num(1);;
//     for (auto _state : state) {
//         Mat dst;
//         add_weighted(src0,alpha,src1,beta,gamma,dst);
//     }
// }

// BENCHMARK_DEFINE_F(AddWeightedBench, GRAYU8_720P)(benchmark::State& state) {
//     Mat src0 = Mat(1280, 720, FCVImageType::GRAY_U8);
//     construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
//     double alpha = 0.333;
//     Mat src1 = Mat(1280, 720, FCVImageType::GRAY_U8);
//     construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
//     double beta = 0.555;
//     double gamma = 0.666;
//     Mat dst = Mat(1280, 720, FCVImageType::GRAY_U8);
//     set_thread_num(1);;
//     for (auto _state : state) {
//         add_weighted(src0,alpha,src1,beta,gamma,dst);
//     }
// }

BENCHMARK_DEFINE_F(AddWeightedBench, RGBU8_720P)(benchmark::State& state) {
    Mat src0 = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
    double alpha = 0.333;
    Mat src1 = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
    double beta = 0.555;
    double gamma = 0.666;
    Mat dst = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    set_thread_num(1);;
    for (auto _state : state) {
        add_weighted(src0,alpha,src1,beta,gamma,dst);
    }
}

BENCHMARK_DEFINE_F(AddWeightedBench, RGBAU8_720P)(benchmark::State& state) {
    Mat src0 = Mat(1280, 720, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
    double alpha = 0.333;
    Mat src1 = Mat(1280, 720, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
    double beta = 0.555;
    double gamma = 0.666;
    Mat dst = Mat(1280, 720, FCVImageType::PKG_BGRA_U8);
    set_thread_num(1);;
    for (auto _state : state) {
        add_weighted(src0,alpha,src1,beta,gamma,dst);
    }
}

// BENCHMARK_DEFINE_F(AddWeightedBench, GRAYU8_1080P)(benchmark::State& state) {
//     Mat src0 = Mat(1920, 1080, FCVImageType::GRAY_U8);
//     construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
//     double alpha = 0.333;
//     Mat src1 = Mat(1920, 1080, FCVImageType::GRAY_U8);
//     construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
//     double beta = 0.555;
//     double gamma = 0.666;
//     Mat dst = Mat(1920, 1080, FCVImageType::GRAY_U8);
//     set_thread_num(1);;
//     for (auto _state : state) {
//         add_weighted(src0,alpha,src1,beta,gamma,dst);
//     }
// }

BENCHMARK_DEFINE_F(AddWeightedBench, RGBU8_1080P)(benchmark::State& state) {
    Mat src0 = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
    double alpha = 0.333;
    Mat src1 = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
    double beta = 0.555;
    double gamma = 0.666;
    Mat dst = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    set_thread_num(1);;
    for (auto _state : state) {
        add_weighted(src0,alpha,src1,beta,gamma,dst);
    }
}

BENCHMARK_DEFINE_F(AddWeightedBench, RGBAU8_1080P)(benchmark::State& state) {
    Mat src0 = Mat(1920, 1080, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
    double alpha = 0.333;
    Mat src1 = Mat(1920, 1080, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
    double beta = 0.555;
    double gamma = 0.666;
    Mat dst = Mat(1920, 1080, FCVImageType::PKG_BGRA_U8);
    set_thread_num(1);;
    for (auto _state : state) {
        add_weighted(src0,alpha,src1,beta,gamma,dst);
    }
}

// BENCHMARK_DEFINE_F(AddWeightedBench, GRAYU8_4K)(benchmark::State& state) {
//     Mat src0 = Mat(4032, 3024, FCVImageType::GRAY_U8);
//     construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
//     double alpha = 0.333;
//     Mat src1 = Mat(4032, 3024, FCVImageType::GRAY_U8);
//     construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
//     double beta = 0.555;
//     double gamma = 0.666;
//     Mat dst = Mat(4032, 3024, FCVImageType::GRAY_U8);
//     set_thread_num(1);;
//     for (auto _state : state) {
//         add_weighted(src0,alpha,src1,beta,gamma,dst);
//     }
// }

BENCHMARK_DEFINE_F(AddWeightedBench, RGBU8_4K)(benchmark::State& state) {
    Mat src0 = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
    double alpha = 0.333;
    Mat src1 = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
    double beta = 0.555;
    double gamma = 0.666;
    Mat dst = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    set_thread_num(1);;
    for (auto _state : state) {
        add_weighted(src0,alpha,src1,beta,gamma,dst);
    }
}

BENCHMARK_DEFINE_F(AddWeightedBench, RGBAU8_4K)(benchmark::State& state) {
    Mat src0 = Mat(4032, 3024, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
    double alpha = 0.333;
    Mat src1 = Mat(4032, 3024, FCVImageType::PKG_BGRA_U8);
    construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
    double beta = 0.555;
    double gamma = 0.666;
    Mat dst = Mat(4032, 3024, FCVImageType::PKG_BGRA_U8);
    set_thread_num(1);;
    for (auto _state : state) {
        add_weighted(src0,alpha,src1,beta,gamma,dst);
    }
}


// BENCHMARK_REGISTER_F(AddWeightedBench, GRAYU8)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(AddWeightedBench, GRAYU8_720P)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(AddWeightedBench, RGBU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(AddWeightedBench, RGBAU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);        

// BENCHMARK_REGISTER_F(AddWeightedBench, GRAYU8_1080P)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(AddWeightedBench, RGBU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(AddWeightedBench, RGBAU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);        

// BENCHMARK_REGISTER_F(AddWeightedBench, GRAYU8_4K)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(AddWeightedBench, RGBU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(AddWeightedBench, RGBAU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);  




// BENCHMARK_DEFINE_F(AddWeightedBench, GRAYU8)(benchmark::State& state) {
//     Mat src0 = Mat(1280, 720, FCVImageType::GRAY_U8);
//     construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
//     double alpha = 0.333;
//     Mat src1 = Mat(1280, 720, FCVImageType::GRAY_U8);
//     construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
//     double beta = 0.555;
//     double gamma = 0.666;
//     Mat dst;
//     set_thread_num(1);;
//     for (auto _state : state) {
//         add_weighted(src0,alpha,src1,beta,gamma,dst);
//     }
// }

// BENCHMARK_DEFINE_F(AddWeightedBench, RGBU8)(benchmark::State& state) {
//     Mat src0 = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
//     construct_data<unsigned char>(src0.total_byte_size(), feed_num, src0.data());
//     double alpha = 0.333;
//     Mat src1 = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
//     construct_data<unsigned char>(src1.total_byte_size(), feed_num, src1.data());
//     double beta = 0.555;
//     double gamma = 0.666;
//     Mat dst;
//     set_thread_num(1);;
//     for (auto _state : state) {
//         add_weighted(src0,alpha,src1,beta,gamma,dst);
//     }
// }

// BENCHMARK_REGISTER_F(AddWeightedBench, GRAYU8)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(AddWeightedBench, RGBU8)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);
