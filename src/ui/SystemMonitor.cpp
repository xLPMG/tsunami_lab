#include "SystemMonitor.h"
#include "imgui.h"
#include "../../lib/cpumem_monitor.h"

#include <iostream>

void tsunami_lab::ui::SystemMonitor::show()
{
    SL::NET::CPUMemMonitor mon;
    auto memusage = mon.getMemoryUsage();
    auto cpuusage = mon.getCPUUsage();
    std::cout << "Total CPU Usage: " << cpuusage.TotalUse << std::endl;
    ImGui::Begin("System Monitor");

    ImGui::End();
}