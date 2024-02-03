/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Entry point of the GUI.
 **/

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>
#include <imfilebrowser.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GUI.h"
#include "xlpmg/Communicator.hpp"
#include "xlpmg/communicator_api.h"
#include "../constants.h"
#include "../io/NetCdf.h"
#include "../io/Csv.h"

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
#include <vector>

#include <thread>
#include <future>

#include "../systeminfo/SystemInfo.h"

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

void tsunami_lab::ui::GUI::updateSystemInfo()
{
    if (m_connected)
    {
        m_communicator.sendToServer(xlpmg::messageToJsonString(xlpmg::GET_SYSTEM_INFORMATION), m_logSystemInfoDataTransmission);
        std::string l_responseString = m_communicator.receiveFromServer(m_logSystemInfoDataTransmission);
        if (json::accept(l_responseString))
        {
            xlpmg::Message l_responseMessage = xlpmg::jsonToMessage(json::parse(l_responseString));
            m_usedRAM = l_responseMessage.args.value("USED_RAM", (double)0);
            m_totalRAM = l_responseMessage.args.value("TOTAL_RAM", (double)0);
            if (l_responseMessage.args.contains("CPU_USAGE"))
            {
                m_cpuData = l_responseMessage.args["CPU_USAGE"].get<std::vector<float>>();
            }
        }
    }
}

void tsunami_lab::ui::GUI::updateTimeValues()
{
    m_communicator.sendToServer(messageToJsonString(xlpmg::GET_TIME_VALUES), m_logTimeValuesDataTransmission);
    std::string response = m_communicator.receiveFromServer(m_logTimeValuesDataTransmission);
    if (json::accept(response))
    {
        xlpmg::Message responseMessage = xlpmg::jsonToMessage(json::parse(response));
        m_currentTimeStep = responseMessage.args.value("currentTimeStep", (int)0);
        m_maxTimeSteps = responseMessage.args.value("maxTimeStep", (int)0);
        m_timePerTimeStep = responseMessage.args.value("timePerTimeStep", (int)0);
        m_estimatedTimeLeft = ((m_maxTimeSteps - m_currentTimeStep) * m_timePerTimeStep) / 1000;
        m_simulationStatus = responseMessage.args.value("status", "UNKNOWN");
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
                   {"boundaryL", m_boundaryL ? "WALL" : "OUTFLOW"},
                   {"boundaryR", m_boundaryR ? "WALL" : "OUTFLOW"},
                   {"boundaryT", m_boundaryT ? "WALL" : "OUTFLOW"},
                   {"boundaryB", m_boundaryB ? "WALL" : "OUTFLOW"},
                   {"setup", m_tsunamiEvents[m_tsunamiEvent]},
                   {"bathymetry", m_bathymetryFilePath},
                   {"displacement", m_displacementFilePath},
                   {"height", m_height},
                   {"timeStepScaling", m_timeStepScaling},
                   {"baseHeight", m_baseHeight},
                   {"diameter", m_diameter}};
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
    ImPlot::CreateContext();
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
    bool show_demo_window = false;
    bool showCompilerOptionsWindow = false;
    bool showClientLog = false;
    bool showSimulationParameterWindow = false;
    bool showSystemInfoWindow = false;
    bool showStationDataVisualizer = false;
    bool showStationsWindow = false;
    bool showDataVisualizer = false;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui::FileBrowser fileBrowser;
    ImGui::FileBrowser fileDialogStation;

    // (optional) set browser properties
    fileBrowser.SetTitle("Filesystem");
    fileDialogStation.SetTypeFilters({".csv"});
    ImColor m_rbColormapRGB[256] = {
        {59, 76, 192}, {59, 76, 192}, {60, 78, 194}, {61, 80, 195}, {62, 81, 197}, {64, 83, 198}, {65, 85, 200}, {66, 87, 201}, {67, 88, 203}, {68, 90, 204}, {69, 92, 206}, {71, 93, 207}, {72, 95, 209}, {73, 97, 210}, {74, 99, 211}, {75, 100, 213}, {77, 102, 214}, {78, 104, 215}, {79, 105, 217}, {80, 107, 218}, {82, 109, 219}, {83, 110, 221}, {84, 112, 222}, {85, 114, 223}, {87, 115, 224}, {88, 117, 225}, {89, 119, 227}, {90, 120, 228}, {92, 122, 229}, {93, 124, 230}, {94, 125, 231}, {96, 127, 232}, {97, 129, 233}, {98, 130, 234}, {100, 132, 235}, {101, 133, 236}, {102, 135, 237}, {103, 137, 238}, {105, 138, 239}, {106, 140, 240}, {107, 141, 240}, {109, 143, 241}, {110, 144, 242}, {111, 146, 243}, {113, 147, 244}, {114, 149, 244}, {116, 150, 245}, {117, 152, 246}, {118, 153, 246}, {120, 155, 247}, {121, 156, 248}, {122, 157, 248}, {124, 159, 249}, {125, 160, 249}, {127, 162, 250}, {128, 163, 250}, {129, 164, 251}, {131, 166, 251}, {132, 167, 252}, {133, 168, 252}, {135, 170, 252}, {136, 171, 253}, {138, 172, 253}, {139, 174, 253}, {140, 175, 254}, {142, 176, 254}, {143, 177, 254}, {145, 179, 254}, {146, 180, 254}, {147, 181, 255}, {149, 182, 255}, {150, 183, 255}, {152, 185, 255}, {153, 186, 255}, {154, 187, 255}, {156, 188, 255}, {157, 189, 255}, {158, 190, 255}, {160, 191, 255}, {161, 192, 255}, {163, 193, 255}, {164, 194, 254}, {165, 195, 254}, {167, 196, 254}, {168, 197, 254}, {169, 198, 254}, {171, 199, 253}, {172, 200, 253}, {173, 201, 253}, {175, 202, 252}, {176, 203, 252}, {177, 203, 252}, {179, 204, 251}, {180, 205, 251}, {181, 206, 250}, {183, 207, 250}, {184, 207, 249}, {185, 208, 249}, {186, 209, 248}, {188, 209, 247}, {189, 210, 247}, {190, 211, 246}, {191, 211, 246}, {193, 212, 245}, {194, 213, 244}, {195, 213, 243}, {196, 214, 243}, {198, 214, 242}, {199, 215, 241}, {200, 215, 240}, {201, 216, 239}, {202, 216, 239}, {204, 217, 238}, {205, 217, 237}, {206, 217, 236}, {207, 218, 235}, {208, 218, 234}, {209, 218, 233}, {210, 219, 232}, {211, 219, 231}, {212, 219, 230}, {214, 220, 229}, {215, 220, 228}, {216, 220, 227}, {217, 220, 225}, {218, 220, 224}, {219, 220, 223}, {220, 221, 222}, {221, 221, 221}, {222, 220, 219}, {223, 220, 218}, {224, 219, 216}, {225, 219, 215}, {226, 218, 214}, {227, 218, 212}, {228, 217, 211}, {229, 216, 209}, {230, 216, 208}, {231, 215, 206}, {232, 215, 205}, {233, 214, 203}, {233, 213, 202}, {234, 212, 200}, {235, 212, 199}, {236, 211, 197}, {237, 210, 196}, {237, 209, 194}, {238, 208, 193}, {239, 208, 191}, {239, 207, 190}, {240, 206, 188}, {240, 205, 187}, {241, 204, 185}, {242, 203, 183}, {242, 202, 182}, {243, 201, 180}, {243, 200, 179}, {243, 199, 177}, {244, 198, 176}, {244, 197, 174}, {245, 196, 173}, {245, 195, 171}, {245, 194, 169}, {246, 193, 168}, {246, 192, 166}, {246, 190, 165}, {246, 189, 163}, {247, 188, 161}, {247, 187, 160}, {247, 186, 158}, {247, 184, 157}, {247, 183, 155}, {247, 182, 153}, {247, 181, 152}, {247, 179, 150}, {247, 178, 149}, {247, 177, 147}, {247, 175, 146}, {247, 174, 144}, {247, 172, 142}, {247, 171, 141}, {247, 170, 139}, {247, 168, 138}, {247, 167, 136}, {247, 165, 135}, {246, 164, 133}, {246, 162, 131}, {246, 161, 130}, {246, 159, 128}, {245, 158, 127}, {245, 156, 125}, {245, 155, 124}, {244, 153, 122}, {244, 151, 121}, {243, 150, 119}, {243, 148, 117}, {242, 147, 116}, {242, 145, 114}, {241, 143, 113}, {241, 142, 111}, {240, 140, 110}, {240, 138, 108}, {239, 136, 107}, {239, 135, 105}, {238, 133, 104}, {237, 131, 102}, {237, 129, 101}, {236, 128, 99}, {235, 126, 98}, {235, 124, 96}, {234, 122, 95}, {233, 120, 94}, {232, 118, 92}, {231, 117, 91}, {230, 115, 89}, {230, 113, 88}, {229, 111, 86}, {228, 109, 85}, {227, 107, 84}, {226, 105, 82}, {225, 103, 81}, {224, 101, 79}, {223, 99, 78}, {222, 97, 77}, {221, 95, 75}, {220, 93, 74}, {219, 91, 73}, {218, 89, 71}, {217, 87, 70}, {215, 85, 69}, {214, 82, 67}, {213, 80, 66}, {212, 78, 65}, {211, 76, 64}, {210, 74, 62}, {208, 71, 61}, {207, 69, 60}, {206, 67, 59}, {204, 64, 57}, {203, 62, 56}, {202, 59, 55}, {200, 57, 54}, {199, 54, 53}, {198, 52, 51}, {196, 49, 50}, {195, 46, 49}, {193, 43, 48}, {192, 40, 47}, {191, 37, 46}, {189, 34, 44}, {188, 30, 43}, {186, 26, 42}, {185, 22, 41}, {183, 17, 40}, {182, 11, 39}, {180, 4, 38}};
    ImVec4 m_rbColormapVec4[256];
    for (long i = 0; i < 256; i++)
    {
        m_rbColormapVec4[i] = m_rbColormapRGB[i];
    }
    ImPlot::AddColormap("WATERHEIGHTSMAP", &m_rbColormapVec4[0], 256, false);

    tsunami_lab::systeminfo::SystemInfo systemInfo;
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

        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
            ImPlot::ShowDemoWindow();
        }

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
                    ImGui::SeparatorText("Connect your client to the server.");

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
                        m_communicator.sendToServer(xlpmg::messageToJsonString(xlpmg::CHECK));
                        m_connected = m_communicator.isConnected;
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

                    if (ImGui::TreeNode("Buffer settings"))
                    {
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
                        ImGui::TreePop();
                    }
                    ImGui::EndTabItem();
                }
                //-------------------------------------------//
                //------------Simulation Controls------------//
                //-------------------------------------------//
                if (ImGui::BeginTabItem("Simulation Controls"))
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2 / 7.0f, 1.0f, 0.8f));

                    ImGui::SeparatorText("Change the state of the simulation.");

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

                    if (ImGui::Button("Pause Simulation"))
                    {
                        if (!m_isPausing)
                        {
                            m_communicator.sendToServer(messageToJsonString(xlpmg::PAUSE_SIMULATION));
                            m_isPausing = true;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Continue Simulation"))
                    {
                        if (m_isPausing)
                        {
                            m_communicator.sendToServer(messageToJsonString(xlpmg::CONTINUE_SIMULATION));
                            m_isPausing = false;
                        }
                    }

                    // SIMULATION STATUS
                    ImGui::SeparatorText("Simulation status");
                    if (m_lastTimeValuesUpdate <= std::chrono::system_clock::now() && m_connected && m_timeValuesUpdateFrequency > 0)
                    {
                        updateTimeValues();
                        m_lastTimeValuesUpdate = std::chrono::system_clock::now() + std::chrono::seconds(m_timeValuesUpdateFrequency);
                    }
                    ImGui::Text("STATUS: %s", m_simulationStatus.c_str());

                    if (m_simulationStatus == "CALCULATING")
                    {
                        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 24);
                        ImGui::ProgressBar((float)m_currentTimeStep / (float)m_maxTimeSteps, ImVec2(0.0f, 0.0f));
                        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                        ImGui::Text("%% complete");
                    }

                    ImGui::Text("%i / %i time steps", m_currentTimeStep, m_maxTimeSteps);
                    ImGui::SameLine();
                    HelpMarker("The continuous dimension of time is discretized using 'time steps'. The length of a time step is computed anew for each simulation in a way such that the waves do not interact with each other.");

                    ImGui::Text("Time per time step: %ims", m_timePerTimeStep);
                    ImGui::SetItemTooltip("in milliseconds");
                    ImGui::SameLine();
                    HelpMarker("The time the solver takes to compute one time step.");

                    ImGui::Text("Estimated time left: %is (= %f min)", (int)m_estimatedTimeLeft, m_estimatedTimeLeft / 60);
                    ImGui::SetItemTooltip("in seconds");
                    ImGui::SameLine();
                    HelpMarker("Time until the solver has finished the whole computation.");

                    ImGui::PushID(472);
                    if (ImGui::TreeNode("Update settings"))
                    {
                        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 16);
                        ImGui::InputInt("Update frequency", &m_timeValuesUpdateFrequency, 0);
                        ImGui::SetItemTooltip("in seconds. Set to 0 for no update.");
                        ImGui::SameLine();
                        HelpMarker("The server will update data with a constant frequency. With this option, you can only specify the frequency with which the client/gui is getting that data from the server.");
                        m_timeValuesUpdateFrequency = abs(m_timeValuesUpdateFrequency);

                        ImGui::Checkbox("Log data transmission", &m_logTimeValuesDataTransmission);
                        ImGui::TreePop();
                    }
                    ImGui::PopID();

                    // CLEANUP
                    ImGui::SeparatorText("Cleanup");

                    if (ImGui::Button("Delete checkpoint"))
                    {
                        xlpmg::Message deleteCPMsg = xlpmg::DELETE_CHECKPOINTS;
                        m_communicator.sendToServer(messageToJsonString(deleteCPMsg));
                    }
                    ImGui::EndTabItem();
                }
                //-------------------------------------------//
                //------------Simulation Options-------------//
                //-------------------------------------------//
                if (ImGui::BeginTabItem("Windows"))
                {
                    ImGui::SeparatorText("Windows for modification and observation of the simulation.");
                    if (ImGui::BeginTabBar("Options"))
                    {
                        if (ImGui::BeginTabItem("Configuration"))
                        {
                            ImGui::Checkbox("Edit simulation parameters", &showSimulationParameterWindow);
                            ImGui::Checkbox("Manage stations", &showStationsWindow);
                            ImGui::Checkbox("Edit compiler/runtime options", &showCompilerOptionsWindow);
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("Observation tools"))
                        {
                            ImGui::Checkbox("Show station data visualizer", &showStationDataVisualizer);
                            ImGui::Checkbox("Show data visualizer", &showDataVisualizer);
                            ImGui::Checkbox("Show client log", &showClientLog);
                            ImGui::Checkbox("Show system info", &showSystemInfoWindow);
                            ImGui::EndTabItem();
                        }
                        ImGui::EndTabBar();
                    }
                    ImGui::EndTabItem();
                }
                //---------------------------------------------//
                //----------------FILE TRANSFER----------------//
                //---------------------------------------------//
                if (ImGui::BeginTabItem("File transfer"))
                {
                    ImGui::SeparatorText("Send and receive files from the server.");
                    if (ImGui::CollapsingHeader("Send to server"))
                    {
                        ImGui::Indent();
                        if (ImGui::Button("Select a file"))
                            fileBrowser.Open();

                        fileBrowser.Display();

                        if (fileBrowser.HasSelected())
                        {
                            strcpy(m_transferLocalFilePath, fileBrowser.GetSelected().c_str());
                            fileBrowser.ClearSelected();
                        }

                        ImGui::InputTextWithHint("Local file path", "or directly input the file path here", m_transferLocalFilePath, IM_ARRAYSIZE(m_transferLocalFilePath));

                        ImGui::InputTextWithHint("Remote file path", "./resources/filename.extension", m_transferRemoteFilePath, IM_ARRAYSIZE(m_transferRemoteFilePath));

                        if (ImGui::Button("Send file"))
                        {
                            if (strlen(m_transferLocalFilePath) > 0 && strlen(m_transferRemoteFilePath) > 0)
                            {
                                xlpmg::Message l_sendFileMsg = xlpmg::SEND_FILE;
                                json l_arguments;
                                l_arguments["path"] = m_transferRemoteFilePath;

                                std::ifstream l_fileData(m_transferLocalFilePath, std::ios::binary);
                                l_fileData.unsetf(std::ios::skipws);
                                std::streampos l_fileSize;
                                l_fileData.seekg(0, std::ios::end);
                                l_fileSize = l_fileData.tellg();
                                l_fileData.seekg(0, std::ios::beg);
                                std::vector<std::uint8_t> vec;
                                vec.reserve(l_fileSize);
                                vec.insert(vec.begin(),
                                           std::istream_iterator<std::uint8_t>(l_fileData),
                                           std::istream_iterator<std::uint8_t>());
                                l_arguments["data"] = json::binary(vec);
                                l_sendFileMsg.args = l_arguments;
                                m_communicator.sendToServer(xlpmg::messageToJsonString(l_sendFileMsg));
                            }
                        }
                        ImGui::Unindent();
                    }
                    if (ImGui::CollapsingHeader("Receive from server"))
                    {
                        ImGui::Indent();
                        ImGui::InputText("Remote file path", m_transferRemoteFilePath, IM_ARRAYSIZE(m_transferRemoteFilePath));
                        ImGui::InputText("Local file path", m_transferLocalFilePath, IM_ARRAYSIZE(m_transferLocalFilePath));
                        if (ImGui::Button("Receive file"))
                        {
                            xlpmg::Message l_rcvMsg = xlpmg::RECV_FILE;
                            json l_rcvArgs;
                            l_rcvArgs["path"] = m_transferLocalFilePath;
                            l_rcvArgs["pathDestination"] = m_transferRemoteFilePath;
                            l_rcvMsg.args = l_rcvArgs;
                            m_communicator.sendToServer(xlpmg::messageToJsonString(l_rcvMsg));

                            std::string l_response = m_communicator.receiveFromServer();
                            if (json::accept(l_response))
                            {
                                xlpmg::Message l_responseMsg = xlpmg::jsonToMessage(json::parse(l_response));
                                std::vector<std::uint8_t> l_byteVector = l_responseMsg.args["data"]["bytes"];
                                auto l_writeFile = std::fstream(l_responseMsg.args.value("path", ""), std::ios::out | std::ios::binary);
                                l_writeFile.write((char *)&l_byteVector[0], l_byteVector.size());
                                l_writeFile.close();
                            }
                        }
                        ImGui::Unindent();
                    }
                    ImGui::SeparatorText("INFO");
                    ImGui::TextWrapped("Our file transfer implementaion uses byte transfer over standard tcp sockets without additional security measures or performance improvements. For large or confident files we recommend using other services such as sftp.");
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

            if (m_runner != 0)
            {
                ImGui::InputInt("Server port", &m_serverRestartPort, 0);
                ImGui::SameLine();
                HelpMarker("Port which the server will use when starting.");
            }

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
                    options.append(" use_filesystem=no");
                }

                if (!m_useGui)
                {
                    options.append(" gui=no");
                }

                json compileArgs;
                compileArgs["ENV"] = "";
                compileArgs["OPT"] = options;
                compileArgs["POR"] = m_serverRestartPort;

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

            ImGui::InputTextWithHint("Config file path", "configs/config.json", m_configFilePath, IM_ARRAYSIZE(m_configFilePath));
            if (ImGui::Button("Load config"))
            {
                xlpmg::Message l_loadConfigMsg = xlpmg::LOAD_CONFIG_FILE;
                l_loadConfigMsg.args = m_configFilePath;
                m_communicator.sendToServer(messageToJsonString(l_loadConfigMsg));
            }

            ImGui::SeparatorText("Custom options");

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::Combo("Tsunami Event", &m_tsunamiEvent, m_tsunamiEvents, IM_ARRAYSIZE(m_tsunamiEvents));

            if (!strcmp(m_tsunamiEvents[m_tsunamiEvent], "CIRCULARDAMBREAK2D"))
            {
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::InputInt("Waterheight", &m_height, 0);
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::InputInt("Base water Height", &m_baseHeight, 0);
                ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
                ImGui::InputInt("Diameter", &m_diameter, 0);
            }

            if (m_tsunamiEvent > 0)
            {
                ImGui::BeginDisabled();
            }
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputText("Bathymetry file", m_bathymetryFilePath, IM_ARRAYSIZE(m_bathymetryFilePath));
            ImGui::SameLine();
            HelpMarker("Must be a valid bathymetry file on the file path. Path is specified relative to the executable.");

            ImGui::SetNextItemWidth(ImGui::GetFontSize() * width);
            ImGui::InputText("Displacement file", m_displacementFilePath, IM_ARRAYSIZE(m_displacementFilePath));
            ImGui::SameLine();
            HelpMarker("Must be a valid displacement file on the file path. Path is specified relative to the executable.");

            if (m_tsunamiEvent > 0)
            {
                ImGui::EndDisabled();
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

            if (m_tsunamiEvent == 1)
            {
                ImGui::BeginDisabled();
            }

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

            if (m_tsunamiEvent == 1)
            {
                ImGui::EndDisabled();
            }

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

            if (ImGui::TreeNode("Time step scaling"))
            {
                ImGui::Text("Current time step scaling: %f", m_timeStepScaling);
                ImGui::SliderFloat("Scaling (0-1)", &m_timeStepScaling, 0.1f, 1.0f, "%.2f");
                ImGui::SameLine();
                HelpMarker("The default value is 1. Smaller numbers lead to more timesteps and thus to a more detailed simulation. However this also increases the computation time.");
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

        if (showStationsWindow)
        {
            ImGui::Begin("Stations", &showStationsWindow);

            ImGui::InputText("Station name", m_currStationName, IM_ARRAYSIZE(m_currStationName));
            ImGui::InputFloat("Location X", &m_currStationX, 0);
            ImGui::InputFloat("Location Y", &m_currStationY, 0);

            if (ImGui::Button("Add to local list"))
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

            if (ImGui::BeginListBox("Local stations"))
            {
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
            }
            ImGui::SetItemTooltip("Click to remove");

            if (ImGui::Button("Add stations to server"))
            {
                xlpmg::Message l_addStationMessage = xlpmg::LOAD_STATIONS;
                json l_args;
                // stations
                for (Station l_s : m_stations)
                {
                    json l_stationData;
                    l_stationData["name"] = l_s.name;
                    l_stationData["locX"] = l_s.positionX;
                    l_stationData["locY"] = l_s.positionY;
                    l_args["stations"].push_back(l_stationData);
                }
                l_addStationMessage.args = l_args;
                m_communicator.sendToServer(messageToJsonString(l_addStationMessage));
            }

            ImGui::SameLine();

            if (ImGui::Button("Delete stations on server"))
            {
                xlpmg::Message deleteStationsMsg = xlpmg::DELETE_STATIONS;
                m_communicator.sendToServer(messageToJsonString(deleteStationsMsg));
            }

            ImGui::End();
        }
        //--------------------------------------------//
        //-------------SYSTEM INFORMATION-------------//
        //--------------------------------------------//
        if (showSystemInfoWindow)
        {
            if (m_lastSystemInfoUpdate <= std::chrono::system_clock::now())
            {
                updateSystemInfo();
                m_lastSystemInfoUpdate = std::chrono::system_clock::now() + std::chrono::seconds(m_systemInfoUpdateFrequency);
            }

            ImGui::Begin("System information", &showSystemInfoWindow);

            ImGui::Text("%f / %f GiB RAM usage", m_usedRAM, m_totalRAM);

            if (m_cpuData.size() > 0)
            {
                ImGui::ProgressBar(m_cpuData[0] / 100, ImVec2(0.0f, 0.0f));
                ImGui::SameLine();
                ImGui::Text("%% overall CPU usage");

                if (ImGui::TreeNode("Core info"))
                {
                    for (unsigned long i = 1; i < m_cpuData.size(); ++i)
                    {
                        ImGui::ProgressBar(m_cpuData[i] / 100, ImVec2(0.0f, 0.0f));
                        ImGui::SameLine();
                        ImGui::Text("%s %lu", "CPU: ", i - 1);
                    }
                    ImGui::TreePop();
                }
            }
            if (ImGui::TreeNode("Update settings"))
            {
                ImGui::InputInt("Update frequency", &m_systemInfoUpdateFrequency, 0);
                ImGui::SetItemTooltip("in seconds");
                ImGui::SameLine();
                HelpMarker("The server will update data with a constant frequency. With this option, you can only specify the frequency with which the client/gui is getting that data from the server.");
                m_systemInfoUpdateFrequency = abs(m_systemInfoUpdateFrequency);

                ImGui::Checkbox("Log data transmission", &m_logSystemInfoDataTransmission);
                ImGui::TreePop();
            }
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

        // STATIONS
        if (showStationDataVisualizer)
        {
            ImGui::Begin("Station data", &showStationDataVisualizer);

            if (ImGui::Button("Select station data file"))
                fileDialogStation.Open();

            fileDialogStation.Display();

            if (fileDialogStation.HasSelected())
            {
                m_stationFilePath = fileDialogStation.GetSelected().string();
                fileDialogStation.ClearSelected();

                m_stationTime.clear();
                m_stationMomentumX.clear();
                m_stationMomentumY.clear();
                m_stationBathymetry.clear();
                m_stationTotalHeight.clear();

                // load data
                std::ifstream l_inputFile(m_stationFilePath);
                std::vector<std::string> l_row;
                std::string l_line;
                std::getline(l_inputFile, l_line); // skip header
                while (getline(l_inputFile, l_line))
                {
                    tsunami_lab::io::Csv::splitLine(std::stringstream(l_line), ',', l_row);
                    m_stationTime.push_back(stof(l_row[0]));
                    m_stationMomentumX.push_back(stof(l_row[2]));
                    m_stationMomentumY.push_back(stof(l_row[3]));
                    m_stationBathymetry.push_back(stof(l_row[4]));
                    m_stationTotalHeight.push_back(stof(l_row[5]));
                }
            }
            ImGui::SameLine();
            ImGui::Text("Selected file: %s", m_stationFilePath.c_str());

            std::string l_plotName = m_stationFilePath.substr(m_stationFilePath.find_last_of("/\\") + 1) + ": heights";
            if (ImPlot::BeginPlot(l_plotName.c_str()))
            {
                ImPlot::SetupAxes("time in seconds", "height in metres");
                ImPlot::PlotLine("bathymetry", &m_stationTime[0], &m_stationBathymetry[0], m_stationTime.size());
                ImPlot::PlotLine("water level", &m_stationTime[0], &m_stationTotalHeight[0], m_stationTime.size());
                ImPlot::EndPlot();
            }
            l_plotName = m_stationFilePath.substr(m_stationFilePath.find_last_of("/\\") + 1) + ": momenta";
            if (ImPlot::BeginPlot(l_plotName.c_str()))
            {
                ImPlot::SetupAxes("time in seconds", "momentum in m^2/s");
                ImPlot::PlotLine("momentum in x-direction", &m_stationTime[0], &m_stationMomentumX[0], m_stationTime.size());
                ImPlot::PlotLine("momentum in y-direction", &m_stationTime[0], &m_stationMomentumY[0], m_stationTime.size());
                ImPlot::EndPlot();
            }
            ImGui::End();
        }
        if (showDataVisualizer)
        {
            ImGui::SetNextWindowSize(ImVec2(640, 640), ImGuiCond_FirstUseEver);
            ImGui::Begin("Data visualizer", &showDataVisualizer);
            if (ImGui::Button("Get data from server"))
            {
                m_communicator.sendToServer(messageToJsonString(xlpmg::GET_SIMULATION_SIZES));
                std::string l_res = m_communicator.receiveFromServer();
                if (json::accept(l_res))
                {
                    xlpmg::Message l_simSizes = xlpmg::jsonToMessage(json::parse(l_res));

                    if (m_heightData != nullptr)
                    {
                        delete[] m_heightData;
                        m_heightData = nullptr;
                    }
                    if (m_bathymetryData != nullptr)
                    {
                        delete[] m_bathymetryData;
                        m_bathymetryData = nullptr;
                    }

                    m_currCellsX = l_simSizes.args.value("cellsX", 0);
                    m_currCellsY = l_simSizes.args.value("cellsY", 0);
                    m_currOffsetX = l_simSizes.args.value("offsetX", 0.f);
                    m_currOffsetY = l_simSizes.args.value("offsetY", 0.f);
                    m_currSimSizeX = l_simSizes.args.value("simulationSizeX", 0.f);
                    m_currSimSizeY = l_simSizes.args.value("simulationSizeY", 0.f);
                    m_bathymetryData = new tsunami_lab::t_real[m_currCellsX * m_currCellsY]{0};
                    m_heightData = new tsunami_lab::t_real[m_currCellsX * m_currCellsY]{0};

                    if (m_communicator.sendToServer(messageToJsonString(xlpmg::GET_BATHYMETRY_DATA)) == 0)
                    {
                        std::string l_response = m_communicator.receiveFromServer(600);

                        if (json::accept(l_response))
                        {
                            xlpmg::Message msg = xlpmg::jsonToMessage(json::parse(l_response));
                            std::stringstream l_stream(msg.args.dump().substr(1, msg.args.dump().size() - 2));
                            std::string l_num;
                            unsigned long l_index = 0;
                            while (getline(l_stream, l_num, ','))
                            {
                                m_bathymetryData[l_index] = std::stof(l_num);
                                l_index++;
                            }
                        }
                    }

                    if (m_communicator.sendToServer(messageToJsonString(xlpmg::GET_HEIGHT_DATA)) == 0)
                    {
                        std::string l_response = m_communicator.receiveFromServer(600);

                        if (json::accept(l_response))
                        {
                            xlpmg::Message msg = xlpmg::jsonToMessage(json::parse(l_response));

                            std::stringstream l_stream(msg.args.dump().substr(1, msg.args.dump().size() - 2));
                            std::string l_num;
                            unsigned long l_index = 0;
                            while (getline(l_stream, l_num, ','))
                            {
                                m_heightData[l_index] = std::stof(l_num) + m_bathymetryData[l_index];
                                l_index++;
                            }
                        }
                    }
                }
            }

            ImGui::SameLine();
            HelpMarker("The GUI might freeze as its waiting for the server response. This might take several minutes depending on how many cells need to be processed.");

            ImGui::SetNextItemWidth(225);
            ImGui::DragFloatRange2("Min / Max", &m_scaleMin, &m_scaleMax, 0.01f, -20, 20);
            ImPlot::PushColormap("WATERHEIGHTSMAP");
            ImPlot::ColormapScale("Colormap scale", m_scaleMin, m_scaleMax, ImVec2(60, 225));
            ImGui::SameLine();
            if (m_bathymetryData != nullptr || m_heightData != nullptr)
            {

                if (ImPlot::BeginPlot("Water height and bathymetry", ImVec2(550, 550)))
                {
                    ImPlot::SetupAxesLimits(m_currOffsetX, m_currOffsetX + m_currSimSizeX, m_currOffsetY, m_currOffsetY + m_currSimSizeY);
                    if (m_bathymetryData != nullptr)
                    {
                        ImPlot::PlotHeatmap("bathymetry", m_bathymetryData, m_currCellsY, m_currCellsX, m_scaleMin, m_scaleMax, nullptr, ImPlotPoint(m_currOffsetX, m_currOffsetY), ImPlotPoint(m_currOffsetX + m_currSimSizeX, m_currOffsetY + m_currSimSizeY), 0);
                    }
                    if (m_heightData != nullptr)
                    {
                        ImPlot::PlotHeatmap("water level", m_heightData, m_currCellsY, m_currCellsX, m_scaleMin, m_scaleMax, nullptr, ImPlotPoint(m_currOffsetX, m_currOffsetY), ImPlotPoint(m_currOffsetX + m_currSimSizeX, m_currOffsetY + m_currSimSizeY), 0);
                    }
                    ImPlot::EndPlot();
                }
            }
            ImPlot::PopColormap();
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
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}