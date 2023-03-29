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

    Mat img_src1(FLAGS_input_width, FLAGS_input_height, FCVImageType::PKG_BGR_U8);
    int status = read_binary_file(
            FLAGS_input_file, img_src1.data(), img_src1.width() * img_src1.height() * img_src1.channels());
    if (status != 0) {
        std::cout << "Failed to read file: " << FLAGS_input_file << std::endl;
        return -1;
    }

    Mat img_src2(FLAGS_input_width, FLAGS_input_height, FCVImageType::PKG_BGR_U8);
    status = read_binary_file(
            FLAGS_input_file, img_src2.data(), img_src2.width() * img_src2.height() * img_src2.channels());
    if (status != 0) {
        std::cout << "Failed to read file: " << FLAGS_input_file << std::endl;
        return -1;
    }

    Mat img_dst;

    double alpha = 0.5;
    double beta = 0.5;
    double gama = 20;

    status = add_weighted(img_src1, alpha, img_src2, beta, gama, img_dst);
    if (status != 0) {
        std::cout << "Failed to transform" << std::endl;
        return -1;
    }

    if (!FLAGS_output_file.empty()) {
        imwrite(FLAGS_output_file, img_dst);
    }

    return 0;
}
