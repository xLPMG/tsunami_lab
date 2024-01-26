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
    double m_bytesToGB = 0.00000000093132257; // 1/(1024*1024*1024) Converts bytes to GB
    bool m_firstCPURead = true;

    struct CPUData
    {
        unsigned long long user;
        unsigned long long nice;
        unsigned long long system;
        unsigned long long idle;
        unsigned long long iowait;
        unsigned long long irq;
        unsigned long long softirq;
        unsigned long long steal;
        unsigned long long guest;
        unsigned long long guest_nice;
    };

    std::vector<CPUData> lastData;

    void readCPUData(std::vector<CPUData>& data);

public:
    SystemInfo();

    void getRAMUsage(double &o_totalRAM, double &o_usedRAM);
    std::vector<float> getCPUUsage();
};
#endif