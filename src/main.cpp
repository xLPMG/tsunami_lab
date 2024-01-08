#include "Launcher.h"

#ifdef USEGUI
#include "ui/GUI.h"
#endif

int main(int i_argc,
         char *i_argv[])
{
#ifdef USEGUI
  tsunami_lab::ui::GUI gui;
  gui.launch();
#endif

  tsunami_lab::Launcher launcher;
  launcher.start();
}