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
  std::string m_clientLog;
  int PORT = 0;
  char IPADDRESS[128] = "127.0.0.1";
public:
  int launch(int PORT);

  
};

#endif