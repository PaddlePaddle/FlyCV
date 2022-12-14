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

#include <dlfcn.h>
#include <memory>
#include <string>
#include <vector>
#include "modules/core/opencl/include/opencl_runtime.h"

namespace ocl {
static const std::vector<std::string> g_opencl_library_paths = {
//default opencl library path
#if defined(__APPLE__) || defined(__MACOSX)
    "libOpenCL.so", "/System/Library/Frameworks/OpenCL.framework/OpenCL"
#elif defined(__ANDROID__)
    "libOpenCL.so",
    "libGLES_mali.so",
    "libmali.so",
#if defined(__aarch64__)
    // Qualcomm Adreno
    "/system/vendor/lib64/libOpenCL.so",
    "/system/lib64/libOpenCL.so",
    // Mali
    "/system/vendor/lib64/egl/libGLES_mali.so",
    "/system/lib64/egl/libGLES_mali.so",
    // Pixel Phone
    "libOpenCL-pixel.so",
#else
    // Qualcomm Adreno
    "/system/vendor/lib/libOpenCL.so", "/system/lib/libOpenCL.so",
    // Mali
    "/system/vendor/lib/egl/libGLES_mali.so", "/system/lib/egl/libGLES_mali.so",
    // other
    "/system/vendor/lib/libPVROCL.so", "/data/data/org.pocl.libs/files/lib/libpocl.so",
    // Pixel Phone
    "libOpenCL-pixel.so",
#endif
#elif defined(__linux__)
    "/usr/lib/libOpenCL.so",
    "/usr/local/lib/libOpenCL.so",
    "/usr/local/lib/libpocl.so",
    "/usr/lib64/libOpenCL.so",
    "/usr/lib32/libOpenCL.so",
    "libOpenCL.so"
#endif
};

std::shared_ptr<OpenCLSymbols> OpenCLSymbols::opencl_symbols_singleton_ = nullptr;

OpenCLSymbols *OpenCLSymbols::get_instance() {
    static std::once_flag opencl_symbol_once;
    std::call_once(opencl_symbol_once, []() { opencl_symbols_singleton_.reset(new OpenCLSymbols()); });

    return opencl_symbols_singleton_.get();
}

OpenCLSymbols::OpenCLSymbols() {
    LOGD("OpenCLSymbols()\n");
}

OpenCLSymbols::~OpenCLSymbols() {
    LOGD("~OpenCLSymbols() start\n");
    if (nullptr == opencl_symbols_singleton_.get())
        return;
    opencl_symbols_singleton_->un_load_open_c_l_library();
    LOGD("~OpenCLSymbols() end\n");
}

//load default library path
bool OpenCLSymbols::load_opencl_library() {
    if (handle_ != nullptr) {
        return true;
    }
    for (const auto &opencl_lib : g_opencl_library_paths) {
        if (load_library_from_path(opencl_lib)) {
            return true;
        }
    }
    return false;
}

bool OpenCLSymbols::un_load_open_c_l_library() {
    if (handle_ != nullptr) {
        if (dlclose(handle_) != 0) {
            return false;
        }
        handle_ = nullptr;
        return true;
    }
    return true;
}

bool OpenCLSymbols::load_library_from_path(const std::string &library_path) {
    handle_ = dlopen(library_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (handle_ == nullptr) {
        return false;
    }
    bool is_pixel = library_path == "libOpenCL-pixel.so";
    typedef void* (*loadOpenCLPointer_t)(const char* name);
    loadOpenCLPointer_t loadOpenCLPointer;
    if(is_pixel){
        typedef void (*enableOpenCL_t)();
        enableOpenCL_t enableOpenCL = reinterpret_cast<enableOpenCL_t>(dlsym(handle_, "enableOpenCL"));
        if (enableOpenCL == nullptr) {
            return false;
        }
        enableOpenCL();
        loadOpenCLPointer = reinterpret_cast<loadOpenCLPointer_t>(dlsym(handle_, "loadOpenCLPointer"));
        if (loadOpenCLPointer == nullptr) {
            return false;
        }
    }

// load function ptr use dlopen and dlsym. if cann't find func_name, will return false.
#define MARLIN_LOAD_FUNCTION_PTR(func_name)                                                                               \
    if(is_pixel){                                                                                                      \
        func_name = reinterpret_cast<func_name##Func>(loadOpenCLPointer(#func_name));                                  \
    } else {                                                                                                           \
        func_name = reinterpret_cast<func_name##Func>(dlsym(handle_, #func_name));                                     \
    }                                                                                                                  \
    if (func_name == nullptr) {                                                                                        \
        LOGE("load func (%s) from (%s) failed!\n", #func_name, library_path.c_str());                                  \
        return false;                                                                                                  \
    }

    MARLIN_LOAD_FUNCTION_PTR(clGetPlatformIDs);
    MARLIN_LOAD_FUNCTION_PTR(clGetPlatformInfo);
    MARLIN_LOAD_FUNCTION_PTR(clBuildProgram);
    MARLIN_LOAD_FUNCTION_PTR(clEnqueueNDRangeKernel);
    MARLIN_LOAD_FUNCTION_PTR(clSetKernelArg);
    MARLIN_LOAD_FUNCTION_PTR(clReleaseKernel);
    MARLIN_LOAD_FUNCTION_PTR(clCreateProgramWithSource);
    MARLIN_LOAD_FUNCTION_PTR(clCreateBuffer);
    MARLIN_LOAD_FUNCTION_PTR(clCreateImage2D);
    MARLIN_LOAD_FUNCTION_PTR(clCreateImage3D);
    MARLIN_LOAD_FUNCTION_PTR(clRetainKernel);
    MARLIN_LOAD_FUNCTION_PTR(clCreateKernel);
    MARLIN_LOAD_FUNCTION_PTR(clGetProgramInfo);
    MARLIN_LOAD_FUNCTION_PTR(clFlush);
    MARLIN_LOAD_FUNCTION_PTR(clFinish);
    MARLIN_LOAD_FUNCTION_PTR(clReleaseProgram);
    MARLIN_LOAD_FUNCTION_PTR(clRetainContext);
    MARLIN_LOAD_FUNCTION_PTR(clGetContextInfo);
    MARLIN_LOAD_FUNCTION_PTR(clCreateProgramWithBinary);
    MARLIN_LOAD_FUNCTION_PTR(clCreateCommandQueue);
    MARLIN_LOAD_FUNCTION_PTR(clGetCommandQueueInfo);
    MARLIN_LOAD_FUNCTION_PTR(clReleaseCommandQueue);
    MARLIN_LOAD_FUNCTION_PTR(clEnqueueMapBuffer);
    MARLIN_LOAD_FUNCTION_PTR(clEnqueueMapImage);
    MARLIN_LOAD_FUNCTION_PTR(clRetainProgram);
    MARLIN_LOAD_FUNCTION_PTR(clGetProgramBuildInfo);
    MARLIN_LOAD_FUNCTION_PTR(clEnqueueReadBuffer);
    MARLIN_LOAD_FUNCTION_PTR(clEnqueueWriteBuffer);
    MARLIN_LOAD_FUNCTION_PTR(clWaitForEvents);
    MARLIN_LOAD_FUNCTION_PTR(clReleaseEvent);
    MARLIN_LOAD_FUNCTION_PTR(clCreateContext);
    MARLIN_LOAD_FUNCTION_PTR(clCreateContextFromType);
    MARLIN_LOAD_FUNCTION_PTR(clReleaseContext);
    MARLIN_LOAD_FUNCTION_PTR(clRetainCommandQueue);
    MARLIN_LOAD_FUNCTION_PTR(clEnqueueUnmapMemObject);
    MARLIN_LOAD_FUNCTION_PTR(clRetainMemObject);
    MARLIN_LOAD_FUNCTION_PTR(clReleaseMemObject);
    MARLIN_LOAD_FUNCTION_PTR(clGetDeviceInfo);
    MARLIN_LOAD_FUNCTION_PTR(clGetDeviceIDs);
    MARLIN_LOAD_FUNCTION_PTR(clRetainEvent);
    MARLIN_LOAD_FUNCTION_PTR(clGetKernelWorkGroupInfo);
    MARLIN_LOAD_FUNCTION_PTR(clGetEventInfo);
    MARLIN_LOAD_FUNCTION_PTR(clGetEventProfilingInfo);
    MARLIN_LOAD_FUNCTION_PTR(clGetImageInfo);
    MARLIN_LOAD_FUNCTION_PTR(clEnqueueCopyImage);
    MARLIN_LOAD_FUNCTION_PTR(clEnqueueCopyBufferToImage);
    MARLIN_LOAD_FUNCTION_PTR(clEnqueueCopyImageToBuffer);
#if CL_HPP_TARGET_OPENCL_VERSION >= 120
    MARLIN_LOAD_FUNCTION_PTR(clRetainDevice);
    MARLIN_LOAD_FUNCTION_PTR(clReleaseDevice);
    MARLIN_LOAD_FUNCTION_PTR(clCreateImage);
#endif
#if CL_HPP_TARGET_OPENCL_VERSION >= 200
    MARLIN_LOAD_FUNCTION_PTR(clGetKernelSubGroupInfoKHR);
    MARLIN_LOAD_FUNCTION_PTR(clCreateCommandQueueWithProperties);
    MARLIN_LOAD_FUNCTION_PTR(clGetExtensionFunctionAddress);
#endif

#undef MARLIN_LOAD_FUNCTION_PTR

    return true;
}

}  // namespace ocl

// clGetPlatformIDs wrapper, use OpenCLSymbols function. use OpenCLSymbols function.
cl_int clGetPlatformIDs(cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetPlatformIDs;    CHECK_NOTNULL(func);
    return func(num_entries, platforms, num_platforms);
}

//clGetPlatformInfo wrapper, use OpenCLSymbols function. use OpenCLSymbols function.
cl_int clGetPlatformInfo(cl_platform_id platform, cl_platform_info param_name, size_t param_value_size,
                         void *param_value, size_t *param_value_size_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetPlatformInfo;    CHECK_NOTNULL(func);
    return func(platform, param_name, param_value_size, param_value, param_value_size_ret);
}

//clGetDeviceIDs wrapper, use OpenCLSymbols function.
cl_int clGetDeviceIDs(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id *devices,
                      cl_uint *num_devices) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetDeviceIDs;    CHECK_NOTNULL(func);
    return func(platform, device_type, num_entries, devices, num_devices);
}

//clGetDeviceInfo wrapper, use OpenCLSymbols function.
cl_int clGetDeviceInfo(cl_device_id device, cl_device_info param_name, size_t param_value_size, void *param_value,
                       size_t *param_value_size_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetDeviceInfo;    CHECK_NOTNULL(func);
    return func(device, param_name, param_value_size, param_value, param_value_size_ret);
}

//clCreateContext wrapper, use OpenCLSymbols function.
cl_context clCreateContext(const cl_context_properties *properties, cl_uint num_devices, const cl_device_id *devices,
                           void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *), void *user_data,
                           cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clCreateContext;    CHECK_NOTNULL(func);
    return func(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
}

//clCreateContextFromType wrapper, use OpenCLSymbols function.
cl_context clCreateContextFromType(const cl_context_properties *properties, cl_device_type device_type,
                                   void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                                   void *user_data, cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clCreateContextFromType;    CHECK_NOTNULL(func);
    return func(properties, device_type, pfn_notify, user_data, errcode_ret);
}

//clRetainContext wrapper, use OpenCLSymbols function.
cl_int clRetainContext(cl_context context) {
    auto func = ocl::OpenCLSymbols::get_instance()->clRetainContext;    CHECK_NOTNULL(func);
    return func(context);
}

//clReleaseContext wrapper, use OpenCLSymbols function.
cl_int clReleaseContext(cl_context context) {
    auto func = ocl::OpenCLSymbols::get_instance()->clReleaseContext;    CHECK_NOTNULL(func);
    return func(context);
}

//clGetContextInfo wrapper, use OpenCLSymbols function.
cl_int clGetContextInfo(cl_context context, cl_context_info param_name, size_t param_value_size, void *param_value,
                        size_t *param_value_size_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetContextInfo;    CHECK_NOTNULL(func);
    return func(context, param_name, param_value_size, param_value, param_value_size_ret);
}

//clCreateProgramWithSource wrapper, use OpenCLSymbols function.
cl_program clCreateProgramWithSource(cl_context context, cl_uint count, const char **strings, const size_t *lengths,
                                     cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clCreateProgramWithSource;    CHECK_NOTNULL(func);
    return func(context, count, strings, lengths, errcode_ret);
}

//clGetProgramInfo wrapper, use OpenCLSymbols function.
cl_int clGetProgramInfo(cl_program program, cl_program_info param_name, size_t param_value_size, void *param_value,
                        size_t *param_value_size_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetProgramInfo;    CHECK_NOTNULL(func);
    return func(program, param_name, param_value_size, param_value, param_value_size_ret);
}

//clGetProgramBuildInfo wrapper, use OpenCLSymbols function.
cl_int clGetProgramBuildInfo(cl_program program, cl_device_id device, cl_program_build_info param_name,
                             size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetProgramBuildInfo;    CHECK_NOTNULL(func);
    return func(program, device, param_name, param_value_size, param_value, param_value_size_ret);
}

//clRetainProgram wrapper, use OpenCLSymbols function.
cl_int clRetainProgram(cl_program program) {
    auto func = ocl::OpenCLSymbols::get_instance()->clRetainProgram;    CHECK_NOTNULL(func);
    return func(program);
}

//clReleaseProgram wrapper, use OpenCLSymbols function.
cl_int clReleaseProgram(cl_program program) {
    auto func = ocl::OpenCLSymbols::get_instance()->clReleaseProgram;    CHECK_NOTNULL(func);
    return func(program);
}

//clBuildProgram wrapper, use OpenCLSymbols function.
cl_int clBuildProgram(cl_program program, cl_uint num_devices, const cl_device_id *device_list, const char *options,
                      void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), void *user_data) {
    auto func = ocl::OpenCLSymbols::get_instance()->clBuildProgram;    CHECK_NOTNULL(func);
    return func(program, num_devices, device_list, options, pfn_notify, user_data);
}

//clCreateKernel wrapper, use OpenCLSymbols function.
cl_kernel clCreateKernel(cl_program program, const char *kernelName, cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clCreateKernel;    CHECK_NOTNULL(func);
    return func(program, kernelName, errcode_ret);
}

//clRetainKernel wrapper, use OpenCLSymbols function.
cl_int clRetainKernel(cl_kernel kernel) {
    auto func = ocl::OpenCLSymbols::get_instance()->clRetainKernel;    CHECK_NOTNULL(func);
    return func(kernel);
}

//clReleaseKernel wrapper, use OpenCLSymbols function.
cl_int clReleaseKernel(cl_kernel kernel) {
    auto func = ocl::OpenCLSymbols::get_instance()->clReleaseKernel;    CHECK_NOTNULL(func);
    return func(kernel);
}

//clSetKernelArg wrapper, use OpenCLSymbols function.
cl_int clSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value) {
    auto func = ocl::OpenCLSymbols::get_instance()->clSetKernelArg;    CHECK_NOTNULL(func);
    return func(kernel, arg_index, arg_size, arg_value);
}

//clCreateBuffer wrapper, use OpenCLSymbols function.
cl_mem clCreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void *host_ptr, cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clCreateBuffer;    CHECK_NOTNULL(func);
    return func(context, flags, size, host_ptr, errcode_ret);
}

//clRetainMemObject wrapper, use OpenCLSymbols function.
cl_int clRetainMemObject(cl_mem memobj) {
    auto func = ocl::OpenCLSymbols::get_instance()->clRetainMemObject;    CHECK_NOTNULL(func);
    return func(memobj);
}

//clReleaseMemObject wrapper, use OpenCLSymbols function.
cl_int clReleaseMemObject(cl_mem memobj) {
    auto func = ocl::OpenCLSymbols::get_instance()->clReleaseMemObject;    CHECK_NOTNULL(func);
    return func(memobj);
}

//clGetImageInfo wrapper, use OpenCLSymbols function.
cl_int clGetImageInfo(cl_mem image, cl_image_info param_name, size_t param_value_size, void *param_value,
                      size_t *param_value_size_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetImageInfo;    CHECK_NOTNULL(func);
    return func(image, param_name, param_value_size, param_value, param_value_size_ret);
}

//clRetainCommandQueue wrapper, use OpenCLSymbols function.
cl_int clRetainCommandQueue(cl_command_queue command_queue) {
    auto func = ocl::OpenCLSymbols::get_instance()->clRetainCommandQueue;    CHECK_NOTNULL(func);
    return func(command_queue);
}

//clReleaseCommandQueue wrapper, use OpenCLSymbols function.
cl_int clReleaseCommandQueue(cl_command_queue command_queue) {
    auto func = ocl::OpenCLSymbols::get_instance()->clReleaseCommandQueue;    CHECK_NOTNULL(func);
    return func(command_queue);
}

//clEnqueueReadBuffer wrapper, use OpenCLSymbols function.
cl_int clEnqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset,
                           size_t size, void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                           cl_event *event) {
    auto func = ocl::OpenCLSymbols::get_instance()->clEnqueueReadBuffer;    CHECK_NOTNULL(func);
    return func(command_queue, buffer, blocking_read, offset, size, ptr, num_events_in_wait_list, event_wait_list,
                event);
}

//clEnqueueWriteBuffer wrapper, use OpenCLSymbols function.
cl_int clEnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset,
                            size_t size, const void *ptr, cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list, cl_event *event) {
    auto func = ocl::OpenCLSymbols::get_instance()->clEnqueueWriteBuffer;    CHECK_NOTNULL(func);
    return func(command_queue, buffer, blocking_write, offset, size, ptr, num_events_in_wait_list, event_wait_list,
                event);
}

//clEnqueueMapBuffer wrapper, use OpenCLSymbols function.
void *clEnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map, cl_map_flags map_flags,
                         size_t offset, size_t size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                         cl_event *event, cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clEnqueueMapBuffer;    CHECK_NOTNULL(func);
    return func(command_queue, buffer, blocking_map, map_flags, offset, size, num_events_in_wait_list, event_wait_list,
                event, errcode_ret);
}

//clEnqueueMapImage wrapper, use OpenCLSymbols function.
void *clEnqueueMapImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_map, cl_map_flags map_flags,
                        const size_t *origin, const size_t *region, size_t *image_row_pitch, size_t *image_slice_pitch,
                        cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event,
                        cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clEnqueueMapImage;    CHECK_NOTNULL(func);
    return func(command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch,
                num_events_in_wait_list, event_wait_list, event, errcode_ret);
}

//clEnqueueUnmapMemObject wrapper, use OpenCLSymbols function.
cl_int clEnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj, void *mapped_ptr,
                               cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) {
    auto func = ocl::OpenCLSymbols::get_instance()->clEnqueueUnmapMemObject;    CHECK_NOTNULL(func);
    return func(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);
}

//clGetKernelWorkGroupInfo wrapper, use OpenCLSymbols function.
cl_int clGetKernelWorkGroupInfo(cl_kernel kernel, cl_device_id device, cl_kernel_work_group_info param_name,
                                size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetKernelWorkGroupInfo;    CHECK_NOTNULL(func);
    return func(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
}

//clGetEventProfilingInfo wrapper, use OpenCLSymbols function.
cl_int clGetEventProfilingInfo(cl_event event, cl_profiling_info param_name, size_t param_value_size, void *param_value,
                               size_t *param_value_size_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetEventProfilingInfo;    CHECK_NOTNULL(func);
    return func(event, param_name, param_value_size, param_value, param_value_size_ret);
}

//clEnqueueNDRangeKernel wrapper, use OpenCLSymbols function.
cl_int clEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim,
                              const size_t *global_work_offset, const size_t *global_work_size,
                              const size_t *local_work_size, cl_uint num_events_in_wait_list,
                              const cl_event *event_wait_list, cl_event *event) {
    auto func = ocl::OpenCLSymbols::get_instance()->clEnqueueNDRangeKernel;    CHECK_NOTNULL(func);
    return func(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size,
                num_events_in_wait_list, event_wait_list, event);
}

//clWaitForEvents wrapper, use OpenCLSymbols function.
cl_int clWaitForEvents(cl_uint num_events, const cl_event *event_list) {
    auto func = ocl::OpenCLSymbols::get_instance()->clWaitForEvents;    CHECK_NOTNULL(func);
    return func(num_events, event_list);
}

//clRetainEvent wrapper, use OpenCLSymbols function.
cl_int clRetainEvent(cl_event event) {
    auto func = ocl::OpenCLSymbols::get_instance()->clRetainEvent;    CHECK_NOTNULL(func);
    return func(event);
}

//clReleaseEvent wrapper, use OpenCLSymbols function.
cl_int clReleaseEvent(cl_event event) {
    auto func = ocl::OpenCLSymbols::get_instance()->clReleaseEvent;    CHECK_NOTNULL(func);
    return func(event);
}

//clGetEventInfo wrapper, use OpenCLSymbols function.
cl_int clGetEventInfo(cl_event event, cl_event_info param_name, size_t param_value_size, void *param_value,
                      size_t *param_value_size_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetEventInfo;    CHECK_NOTNULL(func);
    return func(event, param_name, param_value_size, param_value, param_value_size_ret);
}

//clFlush wrapper, use OpenCLSymbols function.
cl_int clFlush(cl_command_queue command_queue) {
    auto func = ocl::OpenCLSymbols::get_instance()->clFlush;    CHECK_NOTNULL(func);
    return func(command_queue);
}

//clFinish wrapper, use OpenCLSymbols function.
cl_int clFinish(cl_command_queue command_queue) {
    auto func = ocl::OpenCLSymbols::get_instance()->clFinish;    CHECK_NOTNULL(func);
    return func(command_queue);
}

//clCreateImage2D wrapper, use OpenCLSymbols function.
cl_mem clCreateImage2D(cl_context context, cl_mem_flags flags, const cl_image_format *image_format, size_t imageWidth,
                       size_t imageHeight, size_t image_row_pitch, void *host_ptr, cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clCreateImage2D;    CHECK_NOTNULL(func);
    return func(context, flags, image_format, imageWidth, imageHeight, image_row_pitch, host_ptr, errcode_ret);
}

//clCreateImage3D wrapper, use OpenCLSymbols function.
cl_mem clCreateImage3D(cl_context context, cl_mem_flags flags, const cl_image_format *image_format, size_t imageWidth,
                       size_t imageHeight, size_t imageDepth, size_t image_row_pitch, size_t image_slice_pitch,
                       void *host_ptr, cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clCreateImage3D;    CHECK_NOTNULL(func);
    return func(context, flags, image_format, imageWidth, imageHeight, imageDepth, image_row_pitch, image_slice_pitch,
                host_ptr, errcode_ret);
}

//clCreateCommandQueue wrapper, use OpenCLSymbols function.
cl_command_queue clCreateCommandQueue(cl_context context, cl_device_id device, cl_command_queue_properties properties,
                                      cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clCreateCommandQueue;    CHECK_NOTNULL(func);
    return func(context, device, properties, errcode_ret);
}

//clGetCommandQueueInfo wrapper, use OpenCLSymbols function.
cl_int clGetCommandQueueInfo(cl_command_queue command_queue, cl_command_queue_info param_name, size_t param_value_size,
                             void *param_value, size_t *param_value_size_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetCommandQueueInfo;    CHECK_NOTNULL(func);
    return func(command_queue, param_name, param_value_size, param_value, param_value_size_ret);
}

//clEnqueueCopyImage wrapper, use OpenCLSymbols function.
cl_int clEnqueueCopyImage(cl_command_queue queue, cl_mem src_image, cl_mem dst_image, const size_t *src_origin,
                          const size_t *dst_origin, const size_t *region, cl_uint num_events_in_wait_list,
                          const cl_event *event_wait_list, cl_event *event) {
    auto func = ocl::OpenCLSymbols::get_instance()->clEnqueueCopyImage;    CHECK_NOTNULL(func);
    return func(queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list,
                event);
}

//clEnqueueCopyBufferToImage wrapper, use OpenCLSymbols function.
cl_int clEnqueueCopyBufferToImage(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image,
                                  size_t src_offset, const size_t *dst_origin, const size_t *region,
                                  cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) {
    auto func = ocl::OpenCLSymbols::get_instance()->clEnqueueCopyBufferToImage;    CHECK_NOTNULL(func);
    return func(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list,
                event_wait_list, event);
}

//clEnqueueCopyImageToBuffer wrapper, use OpenCLSymbols function.
cl_int clEnqueueCopyImageToBuffer(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer,
                                  const size_t *src_origin, const size_t *region, size_t dst_offset,
                                  cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) {
    auto func = ocl::OpenCLSymbols::get_instance()->clEnqueueCopyImageToBuffer;    CHECK_NOTNULL(func);
    return func(command_queue, src_image, dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list,
                event_wait_list, event);
}

#if CL_HPP_TARGET_OPENCL_VERSION >= 120

//clRetainDevice wrapper, use OpenCLSymbols function.
cl_int clRetainDevice(cl_device_id device) {
    auto func = ocl::OpenCLSymbols::get_instance()->clRetainDevice;    CHECK_NOTNULL(func);
    return func(device);
}

//clReleaseDevice wrapper, use OpenCLSymbols function.
cl_int clReleaseDevice(cl_device_id device) {
    auto func = ocl::OpenCLSymbols::get_instance()->clReleaseDevice;    CHECK_NOTNULL(func);
    return func(device);
}

//clCreateImage wrapper, use OpenCLSymbols function.
cl_mem clCreateImage(cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
                     const cl_image_desc *image_desc, void *host_ptr, cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clCreateImage;    CHECK_NOTNULL(func);
    return func(context, flags, image_format, image_desc, host_ptr, errcode_ret);
}

#endif

#if CL_HPP_TARGET_OPENCL_VERSION >= 200

//clGetKernelSubGroupInfoKHR wrapper, use OpenCLSymbols function.
cl_int clGetKernelSubGroupInfoKHR(cl_kernel kernel, cl_device_id device, cl_kernel_sub_group_info param_name,
                                  size_t input_value_size, const void *input_value, size_t param_value_size,
                                  void *param_value, size_t *param_value_size_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetKernelSubGroupInfoKHR;    CHECK_NOTNULL(func);
    return func(kernel, device, param_name, input_value_size, input_value, param_value_size, param_value,
                param_value_size_ret);
}

//clCreateCommandQueueWithProperties wrapper, use OpenCLSymbols function.
cl_command_queue clCreateCommandQueueWithProperties(cl_context context, cl_device_id device,
                                                    const cl_queue_properties *properties, cl_int *errcode_ret) {
    auto func = ocl::OpenCLSymbols::get_instance()->clCreateCommandQueueWithProperties;    CHECK_NOTNULL(func);
    return func(context, device, properties, errcode_ret);
}

//clGetExtensionFunctionAddress wrapper, use OpenCLSymbols function.
void *clGetExtensionFunctionAddress(const char *func_name) {
    auto func = ocl::OpenCLSymbols::get_instance()->clGetExtensionFunctionAddress;    CHECK_NOTNULL(func);
    return func(func_name);
}
#endif
