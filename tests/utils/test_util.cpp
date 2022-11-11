// Copyright (c) 2021 FlyCV Authors. All Rights Reserved.
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

#include "test_util.h"

#include <iostream>
#include <fstream>
#include <algorithm>

int read_binary_file(
        const std::string& file_path,
        void* buffer,
        long length) {
    std::ifstream ifs(file_path, std::ios::binary);

    if (!ifs) {
        std::cout << "Cannot read file: " << file_path << std::endl;
        return -1;
    }

    long file_len = 0;

    ifs.seekg(0, std::ios::end);
    file_len = ifs.tellg();
    ifs.seekg (0, std::ios::beg);

    ifs.read((char*)buffer, std::min(file_len, length));
    ifs.close();

    return 0;
}

int write_binary_file(
        const std::string& file_path,
        void* buffer,
        long length) {
    std::ofstream ofs(file_path.c_str(), std::ios::binary);

    if (!ofs) {
        printf("Cannot write file: %s", file_path.c_str());
        return -1;
    }

    ofs.write((char*)buffer, length);
    ofs.close();

    return 0;
}
