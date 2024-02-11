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
  //! Version of the GUI
  const char *VERSION = "1.0.2";
  //! Width of the window
  const unsigned int WINDOW_WIDTH = 1500;
  //! Height of the window
  const unsigned int WINDOW_HEIGHT = 1000;

  //! The communcator object
  xlpmg::Communicator m_communicator;
  //! Local copy of the client log
  std::string m_clientLog;
  //! Whether the client log should auto-scroll
  bool m_clientLogAutoScroll = true;

  //! Port of the server
  int PORT = 8080;
  //! IP address of the server
  char IPADDRESS[16] = "127.0.0.1";
  //! Connection status
  bool m_connected = false;

  //! Last system info update
  std::chrono::time_point<std::chrono::system_clock> m_lastSystemInfoUpdate;
  //! System info update frequency
  int m_systemInfoUpdateFrequency = 2;
  //! Log system info data transmission
  bool m_logSystemInfoDataTransmission = false;

  //! Last time values update
  std::chrono::time_point<std::chrono::system_clock> m_lastTimeValuesUpdate;
  //! Time values update frequency
  int m_timeValuesUpdateFrequency = 2;
  //! Log time values data transmission
  bool m_logTimeValuesDataTransmission = false;

  //! Client read buffer size
  int m_clientReadBufferSize = m_communicator.BUFF_SIZE_READ_DEFAULT;
  //! Client send buffer size
  int m_clientSendBufferSize = m_communicator.BUFF_SIZE_SEND_DEFAULT;
  //! Server read buffer size
  int m_serverReadBufferSize = m_communicator.BUFF_SIZE_READ_DEFAULT;
  //! Server send buffer size
  int m_serverSendBufferSize = m_communicator.BUFF_SIZE_SEND_DEFAULT;

  //! Port which the server restarts on
  int m_serverRestartPort = 8080;
  //! Compile modes from the SConstruct file
  const char *m_compileModes[5] = {"release", "debug", "release+san", "debug+san", "benchmark"};
  //! Selected compile mode
  int m_compileMode = 0;
  //! Compiler choice
  char m_compilerChoice[256] = "";
  //! Optimization flags
  const char *m_optFlags[5] = {"0", "1", "2", "3", "fast"};
  //! Selected optimization flag
  int m_optFlag = 2;
  //! OpenMP flags
  const char *m_ompFlags[3] = {"none", "gnu", "intel"};
  //! Selected OpenMP flag
  int m_ompFlag = 0;
  //! Number of OpenMP threads
  int m_ompNumThreads = -1;
  //! Report flagss
  const char *m_reportFlags[7] = {"none", "0", "1", "2", "3", "4", "5"};
  //! Selected report flag
  int m_reportFlag = 0;
  //! Use filesystem flag
  bool m_useFilesystem = true;
  //! Custom environment variables
  char m_customEnv[256] = "";
  //! Whether to use the GUI
  bool m_useGui = true;
  //! Options for the runner
  const char *m_runnerOptions[3] = {"none", "bash", "sbatch"};
  //! Selected runner option
  int m_runner = 0;
  //! Whether to checkpoint before recompiling
  bool m_checkpointBeforeRecomp = false;
  //! SLURM job name
  char m_sbJob[256] = "tsunami_lab_auto";
  //! SLURM job output file
  char m_sbOut[256] = "output.txt";
  //! SLURM job error file
  char m_sbErr[256] = "error.txt";
  //! SLURM job wall clock
  char m_sbTim[256] = "10:00:00";

  //! Path to the config file
  char m_configFilePath[256] = "";
  //! Tsunami event types
  const char *m_tsunamiEvents[3] = {"CUSTOM2D", "ARTIFICIAL2D", "CIRCULARDAMBREAK2D"};
  //! Selected tsunami event
  int m_tsunamiEvent = 0;
  //! Number of cells in x direction
  int m_nx = 1;
  //! Number of cells in y direction
  int m_ny = 1;
  //! Cell grouping size for coarse output
  int m_nk = 1;
  //! Simulation size in x direction
  float m_simulationSizeX = 10;
  //! Simulation size in y direction
  float m_simulationSizeY = 1;
  //! Offset in x direction
  float m_offsetX = 0;
  //! Offset in y direction
  float m_offsetY = 0;
  //! Simulated time in seconds
  int m_endTime = 30;
  //! Whether to use file IO or not
  bool m_useFileIO = true;
  //! Data output method options
  const char *m_outputMethods[2] = {"netcdf", "csv"};
  //! Selected data output method
  int m_outputMethod = 0;
  //! Data output frequency
  int m_writingFrequency = 100;
  //! Output file name
  char m_outputFileName[256] = "solution";
  //! Station frequency
  int m_stationFrequency = 0;
  //! Checkpoint frequency
  int m_checkpointFrequency = 10;
  //! Elevated water height (currently only used for CIRCULARDAMBREAK2D)
  int m_height = 10;
  //! Water height at the base (currently only used for CIRCULARDAMBREAK2D)
  int m_baseHeight = 5;
  //! Diameter of the elevated circular water body (currently only used for CIRCULARDAMBREAK2D)
  int m_diameter = 10;
  //! Scaling factor for the time step
  float m_timeStepScaling = 1.0f;

  //! Path to the local file to transfer
  char m_transferLocalFilePath[256] = "";
  //! Remote path to transfer the file to
  char m_transferRemoteFilePath[256] = "";

  //! Remote path to the bathymetry file
  char m_bathymetryFilePath[256] = "";
  //! Remote path to the displacement file
  char m_displacementFilePath[256] = "";

  //! Boundary condition on the left side
  bool m_boundaryL = false;
  //! Boundary condition on the right side
  bool m_boundaryR = false;
  //! Boundary condition on the top side
  bool m_boundaryT = false;
  //! Boundary condition on the bottom side
  bool m_boundaryB = false;

  //! Station struct
  struct Station
  {
    std::string name;
    float positionX = 0;
    float positionY = 0;
    bool isSelected = false;
  };

  //! List of stations
  std::vector<Station> m_stations;
  //! Currently selected station x position
  float m_currStationX = 0;
  //! Currently selected station y position
  float m_currStationY = 0;
  //! Currently selected station name
  char m_currStationName[256] = "";

  //! Status of the simulation
  std::string m_simulationStatus = "UNKNOWN";
  //! Whether the simulation was paused
  bool m_isPausing = false;
  //! Current time step
  int m_currentTimeStep = 0;
  //! Maximum number of time steps
  int m_maxTimeSteps = 0;
  //! Time per time step in milliseconds
  double m_timePerTimeStep = 0;
  //! Estimated time left in seconds
  double m_estimatedTimeLeft = 0;

  //! Vector of CPU usage data (entries correspond to the individual cores)
  std::vector<float> m_cpuData;
  //! Total RAM available on the server machine
  double m_totalRAM = 0;
  //! Used RAM on the server machine
  double m_usedRAM = 0;

  //! Vector of time values of the station data
  std::vector<float> m_stationTime;
  //! Bathymetry data at the station
  std::vector<float> m_stationBathymetry;
  //! Momentum in x direction at the station
  std::vector<float> m_stationMomentumX;
  //! Momentum in y direction at the station
  std::vector<float> m_stationMomentumY;
  //! Total height at the station
  std::vector<float> m_stationTotalHeight;
  //! Path to the station data file
  std::string m_stationFilePath = "";

  //! Array of water height data
  tsunami_lab::t_real *m_heightData = nullptr;
  //! Array of bathymetry data
  tsunami_lab::t_real *m_bathymetryData = nullptr;
  //! Minimum of the color scale
  float m_scaleMin = -1;
  //! Maximum of the color scale
  float m_scaleMax = 1;
  //! Current number of cells in x direction for data visualization
  long m_currCellsX = 0;
  //! Current number of cells in y direction for data visualization
  long m_currCellsY = 0;
  //! Current offset in x direction for data visualization
  tsunami_lab::t_real m_currOffsetY = 0;
  //! Current offset in y direction for data visualization
  tsunami_lab::t_real m_currOffsetX = 0;
  //! Current simulation size in x direction for data visualization
  tsunami_lab::t_real m_currSimSizeX = 0;
  //! Current simulation size in y direction for data visualization
  tsunami_lab::t_real m_currSimSizeY = 0;

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