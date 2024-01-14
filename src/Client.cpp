#include "ui/GUI.h"
#include <iostream>

int main()
{
    std::cout << "GUI STARTED" << std::endl;
    tsunami_lab::ui::GUI gui;
    return gui.launch();
    
}