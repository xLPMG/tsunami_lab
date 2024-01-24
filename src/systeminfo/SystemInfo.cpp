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
#include <array>
#include <vector>
#include <sstream>
#include <algorithm>
#include <regex>

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
    int mib[2];
    long long physical_memoryLong;
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    unsigned long length = sizeof(int64_t);
    sysctl(mib, 2, &physical_memoryLong, &length, NULL, 0);
    double physical_memory = physical_memoryLong;
    physical_memory *= m_bytesToGB;
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

    // get page info
    std::string pageInfo = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipePageInfo(popen("/usr/bin/vm_stat", "r"), pclose);
    if (!pipePageInfo)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipePageInfo.get()) != nullptr)
    {
        pageInfo += buffer.data();
    }

    std::istringstream lines(pageInfo);
    std::string line;
    while (getline(lines, line))
    {
        if ((line.find("Anonymous") != std::string::npos) || (line.find("wired") != std::string::npos) || (line.find("occupied") != std::string::npos))
        {
            line.pop_back(); // remove dot

            int i = line.length() - 1; // last character
            while (i != 0 && !isspace(line[i]))
            {
                --i;
            }
            std::string valueString = line.substr(i + 1);
            pages += std::stol(valueString);
        }
        else if ((line.find("purgeable") != std::string::npos))
        {
            line.pop_back(); // remove dot

            int i = line.length() - 1; // last character
            while (i != 0 && !isspace(line[i]))
            {
                --i;
            }
            std::string valueString = line.substr(i + 1);
            pages -= std::stol(valueString);
        }
    }
    o_usedRAM = (pages * (double)pagesize) * m_bytesToGB;
#endif
}

#ifdef __linux__

bool compareFirstDigit(const std::string &i_str1, const std::string &i_str2)
{
    // Find the position of the first digit after "%Cpu"
    size_t l_pos1 = i_str1.find_first_of("0123456789");
    size_t l_pos2 = i_str2.find_first_of("0123456789");

    // Extract the digits and convert them to integers for comparison
    int l_num1 = std::stoi(i_str1.substr(l_pos1));
    int l_num2 = std::stoi(i_str2.substr(l_pos2));

    return l_num1 < l_num2;
}

std::vector<std::string> splitString(const std::string &i_input, const std::string &i_delimiter)
{
    std::vector<std::string> l_parts;
    size_t l_startPos = 0;
    size_t l_foundPos = i_input.find(i_delimiter);

    while (l_foundPos != std::string::npos)
    {
        l_parts.push_back(i_input.substr(l_startPos, l_foundPos - l_startPos));
        l_startPos = l_foundPos + i_delimiter.length();
        l_foundPos = i_input.find(i_delimiter, l_startPos);
    }

    // Add the last part after the last delimiter
    l_parts.push_back(i_input.substr(l_startPos));

    return l_parts;
}
#endif

std::vector<float> tsunami_lab::systeminfo::SystemInfo::getCPUUsage()
{
    std::array<char, 128> l_buffer;
    std::string l_stringBuffer;
    std::vector<float> l_result;

#ifdef __linux__
    std::unique_ptr<FILE, decltype(&pclose)> l_pipe(popen("LC_NUMERIC='C' top -bn 1 -1 | grep '^%Cpu'", "r"), pclose);
    if (!l_pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(l_buffer.data(), l_buffer.size(), l_pipe.get()) != nullptr)
    {
        l_stringBuffer += l_buffer.data();
    }
    /* the string buffer now looks like: (example)
    %Cpu0  :  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st     %Cpu1  :  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
    %Cpu2  :  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st     %Cpu3  :  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
    */

    // erase line breaks
    l_stringBuffer.erase(std::remove(l_stringBuffer.begin(), l_stringBuffer.end(), '\n'), l_stringBuffer.cend());

    //split by "%Cpu"
    std::vector<std::string> l_cpuStrings = splitString(l_stringBuffer, "%Cpu");
    /* l_cpuStrings entries:

    0  :  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st   
    1  :  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
    2  :  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st     
    3  :  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
    */
   //notice that the first line is empty because of the way we're splitting.
   //-> remove the first entry
   l_cpuStrings.erase(l_cpuStrings.begin());

    for (const auto &l_cpuData : l_cpuStrings)
    {
        // l_cpuData example: "0  :  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st"
        std::regex l_pattern("\\b(\\d+).\\d+\\s+id\\b");
        std::smatch l_match;
        // match wil look like: "100.0 id"
        if (std::regex_search(l_cpuData, l_match, l_pattern))
        {
            std::string l_IdleValue = l_match[0];

            // remove " id"
            l_IdleValue.erase(l_IdleValue.length() - 3);

            // we now have the idle time in percent of the cpu core. subtract by 100 to get the used amount
            l_result.push_back(100.0 - std::stof(l_IdleValue));
        }
    }

    return l_result;

#elif __APPLE__ || __MACH__
    std::unique_ptr<FILE, decltype(&pclose)> l_pipe(popen("top -l 2 | grep -E '^CPU' | tail -1 | LC_NUMERIC='C' awk '{s=$3+$5} END {print s}' &", "r"), pclose);
    if (!l_pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(l_buffer.data(), l_buffer.size(), l_pipe.get()) != nullptr)
    {
        l_result.push_back(std::stof(l_buffer.data()));
    }
#endif

    return l_result;
}