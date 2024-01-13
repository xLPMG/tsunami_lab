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
#include "xlpmg/Communicator.hpp"
#include "xlpmg/communicator_api.h"
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
        // TODO: update
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
    json config = {{"solver", "fwave"},
                   {"nx", l_simulationSizeX},
                   {"ny", l_simulationSizeY},
                   {"endTime", endTime},
                   {"writingFrequency", writingFrequency},
                   {"hasBoundaryL", outflowL},
                   {"hasBoundaryR", outflowR},
                   {"hasBoundaryT", outflowT},
                   {"hasBoundaryB", outflowB}};

    if (event_current == 1)
    {
        config.push_back({"setup", "tohoku"});
        config.push_back({"outputFileName", "tohoku_solution"});
    }
    else
    {
        config.push_back({"setup", "chile"});
        config.push_back({"outputFileName", "chile_solution"});
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
    bool m_connected = false;

    bool show_demo_window = false;
    bool showCompilerOptionsWindow = false;
    bool showClientLog = false;
    bool showSimulationParameterWindow = false;

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
            ImGui::Begin("Welcome to the Tsunami Simulator GUI!");
            //-------------------------------------------//
            //-----------------MAIN TABS-----------------//
            //-------------------------------------------//
            if (ImGui::BeginTabBar("MainTabs"))
            {
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

                    ImGui::SeparatorText("GUI USAGE:");
                    ImGui::EndTabItem();
                }

                //--------------------------------------------//
                //----------------CONNECTIVITY----------------//
                //--------------------------------------------//
                if (ImGui::BeginTabItem("Connectivity"))
                {
                    ImGui::InputText("IP address", &IPADDRESS[0], IM_ARRAYSIZE(IPADDRESS));
                    ImGui::InputInt("Port", &PORT, 0, 20000);
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
                        if (m_communicator.sendToServer(messageToJsonString(xlpmg::CHECK_MESSAGE)) == 0)
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
                        if (m_communicator.sendToServer(messageToJsonString(xlpmg::SHUTDOWN_SERVER_MESSAGE)) == 0)
                        {
                            m_connected = false;
                        }
                    }
                    ImGui::PopStyleColor(3);
                    ImGui::EndDisabled();

                    ImGui::EndTabItem();
                }

                //------------------------------------------//
                //--------------UNSROTED ITEMS--------------//
                //------------------------------------------//
                if (ImGui::BeginTabItem("TODO"))
                {
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

                    if (ImGui::Button("file io true"))
                    {
                        xlpmg::Message toggleFileIOMsg = xlpmg::TOGGLE_FILEIO_MESSAGE;
                        toggleFileIOMsg.args = "true";
                        m_communicator.sendToServer(messageToJsonString(toggleFileIOMsg));
                    }

                    if (ImGui::Button("file io false"))
                    {
                        xlpmg::Message toggleFileIOMsg = xlpmg::TOGGLE_FILEIO_MESSAGE;
                        toggleFileIOMsg.args = "false";
                        m_communicator.sendToServer(messageToJsonString(toggleFileIOMsg));
                    }

                    ImGui::EndTabItem();
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
                ImGui::EndTabBar();
            }

            ImGui::Separator();

            ImGui::ColorEdit3("clear color", (float *)&clear_color);

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
                m_communicator.clearClientLog();
            }
            ImGui::SameLine();
            ImGui::Checkbox("Auto-Scroll", &m_clientLogAutoScroll);
            if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                m_communicator.getClientLog(m_clientLog);
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

            if (ImGui::Button("Recompile"))
            {
                xlpmg::Message recompileMsg = xlpmg::RECOMPILE_MESSAGE;

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

                recompileMsg.args = compileArgs;
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
            if (disableConfigs == false)
            {
                ImGui::Begin("Simulation parameters", &showSimulationParameterWindow);

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

                if (ImGui::BeginMenu("Outflow"))
                {
                    ImGui::Checkbox("Outflow Left", &outflowL);
                    ImGui::Checkbox("Outflow Right", &outflowR);
                    ImGui::Checkbox("Outflow Top", &outflowT);
                    ImGui::Checkbox("Outflow Bottom", &outflowB);
                    ImGui::EndMenu();
                }
                ImGui::SameLine();
                HelpMarker("Determines whether the wave leaves the corresponding domain.");

                if (ImGui::Button("Set config"))
                {
                    xlpmg::Message saveConfigMsg = xlpmg::LOAD_CONFIG_JSON_MESSAGE;
                    saveConfigMsg.args = createJson();
                    m_communicator.sendToServer(xlpmg::messageToJsonString(saveConfigMsg));
                }

                ImGui::End();
            }
        }

        // Connectivity status
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