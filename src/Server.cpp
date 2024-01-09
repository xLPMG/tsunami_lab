#include "Launcher.h"
#include "Communicator.hpp"
#include "constants.h"
#define USEGUI 1

int PORT = 8080;
bool EXIT = false;

int main(int i_argc, char *i_argv[])
{
    int exitCode = 0;

    tsunami_lab::Launcher *launcher = new tsunami_lab::Launcher;

#ifdef USEGUI

    if (i_argc > 2)
    {
        PORT = atoi(i_argv[1]);
    }
    xlpmg::Communicator communicator;
    communicator.startServer(PORT);
    while (!EXIT)
    {
        std::string data = communicator.receiveFromClient();
        if (strcmp(data.c_str(), tsunami_lab::KEY_SHUTDOWN_SERVER) == 0)
        {
            EXIT = true;
            launcher->exit();
        }
        else if (strcmp(data.c_str(), tsunami_lab::KEY_EXIT_LAUNCHER) == 0 && launcher != nullptr)
        {
            launcher->exit();
        }
        else if (strcmp(data.c_str(), tsunami_lab::KEY_REVIVE_LAUNCHER) == 0)
        {
            launcher->revive();
        }
        else if (strcmp(data.c_str(), tsunami_lab::KEY_RESTART_SERVER) == 0)
        {
            // TODO
            EXIT = true;
            launcher->exit();

            // FUNCTION CALLS
        }
        else if (data[0] == 'F')
        {
            // VOIDS
            if (data[1] == 'V')
            {
                if (strcmp(data.c_str(), tsunami_lab::KEY_START_SIMULATION) == 0)
                {
                    std::string config = communicator.receiveFromClient();
                    launcher->start(config);
                }
                else if (strcmp(data.c_str(), tsunami_lab::KEY_WRITE_CHECKPOINT) == 0)
                {
                    launcher->writeCheckpoint();
                }
                else if (strcmp(data.c_str(), tsunami_lab::KEY_LOAD_CONFIG_JSON) == 0)
                {
                    std::string config = communicator.receiveFromClient();
                    launcher->loadConfigDataJsonString(config);
                }
                else if (strcmp(data.c_str(), tsunami_lab::KEY_LOAD_CONFIG_FILE) == 0)
                {
                    std::string configFile = communicator.receiveFromClient();
                    launcher->loadConfigDataFromFile(configFile);
                }
                // CLIENT WILL WAIT FOR RETURN MESSAGE
            }
            else if (data[1] == 'R')
            {
            }
        }
    }

#else
    if (i_argc > 2)
    {
        launcher->start(i_argv[1]);
    }
    else
    {
        launcher->start("");
    }
#endif
    delete launcher;
    return exitCode;
}