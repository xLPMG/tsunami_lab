/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Provides info on the system such as CPU and RAM usage.
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

    //! Conversion constant from bytes to gibibytes (1/1000^3)
    double m_bytesToGiB = 0.00000000093132257;

    //! To check whether its the first time of reading cpu usage data; Linux only
    bool m_firstCPURead = true;

    //! Struct for CPU usage data; Linux only
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

    //! Vector for last read cpu usage data; Linux only
    std::vector<CPUData> m_lastData;

  /**
   * Reads CPU usage data from /proc/stat
   *
   * @param o_data output vector for collected data.
   */
    void readCPUData(std::vector<CPUData>& o_data);

public:
  /**
   * Contructor.
   */
    SystemInfo();

   /**
   * Gets RAM usage data in GiB.
   *
   * @param o_totalRAM output pointer for total RAM value.
   * @param o_usedRAM output pointer for used RAM value.
   */
    void getRAMUsage(double &o_totalRAM, double &o_usedRAM);

   /**
   * @brief Gets CPU usage data in percent.
   * 
   * This function collects CPU usage data. On MacOS it will return one
   * value as the overall CPU usage over all cores. This value is read from
   * the "top" command. On Linux, /proc/stat is read and the
   * values are calculated for each core. 
   * The first entry will be the overall CPU usage, the following entries are the indivual core usages.
   *
   * @return Vector with CPU usage in percent for each core.
   */
    std::vector<float> getCPUUsage();
};
#endif