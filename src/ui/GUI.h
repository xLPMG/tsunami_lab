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
#include "../constants.h"

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

  int PORT = 8080;
  char IPADDRESS[16] = "127.0.0.1";
  bool m_connected = false;

  std::chrono::time_point<std::chrono::system_clock> m_lastSystemInfoUpdate;
  int m_systemInfoUpdateFrequency = 2;
  bool m_logSystemInfoDataTransmission = false;

  std::chrono::time_point<std::chrono::system_clock> m_lastTimeValuesUpdate;
  int m_timeValuesUpdateFrequency = 2;
  bool m_logTimeValuesDataTransmission = false;

  int m_clientReadBufferSize = m_communicator.BUFF_SIZE_READ_DEFAULT;
  int m_clientSendBufferSize = m_communicator.BUFF_SIZE_READ_DEFAULT;
  int m_serverReadBufferSize = m_communicator.BUFF_SIZE_READ_DEFAULT;
  int m_serverSendBufferSize = m_communicator.BUFF_SIZE_READ_DEFAULT;

  // compiler options
  int m_serverRestartPort = 8080;
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
  const char *m_runnerOptions[3] = {"none", "bash", "sbatch"};
  int m_runner = 0;
  bool m_checkpointBeforeRecomp = false;
  char m_sbJob[256] = "tsunami_lab_auto";
  char m_sbOut[256] = "output.txt";
  char m_sbErr[256] = "error.txt";
  char m_sbTim[256] = "10:00:00";

  // simulation parameters
  const char *m_tsunamiEvents[3] = {"CUSTOM", "ARTIFICIAL2D", "CIRCULARDAMBREAK2D"};
  int m_tsunamiEvent = 0;
  int m_nx = 1;
  int m_ny = 1;
  int m_nk = 1;
  float m_simulationSizeX = 10;
  float m_simulationSizeY = 1;
  float m_offsetX = 0;
  float m_offsetY = 0;
  int m_endTime = 30;
  bool m_useFileIO = true;
  const char *m_outputMethods[2] = {"netcdf", "csv"};
  int m_outputMethod = 0;
  int m_writingFrequency = 100;
  char m_outputFileName[256] = "solution";
  int m_stationFrequency = 0;
  int m_checkpointFrequency = 10;
  int m_height = 0;
  int m_baseHeight = 0;
  int m_diameter = 0;
  float m_timeStepScaling = 1;

  char m_transferLocalFilePath[256] = "";
  char m_transferRemoteFilePath[256] = "";

  char m_bathymetryFilePath[256] = "";
  char m_displacementFilePath[256] = "";

  // outflow conditions
  bool m_boundaryL = false;
  bool m_boundaryR = false;
  bool m_boundaryT = false;
  bool m_boundaryB = false;

  // stations
  struct Station
  {
    std::string name;
    float positionX = 0;
    float positionY = 0;
    bool isSelected = false;
  };

  std::vector<Station> m_stations;
  float m_currStationX = 0;
  float m_currStationY = 0;
  char m_currStationName[256] = "";

  // client log
  bool m_clientLogAutoScroll = true;

  // simulation status
  std::string m_simulationStatus = "UNKNOWN";
  bool m_isPausing = false;
  int m_currentTimeStep = 0;
  int m_maxTimeSteps = 0;
  int m_timePerTimeStep = 0;
  double m_estimatedTimeLeft = 0;

  std::vector<float> m_cpuData;
  double m_totalRAM = 0;
  double m_usedRAM = 0;

  std::vector<float> m_stationTime;
  std::vector<float> m_stationBathymetry;
  std::vector<float> m_stationMomentumX;
  std::vector<float> m_stationMomentumY;
  std::vector<float> m_stationTotalHeight;
  std::string m_stationFilePath = "";

  tsunami_lab::t_real *m_heightData = nullptr;
  tsunami_lab::t_real *m_bathymetryData = nullptr;
  float scale_min = -1;
  float scale_max = 1;
  long currCellsX = 0;
  long currCellsY = 0;
  int currOffsetX = 0;
  int currOffsetY = 0;
  int currSimSizeX = 0;
  int currSimSizeY = 0;

  /**
   * Executes a shell command.
   *
   * @param i_cmd command
   * @param i_outputFile file to pipe the shell output to
   * @return exit code
   */
  int exec(std::string i_cmd, std::string i_outputFile);

  /**
   * Creates a config json from local config parameters.
   *
   * @return config as json object
   */
  json createConfigJson();

  /**
   * Gets info on CPU and RAM usage from the server.
   */
  void updateSystemInfo();

  /**
   * Gets info on time step and time per time step from the server.
   */
  void updateTimeValues();

public:
  /**
   * Entry-point for the GUI.
   *
   * @return exit code
   */
  int launch();
};

#endif