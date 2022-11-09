

/*
 * @Author             : tianhao08
 * @Brief                :
 * @date                 : 2021-12-06 16:08:36
 * Copyright (c) 2021 Baidu.com, Inc. All Rights Reserved
 */


/*
 * @Author             : tianhao08
 * @Brief                :
 * @date                 : 2021-12-14 14:27:41
 * Copyright (c) 2021 Baidu.com, Inc. All Rights Reserved
 */

#include <array>
#include <iomanip>
#include <iostream>
#include <random>
#include <memory>

#include "benchmark/benchmark.h"
#include "flycv.h"

constexpr uint32_t MIN_DIMS = 128;
constexpr uint32_t MAX_DIMS = 512;
constexpr uint32_t DIM_STEP = 128;

constexpr uint32_t MIN_ROWS = 64;
constexpr uint32_t MAX_ROWS = 256;
constexpr uint32_t ROW_STEP = 64;

constexpr float FLOAT_MIN = -128.0f;
constexpr float FLOAT_MAX = 127.0f;


template <typename T>
class AiklCalFixtureVector : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) BENCHMARK_OVERRIDE {
        _vector0 = g_fcv_ns::alloc_vector<T>(state.range(0));
        _vector1 = g_fcv_ns::alloc_vector<T>(state.range(0));
    }

    AiklCalFixtureVector() {}
    ~AiklCalFixtureVector() {
        if (_vector0 != nullptr) {
            g_fcv_ns::free_vector((T*)_vector0);
            _vector0 = nullptr;
        }

        if (_vector1 != nullptr) {
            g_fcv_ns::free_vector((T*)_vector1);
            _vector1 = nullptr;
        }
        assert(_vector0 == nullptr);
        assert(_vector1 == nullptr);
    }

public:
    void* _vector0 = nullptr;
    void* _vector1 = nullptr;
};

template <typename T>
class AiklCalFixtureMatrix : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) BENCHMARK_OVERRIDE {
        _matrix = g_fcv_ns::alloc_matrix<T>(state.range(0), state.range(1));
        _vector = g_fcv_ns::alloc_vector<T>(state.range(0));
    }

    AiklCalFixtureMatrix() {}

    ~AiklCalFixtureMatrix() {
        if (_vector != nullptr) {
            g_fcv_ns::free_vector((T*)_vector);
            _vector = nullptr;
        }
            
        if (_matrix != nullptr) {
            g_fcv_ns::free_matrix((T*)_matrix);
            _matrix = nullptr;
        }
        assert(_matrix == nullptr);
        assert(_vector == nullptr);
    }

public:
    void* _matrix = nullptr;
    void* _vector = nullptr;
};

BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureVector, GetL2, float)
        (benchmark::State& state) {
    for (auto _ : state) {
        g_fcv_ns::get_l2(state.range(0), (float*)_vector0);
    }
}

// GetL2
// dense range
// BENCHMARK(GetL2)->DenseRange(MIN_DIMS, MAX_DIMS, DIM_STEP);
BENCHMARK_REGISTER_F(AiklCalFixtureVector, GetL2)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP)})
    ->Unit(benchmark::kMicrosecond);

// sparse range
// BENCHMARK(GetL2)->RangeMultiplier(2)->Range(MIN_DIMS, MAX_DIMS);
// BENCHMARK_REGISTER_F(AiklCalFixture, GetL2)
//         ->ArgsProduct({benchmark::CreateRange(MIN_DIMS, MAX_DIMS, /*multi=*/2)})
//         ->Unit(benchmark::kMicrosecond);

// NormalizeVector
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureVector, NormalizeVector, float)
        (benchmark::State& state) {
    for (auto _ : state) {
        g_fcv_ns::normalize_vector(state.range(0), (float*)_vector0);
    }
}

BENCHMARK_REGISTER_F(AiklCalFixtureVector, NormalizeVector)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP)})
    ->Unit(benchmark::kMicrosecond);

// normalize_matrix
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureMatrix, NormalizeMatrix, float)
(benchmark::State& state) {
    for (auto _ : state) {
        g_fcv_ns::normalize_matrix(state.range(0), state.range(1), (float*)_matrix);
    }
}

BENCHMARK_REGISTER_F(AiklCalFixtureMatrix, NormalizeMatrix)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP),
        benchmark::CreateDenseRange(MIN_ROWS, MAX_ROWS, /*multi=*/ROW_STEP)})
    ->Unit(benchmark::kMicrosecond);

// dot_vectors f32
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureVector, DotVectorF32, float)
(benchmark::State& state) {
    for (auto _ : state) {
        float dot_res = 0.f;
        g_fcv_ns::dot_vectors(state.range(0), (float*)_vector0,
                (float*)_vector1, &dot_res);
    }
}

BENCHMARK_REGISTER_F(AiklCalFixtureVector, DotVectorF32)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP)})
    ->Unit(benchmark::kMicrosecond);

// dot_vectors s8
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureVector, DotVectorS8, int8_t)
(benchmark::State& state) {
    for (auto _ : state) {
        int32_t dot_res = 0;
        g_fcv_ns::dot_vectors(state.range(0), (int8_t*)_vector0,
                (int8_t*)_vector1, &dot_res);
    }
}

BENCHMARK_REGISTER_F(AiklCalFixtureVector, DotVectorS8)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP)})
    ->Unit(benchmark::kMicrosecond);

// dot_matrix_vector f32
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureMatrix, DotMatrixVectorF32, float)
(benchmark::State& state) {
    for (auto _ : state) {
        float* dot_res = new float[state.range(1)];
        g_fcv_ns::dot_matrix_vector(state.range(0), state.range(1),
                (float*)_matrix, (float*)_vector, dot_res);
		delete[] dot_res;
    }
}

BENCHMARK_REGISTER_F(AiklCalFixtureMatrix, DotMatrixVectorF32)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP),
            benchmark::CreateDenseRange(MIN_ROWS, MAX_ROWS, /*multi=*/ROW_STEP)})
    ->Unit(benchmark::kMicrosecond);

// dot_matrix_vector s8
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureMatrix, DotMatrixVectorS8, int8_t)
(benchmark::State& state) {
    for (auto _ : state) {
        int32_t* dot_res = new int32_t[state.range(1)];
        g_fcv_ns::dot_matrix_vector(state.range(0), state.range(1),
                (int8_t*)_matrix, (int8_t*)_vector, dot_res);
		delete[] dot_res;
    }
}

BENCHMARK_REGISTER_F(AiklCalFixtureMatrix, DotMatrixVectorS8)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP),
            benchmark::CreateDenseRange(MIN_ROWS, MAX_ROWS, /*multi=*/ROW_STEP)})
    ->Unit(benchmark::kMicrosecond);

// put_vector f32
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureMatrix, PutVectorF32, float)
(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i = 0; i < state.range(1); i++) {
            g_fcv_ns::put_vector(state.range(0), i, (float*)_matrix, (float*)_vector);
        }
    }
    state.counters["AvgOfRow"] = benchmark::Counter(state.iterations() *
            state.range(1), benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)) *
            int64_t(state.range(1)) * sizeof(float));
}

BENCHMARK_REGISTER_F(AiklCalFixtureMatrix, PutVectorF32)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP),
            benchmark::CreateDenseRange(MIN_ROWS, MAX_ROWS, /*multi=*/ROW_STEP)})
    ->Unit(benchmark::kMicrosecond);

// put_vector s8
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureMatrix, PutVectorS8, int8_t)
(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i = 0; i < state.range(1); i++) {
            g_fcv_ns::put_vector(state.range(0), i, (int8_t*)_matrix, (int8_t*)_vector);
        }
    }
    state.counters["AvgOfRow"] = benchmark::Counter(state.iterations() *
            state.range(1), benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)) *
            int64_t(state.range(1)) * sizeof(int8_t));
}

BENCHMARK_REGISTER_F(AiklCalFixtureMatrix, PutVectorS8)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP),
            benchmark::CreateDenseRange(MIN_ROWS, MAX_ROWS, /*multi=*/ROW_STEP)})
    ->Unit(benchmark::kMicrosecond);

// get_vector f32
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureMatrix, GetVectorF32, float)
(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i = 0; i < state.range(1); i++) {
            g_fcv_ns::get_vector(state.range(0), i, (float*)_matrix, (float*)_vector);
        }
    }
    state.counters["AvgOfRow"] = benchmark::Counter(state.iterations() *
            state.range(1), benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)) *
            int64_t(state.range(1)) * sizeof(float));
}

BENCHMARK_REGISTER_F(AiklCalFixtureMatrix, GetVectorF32)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP),
            benchmark::CreateDenseRange(MIN_ROWS, MAX_ROWS, /*multi=*/ROW_STEP)})
    ->Unit(benchmark::kMicrosecond);

// get_vector s8
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureMatrix, GetVectorS8, int8_t)
(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i = 0; i < state.range(1); i++) {
            g_fcv_ns::get_vector(state.range(0), i, (int8_t*)_matrix, (int8_t*)_vector);
        }
    }
    state.counters["AvgOfRow"] = benchmark::Counter(state.iterations() *
            state.range(1), benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)) *
            int64_t(state.range(1)) * sizeof(int8_t));
}

BENCHMARK_REGISTER_F(AiklCalFixtureMatrix, GetVectorS8)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP),
            benchmark::CreateDenseRange(MIN_ROWS, MAX_ROWS, /*multi=*/ROW_STEP)})
    ->Unit(benchmark::kMicrosecond);

// axpy_vector f32
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureVector, AxpyVectorF32, float)
(benchmark::State& state) {
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> distr(FLOAT_MIN, FLOAT_MAX);
    float* result = new float[state.range(0)];
    for (auto _ : state) {
        g_fcv_ns::axpy_vector(state.range(0), distr(eng), (float*)_vector0, result);
    }
	delete[] result;
}

BENCHMARK_REGISTER_F(AiklCalFixtureVector, AxpyVectorF32)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP),
            benchmark::CreateDenseRange(MIN_ROWS, MAX_ROWS, /*multi=*/ROW_STEP)})
    ->Unit(benchmark::kMicrosecond);

// axpy_vector s8
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureVector, AxpyVectorS8, int8_t)
(benchmark::State& state) {
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> distr(FLOAT_MIN, FLOAT_MAX);
    float* result = new float[state.range(0)];
    for (auto _ : state) {
        g_fcv_ns::axpy_vector(state.range(0), distr(eng), (int8_t*)_vector0, result);
    }
	delete[] result;
}

BENCHMARK_REGISTER_F(AiklCalFixtureVector, AxpyVectorS8)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP),
            benchmark::CreateDenseRange(MIN_ROWS, MAX_ROWS, /*multi=*/ROW_STEP)})
    ->Unit(benchmark::kMicrosecond);

// product_vector
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureVector, ProductVectorF32, float)
(benchmark::State& state) {
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> distr(FLOAT_MIN, FLOAT_MAX);
    for (auto _ : state) {
        g_fcv_ns::product_vector(state.range(0), distr(eng), (float*)_vector0);
    }
}

BENCHMARK_REGISTER_F(AiklCalFixtureVector, ProductVectorF32)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP),
            benchmark::CreateDenseRange(MIN_ROWS, MAX_ROWS, /*multi=*/ROW_STEP)})
    ->Unit(benchmark::kMicrosecond);

// product_center
BENCHMARK_TEMPLATE_DEFINE_F(AiklCalFixtureMatrix, ProductCenterF32, float)
(benchmark::State& state) {
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> distr(FLOAT_MIN, FLOAT_MAX);
    float* center = new float[state.range(0)];
    for (auto _ : state) {
        g_fcv_ns::product_center(state.range(0), state.range(1),
                (float*) _matrix, distr(eng), center);
    }
	delete[] center;
}

BENCHMARK_REGISTER_F(AiklCalFixtureMatrix, ProductCenterF32)
    ->ArgsProduct({benchmark::CreateDenseRange(MIN_DIMS, MAX_DIMS, /*multi=*/DIM_STEP),
            benchmark::CreateDenseRange(MIN_ROWS, MAX_ROWS, /*multi=*/ROW_STEP)})
    ->Unit(benchmark::kMicrosecond);
