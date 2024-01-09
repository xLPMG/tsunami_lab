#include "ui/GUI.h"
#include <iostream>

int PORT = 8080;

int main(int i_argc, char *i_argv[])
{
    std::cout << "GUI STARTED" << std::endl;
    if (i_argc > 2)
    {
        PORT = atoi(i_argv[1]);
    }

    tsunami_lab::ui::GUI gui;
    return gui.launch(PORT);
}