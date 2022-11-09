/***************************************************************************
 *
 * Copyright (c) 2022 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @contributor     liuyinghan
 * @created         2022-10-20 10:09
 */

#include "benchmark/benchmark.h"
#include "common/utils.h"
#include "flycv.h"

using namespace g_fcv_ns;

class MinAreaRectBench : public benchmark::Fixture
{
public:
    void SetUp(const ::benchmark::State &state)
    {
        feed_num = state.range(0);
        set_thread_num(1);
    }

public:
    int feed_num;
};

BENCHMARK_DEFINE_F(MinAreaRectBench, MinRectof50)
(benchmark::State &state)
{
    int pointsNum = 50;
    std::vector<Point> points(pointsNum);
    RotatedRect dst;
    for (int i = 0; i < pointsNum; i++)
    {
        points[i] = Point(rand() % 3000, rand() % 3000);
    }

    
    for (auto _state : state)
    {
        dst = min_area_rect(points);
    }
}

BENCHMARK_DEFINE_F(MinAreaRectBench, MinRectof500)
(benchmark::State &state)
{
    int pointsNum = 500;
    std::vector<Point> points(pointsNum);
    RotatedRect dst;
    for (int i = 0; i < pointsNum; i++)
    {
        points[i] = Point(rand() % 3000, rand() % 3000);
    }
    
    for (auto _state : state)
    {
        dst = min_area_rect(points);
    }
}

BENCHMARK_DEFINE_F(MinAreaRectBench, MinRectof5000)
(benchmark::State &state)
{
    int pointsNum = 500;
    std::vector<Point> points(pointsNum);
    RotatedRect dst;
    for (int i = 0; i < pointsNum; i++)
    {
        points[i] = Point(rand() % 3000, rand() % 3000);
    }
    
    for (auto _state : state)
    {
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