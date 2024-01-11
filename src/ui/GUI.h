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

#include <nlohmann/json.hpp>
using json = nlohmann::json;


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
  char IPADDRESS[16] = "127.0.0.1";
  char inputBuffer[256] = {0};
public:
  int launch(int PORT);

  json createJson();


    int event_current = 0;
    int l_nx = 1;
    int l_ny = 1;
    float l_simulationSizeX = 0;
    float l_simulationSizeY = 0;
    float endTime = 1000;
    int writingFrequency = 100; // todo: change int to t_idx
    int flag_current = 0;
  
};

#endif