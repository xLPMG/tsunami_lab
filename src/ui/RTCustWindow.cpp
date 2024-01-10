#include "RTCustWindow.h"
#include "imgui.h"
#include "GUI.h"

#include <iostream>
#include <string>

unsigned int frameCount = 0;

void tsunami_lab::ui::RTCustWindow::show()
{
    
    ImGui::Begin("Runtime parameters");

            
            // ImGui::Checkbox("Activate Benchmark mode", &benchmarkMode);
            // ImGui::Checkbox("Activate Report", &reportMode);

            // if (ImGui::BeginMenu("Outflow"))
            // {
            //     ImGui::Checkbox("Outlfow Left", &outflowL);
            //     ImGui::Checkbox("Outlfow Right", &outflowR);
            //     ImGui::Checkbox("Outlfow Top", &outflowT);
            //     ImGui::Checkbox("Outlfow Bottom", &outflowB);
            //     ImGui::EndMenu();
            // }

            // ImGui::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard",    &io.ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);

            // ImGui::SeparatorText("Sliders");
            // {
            //     static float f1 = 0.123f, f2 = 0.0f;
            //     ImGui::SliderFloat("fes", &f1, 0.0f, 1.0f, "ratio = %.3f");
            //     ImGui::SameLine(); HelpMarker("CTRL+click to input value.");

            //     // static float angle = 0.0f;
            //     // ImGui::SliderAngle("slider angle", &angle);
            // }

            // if (ImGui::Button("Close"))
            //     showSimulationParameterWindow = false;

            ImGui::End();
}