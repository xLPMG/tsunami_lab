/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Entry point of the GUI.
 **/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GUI.h"
#include <stdio.h>
#include <iostream>
#include <unistd.h>

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// c headers
#include <string>
#include <filesystem>
#include "../constants.h"
#include <fstream>
#include <sstream>
#include "Communicator.hpp"

// ui components
#include "RTCustWindow.h"

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 500;
const char *LOG_FILE = "log.txt";

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int exec(std::string i_cmd, std::string i_outputFile)
{
    std::string commandString = (i_cmd + " > " + i_outputFile + " 2>&1 &").data();
    const char *commandChars = commandString.data();
    return system(commandChars);
}

static void HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// Main code
int tsunami_lab::ui::GUI::launch(int i_PORT)
{
    PORT = i_PORT;

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tsunami Simulator", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts here

    // Our state
    bool connected = false;

    bool show_demo_window = false;
    bool showRTCustWindow = false;
    bool showCompilerOptionsWindow = false;
    bool showClientLog = false;
    bool showSimulationParameterWindow = false;

    bool btnConnectDisabled = false;
    bool btnDisonnectDisabled = true;

    // Outflow conditions
    bool outflowL = false;
    bool outflowR = false;
    bool outflowT = false;
    bool outflowB = false;

    bool benchmarkMode = false;
    bool reportMode = false;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    unsigned char *pixels = new unsigned char[100 * 100 * 3];
    for (int y = 0; y < 100; ++y)
    {
        for (int x = 0; x < 100; ++x)
        {
            pixels[y * 100 * 3 + x * 3 + 0] = 0xff; // R
            pixels[y * 100 * 3 + x * 3 + 1] = 0x00; // G
            pixels[y * 100 * 3 + x * 3 + 2] = 0x00; // B
        }
    }
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();
        glfwSwapBuffers(window);

        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // OPENGL DRAWING TEST
        glDrawPixels(100, 100, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        // END OPENGL DRAWING TEST

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Main window
        {
            if (connected)
            {
                ImGui::Begin("Welcome to the Tsunami Simulator GUI! (CONNECTED)");
            }
            else
            {
                ImGui::Begin("Welcome to the Tsunami Simulator GUI! (NOT CONNECTED)");
            }

            // CONNECTION THINGS
            if (ImGui::CollapsingHeader("Connectivity"))
            {
                ImGui::InputText("IP address", &IPADDRESS[0], ((int)(sizeof(IPADDRESS) / sizeof(*(IPADDRESS)))));
                ImGui::InputInt("Port", &PORT, 0, 20000);
                PORT = abs(PORT);

                ImGui::BeginDisabled(btnConnectDisabled);
                if (ImGui::Button("Connect"))
                {
                    // SET UP CONNECTION
                    if (m_communicator.startClient(IPADDRESS, PORT) == 0)
                    {
                        btnConnectDisabled = true;
                        btnDisonnectDisabled = false;
                        connected = true;
                    }
                }
                ImGui::EndDisabled();

                ImGui::SameLine();

                ImGui::BeginDisabled(btnDisonnectDisabled);
                if (ImGui::Button("Disconnect"))
                {
                    m_communicator.stopClient();
                    btnConnectDisabled = false;
                    btnDisonnectDisabled = true;
                    connected = false;
                }
                ImGui::EndDisabled();

                ImGui::SameLine();

                if (ImGui::Button("Check"))
                {
                    if (m_communicator.sendToServer("CHECK") != 0)
                    {
                        btnConnectDisabled = false;
                        btnDisonnectDisabled = true;
                        connected = true;
                    }
                }

            }
            // END OF CONNECtiON THINGS

            if (ImGui::Button("Run"))
            {
                if (m_communicator.sendToServer(tsunami_lab::KEY_START_SIMULATION) == 0)
                {
                    usleep(1000);
                    m_communicator.sendToServer("configs/chile5000.json");
                }
            }
            if (ImGui::Button("Shutdown server"))
            {
                m_communicator.sendToServer(tsunami_lab::KEY_SHUTDOWN_SERVER);
            }
            if (ImGui::Button("Exit simulation"))
            {
                m_communicator.sendToServer(tsunami_lab::KEY_EXIT_LAUNCHER);
            }
            if (ImGui::Button("Revive simulation"))
            {
                m_communicator.sendToServer(tsunami_lab::KEY_REVIVE_LAUNCHER);
            }

            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Edit runtime parameters", &showRTCustWindow);
            ImGui::Checkbox("Edit compile options", &showCompilerOptionsWindow);
            ImGui::Checkbox("Edit simulation parameters", &showSimulationParameterWindow);
            ImGui::Checkbox("Show client log", &showClientLog);

            ImGui::ColorEdit3("clear color", (float *)&clear_color);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::Text("made by Luca-Philipp Grumbach and Richard Hofmann");
            ImGui::End();
        }

        // Client log
        if (showClientLog)
        {
            ImGui::Begin("Client log");
            m_communicator.getClientLog(m_clientLog);
            ImGui::TextUnformatted(m_clientLog.c_str());
            ImGui::SetScrollHereY(1.0f);
            ImGui::End();
        }

        // Runtime customization
        if (showRTCustWindow)
        {
            // tsunami_lab::ui::RTCustWindow::show();
            ImGui::Checkbox("Activate Benchmark mode", &benchmarkMode);
            ImGui::Checkbox("Activate Report", &reportMode);

            if (ImGui::BeginMenu("Outflow"))
            {
                ImGui::Checkbox("Outlfow Left", &outflowL);
                ImGui::Checkbox("Outlfow Right", &outflowR);
                ImGui::Checkbox("Outlfow Top", &outflowT);
                ImGui::Checkbox("Outlfow Bottom", &outflowB);
                ImGui::EndMenu();
            }

            // ImGui::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard",    &io.ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);

            // ImGui::SeparatorText("Sliders");
            // {
            //     static float f1 = 0.123f, f2 = 0.0f;
            //     ImGui::SliderFloat("fes", &f1, 0.0f, 1.0f, "ratio = %.3f");
            //     ImGui::SameLine(); HelpMarker("CTRL+click to input value.");

            //     // static float angle = 0.0f;
            //     // ImGui::SliderAngle("slider angle", &angle);
            // }
        }

        if (showCompilerOptionsWindow)
        {

            const char *items[] = {"O0", "O1", "O2"};
            static int item_current = 0;
            ImGui::Combo("Compiler Optimization Flag", &item_current, items, IM_ARRAYSIZE(items));
        }

        // Simulation parameters
        if (showSimulationParameterWindow)
        {
            static int l_nx = 1;
            static int l_ny = 1;
            static tsunami_lab::t_real l_simulationSizeX = 0;
            static tsunami_lab::t_real l_simulationSizeY = 0;

            ImGui::Begin("Simulation parameters");

            ImGui::InputInt("Cells X", &l_nx, 1, 100);
            ImGui::InputInt("Cells Y", &l_ny, 1, 100);
            l_nx = abs(l_nx);
            l_ny = abs(l_ny);

            ImGui::InputFloat("Simulation size X", &l_simulationSizeX);
            ImGui::InputFloat("Simulation size Y", &l_simulationSizeY);
            l_simulationSizeX = abs(l_simulationSizeX);
            l_simulationSizeY = abs(l_simulationSizeY);

            if (ImGui::Button("Close"))
                showSimulationParameterWindow = false;
            ImGui::End();
        }
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
    
}