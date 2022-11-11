project(flycv_test)

# set tests project
set(TARGET_NAME flycv_test)
set(test_sources "")
set(FCV_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../modules)
set(FCV_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules)

get_subdir_list(TEST_FCV_BUILD_MODULES ${FCV_MODULES_DIR})

foreach(module_name ${TEST_FCV_BUILD_MODULES})
    string(TOUPPER ${module_name} _name)
    if(${BUILD_FCV_${_name}})
        if(EXISTS "${FCV_TEST_DIR}/${module_name}/${module_name}_test.cpp")
            list(APPEND test_sources "${FCV_TEST_DIR}/${module_name}/${module_name}_test.cpp")
        else()
            get_subdir_list(TEST_FCV_WITH_MODULES ${FCV_MODULES_DIR}/${module_name})
            foreach(submodule_name ${TEST_FCV_WITH_MODULES})
                string(TOUPPER ${submodule_name} _subname)
                if(${WITH_FCV_${_subname}})
                    if(EXISTS "${FCV_TEST_DIR}/${module_name}/${submodule_name}_test.cpp")
                        list(APPEND test_sources "${FCV_TEST_DIR}/${module_name}/${submodule_name}_test.cpp")
                    endif()
                endif()
            endforeach()
        endif()
    endif()
endforeach()

file(GLOB_RECURSE util_sources ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp)
list(APPEND test_sources ${util_sources})

add_executable(${TARGET_NAME}
    ${test_sources}
)

target_include_directories(${TARGET_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/
    ${GTEST_INCLUDE_DIR}
    ${CMAKE_INSTALL_PREFIX}/flycv/include
)

target_link_libraries(${TARGET_NAME}
    ${FCV_TARGET_NAME}
    ${GTEST_LIBRARIES}
    ${FCV_EXPORT_LIBS}
    ${CMAKE_DL_LIBS}
)

install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
