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

#pragma once

#include "modules/img_calculation/matrix_mul/include/matrix_mul_ocl.h"
#include "modules/core/opencl/interface/opencl.h"
#include "modules/core/opencl/include/opencl_execute_unit.h"
#include "modules/core/opencl/include/opencl_utils.h"
#include "modules/core/opencl/include/opencl_memory_manager.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

void *command_queue_matrix_mul_;
ocl::OpenCLExecuteUnit unit_matrix_mul_[1];
long output_ptr_matrix_mul_;
uint32_t output_size_matrix_mul_;

int create_matrix_mul_unit(
        ocl::OpenCLExecuteUnit &unit,
        const std::string &kernel_name) {
    if (unit.work_group_size_max <= 0) {
        std::set <std::string> build_options;
        std::string program_name = "matrix_mul";
        return ocl::create_execute_unit(unit, program_name, kernel_name, build_options);
    } else {
        return 0;
    }
}

int set_matrix_mul_args(
        ocl::OpenCLExecuteUnit &unit,
        const Mat& src0,
        const Mat& src1,
        ocl::OpenCLBufferInfo **src0_buffer_info,
        ocl::OpenCLBufferInfo **src1_buffer_info,
        ocl::OpenCLBufferInfo **dst_buffer_info,
        uint32_t &dst_size) {
    unit.global_work_size = {
        static_cast <uint32_t>(src1.width()),
        static_cast <uint32_t>(src0.height()),
    };

    unit.local_work_size = {16, 16};

    uint32_t src_size0 = src0.width() * src0.height() * src0.channels();
    uint32_t src_size1 = src1.width() * src1.height() * src1.channels();
    dst_size = src1.width() * src0.height() * src0.channels();

    auto memory_manager = ocl::OpenCLMemoryManager::get_instance();
    *src0_buffer_info = (ocl::OpenCLBufferInfo *) memory_manager->obtain_buffer(src_size0);
    *src1_buffer_info = (ocl::OpenCLBufferInfo *) memory_manager->obtain_buffer(src_size1);
    *dst_buffer_info = (ocl::OpenCLBufferInfo *) memory_manager->obtain_buffer(dst_size);

    int cl_ret = -1;
    cl_ret = static_cast<cl::CommandQueue *>(command_queue_matrix_mul_)->enqueueWriteBuffer(
            *(*src0_buffer_info)->cl_buffer, true, 0, src_size0,
            src0.data(), nullptr, nullptr); CHECK_CL_SUCCESS(cl_ret)

    cl_ret = static_cast<cl::CommandQueue *>(command_queue_matrix_mul_)->enqueueWriteBuffer(
            *(*src1_buffer_info)->cl_buffer, true, 0, src_size1,
            src1.data(), nullptr, nullptr);

    uint32_t idx = 0;
    cl_ret = unit.ocl_kernel.setArg(idx++, *(*src0_buffer_info)->cl_buffer);
    CHECK_CL_SUCCESS(cl_ret)
    cl_ret = unit.ocl_kernel.setArg(idx++, *(*src1_buffer_info)->cl_buffer);
    CHECK_CL_SUCCESS(cl_ret)
    cl_ret = unit.ocl_kernel.setArg(idx++, *(*dst_buffer_info)->cl_buffer);
    CHECK_CL_SUCCESS(cl_ret)
    cl_ret = unit.ocl_kernel.setArg(idx++, src1.width());
    CHECK_CL_SUCCESS(cl_ret)
    return cl_ret;
}

int run_matrix_mul_unit(
        ocl::OpenCLExecuteUnit &unit,
        bool need_wait) {
    int status = ocl::run_kernel(unit.ocl_kernel, unit.global_work_size, unit.local_work_size,
            static_cast<cl::CommandQueue *>(command_queue_matrix_mul_), "matrix_mul");

    if (need_wait) {
        static_cast<cl::CommandQueue *>(command_queue_matrix_mul_)->finish();
    }

    return status;
}

int read_matrix_mul_buffer(
        ocl::OpenCLBufferInfo *src0_buffer_info,
        ocl::OpenCLBufferInfo *src1_buffer_info,
        ocl::OpenCLBufferInfo *dst_buffer_info,
        uint32_t dst_size,
        void **data) {
    if (dst_size > output_size_matrix_mul_ || output_ptr_matrix_mul_ == 0) {
        if (output_ptr_matrix_mul_ != 0) {
            free((void *) output_ptr_matrix_mul_);
        }

        output_ptr_matrix_mul_ = (long) malloc(dst_size);
        *data = (void *) output_ptr_matrix_mul_;
        output_size_matrix_mul_ = dst_size;
    } else {
        *data = (void *) output_ptr_matrix_mul_;
    }

    static_cast<cl::CommandQueue *>(command_queue_matrix_mul_)->enqueueReadBuffer(
            *dst_buffer_info->cl_buffer, true, 0, dst_size, *data, nullptr, nullptr);

    src0_buffer_info->is_busy = false;
    src1_buffer_info->is_busy = false;
    dst_buffer_info->is_busy = false;
    return 0;
}

int matrix_mul_bgr(const Mat& src0, const Mat& src1, Mat& dst){
    int status = create_matrix_mul_unit(unit_matrix_mul_[0], "matrix_multiply_f32");
    if (status != 0) {
        LOG_ERR("Create Convert Unit failed (ret = %d)!\n", (int) status);
        return status;
    }
    ocl::OpenCLBufferInfo *src0_buffer_info = nullptr;
    ocl::OpenCLBufferInfo *src1_buffer_info = nullptr;
    ocl::OpenCLBufferInfo *dst_buffer_info = nullptr;
    uint32_t dst_size;
    status = set_matrix_mul_args(unit_matrix_mul_[0], src0, src1,
            &src0_buffer_info, &src1_buffer_info, &dst_buffer_info, dst_size);

    status = run_matrix_mul_unit(unit_matrix_mul_[0], true);

    if (status != 0) {
        LOG_ERR("Run Convert Unit failed (ret = %d)!\n", (int) status);
        return status;
    }

    void *data = dst.data();

    read_matrix_mul_buffer(src0_buffer_info, src1_buffer_info,
            dst_buffer_info, dst_size, &data);
    return 0;
}

int matrix_mul_opencl(
        const Mat& src0,
        const Mat& src1,
        Mat& dst) {
    if (src0.empty() || src1.empty()) {
        LOG_ERR("Input Mat to cvtColor is empty !\n");
        return -1;
    }

    if (!ocl::init(&command_queue_matrix_mul_)) {
        LOG_ERR("opencl init failed!");
        return -1;
    }

    if (command_queue_matrix_mul_ == nullptr) {
        LOG_ERR("opencl command_queue failed!");
        return -1;
    }

    return matrix_mul_bgr(src0, src1, dst);
}

G_FCV_NAMESPACE1_END()
