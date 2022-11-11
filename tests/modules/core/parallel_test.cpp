// Copyright (c) 2021 FlyCV Authors. All Rights Reserved.
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

#include "gtest/gtest.h"
#include "flycv.h"
#include "test_util.h"
#include <mutex>

using namespace g_fcv_ns;

class AccumulateTest : public ParallelTask {
public:
    AccumulateTest(const std::vector<int>& nums, std::vector<int>& result) :
            _nums(nums),
            _result(result) {}

    void operator()(const Range& range) const {
        int sum = 0;
        for (int i = range.start(); i < range.end(); ++i) {
            sum += _nums[i];
        }

        std::lock_guard<std::mutex> lock(_mutex);
        _result.push_back(sum);
    }

private:
    mutable std::mutex _mutex;
    const std::vector<int>& _nums;
    std::vector<int>& _result;
};

TEST(ParallelTest, PositiveInput) {
    int length = 10e6;
    std::vector<int> nums(length);
    std::vector<int> result;
    int sum = 0;

    for (int i = 0; i < length; ++i) {
        nums[i] = i;
        sum += nums[i];
    }

    AccumulateTest accu_test(nums, result);

    for (int i = 0; i < 1; ++i) {
        Range range(0, length);

        parallel_run(range, accu_test);

        int parallel_sum = 0;

        for (auto iter = result.begin(); iter != result.end(); ++iter) {
            parallel_sum += *iter;
        }

        ASSERT_EQ(sum, parallel_sum);
        result.clear();
    }
}
