if (CMAKE_SYSTEM_PROCESSOR STREQUAL arm 
        OR CMAKE_SYSTEM_PROCESSOR STREQUAL armhf
        OR CMAKE_SYSTEM_PROCESSOR STREQUAL armel
        OR CMAKE_SYSTEM_PROCESSOR STREQUAL aarch64)
    install(DIRECTORY armlinux/modules DESTINATION ${CMAKE_INSTALL_PREFIX}/samples/armlinux)
    install(FILES armlinux/CMakeLists.txt DESTINATION ${CMAKE_INSTALL_PREFIX}/samples/armlinux/)

    if(BUILD_SHARED_LIBS)
        set(SWITCH_LIB "ON")
    else()
        set(SWITCH_LIB "OFF")
    endif()

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/armlinux/scripts/build.sh.in"
        "${CMAKE_INSTALL_PREFIX}/samples/armlinux/scripts/build.sh" @ONLY)

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../tests/utils 
            DESTINATION ${CMAKE_INSTALL_PREFIX}/samples/armlinux)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/platform/armlinux/armlinux.cmake
            DESTINATION ${CMAKE_INSTALL_PREFIX}/samples/armlinux/cmake)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/platform/armlinux/toolchain/armlinux.toolchain.cmake
            DESTINATION ${CMAKE_INSTALL_PREFIX}/samples/armlinux/cmake)
endif()