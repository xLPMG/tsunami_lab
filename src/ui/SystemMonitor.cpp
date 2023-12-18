#include "SystemMonitor.h"
#include "imgui.h"
#include "../../lib/systeminfo.h"

#include <iostream>
#include <string>

unsigned int frameCount = 0;

float cpuUsage = systeminfo::getCPUUsage();
float totalCpuUsage = systeminfo::getTotalCPUUsage();

void tsunami_lab::ui::SystemMonitor::updateData()
{
    frameCount++;
    if (frameCount >= 180)
    {
        cpuUsage = systeminfo::getCPUUsage();
        totalCpuUsage = systeminfo::getTotalCPUUsage();
        frameCount = 0;
    }
}

void tsunami_lab::ui::SystemMonitor::show()
{
    updateData();

    ImGui::Begin("System Monitor");

    std::string res = std::to_string(systeminfo::getCPUCores());
    ImGui::Text("CPU Cores: %s", res.c_str());

    ImGui::Text("CPU Usage: %.2f", cpuUsage);

    ImGui::Text("CPU Total Usage 2: %.2f", totalCpuUsage);

    ImGui::ProgressBar(cpuUsage / 100, ImVec2(0.0f, 0.0f));
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

    ImGui::Text("CPU Usage");

    ImGui::ProgressBar(totalCpuUsage / 100, ImVec2(0.0f, 0.0f));
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Total CPU Usage");

    res = std::to_string(systeminfo::getTotalRAM());
    ImGui::Text("Total RAM: %sMB", res.c_str());

    res = std::to_string(systeminfo::getUsedRAM());
    ImGui::Text("Used RAM: %sMB", res.c_str());

    ImGui::Text("RAM Usage");
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::ProgressBar(systeminfo::getUsedRAM() / systeminfo::getTotalRAM(), ImVec2(0.0f, 0.0f));

    ImGui::End();
}