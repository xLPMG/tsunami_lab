#include "Simulator.h"
#include "xlpmg/Communicator.hpp"
#include "xlpmg/communicator_api.h"
#include <nlohmann/json.hpp>

#include <thread>
using json = nlohmann::json;

int m_PORT = 8080;
bool m_EXIT = false;
tsunami_lab::Simulator *simulator = nullptr;
std::thread m_simulationThread;
bool m_isSimulationRunning = false;

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

#ifndef DNOSERVER
    if (i_argc > 2)
    {
        m_PORT = atoi(i_argv[1]);
    }
    xlpmg::Communicator l_communicator;
    l_communicator.startServer(m_PORT);
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

        if (l_type == xlpmg::SERVER__CALL)
        {
            if (l_key == xlpmg::SHUTDOWN_SERVER_MESSAGE.key)
            {
                m_EXIT = true;
                exitSimulationThread();
                l_communicator.stopServer();
            }
            else if (l_key == xlpmg::START_SIMULATION_MESSAGE.key)
            {
                std::string l_config = l_parsedData.at(xlpmg::ARGS);
                if (!m_isSimulationRunning)
                {
                    m_simulationThread = std::thread(&tsunami_lab::Simulator::start, simulator, l_config);
                    m_isSimulationRunning = true;
                }
            }
            else if (l_key == xlpmg::KILL_SIMULATION_MESSAGE.key)
            {
                exitSimulationThread();
            }
            else if (l_key == xlpmg::COMPILE_MESSAGE.key)
            {
                // Shutdown server
                m_EXIT = true;
                l_communicator.stopServer();
                exitSimulationThread();

                std::string env = l_args.value("ENV", ""); // environment var
                std::string opt = l_args.value("OPT", ""); // compiler opt

                //compile
                exec("chmod +x scripts/compile-bash.sh");
                exec("./scripts/compile-bash.sh \"" + env + "\" \"" + opt + "\" &");
            }
            else if (l_key == xlpmg::COMPILE_RUN_BASH_MESSAGE.key)
            {
                // Shutdown server
                m_EXIT = true;
                l_communicator.stopServer();
                exitSimulationThread();

                std::string env = l_args.value("ENV", ""); // environment var
                std::string opt = l_args.value("OPT", ""); // compiler opt

                //compile
                exec("chmod +x scripts/compile-bash.sh");
                exec("./scripts/compile-bash.sh \"" + env + "\" \"" + opt + "\"");

                //run
                exec("chmod +x run-bash.sh");
                exec("./run-bash.sh &");
            }
            else if (l_key == xlpmg::COMPILE_RUN_SBATCH_MESSAGE.key)
            {
                // Shutdown server
                m_EXIT = true;
                l_communicator.stopServer();
                exitSimulationThread();

                std::string env = l_args.value("ENV", ""); // environment var
                std::string opt = l_args.value("OPT", ""); // compiler opt

                std::string l_job = l_args.value("JOB", "");
                std::string l_out = l_args.value("OUT", "");
                std::string l_err = l_args.value("ERR", "");
                std::string l_tim = l_args.value("TIM", "");

                //compile
                exec("chmod +x scripts/compile-bash.sh");
                exec("./scripts/compile-bash.sh \"" + env + "\" \"" + opt + "\"");

                //generate sbatch
                exec("chmod +x scripts/generateSbatch.sh");
                exec("./scripts/generateSbatch.sh " + l_job + " " + l_out + " " + l_err + " " + l_tim +" > run-sbatch.sh");

                //run
                exec("chmod +x run-sbatch.sh");
                exec("sbatch run-sbatch.sh");
            }
        }
        else if (l_type == xlpmg::FUNCTION_CALL)
        {
            if (l_key == xlpmg::GET_TIMESTEP_MESSAGE.key)
            {
                xlpmg::Message response = {xlpmg::SERVER_RESPONSE, "time_step_data", simulator->getTimeStep()};
                l_communicator.sendToClient(xlpmg::messageToJsonString(response));
            }
            else if (l_key == xlpmg::TOGGLE_FILEIO_MESSAGE.key)
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
            else if (l_key == xlpmg::GET_HEIGHT_DATA_MESSAGE.key)
            {
                xlpmg::Message heightDataMsg = {xlpmg::SERVER_RESPONSE, "height_data", nullptr};
                // get data from simulation
                if (simulator->getWaveProp() != nullptr)
                {
                    tsunami_lab::patches::WavePropagation *waveprop = simulator->getWaveProp();
                    const tsunami_lab::t_real *heightData = waveprop->getHeight();
                    // calculate array size
                    tsunami_lab::t_idx l_ncellsX, l_ncellsY;
                    simulator->getCellAmount(l_ncellsX, l_ncellsY);
                    unsigned long totalCells = l_ncellsX * l_ncellsY;
                    for (tsunami_lab::t_idx i = 0; i < totalCells; i++)
                    {
                        heightDataMsg.args.push_back(heightData[i]);
                    }
                    l_communicator.sendToClient(xlpmg::messageToJsonString(heightDataMsg));
                    l_communicator.sendToClient(xlpmg::messageToJsonString(xlpmg::BUFFERED_SEND_FINISHED));
                }
            }
            else if (l_key == xlpmg::LOAD_CONFIG_JSON_MESSAGE.key)
            {
                simulator->loadConfigDataJson(l_args);
            }
        }
    }

    if (m_simulationThread.joinable())
    {
        m_simulationThread.join();
    }
#else
    if (i_argc > 2)
    {
        simulator->start(i_argv[1]);
    }
    else
    {
        simulator->start("");
    }
#endif
    delete simulator;
    return exitCode;
}