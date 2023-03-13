if (BUILD_C)
    add_definitions(-DUSE_C_API)
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_INSTALL_PREFIX}/flycv/include
)

get_subdir_list(FCV_ALL_MODULE_LIST ${CMAKE_CURRENT_SOURCE_DIR})

foreach(module_name ${FCV_ALL_MODULE_LIST})
    string(TOUPPER ${module_name} _name)
    if (${BUILD_FCV_${_name}})
        list(APPEND FCV_BUILD_MODULE_LIST ${module_name})
    endif()
endforeach()

foreach(module_name ${FCV_BUILD_MODULE_LIST})
    get_subdir_list(FCV_WITH_ALL_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/${module_name})
    foreach(submodule_name ${FCV_WITH_ALL_MODULES})
        string(TOUPPER ${submodule_name} _name)
        if (${WITH_FCV_${_name}})
            fcv_collect_module(${module_name}/${submodule_name})
        endif()
    endforeach()
endforeach()

set(FCV_BUILD_MODULE_LIST ${FCV_BUILD_MODULE_LIST} PARENT_SCOPE)

if(BUILD_JS)
    set(FCV_JS_TEMP_NAME flycv_temp)
    set(FCV_JS_NAME "flycv.js")
    set(FCV_POST_JS "${CMAKE_CURRENT_SOURCE_DIR}/js/utils.js")
    add_executable(${FCV_JS_TEMP_NAME} ${FCV_UNIVERSE_SRCS})

    set(EMSCRIPTEN_LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS} --memory-init-file 0 \
            -s EXPORT_NAME=fcv \
            -s TOTAL_MEMORY=128MB \
            -s WASM_MEM_MAX=1GB \
            -s ALLOW_MEMORY_GROWTH=1 \
            -s MODULARIZE=1 \
            -s SINGLE_FILE=1 \
            -s FORCE_FILESYSTEM=1 \
            --use-preload-plugins --bind \
            --post-js ${FCV_POST_JS}")

    set_target_properties(${FCV_JS_TEMP_NAME}
            PROPERTIES LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS}")

    set(MODULE_JS_PATH ${CMAKE_BINARY_DIR}/modules/${FCV_JS_TEMP_NAME}.js)
    set(OCV_JS_PATH ${CMAKE_INSTALL_PREFIX}/flycv/${FCV_JS_NAME})

    add_custom_command(
        OUTPUT ${OCV_JS_PATH}
        COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/js/make_umd.py" ${MODULE_JS_PATH} "${OCV_JS_PATH}"
        DEPENDS ${FCV_JS_TEMP_NAME}
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/js/make_umd.py")

    add_custom_target(${FCV_JS_NAME} ALL
        DEPENDS ${OCV_JS_PATH}
        DEPENDS ${FCV_JS_TEMP_NAME})
    return()
endif()

# generate flycv library.
add_library(${FCV_TARGET_NAME}
    ${FCV_UNIVERSE_SRCS}
)

target_include_directories(
    ${FCV_TARGET_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(
    ${FCV_TARGET_NAME}
    ${FCV_LINK_DEPS}
)

foreach(item ${FCV_EXTERNAL_DEPS})
    add_dependencies(${FCV_TARGET_NAME} ${item})
endforeach()

set_target_properties(${FCV_TARGET_NAME}
    PROPERTIES
    OUTPUT_NAME
    ${FCV_TARGET_NAME}
    ${FCV_TARGET_PROPERTIES}
)

if(WIN32)
    create_filters(${PROJECT_SOURCE_DIR}/modules)
endif()

if(WIN32)
    target_compile_definitions(${FCV_TARGET_NAME}
        PUBLIC -DUSE_FCV_DLL
        PUBLIC -DFCV_DLL_EXPORT
    )
endif()

# install flycv libraries with header files.
foreach(each_interface ${FCV_UNIVERSE_INTERFACES})
    string(REGEX MATCH "\.*\/+" OUTPUT_INTERFACE_PATH ${each_interface})
    install(FILES
        ${CMAKE_SOURCE_DIR}/${each_interface}
        DESTINATION
        ${CMAKE_INSTALL_PREFIX}/flycv/include/${OUTPUT_INTERFACE_PATH}
    )
endforeach()

install(TARGETS ${FCV_TARGET_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/flycv/${FCV_INSTALL_LIB_NAME})

if(BUILD_SHARED_LIBS AND (BUILD_TEST OR BUILD_BENCHMARK))
    install(TARGETS ${FCV_TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()

foreach(item ${FCV_EXPORT_LIBS})
    string(REGEX MATCH "\/" BLACKSLASH ${item})
    if ("${BLACKSLASH}" STREQUAL "")
        install(TARGETS ${item}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/flycv/${FCV_INSTALL_LIB_NAME})
    else()
        install(FILES ${item}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/flycv/${FCV_INSTALL_LIB_NAME})
    endif()
endforeach()