#include "Simulator.h"
#include "xlpmg/Communicator.hpp"
#include "xlpmg/communicator_api.h"
#include <nlohmann/json.hpp>
#include "systeminfo/SystemInfo.h"

#include <thread>
#include <atomic>
using json = nlohmann::json;

//! Port for the server
int m_PORT = 8080;
//! Exit flag
bool m_EXIT = false;
//! Simulator object pointer
tsunami_lab::Simulator *simulator = nullptr;
//! Thread object which will be used to run simulation tasks
std::thread m_simulationThread;
//! Thread object which will be used to update system info
std::thread m_updateThread;
//! Flag to stop updating
std::atomic<bool> m_stopUpdating = false;
//! Flag to check if simulation is running
bool m_isSimulationRunning = false;

//! Last update time point
std::chrono::time_point m_lastDataUpdate = std::chrono::high_resolution_clock::now();
//! Lata update frequency in ms
int m_dataUpdateFrequency = 10;

//! System info object
tsunami_lab::systeminfo::SystemInfo l_systemInfo;
//! Amount of used RAM
double l_usedRAM = 0;
//! Total amount of RAM
double l_totalRAM = 0;
//! CPU usage vector
std::vector<float> l_cpuUsage;

/**
 * Executes the given command.
 *
 * @param i_cmd The command to be executed.
 * @return The exit status of the command.
 */
int exec(std::string i_cmd)
{
    return system(i_cmd.data());
}

/**
 * @brief Function to exit the simulation thread.
 *
 * This function sets the shouldExit flag of the simulator object to true,
 * indicating that the simulation should be terminated as soon as possible.
 *
 * @return void
 */
void exitSimulationThread()
{
    std::cout << "Terminating as soon as possible..." << std::endl;
    simulator->shouldExit(true);
}

/**
 * Checks if the simulation thread is active.
 * If the simulator is preparing, calculating, or resetting, the simulation is considered running.
 * If the simulation thread is joinable, it is joined and the simulation is considered not running.
 *
 * @return void
 */
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

/**
 * Checks if a simulation thread can be run.
 *
 * @return true if the simulation thread can be run, false otherwise.
 */
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

/**
 * @brief Updates the data related to CPU and RAM usage.
 *
 * This function checks if it is time to update the data and if so, it retrieves the CPU usage and RAM usage.
 * The updated data is stored in the respective variables.
 *
 * @return void
 */
void updateData()
{
    if (m_lastDataUpdate <= std::chrono::high_resolution_clock::now())
    {
        l_cpuUsage = l_systemInfo.getCPUUsage();
        l_systemInfo.getRAMUsage(l_totalRAM, l_usedRAM);
        m_lastDataUpdate = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(m_dataUpdateFrequency);
    }
}

/**
 * @brief Main function for the tsunami_lab server program.
 *
 * @return Exit code (0 for success, non-zero for errors).
 */
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
            xlpmg::MessageExpectation l_expectation = l_message.expectation;
            xlpmg::MessageUrgency l_urgency = l_message.urgency;
            std::string l_key = l_message.key;
            json l_args = l_message.args;

            /////////////////////////////////
            //         NO_RESPONSE         //
            /////////////////////////////////
            if (l_expectation == xlpmg::NO_RESPONSE)
            {
                // CRITICAL
                if (l_urgency == xlpmg::CRITICAL)
                {
                    if (l_key == xlpmg::KILL_SIMULATION.key)
                    {
                        exitSimulationThread();
                    }
                    else if (l_key == xlpmg::WRITE_CHECKPOINT.key)
                    {
                        std::cout << "Writing checkpoint" << std::endl;
                        simulator->writeCheckpoint();
                    }

                    else if (l_key == xlpmg::PAUSE_SIMULATION.key)
                    {
                        std::cout << "Pause simulation" << std::endl;
                        simulator->setPausingStatus(true);
                    }
                    else if (l_key == xlpmg::SHUTDOWN_SERVER.key)
                    {
                        m_EXIT = true;
                        exitSimulationThread();
                        l_communicator.stopServer();
                    }
                }
                // HIGH
                else if (l_urgency == xlpmg::HIGH)
                {
                    if (l_key == xlpmg::CHECK.key)
                    {
                        std::cout << "A client requested a check." << std::endl;
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
                    else if (l_key == xlpmg::CONTINUE_SIMULATION.key)
                    {
                        std::cout << "Continue simulation" << std::endl;
                        simulator->setPausingStatus(false);
                    }
                    else if (l_key == xlpmg::RESET_SIMULATOR.key)
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
                }
                // MEDIUM
                else if (l_urgency == xlpmg::MEDIUM)
                {
                    if (l_key == xlpmg::COMPILE.key)
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
                    else if (l_key == xlpmg::DELETE_CHECKPOINTS.key)
                    {
                        simulator->deleteCheckpoints();
                    }
                    else if (l_key == xlpmg::DELETE_STATIONS.key)
                    {
                        simulator->deleteStations();
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
                    else if (l_key == xlpmg::LOAD_STATIONS.key)
                    {
                        simulator->loadStations(l_args);
                    }
                }
                // LOW
                else if (l_urgency == xlpmg::LOW)
                {
                    if (l_key == xlpmg::SEND_FILE.key)
                    {
                        std::vector<std::uint8_t> l_byteVector = l_args["data"]["bytes"];
                        auto l_writeFile = std::fstream(l_args.value("path", ""), std::ios::out | std::ios::binary);
                        l_writeFile.write((char *)&l_byteVector[0], l_byteVector.size());
                        l_writeFile.close();
                    }
                    else if (l_key == xlpmg::LOAD_CONFIG_JSON.key)
                    {
                        if (json::accept(l_args))
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
                    }
                    else if (l_key == xlpmg::LOAD_CONFIG_FILE.key)
                    {
                        simulator->loadConfigDataFromFile(l_args);
                        if (canRunThread())
                        {
                            m_simulationThread = std::thread(&tsunami_lab::Simulator::resetSimulator, simulator);
                        }
                        else
                        {
                            std::cout << "Warning: Could not reset because the simulation is still running." << std::endl;
                        }
                    }
                    else if (l_key == xlpmg::SET_READ_BUFFER_SIZE.key)
                    {
                        l_communicator.setReadBufferSize(l_args);
                    }
                    else if (l_key == xlpmg::SET_SEND_BUFFER_SIZE.key)
                    {
                        l_communicator.setSendBufferSize(l_args);
                    }
                }
            }
            ////////////////////////////////
            //      EXPECT_RESPONSE       //
            ////////////////////////////////
            else if (l_expectation == xlpmg::EXPECT_RESPONSE)
            {
                // CRITICAL
                if (l_urgency == xlpmg::CRITICAL)
                {
                    if (l_key == xlpmg::GET_TIME_VALUES.key)
                    {
                        xlpmg::Message response = xlpmg::SERVER_RESPONSE;
                        response.key = "time_values";
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
                    else if (l_key == xlpmg::GET_SYSTEM_INFORMATION.key)
                    {
                        xlpmg::Message l_response = xlpmg::SERVER_RESPONSE;
                        l_response.key = "system_information";
                        json l_data;
                        l_data["USED_RAM"] = l_usedRAM;
                        l_data["TOTAL_RAM"] = l_totalRAM;
                        l_data["CPU_USAGE"] = l_cpuUsage;
                        l_response.args = l_data;
                        l_communicator.sendToClient(xlpmg::messageToJsonString(l_response), false);
                    }

                    else if (l_key == xlpmg::GET_SIMULATION_SIZES.key)
                    {
                        xlpmg::Message l_msg = xlpmg::SERVER_RESPONSE;
                        l_msg.key = "simulation_sizes";
                        json l_data;
                        tsunami_lab::t_idx l_ncellsX, l_ncellsY;
                        tsunami_lab::t_real l_simulationSizeX, l_simulationSizeY, l_offsetX, l_offsetY;
                        simulator->getCellAmount(l_ncellsX, l_ncellsY);
                        simulator->getSimulationSize(l_simulationSizeX, l_simulationSizeY);
                        simulator->getSimulationOffset(l_offsetX, l_offsetY);
                        l_data["cellsX"] = l_ncellsX;
                        l_data["cellsY"] = l_ncellsY;
                        l_data["simulationSizeX"] = l_simulationSizeX;
                        l_data["simulationSizeY"] = l_simulationSizeY;
                        l_data["offsetX"] = l_offsetX;
                        l_data["offsetY"] = l_offsetY;
                        l_msg.args = l_data;
                        l_communicator.sendToClient(xlpmg::messageToJsonString(l_msg));
                    }
                }
                // HIGH
                else if (l_urgency == xlpmg::HIGH)
                {
                    if (l_key == xlpmg::GET_HEIGHT_DATA.key)
                    {
                        xlpmg::Message l_heightDataMsg = xlpmg::SERVER_RESPONSE;
                        l_heightDataMsg.key = "height_data";
                        json l_data;
                        // get data from simulation
                        if (simulator->getWaveProp() != nullptr)
                        {
                            tsunami_lab::patches::WavePropagation *l_waveprop = simulator->getWaveProp();
                            const tsunami_lab::t_real *l_heightData = l_waveprop->getHeight();
                            // calculate array size
                            tsunami_lab::t_idx l_ncellsX, l_ncellsY;
                            simulator->getCellAmount(l_ncellsX, l_ncellsY);
                            for (tsunami_lab::t_idx y = 0; y < l_ncellsY; y++)
                            {
                                for (tsunami_lab::t_idx x = 0; x < l_ncellsX; x++)
                                {
                                    l_data.push_back(l_heightData[x + l_waveprop->getStride() * y]);
                                }
                            }
                        }
                        l_heightDataMsg.args = l_data;
                        l_communicator.sendToClient(xlpmg::messageToJsonString(l_heightDataMsg));
                    }
                    else if (l_key == xlpmg::GET_BATHYMETRY_DATA.key)
                    {
                        xlpmg::Message l_bathyDataMsg = xlpmg::SERVER_RESPONSE;
                        l_bathyDataMsg.key = "bathymetry_data";
                        json l_data;

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
                                    l_data.push_back(l_bathymetryData[x + l_waveprop->getStride() * y]);
                                }
                            }
                        }
                        l_bathyDataMsg.args = l_data;
                        l_communicator.sendToClient(xlpmg::messageToJsonString(l_bathyDataMsg));
                    }
                }
                // MEDIUM
                else if (l_urgency == xlpmg::MEDIUM)
                {
                    if (l_key == xlpmg::RECV_FILE.key)
                    {
                        std::string l_file = l_args.value("path", "");
                        std::string l_fileDestination = l_args.value("pathDestination", "");

                        if (l_file.length() > 0 && l_fileDestination.length() > 0)
                        {
                            xlpmg::Message l_response = xlpmg::SERVER_RESPONSE;
                            l_response.key = "file_data";
                            json l_arguments;
                            l_arguments["path"] = l_fileDestination;

                            std::ifstream l_fileData(l_file, std::ios::binary);
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
                            l_response.args = l_arguments;
                            l_communicator.sendToClient(xlpmg::messageToJsonString(l_response));
                        }
                    }
                }
                // LOW
                else if (l_urgency == xlpmg::LOW)
                {
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
