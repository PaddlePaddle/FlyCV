// Copyright (c) 2023 FlyCV Authors. All Rights Reserved.
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

class BoxPointsBench : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        set_thread_num(G_THREAD_NUM);
    }
};

BENCHMARK_DEFINE_F(BoxPointsBench, BoxPoints)
        (benchmark::State& state) {
    RotatedRect rect(50, 60, 300, 200, 50);

    for (auto _state : state) {
        Mat points;
        box_points(rect, points);
    }
};

BENCHMARK_REGISTER_F(BoxPointsBench, BoxPoints)
        ->Unit(benchmark::kMicrosecond)
        ->Iterations(1000)
        ->DenseRange(55, 255, 200);