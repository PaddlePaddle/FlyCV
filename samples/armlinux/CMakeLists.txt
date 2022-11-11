cmake_minimum_required(VERSION 3.10)
project(flycv_demo)

include(cmake/armlinux.cmake)

if(BUILD_SHARED_LIBS)
    set(FLYCV_LIB_NAME "flycv_shared")
else()
    set(FLYCV_LIB_NAME "flycv_static")
endif()

macro(get_subdir_list result parent_dir)
    file(GLOB children RELATIVE ${parent_dir} ${parent_dir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${parent_dir}/${child})
            list(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

set(FCV_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules)
set(FCV_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules)
get_subdir_list(TEST_FCV_BUILD_MODULES ${FCV_MODULES_DIR})

foreach(module_name ${TEST_FCV_BUILD_MODULES})
    if(EXISTS "${FCV_TEST_DIR}/${module_name}/${module_name}_demo.cpp")
        list(APPEND test_sources "${FCV_TEST_DIR}/${module_name}/${module_name}_demo.cpp")
    else()
        get_subdir_list(TEST_FCV_WITH_MODULES ${FCV_MODULES_DIR}/${module_name})
        foreach(submodule_name ${TEST_FCV_WITH_MODULES})
            if(EXISTS "${FCV_TEST_DIR}/${module_name}/${submodule_name}/${submodule_name}_demo.cpp")
                list(APPEND test_sources "${FCV_TEST_DIR}/${module_name}/${submodule_name}/${submodule_name}_demo.cpp")
            endif()
        endforeach()
    endif()
endforeach()

file(GLOB_RECURSE util_sources ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp)

foreach(cpp_name ${test_sources})
    string(FIND ${cpp_name} ".cpp" last_index REVERSE)
    string(FIND ${cpp_name} "/" first_index REVERSE)
    math(EXPR s_len "${last_index} - ${first_index} - 1")
    math(EXPR first_index "${first_index} + 1")
    string(SUBSTRING ${cpp_name} ${first_index} ${s_len} demo_name)

    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../flycv/aarch64)

    add_executable(${demo_name}
        ${cpp_name}
        ${util_sources}
    )

    target_include_directories(${demo_name} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/../../flycv/include
    )

    target_link_libraries(${demo_name}
        ${FLYCV_LIB_NAME}
    )
endforeach()