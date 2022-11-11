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
    for (int i = 0; i < pointsNum; i++) {
        points[i] = Point(rand() % 3000, rand() % 3000);
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
    for (int i = 0; i < pointsNum; i++) {
        points[i] = Point(rand() % 3000, rand() % 3000);
    }
    
    for (auto _state : state) {
        dst = min_area_rect(points);
    }
}

BENCHMARK_DEFINE_F(MinAreaRectBench, MinRectof5000)
        (benchmark::State &state) {
    int pointsNum = 500;
    std::vector<Point> points(pointsNum);
    RotatedRect dst;
    for (int i = 0; i < pointsNum; i++) {
        points[i] = Point(rand() % 3000, rand() % 3000);
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
