#include "Launcher.h"
#include "Communicator.hpp"
#define USEGUI 1

int PORT = 8080;
bool EXIT = false;

int main(int i_argc, char *i_argv[])
{
    int exitCode = 0;

    tsunami_lab::Launcher launcher;

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
        if (strcmp(data.c_str(), "exit") == 0){
            EXIT = true;
        //FUNCTION CALLS
        }else if(data[0]=='F'){
            // VOIDS
            if(data[1]=='V'){
                if(strcmp(data.c_str(), "FVSTART") == 0){
                    std::string config = communicator.receiveFromClient();
                    launcher.start(config);
                }
            // CLIENT WILL WAIT FOR RETURN
            }else if(data[1]=='R'){

            }
        }
    }

#else
    if (i_argc > 2)
    {
        launcher.start(i_argv[1]);
    }
    else
    {
        launcher.start("");
    }
#endif
    return exitCode;
}