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

void tsunami_lab::systeminfo::SystemInfo::getRAMUsage(double &o_totalRAM, double &o_usedRAM)
{
#ifdef __linux__
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    long long totalPhysMem = memInfo.totalram;
    // Multiply in next statement to avoid int overflow on right hand side...
    totalPhysMem *= memInfo.mem_unit;
    totalPhysMem *= m_const;
    o_totalRAM = totalPhysMem;

    long long physMemUsed = memInfo.totalram - memInfo.freeram;
    // Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= memInfo.mem_unit;
    physMemUsed *= m_const;
    o_usedRAM = physMemUsed;

#elif __APPLE__ || __MACH__
    int mib[2];
    long long physical_memoryLong;
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    unsigned long length = sizeof(int64_t);
    sysctl(mib, 2, &physical_memoryLong, &length, NULL, 0);
    double physical_memory = physical_memoryLong;
    physical_memory *= m_const;
    o_totalRAM = physical_memory;

    std::array<char, 128> buffer;
    std::string result;
    // get page size
    int pagesize = 0;
    std::unique_ptr<FILE, decltype(&pclose)> pipePagesize(popen("pagesize", "r"), pclose);
    if (!pipePagesize)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipePagesize.get()) != nullptr)
    {
        pagesize = std::stoi(buffer.data());
    }

    long pages = 0;
    // get app memory
    // = anonymous - pages purgeable
    std::unique_ptr<FILE, decltype(&pclose)> pipeAppMemPages(popen("vm_stat |  LC_NUMERIC='C' awk '{if (NR==8) { sum=-$3 } else if(NR==15) {sum+=$3}}  END { print sum }'", "r"), pclose);
    if (!pipeAppMemPages)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipeAppMemPages.get()) != nullptr)
    {
        result = buffer.data();
    }
    pages += std::stol(result);

    // get wired memory
    std::unique_ptr<FILE, decltype(&pclose)> pipeWiredMem(popen("vm_stat |  LC_NUMERIC='C' awk 'NR==7 { sum+=$4 }; END { print sum }'", "r"), pclose);
    if (!pipeWiredMem)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipeWiredMem.get()) != nullptr)
    {
        result = buffer.data();
    }
    pages += std::stol(result);

    // get compressed memory
    std::unique_ptr<FILE, decltype(&pclose)> pipeCompMem(popen("vm_stat |  LC_NUMERIC='C' awk 'NR==17 { sum+=$5 }; END { print sum }'", "r"), pclose);
    if (!pipeCompMem)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipeCompMem.get()) != nullptr)
    {
        result = buffer.data();
    }
    pages += std::stol(result);

    o_usedRAM = (pages * (double)pagesize) * m_const;
#endif
}

std::vector<float> tsunami_lab::systeminfo::SystemInfo::getCPUUsage()
{
    std::array<char, 128> buffer;
    std::vector<float> result;

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
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("top -l 2 | grep -E '^CPU' | tail -1 | LC_NUMERIC='C' awk '{s=$3+$5} END {print s}' &", "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result.push_back(std::stof(buffer.data()));
    }
#endif

    return result;
}