"""
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
"""

import json
import os
import sys
import re
from datetime import datetime
from jinja2 import FileSystemLoader, Environment

TPL_SEARCH_PATH = 'tools/add_module'
MODULE_TPL_CPP_H_FILE = 'module.h.template'
MODULE_TPL_CPP_SOURCE_FILE = 'module.cpp.template'
MODULE_TPL_COMMON_H_FILE = 'module_common.h.template'
MODULE_TPL_COMMON_CPP_FILE = 'module_common.cpp.template'
MODULE_TPL_C_H_FILE = 'module_c.h.template'
MODULE_TPL_C_SOURCE_FILE = 'module_c.cpp.template'
MODULE_TPL_GTEST_CPP_FILE = 'module_gtest.cpp.template'
MODULE_TPL_BENCHMARK_CPP_FILE = 'module_benchmark.cpp.template'
BUILD_LIST_TPL = 'build_list.cmake.template'
FCV_H_TPL = 'flycv.h.template'
BUILD_LIST_OUTPUT_PATH = 'cmake/FCVBuildList.cmake'
FCV_H_OUTPUT_PATH = 'include/flycv.h.in'
FCV_GTEST_C_PATH = 'tests/c'
FCV_GTEST_CPP_PATH = 'tests/cpp'
FCV_BENCHMARK_PATH = 'benchmark/'


def print_usage():
    print('This tool is used to add new modules.')
    print('Usage:')
    print('    python3 tools/%s modules/<parent_module>/<child_module> <ON|OFF>' % os.path.split(__file__)[-1])
    print('')
    print('Example:')
    print('    command: python3 tools/add_module/%s modules/img_transform/resize ON' % os.path.split(__file__)[-1])
    print('    brief:   The command will create img_transform/resize directory in the\n'
          '             modules folder of the project root directory, and will be \n'
          '             compiled by default. If you expect this module not to be \n'
          '             compiled by default, the last parameter can be set to OFF.')

def parse_argv(argv):
    argv_len = len(argv)

    if argv_len < 2:
        print_usage()
        return '', '', ''

    is_enable = ''

    if argv_len >= 3:
        is_enable = sys.argv[2]

    if is_enable.upper() != 'OFF':
        is_enable = 'ON'

    dir_str = sys.argv[1].strip('/').lower().split('/')
    dir_len = len(dir_str)

    if dir_len < 3:
        print("The parent or child module is empty!")
        return '', '', ''

    return dir_str[1], dir_str[2], is_enable

def mkdir(path):
    if os.path.exists(path):
        print("The path is already exists: %s" % path)
        return False
    else:
        os.makedirs(path)
        return True

def get_sub_dir_list(path):
    search_list = os.listdir(path)
    dir_list = []
    for item in search_list:
        if os.path.isdir(path + '/' + item):
            dir_list.append(item)
    return dir_list

def get_sub_file_list(path):
    search_list = os.listdir(path)
    file_list = []
    for item in search_list:
        file_path = path + '/' + item
        if os.path.isfile(file_path):
            res = re.search(r'\w*_c\.h$', file_path)
            if not res:
                file_list.append(item)
    return file_list

def get_default_config(path):
    f = open(path)
    lines = f.readlines()
    build_list = []
    is_last_func = False
    cnt = -1

    for line in lines:
        line = line.strip()
        if len(line) != 0:
            regex = re.compile('BUILD_FCV_\w*')
            res = regex.search(line)
            if res:
                option = line.split(' ')[-1].strip(')')
                build_list.append({
                    'name': res.group(0).replace('BUILD_FCV_', '').lower(),
                    'items': [],
                    'default_option': option
                })
                cnt = cnt + 1
            else:
                regex = re.compile('WITH_FCV_\w*')
                res = regex.search(line)
                if res:
                    option = line.split(' ')[-1].strip(')')
                    build_list[cnt]['items'].append({
                        'name': res.group(0).replace('WITH_FCV_', '').lower(),
                        'default_option': option,
                        'file_list': []
                    })

    return build_list

def get_class_name(name):
    name_arr = name.split(' ')
    class_name = ''

    for item in name_arr:
        class_name = class_name + item.capitalize()

    return class_name


def add_modules(path, parent, name):
    ret = mkdir(path)
    if not ret:
        return False

    ret = mkdir(path + '/interface')
    if not ret:
        return False

    ret = mkdir(path + '/src')
    if not ret:
        return False

    ret = mkdir(path + '/include')
    if not ret:
        return False

    class_name = get_class_name(name)

    data = {
        'c_out_header_file': parent + '/' + name + '/interface/' + name + '_c.h',
        'cpp_out_header_file': parent + '/' + name + '/interface/' + name + '.h',
        'cpp_in_header_file': parent + '/' + name + '/include/' + name + '_common.h',
        'class_name': class_name,
        'name': name,
    }

    templateLoader = FileSystemLoader(searchpath=TPL_SEARCH_PATH)
    templateEnv = Environment(loader=templateLoader)

    # generate cpp templates
    template = templateEnv.get_template(MODULE_TPL_CPP_H_FILE)
    with open(path + '/interface/' + name + '.h', 'w') as output_file:
        output_file.write(template.render(data=data))

    template = templateEnv.get_template(MODULE_TPL_CPP_SOURCE_FILE)
    with open(path + '/src/' + name + '.cpp', 'w') as output_file:
        output_file.write(template.render(data=data))

    template = templateEnv.get_template(MODULE_TPL_COMMON_H_FILE)
    with open(path + '/include/' + name + '_common.h', 'w') as output_file:
        output_file.write(template.render(data=data))

    template = templateEnv.get_template(MODULE_TPL_COMMON_CPP_FILE)
    with open(path + '/src/' + name + '_common.cpp', 'w') as output_file:
        output_file.write(template.render(data=data))

    #generate c templates
    template = templateEnv.get_template(MODULE_TPL_C_H_FILE)
    with open(path + '/interface/' + name + '_c.h', 'w') as output_file:
        output_file.write(template.render(data=data))

    template = templateEnv.get_template(MODULE_TPL_C_SOURCE_FILE)
    with open(path + '/src/' + name + '_c.cpp', 'w') as output_file:
        output_file.write(template.render(data=data))

    # generate gtest template
    template = templateEnv.get_template(MODULE_TPL_GTEST_CPP_FILE)
    with open(FCV_GTEST_CPP_PATH + '/' +  path + '_test.cpp', 'w') as output_file:
        output_file.write(template.render(data=data))

    # generate benchmark template
    template = templateEnv.get_template(MODULE_TPL_BENCHMARK_CPP_FILE)
    with open(FCV_BENCHMARK_PATH + '/' +  path + '_bench.cpp', 'w') as output_file:
        output_file.write(template.render(data=data))

    return True

def update_modules(is_enable):
    build_list = get_default_config(BUILD_LIST_OUTPUT_PATH)
    parent_module_list = get_sub_dir_list('modules')

    for item in parent_module_list:
        is_exist = False
        for m in build_list:
            if m['name'] == item:
                is_exist = True
                break
        if is_exist:
            continue
        build_list.append({
            'name': item,
            'items': [],
            'default_option': is_enable
        })

    for item in build_list:
        child_list = get_sub_dir_list('modules/' + item['name'])
        for child in child_list:
            is_exist = False
            for m in item['items']:
                if m['name'] == child:
                    is_exist = True
                    break
            if is_exist:
                continue
            item['items'].append({
                'name': child,
                'default_option': is_enable
            })

    for item in build_list:
        for child in item['items']:
            interface_path = 'modules/' + item['name'] + '/' + child['name'] + '/interface'
            child['file_list'] = get_sub_file_list(interface_path)

    templateLoader = FileSystemLoader(searchpath=TPL_SEARCH_PATH)
    templateEnv = Environment(loader=templateLoader)
    template = templateEnv.get_template(BUILD_LIST_TPL)
    with open(BUILD_LIST_OUTPUT_PATH, 'w') as output_file:
        output_file.write(template.render(build_list=build_list))

    template = templateEnv.get_template(FCV_H_TPL)
    with open(FCV_H_OUTPUT_PATH, 'w') as output_file:
        output_file.write(template.render(build_list=build_list))
    return True

if __name__ == "__main__":
    parent_module, child_module, is_enable = parse_argv(sys.argv)

    if parent_module == '' or child_module == '':
        sys.exit()

    module_path = 'modules/' +  parent_module.strip('/') + '/' + child_module.strip('/')

    ret = add_modules(module_path, parent_module, child_module)

    if not ret:
        print('Failed to add new module: %s !' % module_path)
        sys.exit()

    ret = update_modules(is_enable)
    print('You successfully added module: ', sys.argv[1])
