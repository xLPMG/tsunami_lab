#include "Simulator.h"
#include "Communicator.hpp"
#include "constants.h"
#include <thread>
#define USEGUI 1

int PORT = 8080;
bool EXIT = false;
std::thread simulationThread;

int main(int i_argc, char *i_argv[])
{
    int exitCode = 0;

#ifdef USEGUI
    tsunami_lab::Simulator *simulator = nullptr;
    if (i_argc > 2)
    {
        PORT = atoi(i_argv[1]);
    }
    xlpmg::Communicator communicator;
    communicator.startServer(PORT);
    while (!EXIT)
    {
        std::string data = communicator.receiveFromClient();
        // SYSTEM CALLS
        if (data[0] == 'S')
        {
            if (strcmp(data.c_str(), tsunami_lab::KEY_SHUTDOWN_SERVER) == 0)
            {
                EXIT = true;
            }
            else if (strcmp(data.c_str(), tsunami_lab::KEY_CREATE_SIMULATOR) == 0)
            {
                if (simulator != nullptr)
                {
                    delete simulator;
                }
                simulator = new tsunami_lab::Simulator;
            }
            else if (strcmp(data.c_str(), tsunami_lab::KEY_KILL_SIMULATION) == 0)
            {
            }
        }
        // FUNCTION CALLS
        else if (data[0] == 'F' && simulator != nullptr)
        {
            // VOIDS
            if (data[1] == 'V')
            {
                if (strcmp(data.c_str(), tsunami_lab::KEY_WRITE_CHECKPOINT) == 0)
                {
                    simulator->writeCheckpoint();
                }
                else if (strcmp(data.c_str(), tsunami_lab::KEY_START_SIMULATION) == 0)
                {
                    std::string config = communicator.receiveFromClient();
                    std::cout << "start" << std::endl;
                    simulationThread = std::thread(&tsunami_lab::Simulator::start, simulator, config);
                }
                else if (strcmp(data.c_str(), tsunami_lab::KEY_LOAD_CONFIG_JSON) == 0)
                {
                    std::string config = communicator.receiveFromClient();
                    simulator->loadConfigDataJsonString(config);
                }
                else if (strcmp(data.c_str(), tsunami_lab::KEY_LOAD_CONFIG_FILE) == 0)
                {
                    std::string configFile = communicator.receiveFromClient();
                    simulator->loadConfigDataFromFile(configFile);
                }
                else if (strcmp(data.c_str(), tsunami_lab::KEY_TOGGLE_FILEIO) == 0)
                {
                    std::string toggle = communicator.receiveFromClient();
                    if (strcmp(toggle.c_str(), "true") == 0)
                    {
                        simulator->toggleFileIO(true);
                    }
                    else
                    {
                        simulator->toggleFileIO(false);
                    }
                }
                else if (strcmp(data.c_str(), tsunami_lab::KEY_GET_TIMESTEP) == 0)
                {
                    std::cout << simulator->getTimeStep() << std::endl;
                    communicator.sendToClient(std::to_string(simulator->getTimeStep()));
                }
            }
            // CLIENT WILL WAIT FOR RETURN MESSAGE
            else if (data[1] == 'R' && simulator != nullptr)
            {
            }
        }
    }

#else
    tsunami_lab::Simulator *launcher = new tsunami_lab::Simulator;
    if (i_argc > 2)
    {
        launcher->start(i_argv[1]);
    }
    else
    {
        launcher->start("");
    }
    delete launcher;
#endif
    simulationThread.join();
    return exitCode;
}