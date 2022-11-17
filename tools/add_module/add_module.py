# -*- coding: UTF-8 -*-
########################################################################
#
# Copyright (c) 2022 Baidu.com, Inc. All Rights Reserved
#
########################################################################

"""
author: taotianran
date:   2022-03-19 15:13
"""

import json
import os
import sys
import re
from datetime import datetime
from jinja2 import FileSystemLoader, Environment

TPL_SEARCH_PATH = 'tools/add_module'
MODULE_TPL_H_FILE = 'module.h.template'
MODULE_TPL_CPP_FILE = 'module.cpp.template'
MODULE_TPL_COMMON_H_FILE = 'module_common.h.template'
MODULE_TPL_COMMON_CPP_FILE = 'module_common.cpp.template'
BUILD_LIST_TPL = 'build_list.cmake.template'
FCV_H_TPL = 'flycv.h.template'
BUILD_LIST_OUTPUT_PATH = 'cmake/FCVBuildList.cmake'
FCV_H_OUTPUT_PATH = 'include/flycv.h.in'

def print_usage():
    print('This tool is used to add new modules.')
    print('Usage:')
    print('    python3 tools/%s modules/<parent_module>/<child_module>' % os.path.split(__file__)[-1])
    print('')
    print('Example:')
    print('    command: python3 tools/add_module/%s modules/img_transform/resize' % os.path.split(__file__)[-1])
    print('    brief:   The command will create img_transform/resize directory in the\n'
          '             modules folder of the project root directory.')

def parse_argv(argv):
    argv_len = len(argv)

    if argv_len < 2:
        print_usage()
        return '', ''

    dir_str = sys.argv[1].strip('/').lower().split('/')
    dir_len = len(dir_str)

    if dir_len < 3:
        return '', ''

    return dir_str[1], dir_str[2]

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
        if os.path.isfile(path + '/' + item):
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

    data = {
        'out_header_file': parent + '/' + name + '/interface/' + name + '.h',
        'in_header_file': parent + '/' + name + '/include/' + name + '_common.h'
    }

    templateLoader = FileSystemLoader(searchpath=TPL_SEARCH_PATH)
    templateEnv = Environment(loader=templateLoader)

    template = templateEnv.get_template(MODULE_TPL_H_FILE)
    with open(path + '/interface/' + name + '.h', 'w') as output_file:
        output_file.write(template.render(data=data))

    template = templateEnv.get_template(MODULE_TPL_CPP_FILE)
    with open(path + '/src/' + name + '.cpp', 'w') as output_file:
        output_file.write(template.render(data=data))

    template = templateEnv.get_template(MODULE_TPL_COMMON_H_FILE)
    with open(path + '/include/' + name + '_common.h', 'w') as output_file:
        output_file.write(template.render(data=data))

    template = templateEnv.get_template(MODULE_TPL_COMMON_CPP_FILE)
    with open(path + '/src/' + name + '_common.cpp', 'w') as output_file:
        output_file.write(template.render(data=data))

    return True

def update_modules():
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
            'default_option': 'ON'
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
                'default_option': 'ON'
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
    parent_module, child_module= parse_argv(sys.argv)

    if parent_module == '' or child_module == '':
        print("The parent_module or child_module is empty, failed to create modules!")
        sys.exit()

    module_path = 'modules/' +  parent_module.strip('/') + '/' + child_module.strip('/')

    ret = add_modules(module_path, parent_module, child_module)
    
    if not ret:
        print('Failed to add new module: %s !' % module_path)
        sys.exit()

    ret = update_modules()
    print('You successfully added module: ', sys.argv[1])
