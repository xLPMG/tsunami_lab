/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Class that launches and controls the simulation.
 **/

#include "Simulator.h"

// c libraries
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <fstream>
#include <limits>
#include <chrono>
#include <future>

#ifndef NOFILESYSTEM
#include <filesystem>
#endif

// external libraries
#ifdef USEOMP
#include <omp.h>
#endif

//-------------------------------------------//
//-------------PRIVATE FUNCTIONS-------------//
//-------------------------------------------//

bool tsunami_lab::Simulator::endsWith(std::string const &i_str, std::string const &i_suffix)
{
  if (i_str.length() < i_suffix.length())
    return false;
  return i_str.compare(i_str.length() - i_suffix.length(), i_suffix.length(), i_suffix) == 0;
}

void tsunami_lab::Simulator::setupFolders()
{
  std::cout << ">> Setting up folders" << std::endl;
  // create solutions folder
  if (!std::filesystem::exists("solutions"))
    std::filesystem::create_directory("solutions");

  // set up stations folder for stations to save their data in
  if (!std::filesystem::exists("stations"))
    std::filesystem::create_directory("stations");

  if (!std::filesystem::exists("checkpoints"))
    std::filesystem::create_directory("checkpoints");
}

void tsunami_lab::Simulator::configureFiles()
{
  m_outputFileName = m_configData.value("outputFileName", "solution");
  m_netCdfOutputPathString = "solutions/" + m_outputFileName + ".nc";
  m_netcdfOutputPath = m_netCdfOutputPathString.c_str();

  // check if checkpoint exists
  m_checkPointFilePathString = "checkpoints/" + m_outputFileName + ".nc";
  m_checkPointFilePath = m_checkPointFilePathString.c_str();
  m_checkpointExists = std::filesystem::exists(m_checkPointFilePathString);
  // checkpoint file found
  if (m_checkpointExists)
  {
    std::cout << "Found checkpoint file: " << m_checkPointFilePath << std::endl;
    m_setupChoice = "CHECKPOINT";
  }
  else
  {
    // no checkpoint but solution file exists
    if (std::filesystem::exists(m_netCdfOutputPathString))
    {
      std::cout << "Solution file exists but no checkpoint was found. The solution will be deleted." << std::endl;
      std::filesystem::remove(m_netCdfOutputPathString);
    }
    m_setupChoice = m_configData.value("setup", "CIRCULARDAMBREAK2D");
  }
}

void tsunami_lab::Simulator::loadConfiguration()
{
  std::cout << ">> Loading configuration from local json data" << std::endl;

  m_solver = m_configData.value("solver", "fwave");
  // read size config
  m_nx = m_configData.value("nx", 1);
  m_ny = m_configData.value("ny", 1);
  m_nk = m_configData.value("nk", 1);
  m_simulationSizeX = m_configData.value("simulationSizeX", 10);
  m_simulationSizeY = m_configData.value("simulationSizeY", 1);
  m_dx = m_simulationSizeX / m_nx;
  m_dy = m_simulationSizeY / m_ny;
  m_offsetX = m_configData.value("offsetX", 0);
  m_offsetY = m_configData.value("offsetY", 0);
  m_endTime = m_configData.value("endTime", 20);
  // read boundary config
  std::string l_boundaryStringL = m_configData.value("boundaryL", "outflow");
  if (l_boundaryStringL == "outflow" || l_boundaryStringL == "OUTFLOW")
    m_boundaryL = Boundary::OUTFLOW;
  else if (l_boundaryStringL == "wall" || l_boundaryStringL == "WALL")
    m_boundaryL = Boundary::WALL;

  std::string l_boundaryStringR = m_configData.value("boundaryR", "outflow");
  if (l_boundaryStringR == "outflow" || l_boundaryStringR == "OUTFLOW")
    m_boundaryR = Boundary::OUTFLOW;
  else if (l_boundaryStringR == "wall" || l_boundaryStringR == "WALL")
    m_boundaryR = Boundary::WALL;

  std::string l_boundaryStringT = m_configData.value("boundaryT", "outflow");
  if (l_boundaryStringT == "outflow" || l_boundaryStringT == "OUTFLOW")
    m_boundaryT = Boundary::OUTFLOW;
  else if (l_boundaryStringT == "wall" || l_boundaryStringT == "WALL")
    m_boundaryT = Boundary::WALL;

  std::string l_boundaryStringB = m_configData.value("boundaryB", "outflow");
  if (l_boundaryStringB == "outflow" || l_boundaryStringB == "OUTFLOW")
    m_boundaryB = Boundary::OUTFLOW;
  else if (l_boundaryStringB == "wall" || l_boundaryStringB == "WALL")
    m_boundaryB = Boundary::WALL;

  m_useFileIO = m_configData.value("useFileIO", true);
#ifdef NOFILESYSTEM
  m_useFileIO = false;
#endif

  m_bathymetryFilePath = m_configData.value("bathymetry", "");
  m_displacementFilePath = m_configData.value("displacement", "");

  m_writingFrequency = m_configData.value("writingFrequency", 80);
  m_checkpointFrequency = m_configData.value("checkpointFrequency", -1);

  // read station data
  m_stationFrequency = m_configData.value("stationFrequency", 1);
  if (m_useFileIO)
  {
    std::string l_outputMethod = m_configData.value("outputMethod", "netcdf");
    if (l_outputMethod == "netcdf" || l_outputMethod == "NETCDF")
    {
      m_dataWriter = NETCDF;
    }
    else if (l_outputMethod == "csv" || l_outputMethod == "CSV")
    {
      m_dataWriter = CSV;
    }
  }
}

void tsunami_lab::Simulator::constructSetup()
{
  if (m_setup != nullptr)
    return;

  std::cout << ">> Constructing setup" << std::endl;
  if (m_setupChoice == "GENERALDISCONTINUITY1D")
  {
    m_setup = new tsunami_lab::setups::GeneralDiscontinuity1d(10, 10, 10, -10, m_simulationSizeX / 2);
  }
  else if (m_setupChoice == "DAMBREAK1D")
  {
    m_setup = new tsunami_lab::setups::DamBreak1d(10, 5, m_simulationSizeX / 2);
  }
  else if (m_setupChoice == "CIRCULARDAMBREAK2D")
  {
    m_setup = new tsunami_lab::setups::CircularDamBreak2d();
  }
  else if (m_setupChoice == "RARERARE1D")
  {
    m_setup = new tsunami_lab::setups::RareRare1d(10, 5, m_simulationSizeX / 2);
  }
  else if (m_setupChoice == "SHOCKSHOCK1D")
  {
    m_setup = new tsunami_lab::setups::ShockShock1d(10, 5, m_simulationSizeX / 2);
  }
  else if (m_setupChoice == "SUBCRITICAL1D")
  {
    m_simulationSizeX = 10;
    m_setup = new tsunami_lab::setups::Subcritical1d(0.0001, 5);
  }
  else if (m_setupChoice == "SUPERCRITICAL1D")
  {
    m_simulationSizeX = 10;
    m_setup = new tsunami_lab::setups::Supercritical1d(0.0001, 5);
  }
  else if (m_setupChoice == "TSUNAMIEVENT1D")
  {
    m_setup = new tsunami_lab::setups::TsunamiEvent1d(m_bathymetryFilePath);
  }
  else if (m_setupChoice == "TSUNAMIEVENT2D")
  {
    tsunami_lab::io::NetCdf *l_netCdfTE2D = new tsunami_lab::io::NetCdf(m_nx,
                                                                        m_ny,
                                                                        m_nk,
                                                                        m_simulationSizeX,
                                                                        m_simulationSizeY,
                                                                        m_offsetX,
                                                                        m_offsetY,
                                                                        m_netcdfOutputPath,
                                                                        m_checkPointFilePath);
    m_setup = new tsunami_lab::setups::TsunamiEvent2d("resources/artificialtsunami_bathymetry_1000.nc",
                                                      "resources/artificialtsunami_dispm_1000.nc",
                                                      l_netCdfTE2D,
                                                      m_nx);
  }
  else if (m_setupChoice == "ARTIFICIAL2D")
  {
    m_simulationSizeX = 10000;
    m_simulationSizeY = 10000;
    m_offsetX = -5000;
    m_offsetY = -5000;
    m_setup = new tsunami_lab::setups::ArtificialTsunami2d();
  }
  else if (m_setupChoice == "CHILE")
  {
    m_simulationSizeX = 3500000;
    m_simulationSizeY = 2950000;
    m_offsetX = -2999875;
    m_offsetY = -1449875;

    tsunami_lab::io::NetCdf *l_netCdfChile = new tsunami_lab::io::NetCdf(m_nx,
                                                                         m_ny,
                                                                         m_nk,
                                                                         m_simulationSizeX,
                                                                         m_simulationSizeY,
                                                                         m_offsetX,
                                                                         m_offsetY,
                                                                         m_netcdfOutputPath,
                                                                         m_checkPointFilePath);
    m_setup = new tsunami_lab::setups::TsunamiEvent2d("resources/chile/chile_gebco20_usgs_250m_bath_fixed.nc",
                                                      "resources/chile/chile_gebco20_usgs_250m_displ_fixed.nc",
                                                      l_netCdfChile,
                                                      m_nx);
  }
  else if (m_setupChoice == "TOHOKU")
  {
    m_simulationSizeX = 2700000;
    m_simulationSizeY = 1500000;
    m_offsetX = -199875;
    m_offsetY = -749875;

    tsunami_lab::io::NetCdf *l_netCdfTohoku = new tsunami_lab::io::NetCdf(m_nx,
                                                                          m_ny,
                                                                          m_nk,
                                                                          m_simulationSizeX,
                                                                          m_simulationSizeY,
                                                                          m_offsetX,
                                                                          m_offsetY,
                                                                          m_netcdfOutputPath,
                                                                          m_checkPointFilePath);
    m_setup = new tsunami_lab::setups::TsunamiEvent2d("resources/tohoku/tohoku_gebco08_ucsb3_250m_bath.nc",
                                                      "resources/tohoku/tohoku_gebco08_ucsb3_250m_displ.nc",
                                                      l_netCdfTohoku,
                                                      m_nx);
  }
  else if (m_setupChoice == "CUSTOM")
  {
    // DO NOTHING FOR NOW
  }
  else
  {
    m_setup = nullptr;
  }
}

void tsunami_lab::Simulator::setUpNetCdf()
{
  if (m_netCdf != nullptr)
    return;

  std::cout << ">> Setting up netcdf I/O" << std::endl;
  if (m_setupChoice == "CHECKPOINT")
  {
    m_netCdf = new tsunami_lab::io::NetCdf(m_netcdfOutputPath,
                                           m_checkPointFilePath);
    m_netCdf->loadCheckpointDimensions(m_checkPointFilePath,
                                       m_nx,
                                       m_ny,
                                       m_nk,
                                       m_simulationSizeX,
                                       m_simulationSizeY,
                                       m_offsetX,
                                       m_offsetY,
                                       m_simTime,
                                       m_timeStep);
    std::cout << std::endl;
    std::cout << "Loaded following data from checkpoint: " << std::endl;
    std::cout << "  Cells x:                  " << m_nx << std::endl;
    std::cout << "  Cells y:                  " << m_ny << std::endl;
    std::cout << "  Simulation size x:        " << m_simulationSizeX << std::endl;
    std::cout << "  Simulation size y:        " << m_simulationSizeY << std::endl;
    std::cout << "  Offset x:                 " << m_offsetX << std::endl;
    std::cout << "  Offset y:                 " << m_offsetY << std::endl;
    std::cout << "  Current simulation time:  " << m_simTime << std::endl;
    std::cout << "  Current time step:        " << m_timeStep << std::endl;
    std::cout << std::endl;
  }
  else
  {
    m_netCdf = new tsunami_lab::io::NetCdf(m_nx,
                                           m_ny,
                                           m_nk,
                                           m_simulationSizeX,
                                           m_simulationSizeY,
                                           m_offsetX,
                                           m_offsetY,
                                           m_netcdfOutputPath,
                                           m_checkPointFilePath);
  }
}

void tsunami_lab::Simulator::createWaveProp()
{
  std::cout << ">> Creating WavePropagation patch" << std::endl;
  if (m_ny == 1)
  {
    m_waveProp = new tsunami_lab::patches::WavePropagation1d(m_nx,
                                                             m_solver,
                                                             m_boundaryL,
                                                             m_boundaryR);
  }
  else
  {
    m_waveProp = new tsunami_lab::patches::WavePropagation2d(m_nx,
                                                             m_ny,
                                                             m_boundaryL,
                                                             m_boundaryR,
                                                             m_boundaryT,
                                                             m_boundaryB);
  }
}

void tsunami_lab::Simulator::constructSolver()
{
  std::cout << ">> Setting up solver" << std::endl;
  // set up solver
  if (m_setupChoice == "CHECKPOINT" && m_useFileIO)
  {
    if (m_netCdf == nullptr)
      return;
    tsunami_lab::t_real *l_hCheck = new tsunami_lab::t_real[m_nx * m_ny];
    tsunami_lab::t_real *l_huCheck = new tsunami_lab::t_real[m_nx * m_ny];
    tsunami_lab::t_real *l_hvCheck = new tsunami_lab::t_real[m_nx * m_ny];
    tsunami_lab::t_real *l_bCheck = new tsunami_lab::t_real[m_nx * m_ny];
    m_netCdf->read(m_checkPointFilePath, "height", &l_hCheck);
    m_netCdf->read(m_checkPointFilePath, "momentumX", &l_huCheck);
    m_netCdf->read(m_checkPointFilePath, "momentumY", &l_hvCheck);
    m_netCdf->read(m_checkPointFilePath, "bathymetry", &l_bCheck);

    // BREAKPOINT
    if (m_shouldExit)
      return;
      // END BREAKPOINT
#ifdef USEOMP
#pragma omp parallel for
#endif
    for (tsunami_lab::t_idx l_cy = 0; l_cy < m_ny; l_cy++)
    {
      for (tsunami_lab::t_idx l_cx = 0; l_cx < m_nx; l_cx++)
      {
        // BREAKPOINT
        if (m_shouldExit)
          continue;
        // END BREAKPOINT

        m_hMax = std::max(l_hCheck[l_cx + l_cy * m_nx], m_hMax);

        m_waveProp->setHeight(l_cx,
                              l_cy,
                              l_hCheck[l_cx + l_cy * m_nx]);

        m_waveProp->setMomentumX(l_cx,
                                 l_cy,
                                 l_huCheck[l_cx + l_cy * m_nx]);

        m_waveProp->setMomentumY(l_cx,
                                 l_cy,
                                 l_hvCheck[l_cx + l_cy * m_nx]);

        m_waveProp->setBathymetry(l_cx,
                                  l_cy,
                                  l_bCheck[l_cx + l_cy * m_nx]);
      }
    }
    delete[] l_hCheck;
    delete[] l_huCheck;
    delete[] l_hvCheck;
    delete[] l_bCheck;
  }
  else if (m_setup != nullptr)
  {
#ifdef USEOMP
#pragma omp parallel for
#endif
    for (tsunami_lab::t_idx l_cy = 0; l_cy < m_ny; l_cy++)
    {
      tsunami_lab::t_real l_y = l_cy * m_dy + m_offsetY;
      for (tsunami_lab::t_idx l_cx = 0; l_cx < m_nx; l_cx++)
      {
        // BREAKPOINT
        if (m_shouldExit)
          continue;
        // END BREAKPOINT

        tsunami_lab::t_real l_x = l_cx * m_dx + m_offsetX;
        // get initial values of the setup
        tsunami_lab::t_real l_h = m_setup->getHeight(l_x,
                                                     l_y);
        m_hMax = std::max(l_h, m_hMax);

        tsunami_lab::t_real l_hu = m_setup->getMomentumX(l_x,
                                                         l_y);
        tsunami_lab::t_real l_hv = m_setup->getMomentumY(l_x,
                                                         l_y);
        tsunami_lab::t_real l_b = m_setup->getBathymetry(l_x,
                                                         l_y);

        // set initial values in wave propagation solver
        m_waveProp->setHeight(l_cx,
                              l_cy,
                              l_h);

        m_waveProp->setMomentumX(l_cx,
                                 l_cy,
                                 l_hu);

        m_waveProp->setMomentumY(l_cx,
                                 l_cy,
                                 l_hv);

        m_waveProp->setBathymetry(l_cx,
                                  l_cy,
                                  l_b);
      }
    }
  }
}

void tsunami_lab::Simulator::loadBathymetry(std::string *i_file)
{
  // load bathymetry from file
  if (m_bathymetryFilePath.length() > 0)
  {
    if (m_bathymetryFilePath.compare(m_bathymetryFilePath.length() - 4, 4, ".csv") == 0)
    {
      std::cout << "Loading bathymetry from csv file: " << *i_file << std::endl;
      tsunami_lab::io::BathymetryLoader *l_bathymetryLoader = new tsunami_lab::io::BathymetryLoader();
      l_bathymetryLoader->loadBathymetry(*i_file);
      for (tsunami_lab::t_idx l_cy = 0; l_cy < m_ny; l_cy++)
      {
        tsunami_lab::t_real l_y = l_cy * m_dy;
        for (tsunami_lab::t_idx l_cx = 0; l_cx < m_nx; l_cx++)
        {
          tsunami_lab::t_real l_x = l_cx * m_dx;
          tsunami_lab::t_real l_b = l_bathymetryLoader->getBathymetry(l_x, l_y);
          m_waveProp->setBathymetry(l_cx,
                                    l_cy,
                                    l_b);
        }
      }
      m_waveProp->adjustWaterHeight();
      delete l_bathymetryLoader;
      std::cout << "Done loading bathymetry." << std::endl;
    }
    else
    {
      std::cerr << "Error: Don't know how to read file " << *i_file << std::endl;
    }
  }
}

void tsunami_lab::Simulator::loadStations()
{
  // set up stations
  if (m_configData.contains("stations"))
  {
    std::cout << "Setting up stations..." << std::endl;
    std::cout << "Frequency for all stations is " << m_stationFrequency << std::endl;
    for (json &elem : m_configData["stations"])
    {
      // location in meters
      tsunami_lab::t_real l_x = elem.at("locX");
      tsunami_lab::t_real l_y = elem.at("locY");

      // location cell
      tsunami_lab::t_idx l_cx = (l_x - m_offsetX) / m_dx;
      tsunami_lab::t_idx l_cy = (l_y - m_offsetY) / m_dy;

      m_stations.push_back(new tsunami_lab::io::Station(l_cx,
                                                        l_cy,
                                                        elem.at("name"),
                                                        m_waveProp));
      std::cout << "Added station " << elem.at("name") << " at x: " << l_x << " and y: " << l_y << std::endl;
    }
  }
}

void tsunami_lab::Simulator::writeStations()
{
  for (tsunami_lab::io::Station *l_s : m_stations)
  {
    l_s->write();
    // BREAKPOINT
    if (m_shouldExit)
      return;
    // END BREAKPOINT
  }
}

void tsunami_lab::Simulator::deriveTimeStep()
{
  // derive maximum wave speed in setup; the momentum is ignored
  tsunami_lab::t_real l_speedMax = std::sqrt(9.81 * m_hMax);

  // derive constant time step; changes at simulation time are ignored
  if (m_ny == 1)
  {
    m_dt = 0.5 * m_dx / l_speedMax;
  }
  else
  {
    m_dt = 0.45 * std::min(m_dx, m_dy) / l_speedMax;
  }
  // calculate max time steps
  m_timeStepMax = std::ceil(m_endTime / m_dt) + 1;
  std::cout << "Note: max " << m_timeStepMax << " steps will be computed." << std::endl;
  // derive scaling for a time step
  m_scalingX = m_dt / m_dx;
  m_scalingY = m_dt / m_dy;

  // options for checkpointing
  if (m_useFileIO)
  {
    std::cout << "Writing every " << m_writingFrequency << " time steps" << std::endl;
    if (m_checkpointFrequency > 0)
    {
      std::cout << "Saving checkpoint every " << m_checkpointFrequency << " seconds" << std::endl;
    }
    else
    {
      std::cout << "Warning: Checkpoints have been disabled for this run. " << std::endl;
    }
    // set count in case we load from a checkpoint file
    if (m_simTime > 0)
    {
      m_captureCount = std::floor(m_simTime / m_stationFrequency);
    }
  }
}

//-------------------------------------------//
//-----------------DELETERS------------------//
//-------------------------------------------//

void tsunami_lab::Simulator::deleteSetup()
{
  if (m_setup != nullptr)
  {
    delete m_setup;
    m_setup = nullptr;
  }
}

void tsunami_lab::Simulator::deleteWaveProp()
{
  if (m_waveProp != nullptr)
  {
    delete m_waveProp;
    m_waveProp = nullptr;
  }
}

void tsunami_lab::Simulator::deleteNetCdf()
{
  if (m_netCdf != nullptr)
  {
    delete m_netCdf;
    m_netCdf = nullptr;
  }
}

void tsunami_lab::Simulator::freeMemory()
{
  m_isPrepared = false;
  deleteSetup();
  deleteWaveProp();
  if (m_useFileIO)
  {
    std::filesystem::remove(m_checkPointFilePathString);
  }
  deleteNetCdf();
  for (tsunami_lab::io::Station *l_s : m_stations)
  {
    delete l_s;
  }
}
//------------------------------------------//
//-------------PUBLIC FUNCTIONS-------------//
//------------------------------------------//

//-------------------------------------------//
//-----------------DELETERS------------------//
//-------------------------------------------//

void tsunami_lab::Simulator::deleteCheckpoints()
{
  std::filesystem::remove(m_checkPointFilePathString);
}

void tsunami_lab::Simulator::deleteStations()
{
  for (tsunami_lab::io::Station *l_s : m_stations)
  {
    delete l_s;
  }
}

void tsunami_lab::Simulator::resetSimulator()
{
  freeMemory();
  prepareForCalculation();
}

//------------------------------------------//
//----------------FUNCTIONS-----------------//
//------------------------------------------//
void tsunami_lab::Simulator::writeCheckpoint()
{
  m_netCdf->writeCheckpoint(m_checkPointFilePath,
                            m_waveProp->getStride(),
                            m_waveProp->getHeight(),
                            m_waveProp->getMomentumX(),
                            m_waveProp->getMomentumY(),
                            m_waveProp->getBathymetry(),
                            m_simTime,
                            m_timeStep);
}

void tsunami_lab::Simulator::loadConfigDataFromFile(std::string i_configFilePath)
{
  std::ifstream l_configFile(i_configFilePath);
  m_configData = json::parse(l_configFile);
}

void tsunami_lab::Simulator::loadConfigDataJson(json i_config)
{
  m_configData = i_config;
}

void tsunami_lab::Simulator::addStation(tsunami_lab::t_real i_locationX,
                                        tsunami_lab::t_real i_locationY,
                                        std::string i_stationName)
{
  // location cell
  tsunami_lab::t_idx l_cx = (i_locationX - m_offsetX) / m_dx;
  tsunami_lab::t_idx l_cy = (i_locationY - m_offsetY) / m_dy;

  m_stations.push_back(new tsunami_lab::io::Station(l_cx,
                                                    l_cy,
                                                    i_stationName,
                                                    m_waveProp));
}

void tsunami_lab::Simulator::prepareForCalculation()
{
  m_isPreparing = true;
  std::cout << "Preparing Tsunami Solver..." << std::endl;
  //-------------------------------------------//
  //--------------File I/O Config--------------//
  //-------------------------------------------//
  m_configData["null"] = "null";

  // set up folders
  if (m_useFileIO)
  {
    setupFolders();
  }

  // BREAKPOINT
  if (m_shouldExit)
  {
    m_isPreparing = false;
    return;
  }
  // END BREAKPOINT

  if (m_useFileIO)
  {
    std::cout << ">> Configuring file data" << std::endl;
    configureFiles();
  }
  else if (m_setupChoice != "Custom")
  {
    m_setupChoice = m_configData.value("setup", "CIRCULARDAMBREAK2D");
    if (m_setupChoice == "CHECKPOINT")
      std::cerr << "Error: Cannot use checkpoints in benchmarking mode" << std::endl;
  }

  loadConfiguration();

  constructSetup();

  // BREAKPOINT
  if (m_shouldExit)
  {
    m_isPreparing = false;
    return;
  }
  // END BREAKPOINT

  if (m_useFileIO)
  {
    setUpNetCdf();
  }

  createWaveProp();

  constructSolver();

  loadBathymetry(&m_bathymetryFilePath);

  if (m_useFileIO)
  {
    loadStations();
  }
  m_isPreparing = false;
}

void tsunami_lab::Simulator::runCalculation()
{
  deriveTimeStep();
  auto l_lastWrite = std::chrono::system_clock::now();
  while (m_simTime < m_endTime && !m_shouldExit)
  {
    //------------------------------------------//
    //---------------Write output---------------//
    //------------------------------------------//
    if (m_useFileIO)
    {
      if (m_timeStep % m_writingFrequency == 0)
      {
        std::cout << "  simulation time / #time steps: "
                  << m_simTime << " / " << m_timeStep << std::endl;

        switch (m_dataWriter)
        {
        case NETCDF:
        {
          std::cout << "  writing to netcdf " << std::endl;
          m_netCdf->write(m_waveProp->getStride(),
                          m_waveProp->getHeight(),
                          m_waveProp->getMomentumX(),
                          m_waveProp->getMomentumY(),
                          m_waveProp->getBathymetry(),
                          m_simTime);
          break;
        }
        case CSV:
        {
          std::string l_csvOutputPath = "solutions/" + m_outputFileName + "_" + std::to_string(m_nOut) + ".csv";
          std::cout << "  writing wave field to " << l_csvOutputPath << std::endl;
          std::ofstream l_file;
          l_file.open(l_csvOutputPath);
          tsunami_lab::io::Csv::write(m_dx,
                                      m_dy,
                                      m_nx,
                                      m_ny,
                                      m_waveProp->getStride(),
                                      m_waveProp->getHeight(),
                                      m_waveProp->getMomentumX(),
                                      m_waveProp->getMomentumY(),
                                      m_waveProp->getBathymetry(),
                                      l_file);
          l_file.close();
          m_nOut++;
          break;
        }
        }
      }
      // write stations
      if (m_simTime >= m_stationFrequency * m_captureCount)
      {
        for (tsunami_lab::io::Station *l_s : m_stations)
        {
          l_s->capture(m_simTime);
        }
        ++m_captureCount;
      }
      // write checkpoint
      if (m_checkpointFrequency > 0 &&
          std::chrono::system_clock::now() - l_lastWrite >= std::chrono::duration<float>(m_checkpointFrequency))
      {
        std::cout << "saving checkpoint to " << m_checkPointFilePathString << std::endl;
        writeCheckpoint();
        l_lastWrite = std::chrono::system_clock::now();
      }
    }
    // BREAKPOINT
    if (m_shouldExit)
      return;
    // END BREAKPOINT

    //------------------------------------------//
    //------------Update loop params------------//
    //------------------------------------------//
    m_waveProp->setGhostOutflow();
    m_waveProp->timeStep(m_scalingX, m_scalingY);
    m_timeStep++;
    m_simTime += m_dt;
  }
}

//-------------------------------------------//
//----------------ENTRY POINT----------------//
//-------------------------------------------//
int tsunami_lab::Simulator::start(std::string i_config)
{
  m_isCalculating = true;
  std::cout << "####################################" << std::endl;
  std::cout << "### Tsunami Lab                  ###" << std::endl;
  std::cout << "###                              ###" << std::endl;
  std::cout << "### https://scalable.uni-jena.de ###" << std::endl;
  std::cout << "###                              ###" << std::endl;
  std::cout << "### by Luca-Philipp Grumbach     ###" << std::endl;
  std::cout << "### and Richard Hofmann          ###" << std::endl;
  std::cout << "###                              ###" << std::endl;
  std::cout << "####################################" << std::endl;

  if (i_config != "")
  {
    m_configFilePath = i_config;
    std::cout << "runtime configuration file: " << m_configFilePath << std::endl;
    loadConfigDataFromFile(m_configFilePath);
  }

  if (!m_isPrepared)
  {
    prepareForCalculation();
    m_isPrepared = true;
  }

  // BREAKPOINT
  if (m_shouldExit)
  {
    m_isCalculating = false;
    return 0;
  }
  // END BREAKPOINT

  //------------------------------------------//
  //---------------CALCULATION----------------//
  //------------------------------------------//
  std::cout << "entering time loop" << std::endl;
  runCalculation();
  std::cout << "finished time loop" << std::endl;

  // write station data to files
  if (m_useFileIO)
  {
    writeStations();
  }

  std::cout << "finished, exiting" << std::endl;
  m_isCalculating = false;
  return EXIT_SUCCESS;
}

tsunami_lab::Simulator::~Simulator()
{
  freeMemory();
}
