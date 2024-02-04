/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Entry point for the client side of the tsunami_lab program.
 * It launches the GUI.
 **/

#include "ui/GUI.h"
#include <iostream>

/**
 * @brief Main function for the tsunami_lab client program. It creates a GUI object and calls its launch method.
 * 
 * @return Exit code (0 for success, non-zero for errors).
 */
int main()
{
    std::cout << "GUI STARTED" << std::endl;
    tsunami_lab::ui::GUI gui;
    return gui.launch();
}