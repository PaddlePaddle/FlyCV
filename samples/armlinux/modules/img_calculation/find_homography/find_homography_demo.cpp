#include <assert.h>

#include <iostream>

#include "flycv.h"
#include "gflags/gflags.h"
#include "utils/test_util.h"

using namespace g_fcv_ns;

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    std::vector<Point2f> src_points;
    src_points.push_back(Point2f(22, 1));
    src_points.push_back(Point2f(-111.1, 222.1));
    src_points.push_back(Point2f(0, -222.1));
    src_points.push_back(Point2f(161.1, 332.1));

    std::vector<Point2f> dst_points;
    dst_points.push_back(Point2f(1.11, 1));
    dst_points.push_back(Point2f(480 - 1, 1));
    dst_points.push_back(Point2f(480 - 5, 1));
    dst_points.push_back(Point2f(1, 640 - 1));

    // creat Mat
    Mat dst = find_homography(src_points, dst_points);
    double* dst_ptr = (double*)dst.data();

    for (int i = 0; i < dst.width() * dst.height() * dst.channels(); ++i) {
        printf("%f ", dst_ptr[i]);
    }

    return 0;
}
