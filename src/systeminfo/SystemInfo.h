/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Provides info on the system usage.
 **/
#ifndef TSUNAMI_LAB_SYSTEMINFO_SYSTEMINFO_H
#define TSUNAMI_LAB_SYSTEMINFO_SYSTEMINFO_H

#include <string>

namespace tsunami_lab
{
    namespace systeminfo
    {
        class SystemInfo;
    }
}

class tsunami_lab::systeminfo::SystemInfo
{
public:
    SystemInfo();

    void getRAMUsage(long long &o_totalRAM, long long &o_usedRAM);
    std::string getCPUUsage();
};
#endif