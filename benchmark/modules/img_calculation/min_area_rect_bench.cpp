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

class MinAreaRectBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State &state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(MinAreaRectBench, MinRectof50)
        (benchmark::State &state) {
    int pointsNum = 50;
    std::vector<Point> points(pointsNum);
    RotatedRect dst;
    
    Mat points_x = Mat(5, 1, FCVImageType::GRAY_U8);
    Mat points_y = Mat(10, 1, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(points_x.total_byte_size(), feed_num, points_x.data());
    construct_data<unsigned char>(points_y.total_byte_size(), feed_num, points_y.data());

    for (int i = 0; i < 5; i++) {
        for(int j = 0; j < 10; j++) {
            points[i * 10 + j] = Point(points_x.at<unsigned char>(i,0), points_y.at<unsigned char>(j,0));
        }
    }
    
    for (auto _state : state) {
        dst = min_area_rect(points);
    }
}

BENCHMARK_DEFINE_F(MinAreaRectBench, MinRectof500)
        (benchmark::State &state) {
    int pointsNum = 500;
    std::vector<Point> points(pointsNum);
    RotatedRect dst;

    Mat points_x = Mat(50, 1, FCVImageType::GRAY_U8);
    Mat points_y = Mat(10, 1, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(points_x.total_byte_size(), feed_num, points_x.data());
    construct_data<unsigned char>(points_y.total_byte_size(), feed_num, points_y.data());

    for (int i = 0; i < 50; i++) {
        for(int j = 0; j < 10; j++) {
            points[i * 10 + j] = Point(points_x.at<unsigned char>(i,0), points_y.at<unsigned char>(j,0));
        }
    }
    
    for (auto _state : state) {
        dst = min_area_rect(points);
    }
}

BENCHMARK_DEFINE_F(MinAreaRectBench, MinRectof5000)
        (benchmark::State &state) {
    int pointsNum = 5000;
    std::vector<Point> points(pointsNum);
    RotatedRect dst;

    Mat points_x = Mat(50, 1, FCVImageType::GRAY_U8);
    Mat points_y = Mat(100, 1, FCVImageType::GRAY_U8);
    construct_data<unsigned char>(points_x.total_byte_size(), feed_num, points_x.data());
    construct_data<unsigned char>(points_y.total_byte_size(), feed_num, points_y.data());

    for (int i = 0; i < 50; i++) {
        for(int j = 0; j < 100; j++) {
            points[i * 100 + j] = Point(points_x.at<unsigned char>(i,0), points_y.at<unsigned char>(j,0));
        }
    }
    
    for (auto _state : state) {
        dst = min_area_rect(points);
    }
}

BENCHMARK_REGISTER_F(MinAreaRectBench, MinRectof50)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(100)
    ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MinAreaRectBench, MinRectof500)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(100)
    ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(MinAreaRectBench, MinRectof5000)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(100)
    ->DenseRange(55, 255, 200);
