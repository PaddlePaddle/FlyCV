#include <assert.h>

#include <iostream>

#include "flycv.h"
#include "gflags/gflags.h"
#include "utils/test_util.h"

using namespace g_fcv_ns;

DEFINE_string(input_file, "../../bin/" + GRAY_1280X720_U8_BIN, "input image bin file path, generate Mat");
DEFINE_int32(input_width, IMG_720P_WIDTH, "input image width");
DEFINE_int32(input_height, IMG_720P_HEIGHT, "input image height");

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    assert(!FLAGS_input_file.empty());

    Mat img_src(FLAGS_input_width, FLAGS_input_height, FCVImageType::GRAY_U8);
    int status = read_binary_file(FLAGS_input_file, img_src.data(), img_src.total_byte_size());
    if (status != 0) {
        std::cout << "Failed to read file: " << FLAGS_input_file << std::endl;
        return -1;
    }

    Mat img_dst;

    double min_val = -1;
    double max_val = -1;
    Point min_loc(-1, -1);
    Point max_loc(-1, -1);

    status = min_max_loc(img_src, &min_val, &max_val, &min_loc, &max_loc);
    if (status != 0) {
        std::cout << "Failed to transform" << std::endl;
        return -1;
    }

    printf("%d %d:%f %d %d:%f \n", min_loc.x(), min_loc.y(), min_val, max_loc.x(), max_loc.y(), max_val);

    return 0;
}
