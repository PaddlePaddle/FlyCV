#include <assert.h>

#include <iostream>

#include "flycv.h"
#include "gflags/gflags.h"
#include "utils/test_util.h"

using namespace g_fcv_ns;

DEFINE_string(input_file,  "../../bin/" + BGR_1280X720_F32_BIN, "input image bin file path, generate Mat");
DEFINE_int32(input_width, IMG_720P_WIDTH, "input image width");
DEFINE_int32(input_height, IMG_720P_HEIGHT, "input image height");

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    assert(!FLAGS_input_file.empty());

    Mat img_src(FLAGS_input_width, FLAGS_input_height, FCVImageType::PKG_BGR_F32);
    int status =
            read_binary_file(FLAGS_input_file, img_src.data(), img_src.total_byte_size());
    if (status != 0) {
        std::cout << "Failed to read file: " << FLAGS_input_file << std::endl;
        return -1;
    }

    Mat img_dst;

    status = subtract(img_src, {5, 10, 15}, img_dst);
    if (status != 0) {
        std::cout << "Failed to transform" << std::endl;
        return -1;
    }

    return 0;
}
