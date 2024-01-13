/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Entry point of the GUI.
 **/

#ifndef TSUNAMI_LAB_UI_GUI
#define TSUNAMI_LAB_UI_GUI

#include "xlpmg/Communicator.hpp"
#include <chrono>

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
  const unsigned int WINDOW_WIDTH = 1500;
  const unsigned int WINDOW_HEIGHT = 1000;

  xlpmg::Communicator m_communicator;
  std::string m_clientLog;
  int PORT = 0;
  char IPADDRESS[16] = "127.0.0.1";
  std::chrono::time_point<std::chrono::system_clock> lastDataUpdate;
  float dataUpdateFrequency = 1;

  int exec(std::string i_cmd, std::string i_outputFile);
  void updateData();

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
  int omp_current = 0;

  // outflow conditions
  bool outflowL = false;
  bool outflowR = false;
  bool outflowT = false;
  bool outflowB = false;
};

#endif