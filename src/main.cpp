/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Entry point into the program.
 **/
#define USEGUI 1
#include <thread>
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
  int exitCode = 0;
#ifdef USEGUI
  tsunami_lab::ui::GUI gui;
  tsunami_lab::Launcher launcher;
  std::thread t(&tsunami_lab::ui::GUI::launch, &gui, &launcher);
  if (i_argc > 2){
    launcher.loadConfigDataFromFile(i_argv[1]);
  }
  t.join();
  //exitCode = gui.launch(launcher);

#else
  tsunami_lab::Launcher *launcher = new tsunami_lab::Launcher;
  if (i_argc > 2)
  {
    exitCode = launcher->start(i_argv[1]);
  }
  else
  {
    exitCode = launcher->start("");
  }
#endif
return exitCode;
}