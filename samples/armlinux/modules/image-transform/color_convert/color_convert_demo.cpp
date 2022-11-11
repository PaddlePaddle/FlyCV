#include "flycv.h"
#include "utils/test_util.h"
#include <iostream>

using namespace g_fcv_ns;

int main(int argc, char *argv[]) 
{
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
        Mat i420_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::I420);
        status = read_binary_file(INPUT_DIR + I420_1280X720_U8_BIN, i420_src.data(),
                                    i420_src.width() * i420_src.height() * i420_src.channels() / 2);
        if (status != 0) {
            std::cout << "Failed to read file: " << INPUT_DIR + I420_1280X720_U8_BIN << std::endl;
            return -1;
        }
        Mat dst;
        unsigned char *srcy_data = (unsigned char *) i420_src.data();
        unsigned char *srcu_data = srcy_data + IMG_720P_WIDTH * IMG_720P_HEIGHT;
        unsigned char *srcv_data = srcu_data + (IMG_720P_WIDTH * IMG_720P_HEIGHT / 4);
        Mat src_y(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_U8, srcy_data);
        Mat src_v(IMG_720P_WIDTH, IMG_720P_HEIGHT / 4, FCVImageType::GRAY_U8, srcv_data);
        Mat src_u(IMG_720P_WIDTH, IMG_720P_HEIGHT / 4, FCVImageType::GRAY_U8, srcu_data);

        status = cvt_color(src_y, src_u, src_v, dst, ColorConvertType::CVT_I4202PA_BGR);
        if (isimwrite) {
            std::string output_path = OUTPUT_DIR + "CVT_I4202PA_BGR.png";
            fcv::imwrite(output_path, dst);
        }
    }
    
    {
        Mat nv21_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::NV21);
        status = read_binary_file(INPUT_DIR + NV21_1280X720_U8_BIN, nv21_src.data(),
                                      nv21_src.width() * nv21_src.height() * nv21_src.channels() /
                                      2);
        if (status != 0) {
            std::cout << "Failed to read file: " << INPUT_DIR + NV21_1280X720_U8_BIN << std::endl;
            return -1;
        }
        Mat dst;
        status = cvt_color(nv21_src, dst, ColorConvertType::CVT_NV212PA_BGR);
        if (isimwrite) {
            std::string output_path = OUTPUT_DIR + "CVT_NV212PA_BGR.png";
            fcv::imwrite(output_path, dst);
        }
    }

    {
        Mat nv12_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::NV12);
        status = read_binary_file(NV12_1280X720_U8_BIN, nv12_src.data(),
                                      nv12_src.width() * nv12_src.height() * nv12_src.channels() /
                                      2);
        if (status != 0) {
            std::cout << "Failed to read file: " << INPUT_DIR + NV12_1280X720_U8_BIN << std::endl;
            return -1;
        }
        Mat dst;
        status = cvt_color(nv12_src, dst, ColorConvertType::CVT_NV122PA_BGR);
        if (isimwrite) {
            std::string output_path = OUTPUT_DIR + "CVT_NV122PA_BGR.png";
            fcv::imwrite(output_path, dst);
        }
    }

    return 0;
}