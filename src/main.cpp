/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Entry point into the program.
 **/
#include "Launcher.h"
#ifdef USEGUI
#include "ui/GUI.h"
#endif

/**
 * @brief Main function for the tsunami wave equation solver.
 *
 * @param argc Number of command line arguments.
 * @param argv Array of command line argument strings.
 *
 * @return Exit code (0 for success, non-zero for errors).
 */
int main(int i_argc,
         char *i_argv[])
{
  tsunami_lab::Launcher *launcher = new tsunami_lab::Launcher;

#ifdef USEGUI
  tsunami_lab::ui::GUI gui;
  if (i_argc > 2){
    launcher->loadConfigDataFromFile(i_argv[1]);
  }
  gui.launch(launcher);
#else
  if (i_argc > 2)
  {
    return launcher->start("");
  }
  else
  {
    return launcher->start(i_argv[1]);
  }
#endif
}