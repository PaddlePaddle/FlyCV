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

#ifndef MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_WRAPPER_H_
#define MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_WRAPPER_H_

#include <memory>
#include "modules/core/opencl/include/core/macro.h"
#include "modules/core/opencl/include/CL/cl2.hpp"

//support opencl min version is 1.1
#ifndef CL_TARGET_OPENCL_VERSION
#define CL_TARGET_OPENCL_VERSION 200
#endif
#ifndef CL_HPP_TARGET_OPENCL_VERSION
#define CL_HPP_TARGET_OPENCL_VERSION 110
#endif
#ifndef CL_HPP_MINIMUM_OPENCL_VERSION
#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#endif

#define CHECK_NOTNULL(X)                                                       \
    ASSERT(X != NULL)                                                          \
    if (X == NULL) {                                                           \
        LOGE("OpenCL API is null\n");                                          \
    }

#define CHECK_CL_SUCCESS(error)                                                \
    if (error != CL_SUCCESS) {                                                 \
        LOGE("OpenCL ERROR CODE : %d \n", (int)error);                         \
    }

#define CHECK_MARLIN_OK(error)                                                    \
    if (error != MARLIN_OK) {                                                     \
        LOGE("%s\n", error.description().c_str());                             \
        return error;                                                          \
    }

namespace ocl {

// OpenCLSymbols is a opencl function wrapper.
// if device not support opencl or opencl target function,
// app will not crash and can get error code.
class OpenCLSymbols {
public:
    static OpenCLSymbols *get_instance();

    ~OpenCLSymbols();
    OpenCLSymbols(const OpenCLSymbols &) = delete;
    OpenCLSymbols &operator=(const OpenCLSymbols &) = delete;

    bool load_opencl_library();
    bool un_load_open_c_l_library();
    //get platfrom id
    using clGetPlatformIDsFunc  = cl_int (*)(cl_uint, cl_platform_id *,
                                            cl_uint *);
    //get platform info
    using clGetPlatformInfoFunc = cl_int (*)(cl_platform_id, cl_platform_info,
                                             size_t, void *, size_t *);
    // build program
    using clBuildProgramFunc =
        cl_int (*)(cl_program, cl_uint, const cl_device_id *, const char *,
                   void (*pfn_notify)(cl_program, void *), void *);
    //enqueue run kernel
    using clEnqueueNDRangeKernelFunc  = cl_int (*)(cl_command_queue, cl_kernel,
                                                cl_uint, const size_t *,
                                                const size_t *,
                                                const size_t *, cl_uint,
                                                const cl_event *, cl_event *);
    //set kernel parameter
    using clSetKernelArgFunc          = cl_int (*)(cl_kernel, cl_uint, size_t,
                                            const void *);
    using clRetainMemObjectFunc       = cl_int (*)(cl_mem);
    using clReleaseMemObjectFunc      = cl_int (*)(cl_mem);
    using clEnqueueUnmapMemObjectFunc = cl_int (*)(cl_command_queue, cl_mem,
                                                    void *, cl_uint,
                                                    const cl_event *,
                                                    cl_event *);
    using clRetainCommandQueueFunc = cl_int (*)(cl_command_queue command_queue);
    //create context
    using clCreateContextFunc      = cl_context (*)(
        const cl_context_properties *, cl_uint, const cl_device_id *,
        void(CL_CALLBACK *)(  // NOLINT(readability/casting)
            const char *, const void *, size_t, void *),
        void *, cl_int *);
    using clEnqueueCopyImageFunc = cl_int (*)(cl_command_queue, cl_mem, cl_mem,
                                            const size_t *, const size_t *,
                                            const size_t *, cl_uint,
                                            const cl_event *, cl_event *);

    using clCreateContextFromTypeFunc =
        cl_context (*)(const cl_context_properties *, cl_device_type,
                        void(CL_CALLBACK *)(  // NOLINT(readability/casting)
                        const char *, const void *, size_t, void *),
                        void *, cl_int *);
    using clReleaseContextFunc      = cl_int (*)(cl_context);
    using clWaitForEventsFunc       = cl_int (*)(cl_uint, const cl_event *);
    using clReleaseEventFunc        = cl_int (*)(cl_event);
    using clEnqueueWriteBufferFunc  = cl_int (*)(cl_command_queue, cl_mem,
                                                cl_bool, size_t, size_t,
                                                const void *, cl_uint,
                                                const cl_event *, cl_event *);
    using clEnqueueReadBufferFunc   = cl_int (*)(cl_command_queue, cl_mem,
                                                cl_bool, size_t, size_t, void *,
                                                cl_uint, const cl_event *,
                                                cl_event *);
    using clGetProgramBuildInfoFunc = cl_int (*)(cl_program, cl_device_id,
                                                cl_program_build_info, size_t,
                                                void *, size_t *);
    using clRetainProgramFunc       = cl_int (*)(cl_program program);
    using clEnqueueMapBufferFunc = void *(*)(cl_command_queue, cl_mem, cl_bool,
                                            cl_map_flags, size_t, size_t,
                                            cl_uint, const cl_event *,
                                            cl_event *, cl_int *);
    using clEnqueueMapImageFunc  = void *(*)(cl_command_queue, cl_mem, cl_bool,
                                            cl_map_flags, const size_t *,
                                            const size_t *, size_t *, size_t *,
                                            cl_uint, const cl_event *,
                                            cl_event *, cl_int *);
    using clCreateCommandQueueFunc = cl_command_queue(CL_API_CALL *)(  // NOLINT
        cl_context, cl_device_id, cl_command_queue_properties, cl_int *);
    using clGetCommandQueueInfoFunc     = cl_int (*)(cl_command_queue,
                                                cl_command_queue_info, size_t,
                                                void *, size_t *);
    using clReleaseCommandQueueFunc     = cl_int (*)(cl_command_queue);
    using clCreateProgramWithBinaryFunc = cl_program (*)(cl_context, cl_uint,
                                                        const cl_device_id *,
                                                        const size_t *,
                                                        const unsigned char **,
                                                        cl_int *, cl_int *);
    using clRetainContextFunc           = cl_int (*)(cl_context context);
    using clGetContextInfoFunc = cl_int (*)(cl_context, cl_context_info, size_t,
                                            void *, size_t *);
    using clReleaseProgramFunc = cl_int (*)(cl_program program);
    //flush command queue to target device
    using clFlushFunc          = cl_int (*)(cl_command_queue command_queue);
    //sync device command queue
    using clFinishFunc         = cl_int (*)(cl_command_queue command_queue);
    using clGetProgramInfoFunc = cl_int (*)(cl_program, cl_program_info, size_t,
                                            void *, size_t *);
    //create kernel with kernel name
    using clCreateKernelFunc   = cl_kernel (*)(cl_program, const char *,
                                            cl_int *);
    using clRetainKernelFunc   = cl_int (*)(cl_kernel kernel);
    using clCreateBufferFunc   = cl_mem (*)(cl_context, cl_mem_flags, size_t,
                                            void *, cl_int *);
    //create image 2d
    using clCreateImage2DFunc  = cl_mem(CL_API_CALL *)(cl_context,  // NOLINT
                                                    cl_mem_flags,
                                                    const cl_image_format *,
                                                    size_t, size_t, size_t,
                                                    void *, cl_int *);
    //create image 3d
    using clCreateImage3DFunc  = cl_mem(CL_API_CALL *)(cl_context,  // NOLINT
                                                    cl_mem_flags,
                                                    const cl_image_format *,
                                                    size_t, size_t, size_t,
                                                    size_t, size_t, void *,
                                                    cl_int *);
    //crete program with source code
    using clCreateProgramWithSourceFunc = cl_program (*)(cl_context, cl_uint,
                                                        const char **,
                                                        const size_t *,
                                                        cl_int *);
    using clReleaseKernelFunc           = cl_int (*)(cl_kernel kernel);
    using clGetDeviceInfoFunc = cl_int (*)(cl_device_id, cl_device_info, size_t,
                                           void *, size_t *);
    //get device id, two device have different device id
    using clGetDeviceIDsFunc  = cl_int (*)(cl_platform_id, cl_device_type,
                                        cl_uint, cl_device_id *, cl_uint *);
    using clRetainEventFunc   = cl_int (*)(cl_event);
    using clGetKernelWorkGroupInfoFunc = cl_int (*)(cl_kernel, cl_device_id,
                                                    cl_kernel_work_group_info,
                                                    size_t, void *, size_t *);
    using clGetEventInfoFunc           = cl_int (*)(cl_event event,
                                            cl_event_info param_name,
                                            size_t param_value_size,
                                            void *param_value,
                                            size_t *param_value_size_ret);
    using clGetEventProfilingInfoFunc  = cl_int (*)(
        cl_event event, cl_profiling_info param_name, size_t param_value_size,
        void *param_value, size_t *param_value_size_ret);
    using clGetImageInfoFunc = cl_int (*)(cl_mem, cl_image_info, size_t, void *,
                                          size_t *);
    using clEnqueueCopyBufferToImageFunc = cl_int(CL_API_CALL *)(
        cl_command_queue, cl_mem, cl_mem, size_t, const size_t *,
        const size_t *, cl_uint, const cl_event *, cl_event *);
    using clEnqueueCopyImageToBufferFunc = cl_int(CL_API_CALL *)(
        cl_command_queue, cl_mem, cl_mem, const size_t *, const size_t *,
        size_t, cl_uint, const cl_event *, cl_event *);
#if CL_HPP_TARGET_OPENCL_VERSION >= 120
    using clRetainDeviceFunc        = cl_int (*)(cl_device_id);
    using clReleaseDeviceFunc       = cl_int (*)(cl_device_id);
    using clCreateImageFunc         = cl_mem (*)(cl_context, cl_mem_flags,
                                        const cl_image_format *,
                                        const cl_image_desc *, void *,
                                        cl_int *);
#endif
#if CL_HPP_TARGET_OPENCL_VERSION >= 200
    //opencl 2.0 can get sub group info and wave size.
    using clGetKernelSubGroupInfoKHRFunc = cl_int(CL_API_CALL *)(
        cl_kernel, cl_device_id, cl_kernel_sub_group_info, size_t, const void  *, size_t, void *, size_t *);
    using clCreateCommandQueueWithPropertiesFunc = cl_command_queue(CL_API_CALL *)(
        cl_context, cl_device_id, const cl_queue_properties *, cl_int *);
    using clGetExtensionFunctionAddressFunc = void * (CL_API_CALL *)(const char *);
#endif

#define MARLIN_CL_DEFINE_FUNC_PTR(func) func##Func func = nullptr

    MARLIN_CL_DEFINE_FUNC_PTR(clGetPlatformIDs);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetPlatformInfo);
    MARLIN_CL_DEFINE_FUNC_PTR(clBuildProgram);
    MARLIN_CL_DEFINE_FUNC_PTR(clEnqueueNDRangeKernel);
    MARLIN_CL_DEFINE_FUNC_PTR(clSetKernelArg);
    MARLIN_CL_DEFINE_FUNC_PTR(clReleaseKernel);
    MARLIN_CL_DEFINE_FUNC_PTR(clCreateProgramWithSource);
    MARLIN_CL_DEFINE_FUNC_PTR(clCreateBuffer);
    MARLIN_CL_DEFINE_FUNC_PTR(clCreateImage2D);
    MARLIN_CL_DEFINE_FUNC_PTR(clCreateImage3D);
    MARLIN_CL_DEFINE_FUNC_PTR(clRetainKernel);
    MARLIN_CL_DEFINE_FUNC_PTR(clCreateKernel);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetProgramInfo);
    MARLIN_CL_DEFINE_FUNC_PTR(clFlush);
    MARLIN_CL_DEFINE_FUNC_PTR(clFinish);
    MARLIN_CL_DEFINE_FUNC_PTR(clReleaseProgram);
    MARLIN_CL_DEFINE_FUNC_PTR(clRetainContext);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetContextInfo);
    MARLIN_CL_DEFINE_FUNC_PTR(clCreateProgramWithBinary);
    MARLIN_CL_DEFINE_FUNC_PTR(clCreateCommandQueue);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetCommandQueueInfo);
    MARLIN_CL_DEFINE_FUNC_PTR(clReleaseCommandQueue);
    MARLIN_CL_DEFINE_FUNC_PTR(clEnqueueMapBuffer);
    MARLIN_CL_DEFINE_FUNC_PTR(clEnqueueMapImage);
    MARLIN_CL_DEFINE_FUNC_PTR(clEnqueueCopyImage);
    MARLIN_CL_DEFINE_FUNC_PTR(clRetainProgram);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetProgramBuildInfo);
    MARLIN_CL_DEFINE_FUNC_PTR(clEnqueueReadBuffer);
    MARLIN_CL_DEFINE_FUNC_PTR(clEnqueueWriteBuffer);
    MARLIN_CL_DEFINE_FUNC_PTR(clWaitForEvents);
    MARLIN_CL_DEFINE_FUNC_PTR(clReleaseEvent);
    MARLIN_CL_DEFINE_FUNC_PTR(clCreateContext);
    MARLIN_CL_DEFINE_FUNC_PTR(clCreateContextFromType);
    MARLIN_CL_DEFINE_FUNC_PTR(clReleaseContext);
    MARLIN_CL_DEFINE_FUNC_PTR(clRetainCommandQueue);
    MARLIN_CL_DEFINE_FUNC_PTR(clEnqueueUnmapMemObject);
    MARLIN_CL_DEFINE_FUNC_PTR(clRetainMemObject);
    MARLIN_CL_DEFINE_FUNC_PTR(clReleaseMemObject);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetDeviceInfo);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetDeviceIDs);
    MARLIN_CL_DEFINE_FUNC_PTR(clRetainEvent);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetKernelWorkGroupInfo);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetEventInfo);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetEventProfilingInfo);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetImageInfo);
    MARLIN_CL_DEFINE_FUNC_PTR(clEnqueueCopyBufferToImage);
    MARLIN_CL_DEFINE_FUNC_PTR(clEnqueueCopyImageToBuffer);
#if CL_HPP_TARGET_OPENCL_VERSION >= 120
    MARLIN_CL_DEFINE_FUNC_PTR(clRetainDevice);
    MARLIN_CL_DEFINE_FUNC_PTR(clReleaseDevice);
    MARLIN_CL_DEFINE_FUNC_PTR(clCreateImage);
#endif
#if CL_HPP_TARGET_OPENCL_VERSION >= 200
    MARLIN_CL_DEFINE_FUNC_PTR(clGetKernelSubGroupInfoKHR);
    MARLIN_CL_DEFINE_FUNC_PTR(clCreateCommandQueueWithProperties);
    MARLIN_CL_DEFINE_FUNC_PTR(clGetExtensionFunctionAddress);
#endif

#undef MARLIN_CL_DEFINE_FUNC_PTR

private:
    OpenCLSymbols();
    bool load_library_from_path(const std::string &path);

private:
    static std::shared_ptr<OpenCLSymbols> opencl_symbols_singleton_;
    void *handle_ = nullptr;
};

}  // namespace ocl
#endif  // MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_WRAPPER_H_
