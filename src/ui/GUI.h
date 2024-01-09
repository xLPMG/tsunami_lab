/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Entry point of the GUI.
 **/

#ifndef TSUNAMI_LAB_UI_GUI
#define TSUNAMI_LAB_UI_GUI

#include "Communicator.hpp"

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
  xlpmg::Communicator m_communicator;
public:
  int launch(int PORT);
};

#endif