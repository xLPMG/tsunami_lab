/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Provides info on the system usage.
 **/
#ifndef TSUNAMI_LAB_SYSTEMINFO_SYSTEMINFO_H
#define TSUNAMI_LAB_SYSTEMINFO_SYSTEMINFO_H

#include <vector>

namespace tsunami_lab
{
    namespace systeminfo
    {
        class SystemInfo;
    }
}

class tsunami_lab::systeminfo::SystemInfo
{
private:
    double m_const = 0.00000000093132257;// 1/(1024*1024*1024) Converts bytes to GB
public:
    SystemInfo();

    void getRAMUsage(double &o_totalRAM, double &o_usedRAM);
    std::vector<float> getCPUUsage();
};
#endif