/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Entry point of the GUI.
 **/

#ifndef TSUNAMI_LAB_UI_GUI_H
#define TSUNAMI_LAB_UI_GUI_H

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
  int m_clientReadBufferSize = m_communicator.BUFF_SIZE_DEFAULT;

  // compiler options
  const char *m_compileModes[5] = {"release", "debug", "release+san", "debug+san", "benchmark"};
  int m_compileMode = 0;
  char m_compilerChoice[256] = "";
  const char *m_optFlags[5] = {"0", "1", "2", "3", "fast"};
  int m_optFlag = 2;
  const char *m_ompFlags[3] = {"none", "gnu", "intel"};
  int m_ompFlag = 0;
  int m_ompNumThreads = -1;
  const char *m_reportFlags[7] = {"none", "0", "1", "2", "3", "4", "5"};
  int m_reportFlag = 0;
  bool m_useFilesystem = true;
  char m_customEnv[256] = "";
  bool m_useGui = true;

  // simulation parameters
  const char *m_events[3] = {"Tohoku", "Chile", "Custom"};
  int event_current = 0;
  int l_nx = 1;
  int l_ny = 1;
  float l_simulationSizeX = 0;
  float l_simulationSizeY = 0;
  int m_endTime = 1000;
  bool m_useFileIO = true;
  int m_writingFrequency = 100;
  int m_checkpointFrequency = 10;

  // outflow conditions
  bool outflowL = false;
  bool outflowR = false;
  bool outflowT = false;
  bool outflowB = false;

  // client log
  bool m_clientLogAutoScroll = true;

  int exec(std::string i_cmd, std::string i_outputFile);
  json createJson();
  void updateData();

public:
  int launch(int PORT);
};

#endif