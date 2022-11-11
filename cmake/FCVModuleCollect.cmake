#[[
@name    fcv_collect_module()
@brief   collect source files、include directories from current directory recursively,
         and pass them to parent project by variable FCV_UNIVERSE_INCLUDE_DIRS
         with FCV_UNIVERSE_SRCS.
]]

macro(fcv_collect_module _module_path)
    file(GLOB_RECURSE lib_headers
        ${CMAKE_CURRENT_LIST_DIR}/${_module_path}/interface/*.h
        ${CMAKE_CURRENT_LIST_DIR}/${_module_path}/interface/*.hpp
        )

    file(GLOB_RECURSE lib_sources
        ${CMAKE_CURRENT_LIST_DIR}/${_module_path}/src/*.c
        ${CMAKE_CURRENT_LIST_DIR}/${_module_path}/src/*.cpp
        )

    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/${_module_path}/interface
        ${CMAKE_CURRENT_SOURCE_DIR}/${_module_path}/include
        )

    # cpu instructions optimize
    if(NOT ENABLE_NEON)
        list(FILTER lib_sources EXCLUDE REGEX ".+_arm\.cpp")
    endif()

    if(NOT ENABLE_SVE2)
        list(FILTER lib_sources EXCLUDE REGEX ".+_sve\.cpp")
    endif()

    if(NOT ENABLE_SSE)
        list(FILTER lib_sources EXCLUDE REGEX ".+_sse\.cpp")
    endif()

    if(NOT ENABLE_AVX)
        list(FILTER lib_sources EXCLUDE REGEX ".+_avx\.cpp")
    endif()

    # heterogeneous computing support
    if(NOT WITH_FCV_OPENCL)
        list(FILTER lib_sources EXCLUDE REGEX ".+_ocl.cpp")
    endif()

    if(NOT BUILD_RV1109)
        list(FILTER lib_sources EXCLUDE REGEX ".+_rv1109\.cpp")
    endif()

    set(FCV_UNIVERSE_SRCS "" PARENT_SCOPE)
    list(APPEND FCV_UNIVERSE_SRCS ${lib_sources})

    set(FCV_UNIVERSE_INTERFACES "" PARENT_SCOPE)

    foreach(item ${lib_headers})
        string(REGEX REPLACE "${CMAKE_CURRENT_LIST_DIR}" "" MODULE_RELATIVE_PATH ${item})
        list(APPEND FCV_UNIVERSE_INTERFACES "modules/${MODULE_RELATIVE_PATH}")
    endforeach()
endmacro()
