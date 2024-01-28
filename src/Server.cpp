#include "Simulator.h"
#include "xlpmg/Communicator.hpp"
#include "xlpmg/communicator_api.h"
#include <nlohmann/json.hpp>
#include "systeminfo/SystemInfo.h"

#include <thread>
#include <atomic>
using json = nlohmann::json;

int m_PORT = 8080;
bool m_EXIT = false;
tsunami_lab::Simulator *simulator = nullptr;
std::thread m_simulationThread;
std::thread m_updateThread;
std::atomic<bool> m_stopUpdating = false;
bool m_isSimulationRunning = false;

// updating
std::chrono::time_point m_lastDataUpdate = std::chrono::high_resolution_clock::now();
int m_dataUpdateFrequency = 10;

// temp files
std::string m_bathTempFile = "bathymetry_temp.nc";
std::string m_displTempFile = "displacement_temp.nc";

// system info
tsunami_lab::systeminfo::SystemInfo l_systemInfo;
double l_usedRAM = 0;
double l_totalRAM = 0;
std::vector<float> l_cpuUsage;

int execWithOutput(std::string i_cmd, std::string i_outputFile)
{
    std::string commandString = (i_cmd + " > " + i_outputFile + " 2>&1 &").data();
    const char *commandChars = commandString.data();
    return system(commandChars);
}

int exec(std::string i_cmd)
{
    return system(i_cmd.data());
}

void exitSimulationThread()
{
    std::cout << "Terminating as soon as possible..." << std::endl;
    simulator->shouldExit(true);
}

void checkSimThreadActive()
{
    if (simulator->isPreparing() || simulator->isCalculating() || simulator->isResetting())
    {
        m_isSimulationRunning = true;
    }
    else
    {
        if (m_simulationThread.joinable())
        {
            m_simulationThread.join();
        }
        m_isSimulationRunning = false;
    }
}

bool canRunThread()
{
    checkSimThreadActive();
    if (m_isSimulationRunning)
    {
        return false;
    }
    else
    {
        simulator->shouldExit(false);
        m_isSimulationRunning = true;
        return true;
    }
}

void updateData()
{
    if (m_lastDataUpdate <= std::chrono::high_resolution_clock::now())
    {
        l_cpuUsage = l_systemInfo.getCPUUsage();
        l_systemInfo.getRAMUsage(l_totalRAM, l_usedRAM);
        m_lastDataUpdate = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(m_dataUpdateFrequency);
    }
}

int main(int i_argc, char *i_argv[])
{
    int exitCode = 0;
    simulator = new tsunami_lab::Simulator;

    //------------------------------------------//
    //---------------SERVER MODE----------------//
    //------------------------------------------//
    if (i_argc >= 2 && (strcmp(i_argv[1], "server") == 0))
    {
        if (i_argc >= 3)
        {
            m_PORT = atoi(i_argv[2]);
        }

        xlpmg::Communicator l_communicator;
        l_communicator.startServer(m_PORT);
        if (canRunThread())
        {
            m_simulationThread = std::thread(&tsunami_lab::Simulator::prepareForCalculation, simulator);
        }

        m_updateThread = std::thread([]
                                     { while (!m_stopUpdating){ updateData();} });
        m_updateThread.detach();

        while (!m_EXIT)
        {
            std::string l_rawData = l_communicator.receiveFromClient();
            // check if client sent valid json, go back to reading if not
            if (!json::accept(l_rawData))
            {
                continue;
            }

            json l_parsedData = json::parse(l_rawData);
            xlpmg::Message l_message = xlpmg::jsonToMessage(l_parsedData);
            xlpmg::MessageType l_type = l_message.type;
            std::string l_key = l_message.key;
            json l_args = l_message.args;

            //-------------------------------------------//
            //---------------SERVER CALLS----------------//
            //-------------------------------------------//
            if (l_type == xlpmg::SERVER_CALL)
            {
                if (l_key == xlpmg::CHECK.key)
                {
                    l_communicator.sendToClient("OK");
                }
                else if (l_key == xlpmg::SHUTDOWN_SERVER.key)
                {
                    m_EXIT = true;
                    exitSimulationThread();
                    l_communicator.stopServer();
                }
                else if (l_key == xlpmg::START_SIMULATION.key)
                {
                    std::string l_config = l_args;
                    if (simulator->isPreparing() || simulator->isCalculating())
                    {
                        std::cout << "Warning: Did not start simulator because it is still preparing or calculating." << std::endl;
                    }
                    else
                    {
                        if (canRunThread())
                        {
                            m_simulationThread = std::thread(&tsunami_lab::Simulator::start, simulator, l_config);
                        }
                        else
                        {
                            std::cout << "Warning: Did not start simulator because it is still running." << std::endl;
                        }
                    }
                }
                else if (l_key == xlpmg::KILL_SIMULATION.key)
                {
                    exitSimulationThread();
                }
                else if (l_key == xlpmg::GET_SYSTEM_INFORMATION.key)
                {
                    xlpmg::Message l_response = {xlpmg::SERVER_RESPONSE, "system_information"};
                    json l_data;
                    l_data["USED_RAM"] = l_usedRAM;
                    l_data["TOTAL_RAM"] = l_totalRAM;
                    l_data["CPU_USAGE"] = l_cpuUsage;
                    l_response.args = l_data;
                    l_communicator.sendToClient(xlpmg::messageToJsonString(l_response), false);
                }
                else if (l_key == xlpmg::COMPILE.key)
                {
                    // Shutdown server
                    m_EXIT = true;
                    l_communicator.stopServer();
                    exitSimulationThread();

                    std::string env = l_args.value("ENV", ""); // environment var
                    std::string opt = l_args.value("OPT", ""); // compiler opt

                    // compile
                    exec("chmod +x scripts/compile-bash.sh");
                    exec("./scripts/compile-bash.sh \"" + env + "\" \"" + opt + "\" &");
                }
                else if (l_key == xlpmg::COMPILE_RUN_BASH.key)
                {
                    // Shutdown server
                    m_EXIT = true;
                    l_communicator.stopServer();
                    exitSimulationThread();

                    std::string l_env = l_args.value("ENV", ""); // environment var
                    std::string l_opt = l_args.value("OPT", ""); // compiler opt
                    int l_port = l_args.value("POR", 8080);

                    // compile
                    exec("chmod +x scripts/compile-bash.sh");
                    exec("./scripts/compile-bash.sh \"" + l_env + "\" \"" + l_opt + "\"");

                    // run
                    exec("chmod +x run-bash.sh");
                    exec("./run-bash.sh " + std::to_string(l_port) + " &");
                }
                else if (l_key == xlpmg::COMPILE_RUN_SBATCH.key)
                {
                    // Shutdown server
                    m_EXIT = true;
                    l_communicator.stopServer();
                    exitSimulationThread();

                    std::string l_env = l_args.value("ENV", ""); // environment var
                    std::string l_opt = l_args.value("OPT", ""); // compiler opt
                    int l_port = l_args.value("POR", 8080);

                    std::string l_job = l_args.value("JOB", "");
                    std::string l_out = l_args.value("OUT", "");
                    std::string l_err = l_args.value("ERR", "");
                    std::string l_tim = l_args.value("TIM", "");

                    // compile
                    exec("chmod +x scripts/compile-bash.sh");
                    exec("./scripts/compile-bash.sh \"" + l_env + "\" \"" + l_opt + "\"");

                    // generate sbatch
                    exec("chmod +x scripts/generateSbatch.sh");
                    exec("./scripts/generateSbatch.sh " + l_job + " " + l_out + " " + l_err + " " + l_tim + " > run-sbatch.sh");

                    // run
                    exec("chmod +x run-sbatch.sh");
                    exec("sbatch run-sbatch.sh " + std::to_string(l_port));
                }
                else if (l_key == xlpmg::SET_READ_BUFFER_SIZE.key)
                {
                    l_communicator.setReadBufferSize(l_args);
                }
                else if (l_key == xlpmg::SET_SEND_BUFFER_SIZE.key)
                {
                    l_communicator.setSendBufferSize(l_args);
                }

                else if (l_key == xlpmg::SET_BATHYMETRY_DATA.key)
                {
                    std::vector<std::uint8_t> l_byteVector = l_args["bytes"];
                    auto l_writeFile = std::fstream(m_bathTempFile, std::ios::out | std::ios::binary);
                    l_writeFile.write((char *)&l_byteVector[0], l_byteVector.size());
                    l_writeFile.close();
                    simulator->setBathymetryFilePath(m_bathTempFile);
                    simulator->setPrepared(false);
                }
                else if (l_key == xlpmg::SET_DISPLACEMENT_DATA.key)
                {
                    std::vector<std::uint8_t> l_byteVector = l_args["bytes"];
                    auto l_writeFile = std::fstream(m_displTempFile, std::ios::out | std::ios::binary);
                    l_writeFile.write((char *)&l_byteVector[0], l_byteVector.size());
                    l_writeFile.close();
                    simulator->setDisplacementFilePath(m_displTempFile);
                    simulator->setPrepared(false);
                }
                else if (l_key == xlpmg::CONTINUE_SIMULATION.key)
                {
                    std::cout << "Continue simulation" << std::endl;
                    simulator->setPausingStatus(false);
                }
                else if (l_key == xlpmg::PAUSE_SIMULATION.key)
                {
                    std::cout << "Pause simulation" << std::endl;
                    simulator->setPausingStatus(true);
                }
            }
            //-------------------------------------------//
            //--------------FUNCTION CALLS---------------//
            //-------------------------------------------//
            else if (l_type == xlpmg::FUNCTION_CALL)
            {

                if (l_key == xlpmg::RESET_SIMULATOR.key)
                {
                    exitSimulationThread();
                    if (canRunThread())
                    {
                        m_simulationThread = std::thread(&tsunami_lab::Simulator::resetSimulator, simulator);
                    }
                    else
                    {
                        std::cout << "Warning: Could not reset because the simulation is still running." << std::endl;
                    }
                }
                else if (l_key == xlpmg::GET_TIME_VALUES.key)
                {
                    xlpmg::Message response = {xlpmg::SERVER_RESPONSE, "get_time_values"};
                    tsunami_lab::t_idx l_currentTimeStep, l_maxTimeStep;
                    tsunami_lab::t_real l_timePerTimeStep;
                    simulator->getTimeValues(l_currentTimeStep, l_maxTimeStep, l_timePerTimeStep);
                    json l_data;
                    l_data["currentTimeStep"] = l_currentTimeStep;
                    l_data["maxTimeStep"] = l_maxTimeStep;
                    l_data["timePerTimeStep"] = l_timePerTimeStep;
                    if (simulator->isCalculating())
                    {
                        l_data["status"] = "CALCULATING";
                    }
                    else if (simulator->isPreparing())
                    {
                        l_data["status"] = "PREPARING";
                    }
                    else if (simulator->isResetting())
                    {
                        l_data["status"] = "RESETTING";
                    }
                    else
                    {
                        l_data["status"] = "IDLE";
                    }
                    response.args = l_data;
                    l_communicator.sendToClient(xlpmg::messageToJsonString(response), false);
                }
                else if (l_key == xlpmg::TOGGLE_FILEIO.key)
                {
                    if (l_args == "true")
                    {
                        simulator->toggleFileIO(true);
                    }
                    else if (l_args == "false")
                    {
                        simulator->toggleFileIO(false);
                    }
                }
                else if (l_key == xlpmg::GET_HEIGHT_DATA.key)
                {
                    xlpmg::Message l_heightDataMsg = {xlpmg::SERVER_RESPONSE, "height_data", nullptr};

                    // get data from simulation
                    if (simulator->getWaveProp() != nullptr)
                    {
                        tsunami_lab::patches::WavePropagation *l_waveprop = simulator->getWaveProp();
                        const tsunami_lab::t_real *l_heightData = l_waveprop->getHeight();
                        const tsunami_lab::t_real *l_bathymetryData = l_waveprop->getBathymetry();
                        // calculate array size
                        tsunami_lab::t_idx l_ncellsX, l_ncellsY;
                        simulator->getCellAmount(l_ncellsX, l_ncellsY);
                        for (tsunami_lab::t_idx y = 0; y < l_ncellsY; y++)
                        {
                            for (tsunami_lab::t_idx x = 0; x < l_ncellsX; x++)
                            {
                                l_heightDataMsg.args.push_back(l_heightData[x + l_waveprop->getStride() * y] + l_bathymetryData[x + l_waveprop->getStride() * y]);
                            }
                        }
                    }
                    l_communicator.sendToClient(xlpmg::messageToJsonString(l_heightDataMsg));
                }
                else if (l_key == xlpmg::GET_BATHYMETRY_DATA.key)
                {
                    xlpmg::Message l_bathyDataMsg = {xlpmg::SERVER_RESPONSE, "bathymetry_data", nullptr};

                    // get data from simulation
                    if (simulator->getWaveProp() != nullptr)
                    {
                        tsunami_lab::patches::WavePropagation *l_waveprop = simulator->getWaveProp();
                        const tsunami_lab::t_real *l_bathymetryData = l_waveprop->getBathymetry();
                        // calculate array size
                        tsunami_lab::t_idx l_ncellsX, l_ncellsY;
                        simulator->getCellAmount(l_ncellsX, l_ncellsY);
                        for (tsunami_lab::t_idx y = 0; y < l_ncellsY; y++)
                        {
                            for (tsunami_lab::t_idx x = 0; x < l_ncellsX; x++)
                            {
                                l_bathyDataMsg.args.push_back(l_bathymetryData[x + l_waveprop->getStride() * y]);
                            }
                        }
                    }
                    l_communicator.sendToClient(xlpmg::messageToJsonString(l_bathyDataMsg));
                }
                else if (l_key == xlpmg::LOAD_CONFIG_JSON.key)
                {
                    simulator->loadConfigDataJson(l_args);
                    if (canRunThread())
                    {
                        m_simulationThread = std::thread(&tsunami_lab::Simulator::resetSimulator, simulator);
                    }
                    else
                    {
                        std::cout << "Warning: Could not reset because the simulation is still running." << std::endl;
                    }
                }
                else if (l_key == xlpmg::DELETE_CHECKPOINTS.key)
                {
                    simulator->deleteCheckpoints();
                }
                else if (l_key == xlpmg::DELETE_STATIONS.key)
                {
                    simulator->deleteStations();
                }
                else if (l_key == xlpmg::GET_SIMULATION_SIZES.key)
                {
                    xlpmg::Message l_msg = {xlpmg::SERVER_RESPONSE, "simulation_sizes", nullptr};
                    tsunami_lab::t_idx l_ncellsX, l_ncellsY;
                    tsunami_lab::t_real l_simulationSizeX, l_simulationSizeY, l_offsetX, l_offsetY;
                    simulator->getCellAmount(l_ncellsX, l_ncellsY);
                    simulator->getSimulationSize(l_simulationSizeX, l_simulationSizeY);
                    simulator->getSimulationOffset(l_offsetX, l_offsetY);
                    l_msg.args["cellsX"] = l_ncellsX;
                    l_msg.args["cellsY"] = l_ncellsY;
                    l_msg.args["simulationSizeX"] = l_simulationSizeX;
                    l_msg.args["simulationSizeY"] = l_simulationSizeY;
                    l_msg.args["offsetX"] = l_offsetX;
                    l_msg.args["offsetY"] = l_offsetY;
                    l_communicator.sendToClient(xlpmg::messageToJsonString(l_msg));
                }
                else if (l_key == xlpmg::SET_OFFSET.key)
                {
                    tsunami_lab::t_real l_offsetX = l_args.value("offsetX", 0);
                    tsunami_lab::t_real l_offsetY = l_args.value("offsetY", 0);
                    simulator->setOffset(l_offsetX, l_offsetY);
                }
                else if (l_key == xlpmg::SET_CELL_AMOUNT.key)
                {
                    tsunami_lab::t_idx l_nCellsX = l_args["cellsX"];
                    tsunami_lab::t_idx l_nCellsY = l_args["cellsY"];
                    simulator->setCellAmount(l_nCellsX, l_nCellsY);
                }
            }
        }

        if (m_simulationThread.joinable())
        {
            m_simulationThread.join();
        }

        m_stopUpdating = true;
        if (m_updateThread.joinable())
        {
            m_updateThread.join();
        }
        std::remove(m_bathTempFile.c_str());
        std::remove(m_displTempFile.c_str());
    }
    //------------------------------------------//
    //--------------STANDARD MODE---------------//
    //------------------------------------------//
    else
    {
        if (i_argc >= 2)
        {
            simulator->start(std::string(i_argv[1]));
        }
        else
        {
            simulator->start("");
        }
    }

    delete simulator;
    return exitCode;
}