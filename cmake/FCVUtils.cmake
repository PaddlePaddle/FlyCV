#[[
    @brief  Download dependency library from git repository.
    @param  url - git repository address
    @param  branch - the branch or tag name
    @param  lib_name - the expected local name
]]
function(fcv_download_dependency url branch lib_name work_directory)
    execute_process(COMMAND bash "-c" "if [ ! -d ${work_directory} ];\
        then mkdir -p ${work_directory}; fi")

    execute_process(COMMAND bash "-c" "if [ ! -d ${lib_name} ]; then \
        git clone --progress -q --depth=1 -b ${branch} ${url} ${lib_name}; fi"
        WORKING_DIRECTORY ${work_directory})
    #execute_process(COMMAND bash "-c" "if [ -d ${lib_name} ];then cd ${lib_name}; \
    #    if [[ `git rev-parse --abbrev-ref HEAD` != ${branch} && `git tag` != ${branch} ]];then cd .. && rm -rf \
    #    ${lib_name} && git clone --depth=1 -b ${branch} ${url} ${libname}; fi else
    #    git clone --depth=1 -b ${branch} ${url} ${libname}; fi"
    #    WORKING_DIRECTORY ${work_directory})
endfunction()

#========================================================================================

#[[
    @brief  Get all subdirectory names of parent directory.
    @param  result - the result list of all subdirectory names
    @param  parent_dir - the given parent directory
]]
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

#========================================================================================

#[[
    @brief  Define message output function, colored and uncolored.
]]
# different color configuration
string(ASCII 27 Esc)
set(FcvColorReset "${Esc}[m")
set(FcvColorBold  "${Esc}[1m")
set(FcvRed         "${Esc}[31m")
set(FcvGreen       "${Esc}[32m")
set(FcvYellow      "${Esc}[33m")
set(FcvBlue        "${Esc}[34m")
set(FcvMagenta     "${Esc}[35m")
set(FcvCyan        "${Esc}[36m")
set(FcvWhite       "${Esc}[37m")
set(FcvBoldRed     "${Esc}[1;31m")
set(FcvBoldGreen   "${Esc}[1;32m")
set(FcvBoldYellow  "${Esc}[1;33m")
set(FcvBoldBlue    "${Esc}[1;34m")
set(FcvBoldMagenta "${Esc}[1;35m")
set(FcvBoldCyan    "${Esc}[1;36m")
set(FcvBoldWhite   "${Esc}[1;37m")

# print regular message to screen
function(fcv_status)
    message(STATUS ${ARGN})
endfunction()

# print error message to screen
function(fcv_error)
    message(FATAL_ERROR ${ARGN})
endfunction()

function(fcv_status_yellow)
    message(STATUS ${FcvGreen}${ARGN}${FcvColorReset})
endfunction()

function(fcv_status_purple)
    message(STATUS ${FcvBoldMagenta}${ARGN}${FcvColorReset})
endfunction()

function(fcv_error_red)
    message(STATUS ${Red}${ARGN}${ColorReset})
endfunction()

# silence the message
function(message)
    if (NOT MESSAGE_QUIET)
        _message(${ARGN})
    endif()
endfunction()

#========================================================================================

#[[
    @brief  Set the given variable with parent_scope if not defined.
    @param  name - the name of variable want to define
    @param  value - the value of the variable
]]
function(set_if_not_defined name value)
    if(NOT DEFINED ${name})
        set(${name} ${value} PARENT_SCOPE)
    endif()
endfunction()

#========================================================================================

#[[
    @brief  Construct the source tree (for windows visual studio)
]]
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

#[[
    @brief  Create source tree for the given directory (for windows visual studio)
    @param  search_dir - the root directory of creating filters
]]
function(create_filters search_dir)
    set(ALL_COLLECTED_FILES "")

    file(GLOB_RECURSE ALL_COLLECTED_FILES
        ${search_dir}/*.h
        ${search_dir}/*.hpp
        ${search_dir}/*.c
        ${search_dir}/*.cpp
        )

    foreach(_source IN ITEMS ${ALL_COLLECTED_FILES})
        assign_source_group(${_source})
    endforeach()
endfunction(create_filters)
