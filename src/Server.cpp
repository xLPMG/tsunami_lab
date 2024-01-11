#include "Simulator.h"
#include "Communicator.hpp"
#include "communicator_api.h"
#include <nlohmann/json.hpp>

#include <thread>

using json = nlohmann::json;

int PORT = 8080;
bool EXIT = false;
std::thread simulationThread;
bool isSimulationRunning = false;

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

int main(int i_argc, char *i_argv[])
{
    int exitCode = 0;

    tsunami_lab::Simulator *simulator = new tsunami_lab::Simulator;

#ifdef USEGUI
    if (i_argc > 2)
    {
        PORT = atoi(i_argv[1]);
    }
    xlpmg::Communicator communicator;
    communicator.startServer(PORT);
    while (!EXIT)
    {
        std::string rawData = communicator.receiveFromClient();
        // check if client sent valid json, go back to reading if not
        if (!json::accept(rawData))
        {
            communicator.sendToClient("FAIL");
            continue;
        }

        json parsedData = json::parse(rawData);
        json type = parsedData.at(xlpmg::MESSAGE_TYPE);
        json key = parsedData.at(xlpmg::KEY);
        // json data = parsedData.value(xlpmg::DATA, "");

        if (type.get<xlpmg::MessageType>() == xlpmg::SERVER__CALL)
        {
            if (key == xlpmg::KEY_SHUTDOWN_SERVER)
            {
                EXIT = true;
                communicator.stopServer();
                if (simulationThread.joinable())
                {
                    simulationThread.std::thread::~thread();
                    isSimulationRunning = false;
                }
            }
            else if (key == xlpmg::KEY_START_SIMULATION)
            {
                std::string config = parsedData.at(xlpmg::ARGS);
                if (!isSimulationRunning)
                {
                    simulationThread = std::thread(&tsunami_lab::Simulator::start, simulator, config);
                    isSimulationRunning = true;
                }
            }
            else if (key == xlpmg::KEY_KILL_SIMULATION)
            {
                if (simulationThread.joinable())
                {
                    simulationThread.std::thread::~thread();
                    isSimulationRunning = false;
                }
            }
            else if (key == xlpmg::KEY_RECOMPILE)
            {
                //Shutdown server
                EXIT = true;
                communicator.stopServer();
                if (simulationThread.joinable())
                {
                    simulationThread.std::thread::~thread();
                    isSimulationRunning = false;
                }
                //execute recompilation
                std::string env = parsedData.at(xlpmg::ARGS).value("ENV", ""); // environment var
                std::string opt = parsedData.at(xlpmg::ARGS).value("OPT", ""); //compiler opt
                exec("chmod +x runServer.sh");
                exec("./runServer.sh \""+env+"\" \""+opt+"\" &");
            }
        }
        else if (type.get<xlpmg::MessageType>() == xlpmg::FUNCTION_CALL)
        {
            std::cout << "function call" << std::endl;
        }
    }

    if (simulationThread.joinable())
    {
        simulationThread.join();
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