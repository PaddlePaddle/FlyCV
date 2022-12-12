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

class FindHomographyBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        feed_num = state.range(0);
        set_thread_num(G_THREAD_NUM);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(FindHomographyBench, FindHomoIn4_1)(benchmark::State& state) {

    std::vector<Point2f> sPoints(4);
    std::vector<Point2f> dPoints(4);

    sPoints = {
        Point(25, 40),
        Point(45, 90),
        Point(90, 115),
        Point(103, 54)
    };

    dPoints = {
        Point(27, 50),
        Point(55, 103),
        Point(70, 65),
        Point(110, 47)
    };

    for (auto _state : state) {
        find_homography(sPoints,dPoints);
    }
}

BENCHMARK_DEFINE_F(FindHomographyBench, FindHomoIn4_2)(benchmark::State& state) {

    std::vector<Point2f> sPoints(4);
    std::vector<Point2f> dPoints(4);

    sPoints = {
        Point(25, 40),
        Point(45, 90),
        Point(90, 115),
        Point(103, 54)
    };

    dPoints = {
        Point(53, 70),
        Point(100, 103),
        Point(50, 75),
        Point(120, 27)
    };

    for (auto _state : state) {
        find_homography(sPoints,dPoints);
    }
}

BENCHMARK_DEFINE_F(FindHomographyBench, FindHomoIn4_3)(benchmark::State& state) {

    std::vector<Point2f> sPoints(4);
    std::vector<Point2f> dPoints(4);

    sPoints = {
        Point(1,1),
        Point(100,1),
        Point(1,100),
        Point(100,100)
    };

    dPoints = {
        Point(70, 1),
        Point(165, 35),
        Point(5, 67),
        Point(65, 144)
    };

    for (auto _state : state) {
        find_homography(sPoints,dPoints);
    }
}

BENCHMARK_REGISTER_F(FindHomographyBench, FindHomoIn4_1)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(FindHomographyBench, FindHomoIn4_2)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(FindHomographyBench, FindHomoIn4_3)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);                       

