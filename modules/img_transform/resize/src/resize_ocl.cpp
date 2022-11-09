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

#include "modules/core/base/interface/macro_ns.h"
#include "modules/img_transform/resize/include/resize_ocl.h"
#include "modules/core/opencl/interface/opencl.h"
#include "modules/core/opencl/include/opencl_execute_unit.h"
#include "modules/core/opencl/include/opencl_utils.h"
#include "modules/core/opencl/include/opencl_memory_manager.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

void *command_queue_resize_;
ocl::OpenCLExecuteUnit unit_resize_[1];
long output_ptr_resize_;
uint32_t output_size_resize_;

int create_resize_unit(
        ocl::OpenCLExecuteUnit &unit,
        const std::string &kernel_name) {
    if (unit.work_group_size_max <= 0) {
        std::set <std::string> build_options;
        std::string program_name = "resize";
        return ocl::create_execute_unit(unit, program_name, kernel_name, build_options);
    } else {
        return 0;
    }
}

int set_resize_args(
        ocl::OpenCLExecuteUnit &unit,
        const Mat &src,
        const Size &dsize,
        ocl::OpenCLBufferInfo **src_buffer_info,
        ocl::OpenCLBufferInfo **dst_buffer_info,
        uint32_t &dst_size) {
    unit.global_work_size = {
            static_cast <uint32_t>(dsize.width()),
            static_cast <uint32_t>(dsize.height()),
    };
    unit.local_work_size = {16, 16};

    uint32_t src_size = src.width() * src.height() * 3;
    dst_size = dsize.width() * dsize.height() * 3;

    auto memory_manager = ocl::OpenCLMemoryManager::get_instance();
    *src_buffer_info = (ocl::OpenCLBufferInfo *) memory_manager->obtain_buffer(src_size);
    *dst_buffer_info = (ocl::OpenCLBufferInfo *) memory_manager->obtain_buffer(dst_size);

    int cl_ret = -1;
    cl_ret = static_cast<cl::CommandQueue *>(command_queue_resize_)->
            enqueueWriteBuffer( *(*src_buffer_info)->cl_buffer,
            true, 0, src_size, src.data(), nullptr, nullptr);
    CHECK_CL_SUCCESS(cl_ret)
    uint32_t idx = 0;
    cl_ret = unit.ocl_kernel.setArg(idx++, *(*src_buffer_info)->cl_buffer);
    CHECK_CL_SUCCESS(cl_ret)
    cl_ret = unit.ocl_kernel.setArg(idx++, *(*dst_buffer_info)->cl_buffer);
    CHECK_CL_SUCCESS(cl_ret)
    cl_ret = unit.ocl_kernel.setArg(idx++, src.width());
    CHECK_CL_SUCCESS(cl_ret)
    cl_ret = unit.ocl_kernel.setArg(idx++, src.height());
    CHECK_CL_SUCCESS(cl_ret)
    cl_ret = unit.ocl_kernel.setArg(idx++, dsize.width());
    CHECK_CL_SUCCESS(cl_ret)
    cl_ret = unit.ocl_kernel.setArg(idx++, dsize.height());
    CHECK_CL_SUCCESS(cl_ret)
    return cl_ret;
}

int run_resize_unit(
        ocl::OpenCLExecuteUnit &unit,
        bool need_wait) {
    int status = ocl::run_kernel(unit.ocl_kernel, unit.global_work_size,
            unit.local_work_size, static_cast<cl::CommandQueue *>(command_queue_resize_),
            "yuv_to_bgr");

    if (need_wait) {
        static_cast<cl::CommandQueue *>(command_queue_resize_)->finish();
    }

    return status;
}

int read_resize_buffer(
        ocl::OpenCLBufferInfo *src_buffer_info,
        ocl::OpenCLBufferInfo *dst_buffer_info,
        uint32_t dst_size,
        void **data) {
    if (dst_size > output_size_resize_ || output_ptr_resize_ == 0) {
        if (output_ptr_resize_ != 0) {
            free((void *) output_ptr_resize_);
        }
        output_ptr_resize_ = (long) malloc(dst_size);
        *data = (void *) output_ptr_resize_;
        output_size_resize_ = dst_size;
    } else {
        *data = (void *) output_ptr_resize_;
    }

    static_cast<cl::CommandQueue*>(command_queue_resize_)->enqueueReadBuffer(
            *dst_buffer_info->cl_buffer, true, 0, dst_size, *data, nullptr, nullptr);

    src_buffer_info->is_busy = false;
    dst_buffer_info->is_busy = false;
    return 0;
}

int resize_to_bgr(
        const Mat &src,
        Mat &dst,
        const Size &dsize) {
    int status = create_resize_unit(unit_resize_[0], "bgr_resize");

    if (status != 0) {
        LOG_ERR("Create Convert Unit failed (ret = %d)!", (int) status);
        return status;
    }

    ocl::OpenCLBufferInfo *src_buffer_info = nullptr;
    ocl::OpenCLBufferInfo *dst_buffer_info = nullptr;
    uint32_t dst_size;
    status = set_resize_args(unit_resize_[0], src, dsize,
            &src_buffer_info, &dst_buffer_info, dst_size);
    status = run_resize_unit(unit_resize_[0], true);

    if (status != 0) {
        LOG_ERR("Run Convert Unit failed (ret = %d)!", (int)status);
        return status;
    }

    void *data = nullptr;

    read_resize_buffer(src_buffer_info, dst_buffer_info, dst_size, &data);
    dst = Mat(dsize.width(), dsize.height(), FCVImageType::PKG_BGR_U8, data);
    return 0;
}

int resize_opencl(Mat& src, Mat& dst, const Size& dsize){
    if (src.empty()) {
        LOG_ERR("Input Mat to cvtColor is empty !");
        return -1;
    }

    if (!ocl::init(&command_queue_resize_)) {
        LOG_ERR("opencl init failed!");
        return -1;
    }

    if (command_queue_resize_ == nullptr) {
        LOG_ERR("opencl command_queue failed!");
        return -1;
    }

    return resize_to_bgr(src, dst, dsize);
}

G_FCV_NAMESPACE1_END()
