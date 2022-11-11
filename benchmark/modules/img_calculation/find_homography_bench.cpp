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

BENCHMARK_DEFINE_F(FindHomographyBench, FindHomoIn4)(benchmark::State& state) {
    // int pointNums = 10;
    
    // std::vector<int> pointsX(pointNums);
    // std::vector<int> pointsY(pointNums);

    std::vector<Point2f> sPoints(4);
    std::vector<Point2f> dPoints(4);

    // for(int i = 0;i<pointNums;i++){
    //     pointsX[i] = rand()%4096+200;
    //     pointsY[i] = rand()%4096+200;
    // }

    // for(int i = 0;i<pointNums;i++){
    //     sPoints[i] = Point(pointsX[i],pointsY[i]);
    //     sPoints[i] = Point(pointsX[i]+rand()%10,pointsY[i]+rand()%10);
    // }

    sPoints = {
        Point(25,40),
        Point(45,90),
        Point(90,115),
        Point(103,54)
    };

    dPoints = {
        Point(27,50),
        Point(55,103),
        Point(70,65),
        Point(110,47)
    };

    set_thread_num(1);
    for (auto _state : state) {
        find_homography(sPoints,dPoints);
    }
}

BENCHMARK_DEFINE_F(FindHomographyBench, FindHomoIn9)(benchmark::State& state) {
    // int pointNums = 15;
    // std::vector<int> pointsX(pointNums);
    // std::vector<int> pointsY(pointNums);

    std::vector<Point2f> sPoints(9);
    std::vector<Point2f> dPoints(9);

    // for(int i = 0;i<pointNums;i++){
    //     pointsX[i] = rand()%4096+200;
    //     pointsY[i] = rand()%4096+200;
    // }

    // for(int i = 0;i<pointNums;i++){
    //     sPoints[i] = Point(pointsX[i],pointsY[i]);
    //     sPoints[i] = Point(pointsX[i]+rand()%10,pointsY[i]+rand()%10);
    // }

    sPoints = {
        Point(1, 1),
        Point(50, 1),
        Point(100, 1),
        Point(1, 50),
        Point(50, 50),
        Point(100, 50),
        Point(1, 100),
        Point(50, 100),
        Point(100, 100)
    };

    dPoints = {
        Point(1, 1),
        Point(75, 25),
        Point(150, 50),
        Point(25, 75),
        Point(100, 100),
        Point(175, 125),
        Point(50, 125),
        Point(125, 175),
        Point(200, 200)
    };

    set_thread_num(1);
    for (auto _state : state) {
        find_homography(sPoints,dPoints);
    }
}

BENCHMARK_DEFINE_F(FindHomographyBench, FindHomoIn16)(benchmark::State& state) {

    std::vector<Point2f> sPoints(16);
    std::vector<Point2f> dPoints(16);

    sPoints = {
        Point2f(0, 0),
        Point2f(50, 16.67),
        Point2f(100, 33.33),
        Point2f(150, 50),

        Point2f(16.67, 50),
        Point2f(116.67, 83.33),
        Point2f(166.67, 100),
        Point2f(66.67, 66.67),

        Point2f(33.33, 100),
        Point2f(83.33, 116.67),
        Point2f(133.33, 133.33),
        Point2f(183.33, 150),

        Point2f(50, 150),
        Point2f(100, 166.67),
        Point2f(150, 183.33),
        Point2f(200, 200),
    };

    dPoints = {
        Point2f(0,0),
        Point2f(33.3, 0),
        Point2f(66.7, 0),
        Point2f(100, 0),

        Point2f(0, 33.3),
        Point2f(33.3, 33.3),
        Point2f(66.7, 33.3),
        Point2f(100, 33.3),

        Point2f(0, 66.7),
        Point2f(33.3, 66.7),
        Point2f(66.7, 66.7),
        Point2f(100, 66.7),

        Point2f(0, 100),
        Point2f(33.3, 100),
        Point2f(66.7, 100),
        Point2f(100, 100),
    };

    set_thread_num(1);
    for (auto _state : state) {
        find_homography(sPoints, dPoints);
    }
}

BENCHMARK_DEFINE_F(FindHomographyBench, FindHomoIn49)(benchmark::State& state) {
    std::vector<Point2f> sPoints(49);
    std::vector<Point2f> dPoints(49);

    sPoints = {
        Point2f(1, 1),
        Point2f(2, 1),
        Point2f(3, 1),
        Point2f(4, 1),
        Point2f(5, 1),
        Point2f(6, 1),
        Point2f(7, 1),
        
        Point2f(1, 2),
        Point2f(2, 2),
        Point2f(3, 2),
        Point2f(4, 2),
        Point2f(5, 2),
        Point2f(6, 2),
        Point2f(7, 2),

        Point2f(1, 3),
        Point2f(2, 3),
        Point2f(3, 3),
        Point2f(4, 3),
        Point2f(5, 3),
        Point2f(6, 3),
        Point2f(7, 3),

        Point2f(1, 4),
        Point2f(2, 4),
        Point2f(3, 4),
        Point2f(4, 4),
        Point2f(5, 4),
        Point2f(6, 4),
        Point2f(7, 4),

        Point2f(1, 5),
        Point2f(2, 5),
        Point2f(3, 5),
        Point2f(4, 5),
        Point2f(5, 5),
        Point2f(6, 5),
        Point2f(7, 5),

        Point2f(1, 6),
        Point2f(2, 6),
        Point2f(3, 6),
        Point2f(4, 6),
        Point2f(5, 6),
        Point2f(6, 6),
        Point2f(7, 6),

        Point2f(1, 7),
        Point2f(2, 7),
        Point2f(3, 7),
        Point2f(4, 7),
        Point2f(5, 7),
        Point2f(6, 7),
        Point2f(7, 7), 
    };

    dPoints = {
        Point2f(1, 1 * 1.142857),
        Point2f(2, 1 * 1.285714),
        Point2f(3, 1 * 1.428571),
        Point2f(4, 1 * 1.571428),
        Point2f(5, 1 * 1.714285),
        Point2f(6, 1 * 1.857142),
        Point2f(7, 1 * 2),
        
        Point2f(1, 2 * 1.142857),
        Point2f(2, 2 * 1.285714),
        Point2f(3, 2 * 1.428571),
        Point2f(4, 2 * 1.571428),
        Point2f(5, 2 * 1.714285),
        Point2f(6, 2 * 1.857142),
        Point2f(7, 2 * 2),

        Point2f(1, 3 * 1.142857),
        Point2f(2, 3 * 1.285714),
        Point2f(3, 3 * 1.428571),
        Point2f(4, 3 * 1.571428),
        Point2f(5, 3 * 1.714285),
        Point2f(6, 3 * 1.857142),
        Point2f(7, 3 * 2),

        Point2f(1, 4 * 1.142857),
        Point2f(2, 4 * 1.285714),
        Point2f(3, 4 * 1.428571),
        Point2f(4, 4 * 1.571428),
        Point2f(5, 4 * 1.714285),
        Point2f(6, 4 * 1.857142),
        Point2f(7, 4 * 2),

        Point2f(1, 5 * 1.142857),
        Point2f(2, 5 * 1.285714),
        Point2f(3, 5 * 1.428571),
        Point2f(4, 5 * 1.571428),
        Point2f(5, 5 * 1.714285),
        Point2f(6, 5 * 1.857142),
        Point2f(7, 5 * 2),

        Point2f(1, 6 * 1.142857),
        Point2f(2, 6 * 1.285714),
        Point2f(3, 6 * 1.428571),
        Point2f(4, 6 * 1.571428),
        Point2f(5, 6 * 1.714285),
        Point2f(6, 6 * 1.857142),
        Point2f(7, 6 * 2),

        Point2f(1, 7 * 1.142857),
        Point2f(2, 7 * 1.285714),
        Point2f(3, 7 * 1.428571),
        Point2f(4, 7 * 1.571428),
        Point2f(5, 7 * 1.714285),
        Point2f(6, 7 * 1.857142),
        Point2f(7, 7 * 2), 
    };

    set_thread_num(1);
    for (auto _state : state) {
        find_homography(sPoints, dPoints);
    }
}

BENCHMARK_REGISTER_F(FindHomographyBench, FindHomoIn9)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(FindHomographyBench, FindHomoIn16)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

BENCHMARK_REGISTER_F(FindHomographyBench, FindHomoIn49)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(100)
        ->DenseRange(55, 255, 200);

// BENCHMARK_REGISTER_F(FindHomographyBench, FindHomoIn20)
//         ->Unit(benchmark::kMicrosecond)
//         ->Iterations(100)
//         ->DenseRange(55, 255, 200);                  

