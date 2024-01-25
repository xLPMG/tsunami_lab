#include "Simulator.h"
#include "xlpmg/Communicator.hpp"
#include "xlpmg/communicator_api.h"
#include <nlohmann/json.hpp>
#include "systeminfo/SystemInfo.h"

#include <thread>
using json = nlohmann::json;

int m_PORT = 8080;
bool m_EXIT = false;
tsunami_lab::Simulator *simulator = nullptr;
std::thread m_simulationThread;
bool m_isSimulationRunning = false;

std::string m_bathTempFile = "bathymetry_temp.nc";
std::string m_displTempFile = "displacement_temp.nc";

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
    if (m_simulationThread.joinable())
    {
        std::cout << "Terminating..." << std::endl;
        simulator->shouldExit(true);
        // wait until thread has finished
        m_simulationThread.join();
        std::cout << "Thread terminated." << std::endl;
        // prepare for next run
        m_isSimulationRunning = false;
        simulator->shouldExit(false);
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

        tsunami_lab::systeminfo::SystemInfo l_systemInfo;
        double l_usedRAM = 0;
        double l_totalRAM = 0;
        std::vector<float> l_cpuUsage;

        xlpmg::Communicator l_communicator;
        l_communicator.startServer(m_PORT);
        m_simulationThread = std::thread(&tsunami_lab::Simulator::prepareForCalculation, simulator);
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
                    std::cout << "Start simulator" << std::endl;
                    std::string l_config = l_parsedData.at(xlpmg::ARGS);
                    if (simulator->isPreparing())
                    {
                        std::cout << "Warning: Did not start simulator because setup is still in progress." << std::endl;
                    }
                    else
                    {
                        if (!m_isSimulationRunning)
                        {
                            if (m_simulationThread.joinable())
                            {
                                m_simulationThread.join();
                            }
                            m_simulationThread = std::thread(&tsunami_lab::Simulator::start, simulator, l_config);
                            m_isSimulationRunning = true;
                        }
                    }
                }
                else if (l_key == xlpmg::KILL_SIMULATION.key)
                {
                    exitSimulationThread();
                }
                else if (l_key == xlpmg::GET_SYSTEM_INFORMATION.key)
                {
                    std::thread t1([&l_systemInfo, &l_cpuUsage]
                                   { l_cpuUsage = l_systemInfo.getCPUUsage(); });
                    t1.detach();

                    std::thread t2([&l_systemInfo, &l_totalRAM, &l_usedRAM]
                                   { l_systemInfo.getRAMUsage(l_totalRAM, l_usedRAM); });
                    t2.detach();

                    xlpmg::Message l_response = {xlpmg::SERVER_RESPONSE, "system_information"};
                    json l_data;
                    l_data["USED_RAM"] = l_usedRAM;
                    l_data["TOTAL_RAM"] = l_totalRAM;
                    l_data["CPU_USAGE"] = l_cpuUsage;
                    l_response.args = l_data;
                    l_communicator.sendToClient(xlpmg::messageToJsonString(l_response));

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
                    simulator->resetSimulator();
                }
                else if (l_key == xlpmg::GET_TIMESTEP.key)
                {
                    xlpmg::Message response = {xlpmg::SERVER_RESPONSE, "time_step_data", simulator->getTimeStep()};
                    l_communicator.sendToClient(xlpmg::messageToJsonString(response));
                }
                else if (l_key == xlpmg::GET_MAX_TIMESTEPS.key)
                {
                    xlpmg::Message response = {xlpmg::SERVER_RESPONSE, "time_step_data", simulator->getMaxTimeStep()};
                    l_communicator.sendToClient(xlpmg::messageToJsonString(response));
                }
                else if (l_key == xlpmg::GET_CURRENT_RUNTIME.key)
                {
                    xlpmg::Message response = {
                        xlpmg::SERVER_RESPONSE,
                        "run_time_data",
                    };
                    l_communicator.sendToClient(xlpmg::messageToJsonString(response));
                }
                else if (l_key == xlpmg::GET_ESTIMATED_LEFT_TIME.key)
                {
                    xlpmg::Message response = {
                        xlpmg::SERVER_RESPONSE,
                        "time_left_estimation",
                    };
                    l_communicator.sendToClient(xlpmg::messageToJsonString(response));
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
                        // calculate array size
                        tsunami_lab::t_idx l_ncellsX, l_ncellsY;
                        simulator->getCellAmount(l_ncellsX, l_ncellsY);
                        unsigned long totalCells = l_ncellsX * l_ncellsY;
                        for (tsunami_lab::t_idx i = 0; i < totalCells; i++)
                        {
                            l_heightDataMsg.args.push_back(l_heightData[i]);
                        }
                        l_communicator.sendToClient(xlpmg::messageToJsonString(l_heightDataMsg));
                        l_communicator.sendToClient(xlpmg::messageToJsonString(xlpmg::BUFFERED_SEND_FINISHED));
                    }
                }
                else if (l_key == xlpmg::LOAD_CONFIG_JSON.key)
                {
                    simulator->loadConfigDataJson(l_args);
                    simulator->resetSimulator();
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
                    l_msg.args["sizeX"] = l_simulationSizeX;
                    l_msg.args["sizeY"] = l_simulationSizeY;
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