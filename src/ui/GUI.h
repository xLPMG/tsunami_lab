/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Entry point of the GUI.
 **/

#ifndef TSUNAMI_LAB_UI_GUI
#define TSUNAMI_LAB_UI_GUI

#include "../Launcher.h"

namespace tsunami_lab
{
  namespace ui
  {
    class GUI;
  }
}

class tsunami_lab::ui::GUI
{
private:

  tsunami_lab::Launcher* m_launcher;

  void setupFolders();

public:
  int launch(tsunami_lab::Launcher launcher);
};

#endif