project(flycv_bench)

# set benchs project
set(TARGET_NAME flycv_bench)
set(bench_sources "")
set(FCV_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../modules)
set(FCV_BENCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules)

get_subdir_list(BENCH_FCV_BUILD_MODULES ${FCV_MODULES_DIR})

foreach(module_name ${BENCH_FCV_BUILD_MODULES})
    string(TOUPPER ${module_name} _name)
    if(${BUILD_FCV_${_name}})
        if(EXISTS "${FCV_BENCH_DIR}/${module_name}/${module_name}_bench.cpp")
            list(APPEND bench_sources "${FCV_BENCH_DIR}/${module_name}/${module_name}_bench.cpp")
        else()
            get_subdir_list(BENCH_FCV_WITH_MODULES ${FCV_MODULES_DIR}/${module_name})
            foreach(submodule_name ${BENCH_FCV_WITH_MODULES})
                string(TOUPPER ${submodule_name} _subname)
                if(${WITH_FCV_${_subname}})
                    if(EXISTS "${FCV_BENCH_DIR}/${module_name}/${submodule_name}_bench.cpp")
                        list(APPEND bench_sources "${FCV_BENCH_DIR}/${module_name}/${submodule_name}_bench.cpp")
                    endif()
                endif()
            endforeach()
        endif()
    endif()
endforeach()

add_executable(${TARGET_NAME}
    ${bench_sources}
)

target_include_directories(${TARGET_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/
    ${BENCHMARK_INCLUDE_DIR}
    ${CMAKE_INSTALL_PREFIX}/flycv/include
)

target_link_libraries(${TARGET_NAME}
    ${FCV_TARGET_NAME}
    ${BENCHMARK_LIBRARIES}
    ${FCV_EXPORT_LIBS}
    ${CMAKE_DL_LIBS}
)

install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
