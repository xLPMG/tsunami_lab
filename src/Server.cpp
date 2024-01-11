#include "Simulator.h"
#include "Communicator.hpp"
#include "constants.h"
#include <thread>
#define USEGUI 1

int PORT = 8080;
bool EXIT = false;
std::thread simulationThread;
bool isSimulationRunning = false;

int main(int i_argc, char *i_argv[])
{
    int exitCode = 0;

#ifdef USEGUI
    tsunami_lab::Simulator *simulator = new tsunami_lab::Simulator;
    ;
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
            else if (strcmp(data.c_str(), tsunami_lab::KEY_KILL_SIMULATION) == 0)
            {
                if (simulationThread.joinable())
                {
                    simulationThread.std::thread::~thread();
                    isSimulationRunning = false;
                }
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
                    if (!isSimulationRunning)
                    {
                        simulationThread = std::thread(&tsunami_lab::Simulator::start, simulator, config);
                        isSimulationRunning = true;
                    }
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
    if (i_argc > 2)
    {
        simulator->start(i_argv[1]);
    }
    else
    {
        simulator->start("");
    }
#endif
    if (simulationThread.joinable())
    {
        simulationThread.join();
    }
    delete simulator;
    return exitCode;
}