// Copyright (c) 2022 FlyCV Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "benchmark/benchmark.h"
#include "common/utils.h"
#include "flycv.h"

using namespace g_fcv_ns;

class ResizeBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleA_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(600, 300), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleB_720P)(benchmark::State& state) {
    Mat src = Mat(600, 300, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(1280, 720), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_C1_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for(auto _state: state){
        Mat dst;
        resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_C3_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_C4_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_RGBA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for(auto _state: state){
        Mat dst;
        resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C1_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(320, 180), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C3_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(320, 180), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C4_720P)(benchmark::State& state) {
    Mat src = Mat(1280, 720, FCVImageType::PKG_RGBA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(320, 180), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_2X_720P)(benchmark::State& state) {
    Mat src = Mat(640, 360, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(1280, 720), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_4X_720P)(benchmark::State& state) {
    Mat src = Mat(320, 180, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(1280, 720), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

// BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_OCL_720P)(benchmark::State& state) {

//     Mat src = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
//     }
//     ocl::setUseOpenCL(false);
// }

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleA_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleB_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_C1_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_C3_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_C4_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C1_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C3_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C4_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_2X_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_4X_720P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_OCL_720P)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

// 1080

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleA_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(600, 300), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleB_1080P)(benchmark::State& state) {
    Mat src = Mat(600, 300, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(1920, 1080), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_C1_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for(auto _state: state){
        Mat dst;
        resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_C3_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_C4_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_RGBA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for(auto _state: state){
        Mat dst;
        resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C1_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(320, 180), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C3_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(320, 180), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C4_1080P)(benchmark::State& state) {
    Mat src = Mat(1920, 1080, FCVImageType::PKG_RGBA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(320, 180), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_2X_1080P)(benchmark::State& state) {
    Mat src = Mat(640, 360, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(1920, 1080), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_4X_1080P)(benchmark::State& state) {
    Mat src = Mat(320, 180, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(1920, 1080), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

// BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_OCL_1080P)(benchmark::State& state) {

//     Mat src = Mat(1920, 1080, FCVImageType::PKG_BGR_U8);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
//     }
//     ocl::setUseOpenCL(false);
// }

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleA_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleB_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_C1_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_C3_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_C4_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C1_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C3_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C4_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_2X_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_4X_1080P)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_OCL_1080P)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

//4K

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleA_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(600, 300), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleB_4K)(benchmark::State& state) {
    Mat src = Mat(600, 300, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(4032, 3024), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_C1_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for(auto _state: state){
        Mat dst;
        resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_C3_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_C4_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_RGBA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for(auto _state: state){
        Mat dst;
        resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C1_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(320, 180), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C3_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(320, 180), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C4_4K)(benchmark::State& state) {
    Mat src = Mat(4032, 3024, FCVImageType::PKG_RGBA_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(320, 180), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_2X_4K)(benchmark::State& state) {
    Mat src = Mat(640, 360, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(4032, 3024), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_4X_4K)(benchmark::State& state) {
    Mat src = Mat(320, 180, FCVImageType::PKG_BGR_U8);
    construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

    for (auto _state : state) {
        Mat dst;
        resize(src, dst, Size(4032, 3024), 0, 0, InterpolationType::INTER_LINEAR);
    }
}

// BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_Half_OCL_4K)(benchmark::State& state) {

//     Mat src = Mat(4032, 3024, FCVImageType::PKG_BGR_U8);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         resize(src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
//     }
//     ocl::setUseOpenCL(false);
// }

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleA_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_IrregularScaleB_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_C1_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_C3_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_C4_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C1_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C3_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Quarter_C4_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_2X_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_4X_4K)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_Half_OCL_4K)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

//暂时关闭8K
// BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_8K)(benchmark::State& state) {

//     Mat src = Mat(7680, 4320, FCVImageType::PKG_BGR_U8);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     for (auto _state : state) {
//         Mat dst;
//         resize(src, dst, Size(1920, 1080), 0, 0, InterpolationType::INTER_LINEAR);
//     }
// }

// BENCHMARK_DEFINE_F(ResizeBench, Resize_INTER_LINEAR_8K_OCL)(benchmark::State& state) {

//     Mat src = Mat(7680, 4320, FCVImageType::PKG_BGR_U8);
//     construct_data<unsigned char>(src.total_byte_size(), feed_num, src.data());

//     ocl::setUseOpenCL(true);
//     for (auto _state : state) {
//         Mat dst;
//         resize(src, dst, Size(1920, 1080), 0, 0, InterpolationType::INTER_LINEAR);
//     }
//     ocl::setUseOpenCL(false);
// }

// BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_8K)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(ResizeBench, Resize_INTER_LINEAR_8K_OCL)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);
