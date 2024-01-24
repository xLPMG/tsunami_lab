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
#include <vector>
#include <sstream>
#include <algorithm>
#include <regex>

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
    totalPhysMem *= m_bytesToGB;
    o_totalRAM = totalPhysMem;

    long long physMemUsed = memInfo.totalram - memInfo.freeram;
    // Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= memInfo.mem_unit;
    physMemUsed *= m_bytesToGB;
    o_usedRAM = physMemUsed;

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

bool compareFirstDigit(const std::string &str1, const std::string &str2)
{
    // Find the position of the first digit after "%Cpu"
    size_t pos1 = str1.find_first_of("0123456789");
    size_t pos2 = str2.find_first_of("0123456789");

    // Extract the digits and convert them to integers for comparison
    int num1 = std::stoi(str1.substr(pos1));
    int num2 = std::stoi(str2.substr(pos2));

    return num1 < num2;
}

std::vector<std::string> splitString(const std::string &input, const std::string &delimiter)
{
    std::vector<std::string> parts;
    size_t startPos = 0;
    size_t foundPos = input.find(delimiter);

    while (foundPos != std::string::npos)
    {
        parts.push_back(input.substr(startPos, foundPos - startPos));
        startPos = foundPos + delimiter.length();
        foundPos = input.find(delimiter, startPos);
    }

    // Add the last part after the last delimiter
    parts.push_back(input.substr(startPos));

    return parts;
}
#endif

std::vector<float> tsunami_lab::systeminfo::SystemInfo::getCPUUsage()
{
    std::array<char, 128> buffer;
    std::string stringBuffer;
    std::vector<float> result;

#ifdef __linux__
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("LC_NUMERIC='C' top -bn 1 -1 | grep '^%Cpu'", "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        stringBuffer += buffer.data();
    }
    stringBuffer.erase(std::remove(stringBuffer.begin(), stringBuffer.end(), '\n'), stringBuffer.cend());
    std::vector<std::string> cpuParts = splitString(stringBuffer, "%Cpu");
    std::sort(cpuParts.begin() + 1, cpuParts.end(), compareFirstDigit);

    for (const auto &cpu : cpuParts)
    {
        std::regex pattern("\\b(\\d+).\\d+\\s+id\\b");
        std::smatch match;
        if (std::regex_search(cpu, match, pattern))
        {
            std::string value_before_id = match[0];
            value_before_id.erase(value_before_id.length()-3);
            std::cout << 100.0 - std::stof(value_before_id) << std::endl;
            result.push_back(100.0 - std::stof(value_before_id));
        }
    }

    return result;

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