#include <assert.h>

#include <iostream>

#include "flycv.h"
#include "gflags/gflags.h"
#include "utils/test_util.h"

using namespace g_fcv_ns;

DEFINE_string(input_file, "../../bin/" + BGR_1280X720_U8_BIN, "input image bin file path, generate Mat");
DEFINE_int32(input_width, IMG_720P_WIDTH, "input image width");
DEFINE_int32(input_height, IMG_720P_HEIGHT, "input image height");
DEFINE_string(output_file, "", "output image file path");

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    assert(!FLAGS_input_file.empty());

    Mat img_src(FLAGS_input_width, FLAGS_input_height, FCVImageType::PKG_BGR_U8);
    int status =
            read_binary_file(FLAGS_input_file, img_src.data(), img_src.total_byte_size());
    if (status != 0) {
        std::cout << "Failed to read file: " << FLAGS_input_file << std::endl;
        return -1;
    }

    Mat img_dst = img_src.clone();

    Point p1(118, 301);
    Point p2(518, 301);
    Point p3(518, 600);
    Point p4(118, 600);
    Point arr_p[1][4] = {{p1, p2, p3, p4}};
    const Point* ppt[1] = {arr_p[0]};
    int arr_n[1] = {4};

    status = fill_poly(img_dst, ppt, arr_n, 1, Scalar(0, 0, 255));
    if (status != 0) {
        std::cout << "Failed to draw" << std::endl;
        return -1;
    }

    if (!FLAGS_output_file.empty()) {
        imwrite(FLAGS_output_file, img_dst);
    }

    return 0;
}
