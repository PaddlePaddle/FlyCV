#include <assert.h>

#include <iostream>

#include "flycv.h"
#include "gflags/gflags.h"
#include "utils/test_util.h"

using namespace g_fcv_ns;

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    std::vector<Point> pts(11);
    pts[0] = {1, 1};
    pts[1] = {3, 5};
    pts[2] = {20, 100};
    pts[3] = {74, 37};
    pts[4] = {532, 77};
    pts[5] = {94, 333};
    pts[6] = {149, 633};
    pts[7] = {20, 30};
    pts[8] = {57, 22};
    pts[9] = {44, 48};
    pts[10] = {331, 232};

    RotatedRect box = min_area_rect(pts);
    printf("x: %f y: %f width: %f height: %f angle: %f \n",
           box.center_x(),
           box.center_y(),
           box.width(),
           box.height(),
           box.angle());

    return 0;
}
