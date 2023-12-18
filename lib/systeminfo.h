#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mach/mach.h>
#include <sys/proc_info.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

std::string exec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

namespace systeminfo
{

    int getCPUCores()
    {
        int mib[2] = {CTL_HW, HW_NCPU};
        int ncpu;
        size_t len = sizeof(ncpu);

        // Get the number of CPU cores
        sysctl(mib, 2, &ncpu, &len, NULL, 0);

        return ncpu;
    }

    float getTotalCPUUsage()
    {
        std::string result = exec("ps -e -o %cpu | awk '{s+=$1} END {print s}'");
        std::replace(result.begin(), result.end(), ',', '.');
        return std::stof(result)/ getCPUCores();
    }

    float getCPUUsage()
    {
        std::string result = exec("top -l  2 | grep -E \"^CPU\" | tail -1 | awk '{ print $3 + $5\"%\" }' | sed s/\\%/\\/");
        std::replace(result.begin(), result.end(), ',', '.');
        return std::stof(result)/ getCPUCores();
    }

    unsigned long getTotalRAM()
    {
        int mib[2];
        int64_t physical_memory;
        mib[0] = CTL_HW;
        mib[1] = HW_MEMSIZE;
        unsigned long length = sizeof(int64_t);
        sysctl(mib, 2, &physical_memory, &length, NULL, 0);
        return physical_memory / 1000000;
    }

    unsigned long getUsedRAM()
    {
        struct task_basic_info t_info;
        mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

        if (KERN_SUCCESS != task_info(mach_task_self(),
                                      TASK_BASIC_INFO, (task_info_t)&t_info,
                                      &t_info_count))
        {
            return -1;
        }
        return t_info.resident_size / 1000000;
    }
}