#include "flycv.h"
#include "utils/test_util.h"
#include <iostream>

using namespace g_fcv_ns;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr
            << "usage: ./" << argv[0] << "\n"
            << "  <input_file_dir>\n"
            << "  <output_file_dir>\n"
            << std::endl;
        return 0;
    }

    std::string INPUT_DIR = argv[1];
    std::string OUTPUT_DIR = argv[2];
    int status = 0;
    bool isimwrite = true;

    {
        Mat src;
        int src_width = IMG_720P_WIDTH;
        int src_height = IMG_720P_HEIGHT;
        int dst_width = IMG_720P_WIDTH >> 1;
        int dst_height = IMG_720P_HEIGHT >> 1;
        src = Mat(src_width, src_height, FCVImageType::PKG_BGR_U8);
        status = read_binary_file(BGR_1280X720_U8_BIN, src.data(),
                        src.width() * src.height() * src.channels());
        if (status != 0) {
            std::cout << "Failed to read file: " << INPUT_DIR + I420_1280X720_U8_BIN << std::endl;
            return -1;
        }

        Mat l_dst = Mat(dst_width, dst_height, FCVImageType::PKG_BGR_U8);
        fcv::Size dst_size(dst_width, dst_height);
        fcv::resize(src, l_dst, dst_size, 0, 0, fcv::InterpolationType::INTER_LINEAR);
        if (isimwrite) {
            std::string output_path = OUTPUT_DIR + "LINEAR_REDUCE_2X.png";
            fcv::imwrite(output_path, l_dst);
        }

        Mat a_dst = Mat(dst_width, dst_height, FCVImageType::PKG_BGR_U8);
        fcv::resize(src, a_dst, dst_size, 0, 0, fcv::InterpolationType::INTER_AREA);
        if (isimwrite) {
            std::string output_path = OUTPUT_DIR + "AREA_REDUCE_2X.png";
            fcv::imwrite(output_path, a_dst);
        }
    }
    
    return 0;
}
