#include "Simulator.h"
#include "Communicator.hpp"
#include "communicator_api.h"
#include <nlohmann/json.hpp>

#include <thread>

#define USEGUI 1

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
        //prepare for next run
        m_isSimulationRunning = false;
        simulator->shouldExit(false);
    }
}

int main(int i_argc, char *i_argv[])
{
    int exitCode = 0;
    simulator = new tsunami_lab::Simulator;

#ifdef USEGUI
    if (i_argc > 2)
    {
        m_PORT = atoi(i_argv[1]);
    }
    xlpmg::Communicator l_communicator;
    l_communicator.startServer(m_PORT);
    while (!m_EXIT)
    {
        std::string rawData = l_communicator.receiveFromClient();
        // check if client sent valid json, go back to reading if not
        if (!json::accept(rawData))
        {
            continue;
        }

        json parsedData = json::parse(rawData);
        json type = parsedData.at(xlpmg::MESSAGE_TYPE);
        json key = parsedData.at(xlpmg::KEY);

        if (type.get<xlpmg::MessageType>() == xlpmg::SERVER__CALL)
        {
            if (key == xlpmg::KEY_SHUTDOWN_SERVER)
            {
                m_EXIT = true;
                l_communicator.stopServer();
                exitSimulationThread();
            }
            else if (key == xlpmg::KEY_START_SIMULATION)
            {
                std::string config = parsedData.at(xlpmg::ARGS);
                if (!m_isSimulationRunning)
                {
                    m_simulationThread = std::thread(&tsunami_lab::Simulator::start, simulator, config);
                    m_isSimulationRunning = true;
                }
            }
            else if (key == xlpmg::KEY_KILL_SIMULATION)
            {
                exitSimulationThread();
            }
            else if (key == xlpmg::KEY_RECOMPILE)
            {
                // Shutdown server
                m_EXIT = true;
                l_communicator.stopServer();
                exitSimulationThread();
                // execute recompilation
                std::string env = parsedData.at(xlpmg::ARGS).value("ENV", ""); // environment var
                std::string opt = parsedData.at(xlpmg::ARGS).value("OPT", ""); // compiler opt
                exec("chmod +x runServer.sh");
                exec("./runServer.sh \"" + env + "\" \"" + opt + "\" &");
            }
        }
        else if (type.get<xlpmg::MessageType>() == xlpmg::FUNCTION_CALL)
        {
            std::cout << "function call" << std::endl;
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