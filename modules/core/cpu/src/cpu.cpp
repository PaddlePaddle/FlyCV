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

#include "modules/core/cpu/interface/cpu.h"

#include <iostream>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <thread>
#include <cstring>

#include "modules/core/base/include/macro_platforms.h"
#include "modules/core/base/include/macro_utils.h"
#include "modules/core/base/interface/log.h"

#if defined(FCV_OS_ANDROID) || defined(FCV_OS_LINUX)
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#elif defined FCV_OS_IPHONE
#include <sys/sysctl.h>
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

static int parse_cpu_num() {
    int cpu_num = 0;

#ifdef FCV_OS_ANDROID
    while (true) {
        char path[128];
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/uevent", cpu_num);

        struct stat buffer;

        if (stat(path, &buffer) == -1) {
            break;
        }

        cpu_num++;
    }
#elif defined FCV_OS_IPHONE
    size_t len = sizeof(cpu_num);
    sysctlbyname("hw.ncpu", &cpu_num, &len, NULL, 0);
#else
    cpu_num = std::thread::hardware_concurrency();
#endif

    return cpu_num > 0 ? cpu_num : 1;
}

#if defined(FCV_OS_ANDROID) || defined(FCV_OS_LINUX)

const int FCV_CPU_SETSIZE = 1024;
const int FCV_NCPUBITS = (8 * sizeof(unsigned long));

struct CpuSetT {
    unsigned long __bits[FCV_CPU_SETSIZE / FCV_NCPUBITS];
};

static inline void fcv_cpu_set(int cpuid, CpuSetT& cpusetp) {
    cpusetp.__bits[cpuid / FCV_NCPUBITS] |= 1UL << (cpuid % FCV_NCPUBITS);
}

static inline void fcv_cpu_zero(CpuSetT& cpusetp) {
    memset(&cpusetp, 0, sizeof(CpuSetT));
}

#endif

static int get_max_cpu_freq(int cpuid) {
#ifdef FCV_OS_ANDROID
    char path[128];
    snprintf(path, sizeof(path),
            "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cpuid);

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        return -1;
    }

    int max_freq_khz = -1;
    fscanf(fp, "%d", &max_freq_khz);
    fclose(fp);
    return max_freq_khz;
#else
    UN_USED(cpuid);
    return 0;
#endif
}

int get_cpu_num() {
    static int cpu_num = parse_cpu_num();
    return cpu_num;
}

int get_ordered_cpu_ids(std::vector<int>& cpu_ids) {
    int cpu_num = get_cpu_num();
    std::vector<std::pair<int, int> > cpu_freqs;
    cpu_freqs.resize(cpu_num);

    for (int i = 0; i < cpu_num; ++i) {
        cpu_freqs[i] = std::make_pair(i, get_max_cpu_freq(i));
    }

    std::sort(cpu_freqs.begin(), cpu_freqs.end(),
            [] (const std::pair<int, int>& a, const std::pair<int, int>& b) {
        return a.second > b.second;
    });

    for (auto iter = cpu_freqs.begin(); iter != cpu_freqs.end(); ++iter) {
        cpu_ids.emplace_back(iter->first);
    }

    return 0;
}

int set_sched_affinity(const std::vector<int>& cpu_ids) {
#if defined(FCV_OS_ANDROID) || defined(FCV_OS_LINUX)
// set affinity for thread
#if defined(__GLIBC__)
    pid_t pid = syscall(SYS_gettid);
#else
    pid_t pid = gettid();
#endif

    CpuSetT mask;
    fcv_cpu_zero(mask);
    for (size_t i = 0; i < cpu_ids.size(); ++i) {
        fcv_cpu_set(cpu_ids[i], mask);
    }

    int syscallret = -1;

    for (int i = 0; i < 100; ++i) {
        syscallret = syscall(__NR_sched_setaffinity, pid, sizeof(mask), &mask);

        if (syscallret == 0) {
            break;
        }
    }

    if (syscallret) {
        printf("Failed to bind specific core, core id: %d\n", cpu_ids[0]);
        return -1;
    }
#else
    UN_USED(cpu_ids);
#endif

    return 0;
}

G_FCV_NAMESPACE1_END()
