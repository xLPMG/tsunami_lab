/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Provides info on the system usage.
 **/

#include "SystemInfo.h"

#ifdef __linux__
#include "sys/types.h"
#include "sys/sysinfo.h"
#elif __APPLE__ || __MACH__
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#endif

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#ifdef __linux__
static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
#endif

tsunami_lab::systeminfo::SystemInfo::SystemInfo()
{
#ifdef __linux__
    FILE *file = fopen("/proc/stat", "r");
    int codes = 0;
    codes = fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow,
                   &lastTotalSys, &lastTotalIdle);
    codes = fclose(file);
    if (codes == 0)
    {
        // do nothing; this is just to get rid of
        // [-Werror=unused-but-set-variable]
    }
#endif
}

void tsunami_lab::systeminfo::SystemInfo::getRAMUsage(long long &o_totalRAM, long long &o_usedRAM)
{
#ifdef __linux__
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    long long totalPhysMem = memInfo.totalram;
    // Multiply in next statement to avoid int overflow on right hand side...
    totalPhysMem *= memInfo.mem_unit;
    o_totalRAM = totalPhysMem;

    long long physMemUsed = memInfo.totalram - memInfo.freeram;
    // Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= memInfo.mem_unit;
    o_usedRAM = physMemUsed;

#elif __APPLE__ || __MACH__
    int mib[2];
    int64_t physical_memory;
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    length = sizeof(int64_t);
    sysctl(mib, 2, &physical_memory, &length, NULL, 0);
    o_totalRAM = HW_MEMSIZE;

    vm_size_t page_size;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    vm_statistics64_data_t vm_stats;

    mach_port = mach_host_self();
    count = sizeof(vm_stats) / sizeof(natural_t);
    if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
        KERN_SUCCESS == host_statistics64(mach_port, HOST_VM_INFO,
                                          (host_info64_t)&vm_stats, &count))
    {
        long long used_memory = ((int64_t)vm_stats.active_count +
                                 (int64_t)vm_stats.inactive_count +
                                 (int64_t)vm_stats.wire_count) *
                                (int64_t)page_size;

        o_usedRAM = used_memory;
    }
#endif
}

std::string tsunami_lab::systeminfo::SystemInfo::getCPUUsage()
{
    std::array<char, 128> buffer;
    std::string result;

#ifdef __linux__
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("top -bn 1 -1 | grep '^%Cpu'", "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
#elif __APPLE__ || __MACH__
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("top -l 2 | grep -E '^CPU' | tail -1", "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
#endif

    return result;
}