#include <assert.h>

#include <iostream>

#include "flycv.h"
#include "gflags/gflags.h"
#include "utils/test_util.h"

using namespace g_fcv_ns;

int main(int argc, char* argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    Mat src0(5, 4, FCVImageType::GRAY_F32);
    Mat src1(10, 5, FCVImageType::GRAY_F32);

    float* src0_data = (float*)src0.data();
    float* src1_data = (float*)src1.data();

    for (int i = 0; i < src0.width() * src0.height() * src0.channels(); ++i) {
        src0_data[i] = 1.0f / (i + 1);
    }

    for (int i = 0; i < src1.width() * src1.height() * src1.channels(); ++i) {
        src1_data[i] = 1.0f / (i + 1);
    }

    Mat result = matrix_mul(src0, src1);

    return 0;
}
