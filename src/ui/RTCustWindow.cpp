#include "RTCustWindow.h"
#include "imgui.h"

#include <iostream>
#include <string>

unsigned int frameCount = 0;




void tsunami_lab::ui::RTCustWindow::show()
{
    
    ImGui::Begin("Runtime Customization");

        {
            // ImGui::Checkbox("Edit simulation parameters",);
            // if (ImGui::BeginMenu("Outflow"))
            // {
            

            //     ImGui::EndMenu();
            // }

            // ImGui::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard",    &io.ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);


            // ImGui::SeparatorText("Sliders");
            // {
            //     static float f1 = 0.123f, f2 = 0.0f;
            //     ImGui::SliderFloat("", &f1, 0.0f, 1.0f, "ratio = %.3f");
            //     ImGui::SameLine(); HelpMarker("CTRL+click to input value.");

            //     // static float angle = 0.0f;
            //     // ImGui::SliderAngle("slider angle", &angle);
            // }
        }


    ImGui::End();
}