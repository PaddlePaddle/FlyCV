#include <assert.h>

#include <iostream>

#include "flycv.h"
#include "gflags/gflags.h"
#include "utils/test_util.h"

using namespace g_fcv_ns;

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    int length = 3;
    float nums[6] = {2, 4, 6, 1, 1, 1};
    float result = get_l2(length, nums);

    return 0;
}
