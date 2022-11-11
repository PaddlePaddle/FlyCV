// Copyright (c) 2022 FlyCV Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <jni.h>
#include "flycv.h"
#include "utils/log_utils.h"
#include "utils/test_util.h"
#include "utils/time_utils.h"

using namespace g_fcv_ns;

extern "C"
JNIEXPORT void JNICALL
Java_com_org_flycv_samples_imagetransform_ImageTramsform_nativeCvColor(JNIEnv *env, jclass thiz) {

    int loop = 1;
    bool isimwrite = true;

    {
        int status = 0;
        Mat i420_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::I420);
        status = read_binary_file(I420_1280X720_U8_BIN, i420_src.data(),
                                  i420_src.width() * i420_src.height() * i420_src.channels() / 2);

        LOGE("%d %d %d", status, i420_src.width(), i420_src.height());

        for (int index = 0; index < loop; index++) {
            Mat dst;
            TIME_START(I4202PA_BGR)
            unsigned char *srcy_data = (unsigned char *) i420_src.data();
            unsigned char *srcu_data = srcy_data + IMG_720P_WIDTH * IMG_720P_HEIGHT;
            unsigned char *srcv_data = srcu_data + (IMG_720P_WIDTH * IMG_720P_HEIGHT / 4);
            Mat src_y(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_U8, srcy_data);
            Mat src_v(IMG_720P_WIDTH, IMG_720P_HEIGHT / 4, FCVImageType::GRAY_U8, srcv_data);
            Mat src_u(IMG_720P_WIDTH, IMG_720P_HEIGHT / 4, FCVImageType::GRAY_U8, srcu_data);

            status = cvt_color(src_y, src_u, src_v, dst, ColorConvertType::CVT_I4202PA_BGR);
            TIME_END(I4202PA_BGR)
            LOGE("%d %d %d %d", status, dst.width(), dst.height(), dst.type());
            if (isimwrite) {
                std::string output_path = "/sdcard/flycv/dst/0000.png";
                fcv::imwrite(output_path, dst);
            }
        }
    }

    {
        Mat nv21_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::NV21);
        int status = read_binary_file(NV21_1280X720_U8_BIN, nv21_src.data(),
                                      nv21_src.width() * nv21_src.height() * nv21_src.channels() /
                                      2);
        LOGE("%d %d %d %d", status, nv21_src.width(), nv21_src.height(), nv21_src.type());

        for (int index = 0; index < loop; index++) {
            Mat dst;
            TIME_START(NV212PA_BGR)
            status = cvt_color(nv21_src, dst, ColorConvertType::CVT_NV212PA_BGR);
            TIME_END(NV212PA_BGR)
            LOGE("%d %d %d %d", status, dst.width(), dst.height(), dst.type());

            if (isimwrite) {
                std::string output_path = "/sdcard/flycv/dst/0001.png";
                fcv::imwrite(output_path, dst);
            }
        }
    }

    {
        Mat nv12_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::NV12);
        int status = read_binary_file(NV12_1280X720_U8_BIN, nv12_src.data(),
                                      nv12_src.width() * nv12_src.height() * nv12_src.channels() /
                                      2);
        LOGE("%d %d %d %d", status, nv12_src.width(), nv12_src.height(), nv12_src.type());

        for (int index = 0; index < loop; index++) {
            Mat dst;
            TIME_START(NV122PA_BGR)
            status = cvt_color(nv12_src, dst, ColorConvertType::CVT_NV122PA_BGR);
            TIME_END(NV122PA_BGR)
            LOGE("%d %d %d %d", status, dst.width(), dst.height(), dst.type());

            if (isimwrite) {
                std::string output_path = "/sdcard/flycv/dst/0002.png";
                fcv::imwrite(output_path, dst);
            }
        }
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_org_flycv_samples_imagetransform_ImageTramsform_nativeCvColorCL(JNIEnv *env, jclass thiz) {

    // ocl::setUseOpenCL(true);

    int loop = 1;
    bool isimwrite = true;

    {
        int status = 0;
        Mat i420_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::I420);
        status = read_binary_file(I420_1280X720_U8_BIN, i420_src.data(),
                                  i420_src.width() * i420_src.height() * i420_src.channels() / 2);

        LOGE("%d %d %d", status, i420_src.width(), i420_src.height());

        for (int index = 0; index < loop; index++) {
            Mat dst;
            TIME_START(I4202PA_BGR)
            status = cvt_color(i420_src, dst, ColorConvertType::CVT_I4202PA_BGR);
            TIME_END(I4202PA_BGR)
            LOGE("%d %d %d %d", status, dst.width(), dst.height(), dst.type());
            if (isimwrite) {
                std::string output_path = "/sdcard/flycv/dst/0010.png";
                fcv::imwrite(output_path, dst);
            }
        }
    }

    {
        Mat nv21_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::NV21);
        int status = read_binary_file(NV21_1280X720_U8_BIN, nv21_src.data(),
                                      nv21_src.width() * nv21_src.height() * nv21_src.channels() /
                                      2);
        LOGE("%d %d %d %d", status, nv21_src.width(), nv21_src.height(), nv21_src.type());

        for (int index = 0; index < loop; index++) {
            Mat dst;
            TIME_START(NV212PA_BGR)
            status = cvt_color(nv21_src, dst, ColorConvertType::CVT_NV212PA_BGR);
            TIME_END(NV212PA_BGR)
            LOGE("%d %d %d %d", status, dst.width(), dst.height(), dst.type());

            if (isimwrite) {
                std::string output_path = "/sdcard/flycv/dst/0011.png";
                fcv::imwrite(output_path, dst);
            }
        }
    }

    {
        Mat nv12_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::NV12);
        int status = read_binary_file(NV12_1280X720_U8_BIN, nv12_src.data(),
                                      nv12_src.width() * nv12_src.height() * nv12_src.channels() /
                                      2);
        LOGE("%d %d %d %d", status, nv12_src.width(), nv12_src.height(), nv12_src.type());

        for (int index = 0; index < loop; index++) {
            Mat dst;
            TIME_START(NV122PA_BGR)
            status = cvt_color(nv12_src, dst, ColorConvertType::CVT_NV122PA_BGR);
            TIME_END(NV122PA_BGR)
            LOGE("%d %d %d %d", status, dst.width(), dst.height(), dst.type());

            if (isimwrite) {
                std::string output_path = "/sdcard/flycv/dst/0012.png";
                fcv::imwrite(output_path, dst);
            }
        }
    }

    // ocl::setUseOpenCL(false);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_org_flycv_samples_imagetransform_ImageTramsform_nativeResize(JNIEnv *env, jclass thiz) {

    int loop = 1;
    bool isimwrite = true;

    Mat src;
    Mat dst;
    int src_width = IMG_720P_WIDTH;
    int src_height = IMG_720P_HEIGHT;
    int dst_width = IMG_720P_WIDTH >> 1;
    int dst_height = IMG_720P_HEIGHT >> 1;
    src = Mat(src_width, src_height, FCVImageType::PKG_BGR_U8);
    read_binary_file(BGR_1280X720_U8_BIN, src.data(),
                     src.width() * src.height() * src.channels());

    for (int index = 0; index < loop; index++) {
        TIME_START(RESIZECL)
        Mat dst;
        dst = Mat(dst_width, dst_height, FCVImageType::PKG_BGR_U8);
        fcv::Size dst_size(dst_width, dst_height);
        fcv::resize(src, dst, dst_size, 0, 0, fcv::InterpolationType::INTER_LINEAR);
        TIME_END(RESIZECL)
        if (isimwrite) {
            std::string img_name = std::to_string(index);
            int zero_size = img_name.size();
            for (int j = zero_size; j < 5; j++) {
                img_name = "0" + img_name;
            }
            img_name += ".png";

            std::string output_path = "/sdcard/flycv/dst/" + img_name;
            fcv::imwrite(output_path, dst);
        }
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_org_flycv_samples_imagetransform_ImageTramsform_nativeResizeCL(JNIEnv *env, jclass thiz) {

    // ocl::setUseOpenCL(true);

    int loop = 1;
    bool isimwrite = true;

    Mat src;
    int src_width = IMG_720P_WIDTH;
    int src_height = IMG_720P_HEIGHT;
    int dst_width = IMG_720P_WIDTH >> 1;
    int dst_height = IMG_720P_HEIGHT >> 1;
    src = Mat(src_width, src_height, FCVImageType::PKG_BGR_U8);
    read_binary_file(BGR_1280X720_U8_BIN, src.data(),
                     src.width() * src.height() * src.channels());

    for (int index = 0; index < loop; index++) {
        TIME_START(RESIZECL)
        Mat dst;
        dst = Mat(dst_width, dst_height, FCVImageType::PKG_BGR_U8);
        fcv::Size dst_size(dst_width, dst_height);
        int status = fcv::resize(src, dst, dst_size, 0, 0, fcv::InterpolationType::INTER_LINEAR);
        TIME_END(RESIZECL)
        if (isimwrite) {
            std::string img_name = std::to_string(index);
            int zero_size = img_name.size();
            for (int j = zero_size; j < 5; j++) {
                img_name = "1" + img_name;
            }
            img_name += ".png";

            std::string output_path = "/sdcard/flycv/dst/" + img_name;
            fcv::imwrite(output_path, dst);
        }
    }

    // ocl::setUseOpenCL(false);
}