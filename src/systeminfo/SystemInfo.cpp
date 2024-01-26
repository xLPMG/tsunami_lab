/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Provides info on the RAM and CPU.
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
#include <vector>
#include <fstream>
#include <sstream>
#include <array>

#include <chrono>

#ifdef __linux__
static unsigned long long l_lastTotalUser, l_lastTotalUserLow, l_lastTotalSys, l_lastTotalIdle;
#endif

tsunami_lab::systeminfo::SystemInfo::SystemInfo()
{
#ifdef __linux__
    FILE *l_file = fopen("/proc/stat", "r");
    if (fscanf(l_file, "cpu %llu %llu %llu %llu", &l_lastTotalUser, &l_lastTotalUserLow,
               &l_lastTotalSys, &l_lastTotalIdle) >= 0)
    {
        fclose(l_file);
    }
#endif
}

void tsunami_lab::systeminfo::SystemInfo::getRAMUsage(double &o_totalRAM, double &o_usedRAM)
{
#ifdef __linux__
    struct sysinfo l_memInfo;
    sysinfo(&l_memInfo);
    long long l_totalPhysMem = l_memInfo.totalram;
    // Multiply in next statement to avoid int overflow on right hand side...
    l_totalPhysMem *= l_memInfo.mem_unit;
    l_totalPhysMem *= m_bytesToGB;
    o_totalRAM = l_totalPhysMem;

    long long l_physMemUsed = l_memInfo.totalram - l_memInfo.freeram;
    // Multiply in next statement to avoid int overflow on right hand side...
    l_physMemUsed *= l_memInfo.mem_unit;
    l_physMemUsed *= m_bytesToGB;
    o_usedRAM = l_physMemUsed;

#elif __APPLE__ || __MACH__
    int l_mib[2];
    long long l_physicalMemoryLong;
    l_mib[0] = CTL_HW;
    l_mib[1] = HW_MEMSIZE;
    unsigned long l_length = sizeof(int64_t);
    sysctl(l_mib, 2, &l_physicalMemoryLong, &l_length, NULL, 0);
    double l_physicalMemoryDouble = l_physicalMemoryLong;
    l_physicalMemoryDouble *= m_bytesToGB;
    o_totalRAM = l_physicalMemoryDouble;

    std::array<char, 128> l_buffer;
    std::string l_result;
    // get page size
    int l_pagesize = 0;
    std::unique_ptr<FILE, decltype(&pclose)> l_pipePagesize(popen("pagesize", "r"), pclose);
    if (!l_pipePagesize)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(l_buffer.data(), l_buffer.size(), l_pipePagesize.get()) != nullptr)
    {
        l_pagesize = std::stoi(l_buffer.data());
    }

    long l_pages = 0;

    // get page info
    std::string l_pageInfo = "";
    std::unique_ptr<FILE, decltype(&pclose)> l_pipePageInfo(popen("/usr/bin/vm_stat", "r"), pclose);
    if (!l_pipePageInfo)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(l_buffer.data(), l_buffer.size(), l_pipePageInfo.get()) != nullptr)
    {
        l_pageInfo += l_buffer.data();
    }

    std::istringstream l_lines(l_pageInfo);
    std::string l_line;
    while (getline(l_lines, l_line))
    {
        if ((l_line.find("Anonymous") != std::string::npos) || (l_line.find("wired") != std::string::npos) || (l_line.find("occupied") != std::string::npos))
        {
            l_line.pop_back(); // remove dot

            int l_i = l_line.length() - 1; // last character
            // iterate to start of number
            while (l_i != 0 && !isspace(l_line[l_i]))
            {
                --l_i;
            }
            std::string l_valueString = l_line.substr(l_i + 1);
            l_pages += std::stol(l_valueString);
        }
        else if ((l_line.find("purgeable") != std::string::npos))
        {
            l_line.pop_back(); // remove dot

            int l_i = l_line.length() - 1; // last character
            // iterate to start of number
            while (l_i != 0 && !isspace(l_line[l_i]))
            {
                --l_i;
            }
            std::string l_valueString = l_line.substr(l_i + 1);
            l_pages -= std::stol(l_valueString);
        }
    }
    o_usedRAM = (l_pages * (double)l_pagesize) * m_bytesToGB;
#endif
}

void tsunami_lab::systeminfo::SystemInfo::readCPUData(std::vector<CPUData> &o_data)
{
    std::ifstream l_file("/proc/stat");
    std::string l_line;

    while (std::getline(l_file, l_line))
    {
        std::istringstream ss(l_line);
        std::string l_cpuLabel;
        ss >> l_cpuLabel;

        if (l_cpuLabel.find("cpu") != std::string::npos)
        {
            CPUData cpu;
            ss >> cpu.user >> cpu.nice >> cpu.system >> cpu.idle >> cpu.iowait >> cpu.irq >> cpu.softirq >> cpu.steal >> cpu.guest >> cpu.guest_nice;
            o_data.push_back(cpu);
        }
    }
}

std::vector<float> tsunami_lab::systeminfo::SystemInfo::getCPUUsage()
{
    std::vector<float> l_cpuUsage;
#ifdef __linux__
    if (m_firstCPURead)
    {
        m_firstCPURead = false;
        readCPUData(m_lastData);
        return std::vector<float>(m_lastData.size(), 0.0);
    }

    std::vector<CPUData> l_currentData;
    readCPUData(l_currentData);

    for (size_t i = 0; i < l_currentData.size(); ++i)
    {
        unsigned long long l_totalDelta = (l_currentData[i].user + l_currentData[i].nice +
                                           l_currentData[i].system + l_currentData[i].idle +
                                           l_currentData[i].iowait + l_currentData[i].irq +
                                           l_currentData[i].softirq + l_currentData[i].steal +
                                           l_currentData[i].guest + l_currentData[i].guest_nice) -
                                          (m_lastData[i].user + m_lastData[i].nice +
                                           m_lastData[i].system + m_lastData[i].idle +
                                           m_lastData[i].iowait + m_lastData[i].irq +
                                           m_lastData[i].softirq + m_lastData[i].steal +
                                           m_lastData[i].guest + m_lastData[i].guest_nice);

        unsigned long long l_idleDelta = (l_currentData[i].idle + l_currentData[i].iowait) -
                                         (m_lastData[i].idle + m_lastData[i].iowait);

        double l_coreUsage = 100.0 * (1.0 - (static_cast<float>(l_idleDelta) / l_totalDelta));
        l_cpuUsage.push_back(l_coreUsage);
    }
#elif __APPLE__ || __MACH__
    std::array<char, 128> l_buffer;
    std::unique_ptr<FILE, decltype(&pclose)> l_pipe(popen("top -l 2 | grep -E '^CPU' | tail -1 | LC_NUMERIC='C' awk '{s=$3+$5} END {print s}' &", "r"), pclose);
    if (!l_pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(l_buffer.data(), l_buffer.size(), l_pipe.get()) != nullptr)
    {
        l_cpuUsage.push_back(std::stof(l_buffer.data()));
    }
#endif
    return l_cpuUsage;
}