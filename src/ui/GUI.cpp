/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Entry point of the GUI.
 **/

#include <imgui.h>
#include <imfilebrowser.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GUI.h"
#include "xlpmg/Communicator.hpp"
#include "xlpmg/communicator_api.h"
#include "../constants.h"
#include "../io/NetCdf.h"

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
#include <algorithm>

// ui components

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
    if (std::chrono::system_clock::now() - m_lastDataUpdate >= std::chrono::duration<float>(m_dataUpdateFrequency))
    {
        // TODO: update
        m_lastDataUpdate = std::chrono::system_clock::now();
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

json tsunami_lab::ui::GUI::createConfigJson()
{
    json config = {{"solver", "fwave"},
                   {"nx", m_nx},
                   {"ny", m_ny},
                   {"nk", m_nk},
                   {"simulationSizeX", m_simulationSizeX},
                   {"simulationSizeY", m_simulationSizeY},
                   {"offsetX", m_offsetX},
                   {"offsetY", m_offsetY},
                   {"endTime", m_endTime},
                   {"useFileIO", m_useFileIO},
                   {"writingFrequency", m_writingFrequency},
                   {"outputFileName", m_outputFileName},
                   {"checkpointFrequency", m_checkpointFrequency},
                   {"hasBoundaryL", m_boundaryL},
                   {"hasBoundaryR", m_boundaryR},
                   {"hasBoundaryT", m_boundaryT},
                   {"hasBoundaryB", m_boundaryB},
                   {"setup", m_tsunamiEvents[m_tsunamiEvent]}};

    // stations
    for (Station l_s : m_stations)
    {
        json l_stationData;
        l_stationData["name"] = l_s.name;
        l_stationData["locX"] = l_s.positionX;
        l_stationData["locY"] = l_s.positionY;
        config["stations"].push_back(l_stationData);
    }

    return config;
}

// Main code
int tsunami_lab::ui::GUI::launch()
{
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
    bool m_connected = false;

    bool show_demo_window = false;
    bool showCompilerOptionsWindow = false;
    bool showClientLog = false;
    bool showSimulationParameterWindow = false;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui::FileBrowser fileDialogBath;
    ImGui::FileBrowser fileDialogDis;

    // (optional) set browser properties
    fileDialogBath.SetTitle("Filesystem");
    fileDialogBath.SetTypeFilters({".nc"});
    fileDialogDis.SetTitle("Filesystem");
    fileDialogDis.SetTypeFilters({".nc"});

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
            ImGui::Begin("Welcome to the Tsunami Simulator GUI!");
            //-------------------------------------------//
            //-----------------MAIN TABS-----------------//
            //-------------------------------------------//
            if (ImGui::BeginTabBar("MainTabs"))
            {
                //------------------------------------------//
                //-----------------HELP TAB-----------------//
                //------------------------------------------//
                if (ImGui::BeginTabItem("Help"))
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

                    // ImGui::SeparatorText("GUI USAGE:");
                    ImGui::EndTabItem();
                }

                //--------------------------------------------//
                //----------------CONNECTIVITY----------------//
                //--------------------------------------------//
                if (ImGui::BeginTabItem("Connectivity"))
                {
                    int width = 32;
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                    ImGui::InputText("IP address", &IPADDRESS[0], IM_ARRAYSIZE(IPADDRESS));
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                    ImGui::InputInt("Port", &PORT, 0);
                    PORT = abs(PORT);

                    ImGui::BeginDisabled(m_connected);
                    if (ImGui::Button("Connect"))
                    {
                        // SET UP CONNECTION
                        if (m_communicator.startClient(IPADDRESS, PORT) == 0)
                        {
                            m_connected = true;
                        }
                    }
                    ImGui::EndDisabled();

                    ImGui::SameLine();

                    ImGui::BeginDisabled(!m_connected);
                    if (ImGui::Button("Disconnect"))
                    {
                        m_communicator.stopClient();
                        m_connected = false;
                    }
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    if (ImGui::Button("Check connection"))
                    {
                        m_communicator.sendToServer(messageToJsonString(xlpmg::CHECK));
                        std::string l_response = m_communicator.receiveFromServer();
                        if (l_response == "OK")
                        {
                            m_connected = true;
                        }
                        else
                        {
                            m_connected = false;
                        }
                    }
                    ImGui::BeginDisabled(!m_connected);
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 0.6f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1.0f, 0.8f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(1.0f, 1.0f, 1.0f));
                    if (ImGui::Button("Shutdown server"))
                    {
                        if (m_communicator.sendToServer(messageToJsonString(xlpmg::SHUTDOWN_SERVER)) == 0)
                        {
                            m_connected = false;
                        }
                    }
                    ImGui::PopStyleColor(3);
                    ImGui::EndDisabled();

                    ImGui::PushID(301);
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                    ImGui::InputInt("", &m_clientReadBufferSize, 0);
                    ImGui::SetItemTooltip("%s", (std::string("in bytes. Default: ") + std::to_string(m_communicator.BUFF_SIZE_READ_DEFAULT) + std::string(". Max recommended: 8.000.000")).c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("Set"))
                    {
                        m_communicator.setReadBufferSize(m_clientReadBufferSize);
                    }

                    ImGui::SetItemTooltip("Sets the input.");
                    ImGui::SameLine();
                    ImGui::Text("Buffer size for receiving (client)");
                    ImGui::SameLine();
                    HelpMarker("Size of the TCP Receive Window: generally the amount of data that the recipient can accept without acknowledging the sender.");
                    if (m_clientReadBufferSize < 256)
                    {
                        m_clientReadBufferSize = 256;
                    }
                    ImGui::PopID();

                    ImGui::PushID(302);
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                    ImGui::InputInt("", &m_clientSendBufferSize, 0);
                    ImGui::SetItemTooltip("%s", (std::string("in bytes. Default: ") + std::to_string(m_communicator.BUFF_SIZE_SEND_DEFAULT) + std::string(". Max recommended: 8.000.000")).c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("Set"))
                    {
                        m_communicator.setSendBufferSize(m_clientSendBufferSize);
                    }
                    ImGui::SetItemTooltip("Sets the input.");
                    ImGui::SameLine();
                    ImGui::Text("Buffer size for sending (client)");
                    ImGui::SameLine();
                    HelpMarker("Size of the TCP Send Window.");
                    if (m_clientSendBufferSize < 256)
                    {
                        m_clientSendBufferSize = 256;
                    }
                    ImGui::PopID();

                    ImGui::PushID(303);
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                    ImGui::InputInt("", &m_serverReadBufferSize, 0);
                    ImGui::SetItemTooltip("%s", (std::string("in bytes. Default: ") + std::to_string(m_communicator.BUFF_SIZE_READ_DEFAULT) + std::string(". Max recommended: 8.000.000")).c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("Set"))
                    {

                        xlpmg::Message msg = xlpmg::SET_READ_BUFFER_SIZE;
                        msg.args = m_serverReadBufferSize;
                        m_communicator.sendToServer(messageToJsonString(msg));
                    }
                    ImGui::SetItemTooltip("Sets the input.");
                    ImGui::SameLine();
                    ImGui::Text("Buffer size for receiving (server)");
                    ImGui::SameLine();
                    HelpMarker("Size of the TCP Receive Window: generally the amount of data that the recipient can accept without acknowledging the sender.");
                    if (m_serverReadBufferSize < 256)
                    {
                        m_serverReadBufferSize = 256;
                    }
                    ImGui::PopID();

                    ImGui::PushID(304);
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                    ImGui::InputInt("", &m_serverSendBufferSize, 0);
                    ImGui::SetItemTooltip("%s", (std::string("in bytes. Default: ") + std::to_string(m_communicator.BUFF_SIZE_SEND_DEFAULT) + std::string(". Max recommended: 8.000.000")).c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("Set"))
                    {
                        xlpmg::Message msg = xlpmg::SET_SEND_BUFFER_SIZE;
                        msg.args = m_serverSendBufferSize;
                        m_communicator.sendToServer(messageToJsonString(msg));
                    }
                    ImGui::SetItemTooltip("Sets the input.");
                    ImGui::SameLine();
                    ImGui::Text("Buffer size for sending (server)");
                    ImGui::SameLine();
                    HelpMarker("Size of the TCP Send Window.");
                    if (m_serverSendBufferSize < 256)
                    {
                        m_serverSendBufferSize = 256;
                    }
                    ImGui::PopID();

                    ImGui::EndTabItem();
                }
                //--------------------------------------------//
                //----------------CONNECTIVITY----------------//
                //--------------------------------------------//
                if (ImGui::BeginTabItem("Simulator"))
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2 / 7.0f, 1.0f, 0.8f));
                    if (ImGui::Button("Run simulation"))
                    {
                        xlpmg::Message startSimMsg = xlpmg::START_SIMULATION;
                        m_communicator.sendToServer(messageToJsonString(startSimMsg));
                    }
                    ImGui::PopStyleColor(3);
                    ImGui::SetItemTooltip("Will start the computational loop. If you have already run a simulation, make sure to reset it first to clear the old data.");

                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1 / 7.0f, 0.6f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1 / 7.0f, 0.8f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(1 / 7.0f, 1.0f, 0.8f));
                    if (ImGui::Button("Reset simulation"))
                    {
                        xlpmg::Message startSimMsg = xlpmg::RESET_SIMULATOR;
                        m_communicator.sendToServer(messageToJsonString(startSimMsg));
                    }
                    ImGui::PopStyleColor(3);
                    ImGui::SetItemTooltip("Deletes previous cached computated data but keeps the loaded config, stations and checkpoint files.");

                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 1.0f, 8.0f));
                    if (ImGui::Button("Kill simulation"))
                    {
                        m_communicator.sendToServer(messageToJsonString(xlpmg::KILL_SIMULATION));
                    }
                    ImGui::PopStyleColor(3);
                    ImGui::EndTabItem();

                    if (ImGui::Button("Delete checkpoint"))
                    {
                        xlpmg::Message deleteCPMsg = xlpmg::DELETE_CHECKPOINTS;
                        m_communicator.sendToServer(messageToJsonString(deleteCPMsg));
                    }
                    if (ImGui::Button("Delete stations"))
                    {
                        xlpmg::Message deleteStationsMsg = xlpmg::DELETE_STATIONS;
                        m_communicator.sendToServer(messageToJsonString(deleteStationsMsg));
                    }
                }
                //-------------------------------------------//
                //------------------WINDOWS------------------//
                //-------------------------------------------//
                if (ImGui::BeginTabItem("Windows"))
                {
                    ImGui::Checkbox("Show Demo Window", &show_demo_window);
                    ImGui::Checkbox("Edit compiler/runtime options", &showCompilerOptionsWindow);
                    ImGui::Checkbox("Edit simulation parameters", &showSimulationParameterWindow);
                    ImGui::Checkbox("Show client log", &showClientLog);
                    ImGui::EndTabItem();
                }
                //------------------------------------------//
                //--------------UNSROTED ITEMS--------------//
                //------------------------------------------//
                if (ImGui::BeginTabItem("Experimental"))
                {
                    if (ImGui::Button("Pause Simulation"))
                    {
                        if (!m_isPausing)
                        {
                            m_communicator.sendToServer(messageToJsonString(xlpmg::PAUSE_SIMULATION));
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Continue Simulation"))
                    {
                        if (m_isPausing)
                        {
                            m_communicator.sendToServer(messageToJsonString(xlpmg::CONTINUE_SIMULATION));
                        }
                    }

                    if (ImGui::Button("Get height data"))
                    {
                        if (m_communicator.sendToServer(messageToJsonString(xlpmg::GET_HEIGHT_DATA)) == 0)
                        {
                            unsigned long l_index = 0;
                            bool l_finished = false;
                            tsunami_lab::t_real l_heights[500 * 500]{0};

                            std::string data = "";
                            while (!l_finished)
                            {
                                std::string l_response = m_communicator.receiveFromServer();
                                if (json::accept(l_response) && xlpmg::jsonToMessage(json::parse(l_response)).key == xlpmg::BUFFERED_SEND_FINISHED.key)
                                {
                                    l_finished = true;
                                }
                                else
                                {
                                    data += l_response;
                                }
                            }
                            if (json::accept(data))
                            {
                                xlpmg::Message msg = xlpmg::jsonToMessage(json::parse(data));

                                std::stringstream l_stream(msg.args.dump().substr(1, msg.args.dump().size() - 2));
                                std::string l_num;
                                while (getline(l_stream, l_num, ','))
                                {
                                    l_heights[l_index] = std::stof(l_num);
                                    l_index++;
                                }
                            }

                            for (int y = 0; y < 500; y++)
                            {
                                for (int x = 0; x < 500; x++)
                                {
                                    std::cout << l_heights[x + y * 500] << " ";
                                }
                                std::cout << std::endl;
                            }
                        }
                    }
                    if (ImGui::Button("Get time step"))
                    {
                        m_communicator.sendToServer(messageToJsonString(xlpmg::GET_TIMESTEP));
                        std::string response = m_communicator.receiveFromServer();
                        if (json::accept(response))
                        {
                            xlpmg::Message responseMessage = xlpmg::jsonToMessage(json::parse(response));
                            std::cout << responseMessage.args << std::endl;
                        }
                    }

                    if (ImGui::Button("Get simulation sizes"))
                    {
                        m_communicator.sendToServer(messageToJsonString(xlpmg::GET_SIMULATION_SIZES));
                    }

                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::Separator();

            ImGui::ColorEdit3("Background color", (float *)&clear_color);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::Text("made by Luca-Philipp Grumbach and Richard Hofmann");
            ImGui::End();
        }

        //-------------------------------------------//
        //-------------CLIENT LOG WINDOW-------------//
        //-------------------------------------------//
        if (showClientLog)
        {
            ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
            ImGui::Begin("Client log", &showClientLog);

            if (ImGui::Button("Clear"))
            {
                m_communicator.clearLog();
            }
            ImGui::SameLine();
            ImGui::Checkbox("Auto-Scroll", &m_clientLogAutoScroll);
            if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                m_communicator.getLog(m_clientLog);
                ImGui::TextUnformatted(m_clientLog.c_str());
                ImGui::PopStyleVar();

                if (m_clientLogAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                {
                    ImGui::SetScrollHereY(1.0f);
                }
            }
            ImGui::EndChild();
            ImGui::End();
        }

        //--------------------------------------------//
        //----------COMPILER/RUNTIME OPTIONS----------//
        //--------------------------------------------//
        if (showCompilerOptionsWindow)
        {
            ImGui::Begin("Compiler/runtime options", &showCompilerOptionsWindow);
            short width = 24;
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::Combo("Compile mode", &m_compileMode, m_compileModes, IM_ARRAYSIZE(m_compileModes));
            ImGui::SameLine();
            HelpMarker("Option \'san\' enables address and undefined behavior sanitizers. \'benchmark\' disables file I/O");

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputTextWithHint("Compiler choice", "For example: \'g++-13\' or \'icpc\'", m_compilerChoice, IM_ARRAYSIZE(m_compilerChoice));
            ImGui::SameLine();
            HelpMarker("Leave this empty to use the default compiler.");

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::Combo("Optimization level", &m_optFlag, m_optFlags, IM_ARRAYSIZE(m_optFlags));

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::Combo("OpenMP flag", &m_ompFlag, m_ompFlags, IM_ARRAYSIZE(m_ompFlags));

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::BeginDisabled(m_ompFlag == 0);
            ImGui::InputInt("Number of threads", &m_ompNumThreads, 1);
            if (m_ompNumThreads < -1)
            {
                m_ompNumThreads = -1;
            }
            ImGui::SameLine();
            HelpMarker("Number of OMP threads. -1 means that the variable won't be set.");
            ImGui::EndDisabled();

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::Combo("Report level", &m_reportFlag, m_reportFlags, IM_ARRAYSIZE(m_reportFlags));
            ImGui::SameLine();
            HelpMarker("Creates compiler reports with different levels of detail.");

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::Checkbox("Use filesystem", &m_useFilesystem);
            ImGui::SameLine();
            HelpMarker("Disables the include of the <filesystem> header.");

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::Checkbox("Use GUI", &m_useGui);
            ImGui::SameLine();
            HelpMarker("You may disable the building and compilation of the GUI with this.");

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputText("Custom environmental variables", m_customEnv, IM_ARRAYSIZE(m_customEnv));
            ImGui::SameLine();
            HelpMarker("The input will be set before compiling using \'export <input>;\'");

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::Combo("Runner", &m_runner, m_runnerOptions, IM_ARRAYSIZE(m_runnerOptions));
            ImGui::SameLine();
            HelpMarker("If you choose a runner, the server will be restarted after compilation.");

            if (m_runner == 2)
            {

                ImGui::SeparatorText("SLURM job options");

                ImGui::Indent();

                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::InputText("Job name", m_sbJob, IM_ARRAYSIZE(m_sbJob));

                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::InputText("Output file", m_sbOut, IM_ARRAYSIZE(m_sbOut));

                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::InputText("Error output file", m_sbErr, IM_ARRAYSIZE(m_sbErr));

                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::InputTextWithHint("Time limit", "HH:MM:SS", m_sbTim, IM_ARRAYSIZE(m_sbTim));

                ImGui::Unindent();
            }

            ImGui::BeginDisabled(!m_useFileIO);
            ImGui::Checkbox("Write checkpoint before restart", &m_checkpointBeforeRecomp);
            ImGui::EndDisabled();

            if (ImGui::Button("Recompile"))
            {
                xlpmg::Message recompileMsg;
                switch (m_runner)
                {
                case 0:
                    recompileMsg = xlpmg::COMPILE;
                    break;
                case 1:
                    recompileMsg = xlpmg::COMPILE_RUN_BASH;
                    break;
                case 2:
                    recompileMsg = xlpmg::COMPILE_RUN_SBATCH;
                    break;
                }

                // SET ARGUMENTS
                std::string envVars;
                if (strcmp(m_customEnv, "") != 0)
                {
                    envVars = std::string(m_customEnv);
                }
                if (strcmp(m_compilerChoice, "") != 0)
                {
                    envVars.append(" CXX=" + std::string(m_compilerChoice));
                }
                if (m_ompNumThreads >= 0)
                {
                    envVars.append(" OMP_NUM_THREADS=" + std::to_string(m_ompNumThreads));
                }
                std::string options;
                options.append("mode=" + std::string(m_compileModes[m_compileMode]));
                options.append(" opt=-O" + std::string(m_optFlags[m_optFlag]));
                options.append(" omp=" + std::string(m_ompFlags[m_ompFlag]));
                if (m_reportFlag != 0)
                {
                    options.append(" -qopt-report=" + std::string(m_reportFlags[m_reportFlag]));
                }
                if (!m_useFilesystem)
                {
                    options.append("use_filesystem=no");
                }

                if (!m_useGui)
                {
                    options.append("gui=no");
                }

                json compileArgs;
                compileArgs["ENV"] = "";
                compileArgs["OPT"] = options;

                if (m_runner == 2)
                {
                    compileArgs["JOB"] = m_sbJob;
                    compileArgs["OUT"] = m_sbOut;
                    compileArgs["ERR"] = m_sbErr;
                    compileArgs["TIM"] = m_sbTim;
                }

                recompileMsg.args = compileArgs;

                if (m_checkpointBeforeRecomp)
                {
                    m_communicator.sendToServer(messageToJsonString(xlpmg::WRITE_CHECKPOINT));
                }
                m_communicator.sendToServer(messageToJsonString(recompileMsg));
                m_connected = false;
            }
            ImGui::End();
        }

        //-------------------------------------------//
        //-----------SIMULATION PARAMETERS-----------//
        //-------------------------------------------//
        if (showSimulationParameterWindow)
        {

            ImGui::SetNextWindowSize(ImVec2(550, 324), ImGuiCond_FirstUseEver);
            ImGui::Begin("Simulation parameters", &showSimulationParameterWindow);

            short width = 24;

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::Combo("Tsunami Event", &m_tsunamiEvent, m_tsunamiEvents, IM_ARRAYSIZE(m_tsunamiEvents));

            ImGui::BeginDisabled(m_tsunamiEvent != 0);
            if (ImGui::TreeNode("Bathymetry"))
            {
                // open file dialog when user clicks this button
                if (ImGui::Button("Select bathymetry file"))
                    fileDialogBath.Open();

                fileDialogBath.Display();

                if (fileDialogBath.HasSelected())
                {
                    m_bathymetryFilePath = fileDialogBath.GetSelected().string();
                    fileDialogBath.ClearSelected();
                }

                ImGui::SameLine();
                ImGui::TextWrapped("%s", ("File: " + m_bathymetryFilePath).c_str());

                // send bathymetry
                ImGui::PushID(438);
                if (ImGui::Button("Send to server"))
                {
                    xlpmg::Message l_bathymetryDataMsg = xlpmg::SET_BATHYMETRY_DATA;
                    std::ifstream l_bathFile(m_bathymetryFilePath, std::ios::binary);
                    l_bathFile.unsetf(std::ios::skipws);
                    std::streampos l_fileSize;
                    l_bathFile.seekg(0, std::ios::end);
                    l_fileSize = l_bathFile.tellg();
                    l_bathFile.seekg(0, std::ios::beg);
                    std::vector<std::uint8_t> vec;
                    vec.reserve(l_fileSize);
                    vec.insert(vec.begin(),
                               std::istream_iterator<std::uint8_t>(l_bathFile),
                               std::istream_iterator<std::uint8_t>());
                    l_bathymetryDataMsg.args = json::binary(vec);
                    m_communicator.sendToServer(xlpmg::messageToJsonString(l_bathymetryDataMsg));
                }
                if (ImGui::Button("Load dimensions from file"))
                {
                    // set local variables
                    tsunami_lab::t_idx l_nCellsX, l_nCellsY;
                    tsunami_lab::io::NetCdf::getDimensionSize(m_bathymetryFilePath.c_str(), "x", l_nCellsX);
                    tsunami_lab::io::NetCdf::getDimensionSize(m_bathymetryFilePath.c_str(), "y", l_nCellsY);

                    tsunami_lab::t_real *m_xData = new tsunami_lab::t_real[l_nCellsX];
                    tsunami_lab::t_real *m_yData = new tsunami_lab::t_real[l_nCellsY];
                    tsunami_lab::t_real *m_data = new tsunami_lab::t_real[l_nCellsX * l_nCellsY];
                    tsunami_lab::io::NetCdf::read(m_bathymetryFilePath.c_str(),
                                                  "z",
                                                  &m_xData,
                                                  &m_yData,
                                                  &m_data);
                    m_nx = l_nCellsX;
                    m_ny = l_nCellsY;
                    if (m_xData[0] < m_xData[l_nCellsX - 1])
                    {
                        m_simulationSizeX = m_xData[l_nCellsX - 1] - m_xData[0];
                    }
                    else
                    {
                        m_simulationSizeX = m_xData[0] - m_xData[l_nCellsX - 1];
                    }
                    if (m_yData[0] < m_yData[l_nCellsY - 1])
                    {
                        m_simulationSizeY = m_yData[l_nCellsY - 1] - m_yData[0];
                    }
                    else
                    {
                        m_simulationSizeY = m_yData[0] - m_yData[l_nCellsY - 1];
                    }
                    m_offsetX = m_xData[0];
                    m_offsetY = m_yData[0];
                }
                ImGui::SameLine();
                HelpMarker("Sets cell amount, simulation size and offset based on estimates from the loaded file.");
                ImGui::PopID();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Displacement"))
            {
                if (ImGui::Button("Select displacement file"))
                    fileDialogDis.Open();

                fileDialogDis.Display();

                if (fileDialogDis.HasSelected())
                {
                    m_displacementFilePath = fileDialogDis.GetSelected().string();
                    fileDialogDis.ClearSelected();
                }
                ImGui::SameLine();
                ImGui::TextWrapped("%s", ("File: " + m_displacementFilePath).c_str());

                // send displacement
                ImGui::PushID(439);
                if (ImGui::Button("Send to server"))
                {
                    xlpmg::Message l_displacementMsg = xlpmg::SET_DISPLACEMENT_DATA;
                    std::ifstream l_displFile(m_displacementFilePath, std::ios::binary);
                    l_displFile.unsetf(std::ios::skipws);
                    std::streampos l_fileSize;
                    l_displFile.seekg(0, std::ios::end);
                    l_fileSize = l_displFile.tellg();
                    l_displFile.seekg(0, std::ios::beg);
                    std::vector<std::uint8_t> vec;
                    vec.reserve(l_fileSize);
                    vec.insert(vec.begin(),
                               std::istream_iterator<std::uint8_t>(l_displFile),
                               std::istream_iterator<std::uint8_t>());
                    l_displacementMsg.args = json::binary(vec);
                    m_communicator.sendToServer(xlpmg::messageToJsonString(l_displacementMsg));
                }
                if (ImGui::Button("Load dimensions from file"))
                {
                    // set local variables
                    tsunami_lab::t_idx l_nCellsX, l_nCellsY;
                    tsunami_lab::io::NetCdf::getDimensionSize(m_displacementFilePath.c_str(), "x", l_nCellsX);
                    tsunami_lab::io::NetCdf::getDimensionSize(m_displacementFilePath.c_str(), "y", l_nCellsY);

                    tsunami_lab::t_real *m_xData = new tsunami_lab::t_real[l_nCellsX];
                    tsunami_lab::t_real *m_yData = new tsunami_lab::t_real[l_nCellsY];
                    tsunami_lab::t_real *m_data = new tsunami_lab::t_real[l_nCellsX * l_nCellsY];
                    tsunami_lab::io::NetCdf::read(m_displacementFilePath.c_str(),
                                                  "z",
                                                  &m_xData,
                                                  &m_yData,
                                                  &m_data);
                    m_nx = l_nCellsX;
                    m_ny = l_nCellsY;
                    if (m_xData[0] < m_xData[l_nCellsX - 1])
                    {
                        m_simulationSizeX = m_xData[l_nCellsX - 1] - m_xData[0];
                    }
                    else
                    {
                        m_simulationSizeX = m_xData[0] - m_xData[l_nCellsX - 1];
                    }
                    if (m_yData[0] < m_yData[l_nCellsY - 1])
                    {
                        m_simulationSizeY = m_yData[l_nCellsY - 1] - m_yData[0];
                    }
                    else
                    {
                        m_simulationSizeY = m_yData[0] - m_yData[l_nCellsY - 1];
                    }
                    m_offsetX = m_xData[0];
                    m_offsetY = m_yData[0];
                }
                ImGui::SameLine();
                HelpMarker("Sets cell amount, simulation size and offset based on estimates from the loaded file.");
                ImGui::PopID();
                ImGui::TreePop();
            }

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputInt("Cells X", &m_nx, 0);
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputInt("Cells Y", &m_ny, 0);
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputInt("Coarse output resolution", &m_nk, 0);
            m_nx = abs(m_nx);
            m_ny = abs(m_ny);
            m_nk = abs(m_nk);

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputFloat("Simulation size X", &m_simulationSizeX, 0);
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputFloat("Simulation size Y", &m_simulationSizeY, 0);
            m_simulationSizeX = abs(m_simulationSizeX);
            m_simulationSizeY = abs(m_simulationSizeY);

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputFloat("Offset X", &m_offsetX, 0);
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputFloat("Offset Y", &m_offsetY, 0);

            ImGui::EndDisabled();

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputInt("End time", &m_endTime, 0);
            ImGui::SetItemTooltip("in simulated seconds");
            m_endTime = abs(m_endTime);

            // FILE I/O
            if (ImGui::TreeNode("File I/O"))
            {

                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::Checkbox("Use file I/O", &m_useFileIO);
                ImGui::SetItemTooltip("The value of this checkbox will only take effect after sending the config to the server.");

                if (ImGui::Button("Enable file I/O"))
                {
                    xlpmg::Message toggleFileIOMsg = xlpmg::TOGGLE_FILEIO;
                    toggleFileIOMsg.args = "true";
                    m_communicator.sendToServer(messageToJsonString(toggleFileIOMsg));
                    m_useFileIO = true;
                }
                ImGui::SetItemTooltip("You may use this button during a running simulation and it should take effect immediately.");
                ImGui::SameLine();
                if (ImGui::Button("Disable file I/O"))
                {
                    xlpmg::Message toggleFileIOMsg = xlpmg::TOGGLE_FILEIO;
                    toggleFileIOMsg.args = "false";
                    m_communicator.sendToServer(messageToJsonString(toggleFileIOMsg));
                    m_useFileIO = false;
                }
                ImGui::SetItemTooltip("You may use this button during a running simulation and it should take effect immediately.");

                ImGui::BeginDisabled(!m_useFileIO);

                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::Combo("Output Method", &m_outputMethod, m_outputMethods, IM_ARRAYSIZE(m_outputMethods));

                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::InputInt("Writing frequency", &m_writingFrequency, 0);
                ImGui::SetItemTooltip("in time steps");
                ImGui::SameLine();
                HelpMarker("Sets the frequency of writing into a solution file.");
                m_writingFrequency = abs(m_writingFrequency);

                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::InputText("Output file name", m_outputFileName, IM_ARRAYSIZE(m_outputFileName));

                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::InputInt("Station capture frequency", &m_stationFrequency, 0);
                ImGui::SetItemTooltip("in simulated seconds");
                ImGui::SameLine();
                HelpMarker("Sets the frequency with which stations capture data.");
                m_writingFrequency = abs(m_writingFrequency);

                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::InputInt("Checkpointing frequency", &m_checkpointFrequency, 0);
                ImGui::SetItemTooltip("in real-time seconds");
                ImGui::SameLine();
                HelpMarker("A negative frequency will disable file output.");

                ImGui::EndDisabled();

                ImGui::TreePop();
            }
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            bool outFlowNode = ImGui::TreeNode("Boundary conditions");
            ImGui::SameLine();
            HelpMarker("Determines whether the wave leaves the corresponding domain or hits an invisible boundary.");
            if (outFlowNode)
            {
                ImGui::TextWrapped("Check if that side has a boundary, preventing the waves to leave the domain.");
                ImGui::Checkbox("Left", &m_boundaryL);
                ImGui::Checkbox("Right", &m_boundaryR);
                ImGui::Checkbox("Top", &m_boundaryT);
                ImGui::Checkbox("Bottom", &m_boundaryB);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Stations"))
            {
                ImGui::InputText("Station name", m_currStationName, IM_ARRAYSIZE(m_currStationName));
                ImGui::InputFloat("Location X", &m_currStationX, 0);
                ImGui::InputFloat("Location Y", &m_currStationY, 0);

                if (ImGui::Button("Add"))
                {
                    if (strlen(m_currStationName) > 0)
                    {
                        bool l_nameExists = false;
                        for (Station l_s : m_stations)
                        {
                            if (l_s.name == m_currStationName)
                            {
                                l_nameExists = true;
                                break;
                            }
                        }

                        if (!l_nameExists)
                        {
                            Station l_station = {m_currStationName, m_currStationX, m_currStationY};
                            m_stations.push_back(l_station);
                        }
                    }
                }

                ImGui::BeginListBox("Stations");
                auto it = m_stations.begin();
                while (it != m_stations.end())
                {
                    std::string name = it->name + " (" + std::to_string(it->positionX) + ", " + std::to_string(it->positionY) + ")";
                    if (ImGui::Selectable(name.c_str(), it->isSelected))
                    {
                        it = m_stations.erase(it);
                    }
                    else
                    {
                        it++;
                    }
                }
                ImGui::EndListBox();
                ImGui::SetItemTooltip("Click to remove");

                ImGui::TreePop();
            }

            if (ImGui::Button("Update server with changes"))
            {
                xlpmg::Message saveConfigMsg = xlpmg::LOAD_CONFIG_JSON;
                saveConfigMsg.args = createConfigJson();
                m_communicator.sendToServer(xlpmg::messageToJsonString(saveConfigMsg));
            }
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGui::SeparatorText("WARNING");
            ImGui::PopStyleColor();
            ImGui::TextWrapped("Nobody will stop you from changing these parameters at any time, even during simulations.");
            ImGui::TextWrapped("This could lead to unwanted results: for example changing the cell amount and then writing into a previous netcdf solution file could result in its corruption.");
            ImGui::Spacing();
            ImGui::TextWrapped("Our advice: only set config data before running a simulation and if you are not writing into an existing solution file.");
            ImGui::End();
        }

        //-------------------------------------------//
        //-------------CONNECTION STATUS-------------//
        //-------------------------------------------//
        ImGuiWindowFlags l_cSFLags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = work_pos.x;
        window_pos.y = work_pos.y;
        window_pos_pivot.x = 0.0f;
        window_pos_pivot.y = 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::Begin("Connection status", nullptr, l_cSFLags);
        if (m_connected)
        {
            ImGui::Text("%s", ("CONNECTED TO " + std::string(IPADDRESS) + ":" + std::to_string(PORT)).c_str());
        }
        else
        {
            ImGui::Text("NOT CONNECTED");
        }
        ImGui::End();

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