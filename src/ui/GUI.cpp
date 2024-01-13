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
#include "Communicator.hpp"
#include "communicator_api.h"
#include "../constants.h"

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// c headers
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>

// ui components
#include "RTCustWindow.h"


static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int tsunami_lab::ui::GUI::exec(std::string i_cmd, std::string i_outputFile)
{
    std::string commandString = (i_cmd + " > " + i_outputFile + " 2>&1 &").data();
    const char *commandChars = commandString.data();
    return system(commandChars);
}

void tsunami_lab::ui::GUI::updateData()
{
    if (std::chrono::system_clock::now() - lastDataUpdate >= std::chrono::duration<float>(dataUpdateFrequency))
    {
        // update
        lastDataUpdate = std::chrono::system_clock::now();
    }
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

json tsunami_lab::ui::GUI::createJson()
{
        json config = { {"solver", "fwave"},
                        {"nx",l_simulationSizeX},
                        {"ny",l_simulationSizeY},
                        {"endTime",endTime},
                        {"writingFrequency",writingFrequency},
                        {"hasBoundaryL",outflowL},
                        {"hasBoundaryR",outflowR},
                        {"hasBoundaryT",outflowT},
                        {"hasBoundaryB",outflowB}
                       };

        if(event_current == 1)
        {
            config.push_back({"setup", "tohoku"});
            config.push_back({"outputFileName","tohoku_solution"});
        } 
        else
        {
            config.push_back({"setup", "chile"});
            config.push_back({"outputFileName","chile_solution"});
        }
        
        return config;
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

    bool benchmarkMode = false;
    bool reportMode = false;
    bool openMp = false;
    bool Checkpointing = false;

    bool disableConfigs = false;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();
        glfwSwapBuffers(window);

        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        updateData();

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

            if (ImGui::CollapsingHeader("Help"))
            {
                ImGui::SeparatorText("ABOUT:");
                ImGui::Text("This GUI was created in the final phase of the tsunami lab during the winter semester 2023/24 at FSU Jena. ");
                ImGui::Text("For information on the project, visit our documentation at");
                ImGui::Indent();
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "https://xlpmg.github.io/tsunami_lab/");
                ImGui::Unindent();
                ImGui::Text("The source code can be found at");
                ImGui::Indent();
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "https://github.com/xLPMG/tsunami_lab");
                ImGui::Unindent();

                ImGui::SeparatorText("GUI USAGE:");
            }

            // CONNECTION THINGS
            if (ImGui::CollapsingHeader("Connectivity"))
            {
                ImGui::InputText("IP address", &IPADDRESS[0], IM_ARRAYSIZE(IPADDRESS));
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
                    if (m_communicator.sendToServer(messageToJsonString(xlpmg::CHECK_MESSAGE)) != 0)
                    {
                        btnConnectDisabled = false;
                        btnDisonnectDisabled = true;
                        connected = false;
                    }
                }
            }
            // END OF CONNECTION THINGS
            if(ImGui::Button("Save Config"))
            {   
                xlpmg::Message saveConfigMsg = xlpmg::LOAD_CONFIG_JSON_MESSAGE;
                saveConfigMsg.args = createJson();
                m_communicator.sendToServer(xlpmg::messageToJsonString(saveConfigMsg));

            }
            if (ImGui::Button("Run"))
            {
                xlpmg::Message startSimMsg = xlpmg::START_SIMULATION_MESSAGE;
                if (m_communicator.sendToServer(messageToJsonString(startSimMsg)) == 0)
                {
                    disableConfigs = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Kill"))
            {
                m_communicator.sendToServer(messageToJsonString(xlpmg::KILL_SIMULATION_MESSAGE));
            }

            if (ImGui::Button("Shutdown server"))
            {
                m_communicator.sendToServer(messageToJsonString(xlpmg::SHUTDOWN_SERVER_MESSAGE));
            }

            if (ImGui::Button("get height data"))
            {
                m_communicator.sendToServer(messageToJsonString(xlpmg::GET_HEIGHT_DATA_MESSAGE));
                bool l_finished = false;
                unsigned long l_index = 0;
                tsunami_lab::t_real l_heights[700 * 590]{0};
                while (!l_finished)
                {
                    std::string data = m_communicator.receiveFromServer();
                    if (json::accept(data))
                    {
                        xlpmg::Message msg = xlpmg::jsonToMessage(json::parse(data));
                        if (msg.key == xlpmg::BUFFERED_SEND_FINISHED.key)
                        {
                            l_finished = true;
                        }
                        else
                        {
                            std::stringstream l_stream(msg.args.dump().substr(1, msg.args.dump().size() - 2));
                            std::string l_num;
                            while (getline(l_stream, l_num, ','))
                            {
                                l_heights[l_index] = std::stof(l_num);
                                l_index++;
                            }
                        }
                    }
                }
                for (int y = 0; y < 590; y++)
                {
                    for (int x = 0; x < 700; x++)
                    {
                        std::cout << l_heights[x + y * 700] << " ";
                    }
                    std::cout << std::endl;
                }
            }
            if (ImGui::Button("Get time step"))
            {
                m_communicator.sendToServer(messageToJsonString(xlpmg::GET_TIMESTEP_MESSAGE));
                std::string response = m_communicator.receiveFromServer();
                if (json::accept(response))
                {
                    xlpmg::Message responseMessage = xlpmg::jsonToMessage(json::parse(response));
                    std::cout << responseMessage.args << std::endl;
                }
            }

            // if (ImGui::Button("file io true"))
            // {
            //     if (m_communicator.sendToServer(xlpmg::KEY_TOGGLE_FILEIO) == 0)
            //     {
            //         usleep(1000);
            //         m_communicator.sendToServer("true");
            //     }
            // }

            // if (ImGui::Button("file io false"))
            // {
            //     if (m_communicator.sendToServer(xlpmg::KEY_TOGGLE_FILEIO) == 0)
            //     {
            //         usleep(1000);
            //         m_communicator.sendToServer("false");
            //     }
            // }

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
            tsunami_lab::ui::RTCustWindow::show();

            ImGui::Begin("Runtime parameters");

            ImGui::Checkbox("Benchmark mode", &benchmarkMode);
            ImGui::SameLine();
            HelpMarker("In and output gets deactivated for measurment.");
            ImGui::Checkbox("Report", &reportMode);
            ImGui::SameLine();
            HelpMarker("Creates reports for.....");
            ImGui::Checkbox("Checkpointing", &Checkpointing);
            ImGui::SameLine();
            HelpMarker("Activates Checkpointing.");

            if (ImGui::BeginMenu("Outflow"))
            {
                ImGui::Checkbox("Outlflow Left", &outflowL);
                ImGui::Checkbox("Outlflow Right", &outflowR);
                ImGui::Checkbox("Outlflow Top", &outflowT);
                ImGui::Checkbox("Outlflow Bottom", &outflowB);
                ImGui::EndMenu();
            }
            ImGui::SameLine();
            HelpMarker("Determines whether the wave leaves the corresponding domain.");

            if (ImGui::Button("Close"))
                showSimulationParameterWindow = false;

            ImGui::End();
        }

        if (showCompilerOptionsWindow)
        {
            ImGui::Begin("Simulation parameters");
            const char *flags[] = {"O0", "O1", "O2"};
            ImGui::Combo("Compiler Optimization Flag", &flag_current, flags, IM_ARRAYSIZE(flags));
            const char *omp[] = {"none", "gnu", "intel"};
            ImGui::Combo("OMP", &omp_current, omp, IM_ARRAYSIZE(omp));

            if (ImGui::Button("Recompile"))
            {
                xlpmg::Message recompileMsg = xlpmg::RECOMPILE_MESSAGE;

                json compileArgs;
                std::string opt;
                opt = "opt=-"+std::string(flags[flag_current]);
                if(omp_current != 0){
                    opt.append(" omp=" + std::string(omp[omp_current]));
                }
                compileArgs["ENV"] = ""; // eg. CXX=g++-13
                compileArgs["OPT"] = opt; // eg. omp=gnu opt=-O2

                recompileMsg.args = compileArgs;
                m_communicator.sendToServer(messageToJsonString(recompileMsg));
            }

            if (ImGui::Button("Close"))
                showSimulationParameterWindow = false;

            ImGui::End();
        }

        // Simulation parameters
        if (showSimulationParameterWindow)
        {
            if (disableConfigs == false)
            {
                ImGui::Begin("Simulation parameters");

                ImGui::InputInt("Cells X", &l_nx, 1, 100);
                ImGui::InputInt("Cells Y", &l_ny, 1, 100);
                l_nx = abs(l_nx);
                l_ny = abs(l_ny);

                const char *events[] = {"Tohoku", "Chile", "Custom"};
                ImGui::Combo("Tsunami Event", &event_current, events, IM_ARRAYSIZE(events));

                ImGui::InputFloat("Simulation size X", &l_simulationSizeX);
                ImGui::InputFloat("Simulation size Y", &l_simulationSizeY);
                l_simulationSizeX = abs(l_simulationSizeX);
                l_simulationSizeY = abs(l_simulationSizeY);

                ImGui::InputFloat("Endtime", &endTime);
                ImGui::SeparatorText("Sliders");
                {
                    ImGui::SliderInt("Writingfrequency", &writingFrequency, 10, 1000);
                    ImGui::SameLine();
                    HelpMarker("CTRL+click to input value.");
                }
                endTime = abs(endTime);
                writingFrequency = abs(writingFrequency);

                if (ImGui::Button("Close"))
                    showSimulationParameterWindow = false;
                ImGui::End();
            }
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