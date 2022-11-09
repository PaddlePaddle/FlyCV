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

#include "modules/core/opencl/include/opencl_utils.h"

namespace ocl {

Status run_kernel(const cl::Kernel &kernel, const std::vector<uint32_t> &gws, const std::vector<uint32_t> &lws,
                  cl::CommandQueue *command_queue, std::string name) {
    LOGD("start RunKernel !\n");

    ASSERT(lws.empty() || lws.size() == gws.size());

    std::vector<uint32_t> internal_global_ws = gws;
    for (size_t i = 0; i < lws.size(); i++) {
        internal_global_ws[i] = ROUND_UP(gws[i], lws[i]);
    }

    cl::Event event;
    cl_int error = CL_SUCCESS;

    LOGD("gws size: %d , lws size: %d \n", gws.size(), lws.size());

    if (gws.size() == 1) {
        //1d group size
        if (lws.size() == 0) {
            error = command_queue->enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(internal_global_ws[0]),
                                                        cl::NullRange, nullptr, &event);
            LOGD("run %s,gws:[%u],lws:NullRange \n", name.c_str(), internal_global_ws[0]);
        } else {
            error = command_queue->enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(internal_global_ws[0]),
                                                        cl::NDRange(lws[0]), nullptr, &event);
            LOGD("run %s,gws:[%u],lws:[%u]\n", name.c_str(), internal_global_ws[0], lws[0]);
        }
    } else if (gws.size() == 2) {
        //2d group
        if (lws.size() == 0) {
            error = command_queue->enqueueNDRangeKernel(kernel, cl::NullRange,
                                                        cl::NDRange(internal_global_ws[0], internal_global_ws[1]),
                                                        cl::NullRange, nullptr, &event);
            LOGD("run %s,gws:[%u,%u],lws:NullRange \n", name.c_str(), internal_global_ws[0], internal_global_ws[1]);
        } else {
            error = command_queue->enqueueNDRangeKernel(kernel, cl::NullRange,
                                                        cl::NDRange(internal_global_ws[0], internal_global_ws[1]),
                                                        cl::NDRange(lws[0], lws[1]), nullptr, &event);
            LOGD("run %s,gws:[%u,%u],lws:[%u,%u] \n", name.c_str(), internal_global_ws[0], internal_global_ws[1],
                 lws[0], lws[1]);
        }
    } else {
        //3d group
        if (lws.size() == 0) {
            error = command_queue->enqueueNDRangeKernel(
                kernel, cl::NullRange, cl::NDRange(internal_global_ws[0], internal_global_ws[1], internal_global_ws[2]),
                cl::NullRange, nullptr, &event);
            LOGD("run %s,gws:[%u,%u,%u],lws:NullRange \n", name.c_str(), internal_global_ws[0],
                 internal_global_ws[1], internal_global_ws[2]);
        } else {
            error = command_queue->enqueueNDRangeKernel(
                kernel, cl::NullRange, cl::NDRange(internal_global_ws[0], internal_global_ws[1], internal_global_ws[2]),
                cl::NDRange(lws[0], lws[1], lws[2]), nullptr, &event);
            LOGD("run %s,gws:[%u,%u,%u],lws:[%u,%u,%u] \n", name.c_str(), internal_global_ws[0],
                 internal_global_ws[1], internal_global_ws[2], lws[0], lws[1], lws[2]);
        }
    }

    if (error != CL_SUCCESS) {
        CHECK_CL_SUCCESS(error);
        return Status(MARLINERR_OPENCL_API_ERROR, "OpenCL NDRange failed");
    }
    LOGD("end RunKernel !\n");
    return MARLIN_OK;
}

bool adreno_local_size_valid(const std::vector<uint32_t> &gws, std::vector<uint32_t> &lws,
                             const uint32_t subgroup_size) {
    return 0 == (lws[0] * lws[1]) % subgroup_size &&
           0 == gws[0] % lws[0] &&
           0 == gws[1] % lws[1] &&
           ((lws[0] < lws[1]) == (gws[0] < gws[1]));
}

std::vector<uint32_t> adreno_local_size2d(const std::vector<uint32_t> &gws, const GpuInfo gpu_info,
                                          const uint32_t compute_units, const uint32_t max_workgroup_size,
                                          const uint32_t subgroup_size) {
    std::vector<uint32_t> lws;
    lws.clear();

    int min_workgroup_count = compute_units;
    if (gpu_info.model_num >= 540) {
        min_workgroup_count = 2 * compute_units;
    }
    //calc gws[1]
    if (gws[1] % min_workgroup_count == 0) {
        lws.resize(2);
        lws[1] = std::min<uint32_t>(gws[1] / min_workgroup_count, max_workgroup_size);

        if (0 != subgroup_size) {
            int min_workgroup_size = subgroup_size * 2;
            int max_val = std::max<uint32_t>(max_workgroup_size / lws[1], 1);
            int min_val = std::max<uint32_t>(min_workgroup_size / lws[1], 1);
            lws[0] = std::min<uint32_t>(gws[0], max_val);
            for (; lws[0] >= min_val; lws[0]--) {
                if (adreno_local_size_valid(gws, lws, subgroup_size)) {
                    return lws;
                }
            }
        }

        lws[0] = max_workgroup_size / lws[1];
        lws[0] = std::max<uint32_t>(std::min<uint32_t>(gws[0], lws[0]), 1);

        if (0 == gws[0] % lws[0] && 0 == gws[1] % lws[1] && ((lws[0] < lws[1]) == (gws[0] < gws[1]))) {
            return lws;
        }
    }

    //calc gws[0]
    if (gws[0] % min_workgroup_count == 0) {
        lws.resize(2);
        lws[0] = std::min<uint32_t>(gws[0] / min_workgroup_count, max_workgroup_size);

        if (0 != subgroup_size) {
            int min_workgroup_size = subgroup_size * 2;
            int max_val = std::max<uint32_t>(max_workgroup_size / lws[0], 1);
            int min_val = std::max<uint32_t>(min_workgroup_size / lws[0], 1);
            lws[1] = std::min<uint32_t>(gws[1], max_val);
            for (; lws[1] >= min_val; lws[1]--) {
                if (0 == (lws[0] * lws[1]) % subgroup_size && 0 == gws[0] % lws[0] && 0 == gws[1] % lws[1] &&
                    ((lws[0] < lws[1]) == (gws[0] < gws[1]))) {
                    return lws;
                }
            }
        }

        lws[1] = max_workgroup_size / lws[0];
        lws[1] = std::max<uint32_t>(std::min<uint32_t>(gws[1], lws[1]), 1);
        if (0 == gws[0] % lws[0] && 0 == gws[1] % lws[1] && ((lws[0] < lws[1]) == (gws[0] < gws[1]))) {
            return lws;
        }
    }

    lws.clear();

    return lws;
}

Status adjust_build_option_for_fp32(std::set<std::string> &build_options) {
    bool force_fp32 = false;
#if (defined __ANDROID_API__) && (__ANDROID_API__ < 21)
    force_fp32 = true;
#endif
    if (force_fp32) {
    }
    build_options.emplace("-DFORCE_FP32");
    return MARLIN_OK;
}

std::vector<uint32_t> local_ws3d_default(OpenCLExecuteUnit &unit) {
    return local_ws3d_default(unit.global_work_size, unit.work_group_size_max, unit.sub_group_size);
}

std::vector<uint32_t> local_ws3d_default(const std::vector<uint32_t> &gws, const uint32_t max_workgroup_size,
                                         const uint32_t subgroup_size) {
    GpuInfo gpu_info = OpenCLRuntime::get_instance()->get_gpu_info();
    std::vector<uint32_t> lws;
    lws.clear();

    if (gpu_info.type == GpuType::ADRENO) {
        uint32_t compute_units = OpenCLRuntime::get_instance()->get_compute_units();
        lws.resize(3);
        if (max_workgroup_size == 0) {
            lws[0] = lws[1] = lws[2] = 1;
        } else {
            std::vector<uint32_t> lws_2d_temp;
            lws_2d_temp =
                adreno_local_size2d({gws[1], gws[2]}, gpu_info, compute_units, max_workgroup_size, subgroup_size);

            if (!lws_2d_temp.empty()) {
                lws[1] = lws_2d_temp[0];
                lws[2] = lws_2d_temp[1];
                const uint32_t lws_size = lws[1] * lws[2];
                lws[0] = std::max<uint32_t>(max_workgroup_size / lws_size, 1);
                while (gws[0] % lws[0] != 0) {
                    lws[0]--;
                }
            } else {
                lws.clear();
            }
        }
    }
    return lws;
}

std::vector<uint32_t> local_ws2d_default(OpenCLExecuteUnit &unit) {
    return local_ws2d_default(unit.global_work_size, unit.work_group_size_max, unit.sub_group_size);
}

std::vector<uint32_t> local_ws2d_default(const std::vector<uint32_t> &gws, const uint32_t max_workgroup_sze,
                                         const uint32_t subgroup_size) {
    GpuInfo gpu_info = OpenCLRuntime::get_instance()->get_gpu_info();
    std::vector<uint32_t> lws;
    if (gpu_info.type == GpuType::ADRENO) {
        uint32_t compute_units = OpenCLRuntime::get_instance()->get_compute_units();
        lws.resize(2);
        if (max_workgroup_sze == 0) {
            lws[0] = lws[1] = 1;
        } else {
            lws = adreno_local_size2d(gws, gpu_info, compute_units, max_workgroup_sze, subgroup_size);
        }
    }

    return lws;
}

Status create_execute_unit(OpenCLExecuteUnit &unit, const std::string &program_name, const std::string &kernel_name,
                           const std::set<std::string> &build_opt) {
    OpenCLRuntime *opencl_runtime = OpenCLRuntime::get_instance();

    Status ret = opencl_runtime->build_kernel(unit.ocl_kernel, program_name, kernel_name, build_opt);
    if (ret != MARLIN_OK) {
        LOGE("kernel (%s) build failed!\n", kernel_name.c_str());
        return ret;
    }
    unit.work_group_size_max = static_cast<uint32_t>(opencl_runtime->get_max_work_group_size(unit.ocl_kernel));
    if (unit.work_group_size_max == 0) {
        LOGE("Get max workgroup size failed!\n");
        return Status(MARLINERR_OPENCL_ACC_INIT_ERROR, "Get max workgroup size failed!");
    }

    unit.sub_group_size = static_cast<uint32_t>(opencl_runtime->get_sub_group_size(unit.ocl_kernel));

    return MARLIN_OK;
}

uint32_t set_execute_unit3d_size_info_default(OpenCLExecuteUnit &unit, DimsVector dims) {
    //NCHW
    unit.global_work_size = {
        //        //width
        //        static_cast<uint32_t>(dims[3]),
        //        //channel-blocks/4
        //        static_cast<uint32_t>(UP_DIV(dims[1], 4)),
        //        //batch * height
        //        static_cast<uint32_t >(dims[0] * dims[2]),
        static_cast<uint32_t>(dims[0]),
        static_cast<uint32_t>(dims[2]),
        static_cast<uint32_t>(dims[3])};
    unit.local_work_size = {1, 1, 1};
    //    std::vector<uint32_t> temp_gws = {unit.global_work_size[1], unit.global_work_size[0], unit.global_work_size[2]};
    //    std::vector<uint32_t> temp_lws = local_ws3d_default(temp_gws, unit.work_group_size_max, unit.sub_group_size);
    //
    //    if (3 == temp_lws.size()) {
    //        unit.local_work_size = {temp_lws[1], temp_lws[0], temp_lws[2]};
    //    } else {
    //        unit.local_work_size.clear();
    //    }
    uint32_t idx = 0;
    //    unit.ocl_kernel.setArg(idx++, unit.global_work_size[0]);
    //    unit.ocl_kernel.setArg(idx++, unit.global_work_size[1]);
    //    unit.ocl_kernel.setArg(idx++, unit.global_work_size[2]);

    return idx;
}

uint32_t set_execute_unit2d_size_info_default(OpenCLExecuteUnit &unit, DimsVector dims) {
    unit.global_work_size = {
            //channel-blocks*[width]
            static_cast <uint32_t>(UP_DIV(dims[1], 4) * dims[2]),
            //batch*height
            static_cast <uint32_t>(dims[0] * dims[3]),
    };
    LOGD("%d  %d ",unit.global_work_size[0],unit.global_work_size[1]);
    // unit.global_work_size = {
    //     static_cast<uint32_t>(dims[2]),
    //     static_cast<uint32_t>(dims[3]),
    // };
    //unit.local_work_size = local_ws2d_default(unit);
    uint32_t idx = 0;
    //    unit.ocl_kernel.setArg(idx++, unit.global_work_size[0]);
    //    unit.ocl_kernel.setArg(idx++, unit.global_work_size[1]);
    unit.local_work_size = {16, 16};
    return idx;
}

} // namespace ocl
