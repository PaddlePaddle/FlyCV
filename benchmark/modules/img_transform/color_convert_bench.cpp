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

using namespace g_fcv_ns;

class ColorConvertBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(ColorConvertBench, PkgBGRU8ToGrayU8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_BGR2GRAY);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgRGBU8ToGrayU8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_RGB_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_RGB2GRAY);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgBGRU8ToPkgRGBU8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
        
    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_BGR2PA_RGB);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgRGBU8ToPkgBGRU8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_RGB_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
        
    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_RGB2PA_BGR);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgI420ToBGRU8_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::I420);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_I4202PA_BGR);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV21ToBGRU8_720P)(benchmark::State& state) {

    Mat src = Mat(1280, 720, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_NV212PA_BGR);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV12ToBGRU8_720P)(benchmark::State& state) {

    Mat src = Mat(1280, 720, FCVImageType::NV12);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_NV122PA_BGR);
    }
}

// BENCHMARK_DEFINE_F(ColorConvertBench, PkgI420ToBGRU8_OCL_720P)(benchmark::State& state) {

//     Mat src = Mat(1280, 720, FCVImageType::I420);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         cvt_color(src, dst, ColorConvertType::CVT_I4202PA_BGR);
//     }
//     ocl::setUseOpenCL(false);
// }

// BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV21ToBGRU8_OCL_720P)(benchmark::State& state) {

//     Mat src = Mat(1280, 720, FCVImageType::NV21);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         cvt_color(src, dst, ColorConvertType::CVT_NV212PA_BGR);
//     }
//     ocl::setUseOpenCL(false);
// }

// BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV12ToBGRU8_OCL_720P)(benchmark::State& state) {

//     Mat src = Mat(1280, 720, FCVImageType::NV12);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         cvt_color(src, dst, ColorConvertType::CVT_NV122PA_BGR);
//     }
//     ocl::setUseOpenCL(false);
// }

//1080

BENCHMARK_DEFINE_F(ColorConvertBench, PkgBGRU8ToGrayU8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_BGR2GRAY);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgRGBU8ToGrayU8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_RGB_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_RGB2GRAY);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgBGRU8ToPkgRGBU8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
        
    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_BGR2PA_RGB);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgRGBU8ToPkgBGRU8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_RGB_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
        
    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_RGB2PA_BGR);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgI420ToBGRU8_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::I420);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_I4202PA_BGR);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV21ToBGRU8_1080P)(benchmark::State& state) {

    Mat src = Mat(1920, 1080, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_NV212PA_BGR);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV12ToBGRU8_1080P)(benchmark::State& state) {

    Mat src = Mat(1920, 1080, FCVImageType::NV12);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_NV122PA_BGR);
    }
}

// BENCHMARK_DEFINE_F(ColorConvertBench, PkgI420ToBGRU8_OCL_1080P)(benchmark::State& state) {

//     Mat src = Mat(1920, 1080, FCVImageType::I420);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         cvt_color(src, dst, ColorConvertType::CVT_I4202PA_BGR);
//     }
//     ocl::setUseOpenCL(false);
// }

// BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV21ToBGRU8_OCL_1080P)(benchmark::State& state) {

//     Mat src = Mat(1920, 1080, FCVImageType::NV21);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         cvt_color(src, dst, ColorConvertType::CVT_NV212PA_BGR);
//     }
//     ocl::setUseOpenCL(false);
// }

// BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV12ToBGRU8_OCL_1080P)(benchmark::State& state) {

//     Mat src = Mat(1920, 1080, FCVImageType::NV12);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         cvt_color(src, dst, ColorConvertType::CVT_NV122PA_BGR);
//     }
//     ocl::setUseOpenCL(false);
//}

//4K

BENCHMARK_DEFINE_F(ColorConvertBench, PkgBGRU8ToGrayU8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_BGR2GRAY);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgRGBU8ToGrayU8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_RGB_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_RGB2GRAY);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgBGRU8ToPkgRGBU8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
        
    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_BGR2PA_RGB);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgRGBU8ToPkgBGRU8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_RGB_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());
        
    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_PA_RGB2PA_BGR);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgI420ToBGRU8_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::I420);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_I4202PA_BGR);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV21ToBGRU8_4K)(benchmark::State& state) {

    Mat src = Mat(4032, 3024, FCVImageType::NV21);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_NV212PA_BGR);
    }
}

BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV12ToBGRU8_4K)(benchmark::State& state) {

    Mat src = Mat(4032, 3024, FCVImageType::NV12);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        cvt_color(src, dst, ColorConvertType::CVT_NV122PA_BGR);
    }
}

// BENCHMARK_DEFINE_F(ColorConvertBench, PkgI420ToBGRU8_OCL_4K)(benchmark::State& state) {

//     Mat src = Mat(4032, 3024, FCVImageType::I420);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         cvt_color(src, dst, ColorConvertType::CVT_I4202PA_BGR);
//     }
//     ocl::setUseOpenCL(false);
// }

// BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV21ToBGRU8_OCL_4K)(benchmark::State& state) {

//     Mat src = Mat(4032, 3024, FCVImageType::NV21);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         cvt_color(src, dst, ColorConvertType::CVT_NV212PA_BGR);
//     }
//     ocl::setUseOpenCL(false);
// }

// BENCHMARK_DEFINE_F(ColorConvertBench, PkgNV12ToBGRU8_OCL_4K)(benchmark::State& state) {

//     Mat src = Mat(4032, 3024, FCVImageType::NV12);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         cvt_color(src, dst, ColorConvertType::CVT_NV122PA_BGR);
//     }
//     ocl::setUseOpenCL(false);
// }


BENCHMARK_REGISTER_F(ColorConvertBench, PkgBGRU8ToGrayU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgRGBU8ToGrayU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgBGRU8ToPkgRGBU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgRGBU8ToPkgBGRU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgI420ToBGRU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV21ToBGRU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV12ToBGRU8_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ColorConvertBench, PkgI420ToBGRU8_OCL_720P)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV21ToBGRU8_OCL_720P)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV12ToBGRU8_OCL_720P)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

//1080

BENCHMARK_REGISTER_F(ColorConvertBench, PkgBGRU8ToGrayU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgRGBU8ToGrayU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgBGRU8ToPkgRGBU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgRGBU8ToPkgBGRU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgI420ToBGRU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV21ToBGRU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV12ToBGRU8_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ColorConvertBench, PkgI420ToBGRU8_OCL_1080P)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV21ToBGRU8_OCL_1080P)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV12ToBGRU8_OCL_1080P)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgBGRU8ToGrayU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgRGBU8ToGrayU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgBGRU8ToPkgRGBU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgRGBU8ToPkgBGRU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgI420ToBGRU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV21ToBGRU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV12ToBGRU8_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ColorConvertBench, PkgI420ToBGRU8_OCL_4K)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV21ToBGRU8_OCL_4K)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ColorConvertBench, PkgNV12ToBGRU8_OCL_4K)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200); 