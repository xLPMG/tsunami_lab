/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Entry-point for simulations.
 **/

// wave prop patches
#include "patches/WavePropagation1d.h"
#include "patches/WavePropagation2d.h"

// setups
#include "setups/DamBreak1d.h"
#include "setups/CircularDamBreak2d.h"
#include "setups/RareRare1d.h"
#include "setups/ShockShock1d.h"
#include "setups/Subcritical1d.h"
#include "setups/Supercritical1d.h"
#include "setups/GeneralDiscontinuity1d.h"
#include "setups/TsunamiEvent1d.h"
#include "setups/TsunamiEvent2d.h"
#include "setups/ArtificialTsunami2d.h"

// io
#include "io/Csv.h"
#include "io/BathymetryLoader.h"
#include "io/Station.h"
#include "io/NetCdf.h"

// c libraries
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <fstream>
#include <limits>
#include <filesystem>

// external libraries
#include <nlohmann/json.hpp>
#include <netcdf.h>

using json = nlohmann::json;
using Boundary = tsunami_lab::patches::WavePropagation::Boundary;

bool endsWith(std::string const &str, std::string const &suffix)
{
  if (str.length() < suffix.length())
  {
    return false;
  }
  return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

void setupFolders()
{
  // create solutions folder
  if (!std::filesystem::exists("solutions"))
    std::filesystem::create_directory("solutions");

  // set up stations folder for stations to save their data in
  if (!std::filesystem::exists("stations"))
    std::filesystem::create_directory("stations");

  if (!std::filesystem::exists("checkpoints"))
    std::filesystem::create_directory("checkpoints");
}

int main(int i_argc,
         char *i_argv[])
{
  // config file path
  std::string l_configFilePath = "configs/config.json";
  // output file name
  std::string l_outputFileName = "";
  // load from checkpoint if true
  bool l_loadFromCheckpoint = false;
  // setup choice
  std::string l_setupChoice = "";
  // wave propagation patch
  tsunami_lab::patches::WavePropagation *l_waveProp;
  // number of cells in x- and y-direction
  tsunami_lab::t_idx l_nx = 0;
  tsunami_lab::t_idx l_ny = 0;
  // set simulation size in metres
  tsunami_lab::t_real l_simulationSizeX = 0;
  tsunami_lab::t_real l_simulationSizeY = 0;
  // set offset in metres
  tsunami_lab::t_real l_offsetX = 0;
  tsunami_lab::t_real l_offsetY = 0;
  // set cell size
  tsunami_lab::t_real l_dx = 0;
  tsunami_lab::t_real l_dy = 0;
  // solver default
  std::string l_solver = "";
  // boundary conditions
  Boundary l_boundaryL = Boundary::OUTFLOW;
  Boundary l_boundaryR = Boundary::OUTFLOW;
  Boundary l_boundaryT = Boundary::OUTFLOW;
  Boundary l_boundaryB = Boundary::OUTFLOW;
  // input file paths
  std::string l_bathymetryFilePath = "";
  std::string l_displacementFilePath = "";
  // simulation time limit
  tsunami_lab::t_real l_endTime = 0;
  // keep track of all stations
  std::vector<tsunami_lab::io::Station *> l_stations;
  // frequency at which stations are written
  tsunami_lab::t_real l_stationFrequency = 0;
  // writing frequency in timesteps
  tsunami_lab::t_idx l_writingFrequency = 0;
  // frequency at which checkpoints are written
  tsunami_lab::t_real l_checkpointFrequency = -1;
  // data writer choice
  enum DataWriter
  {
    NETCDF = 0,
    CSV = 1
  };
  DataWriter l_dataWriter = NETCDF;

  // set up time and print control
  tsunami_lab::t_idx l_timeStep = 0;
  tsunami_lab::t_idx l_nOut = 0;
  tsunami_lab::t_real l_simTime = 0;
  tsunami_lab::t_idx l_captureCount = 0;
  tsunami_lab::t_idx l_checkpointCount = 0;

  std::cout << "####################################" << std::endl;
  std::cout << "### Tsunami Lab                  ###" << std::endl;
  std::cout << "###                              ###" << std::endl;
  std::cout << "### https://scalable.uni-jena.de ###" << std::endl;
  std::cout << "####################################" << std::endl;

  if (i_argc == 2)
    l_configFilePath = i_argv[1];
  std::cout << "runtime configuration file: " << l_configFilePath << std::endl;

  // set up folders
  setupFolders();

  // read configuration data from file
  std::ifstream l_configFile(l_configFilePath);
  json l_configData = json::parse(l_configFile);
  l_outputFileName = l_configData.value("outputFileName", "solution");
  std::string l_netCdfOutputPathString = "solutions/" + l_outputFileName + ".nc";
  const char *l_netcdfOutputPath = l_netCdfOutputPathString.c_str();

  // check if checkpoint exists
  std::string l_checkPointFilePathString = "checkpoints/" + l_outputFileName + ".nc";
  const char *l_checkPointFilePath = l_checkPointFilePathString.c_str();
  l_loadFromCheckpoint = (std::filesystem::exists(l_checkPointFilePathString) && std::filesystem::exists(l_netCdfOutputPathString));
  if (l_loadFromCheckpoint)
    std::cout << "Found checkpoint file: " << l_checkPointFilePath << std::endl;

  if (l_loadFromCheckpoint)
    l_setupChoice = "CHECKPOINT";
  else
    l_setupChoice = l_configData.value("setup", "CIRCULARDAMBREAK2D");

  l_solver = l_configData.value("solver", "fwave");
  // read size config
  l_nx = l_configData.value("nx", 1);
  l_ny = l_configData.value("ny", 1);
  l_simulationSizeX = l_configData.value("simulationSizeX", 10);
  l_simulationSizeY = l_configData.value("simulationSizeY", 1);
  l_offsetX = l_configData.value("offsetX", 0);
  l_offsetY = l_configData.value("offsetY", 0);
  l_endTime = l_configData.value("endTime", 20);
  // read boundary config
  std::string l_boundaryStringL = l_configData.value("boundaryL", "outflow");
  if (l_boundaryStringL == "outflow" || l_boundaryStringL == "OUTFLOW")
    l_boundaryL = Boundary::OUTFLOW;
  else if (l_boundaryStringL == "wall" || l_boundaryStringL == "WALL")
    l_boundaryL = Boundary::WALL;

  std::string l_boundaryStringR = l_configData.value("boundaryR", "outflow");
  if (l_boundaryStringR == "outflow" || l_boundaryStringR == "OUTFLOW")
    l_boundaryR = Boundary::OUTFLOW;
  else if (l_boundaryStringR == "wall" || l_boundaryStringR == "WALL")
    l_boundaryR = Boundary::WALL;

  std::string l_boundaryStringT = l_configData.value("boundaryT", "outflow");
  if (l_boundaryStringT == "outflow" || l_boundaryStringT == "OUTFLOW")
    l_boundaryT = Boundary::OUTFLOW;
  else if (l_boundaryStringT == "wall" || l_boundaryStringT == "WALL")
    l_boundaryT = Boundary::WALL;

  std::string l_boundaryStringB = l_configData.value("boundaryB", "outflow");
  if (l_boundaryStringB == "outflow" || l_boundaryStringB == "OUTFLOW")
    l_boundaryB = Boundary::OUTFLOW;
  else if (l_boundaryStringB == "wall" || l_boundaryStringB == "WALL")
    l_boundaryB = Boundary::WALL;

  l_bathymetryFilePath = l_configData.value("bathymetry", "");
  l_displacementFilePath = l_configData.value("displacement", "");

  l_writingFrequency = l_configData.value("writingFrequency", 80);
  l_checkpointFrequency = l_configData.value("checkpointFrequency", -1);

  // read station data
  l_stationFrequency = l_configData.value("stationFrequency", 1);
  std::string l_outputMethod = l_configData.value("outputMethod", "netcdf");
  if (l_outputMethod == "netcdf" || l_outputMethod == "NETCDF")
  {
    l_dataWriter = NETCDF;
  }
  else if (l_outputMethod == "csv" || l_outputMethod == "CSV")
  {
    l_dataWriter = CSV;
  }

  // set up netCdf I/O
  tsunami_lab::io::NetCdf *l_netCdf;
  if (l_setupChoice == "CHECKPOINT")
  {
    l_netCdf = new tsunami_lab::io::NetCdf(l_netcdfOutputPath,
                                           l_checkPointFilePath);
  }
  else
  {
    l_netCdf = new tsunami_lab::io::NetCdf(l_nx,
                                           l_ny,
                                           l_simulationSizeX,
                                           l_simulationSizeY,
                                           l_offsetX,
                                           l_offsetY,
                                           l_netcdfOutputPath,
                                           l_checkPointFilePath);
  }

  // construct setup
  tsunami_lab::setups::Setup *l_setup;
  if (l_setupChoice == "CHECKPOINT")
  {
    l_netCdf->loadCheckpointDimensions(l_checkPointFilePath,
                                       l_nx,
                                       l_ny,
                                       l_simulationSizeX,
                                       l_simulationSizeY,
                                       l_offsetX,
                                       l_offsetY,
                                       l_simTime,
                                       l_timeStep);
    std::cout << std::endl;
    std::cout << "Loaded following data from checkpoint: " << std::endl;
    std::cout << "Cells x:                  " << l_nx << std::endl;
    std::cout << "Cells y:                  " << l_ny << std::endl;
    std::cout << "Simulation size x:        " << l_simulationSizeX << std::endl;
    std::cout << "Simulation size y:        " << l_simulationSizeY << std::endl;
    std::cout << "Offset x:                 " << l_offsetX << std::endl;
    std::cout << "Offset y:                 " << l_offsetY << std::endl;
    std::cout << "Current simulation time:  " << l_simTime << std::endl;
    std::cout << "Current time step:        " << l_timeStep << std::endl;
    std::cout << std::endl;
    l_setup = nullptr;
  }
  else if (l_setupChoice == "GENERALDISCONTINUITY1D")
  {
    l_setup = new tsunami_lab::setups::GeneralDiscontinuity1d(10, 10, 10, -10, l_simulationSizeX / 2);
  }
  else if (l_setupChoice == "DAMBREAK1D")
  {
    l_setup = new tsunami_lab::setups::DamBreak1d(10, 5, l_simulationSizeX / 2);
  }
  else if (l_setupChoice == "CIRCULARDAMBREAK2D")
  {
    l_setup = new tsunami_lab::setups::CircularDamBreak2d();
  }
  else if (l_setupChoice == "RARERARE1D")
  {
    l_setup = new tsunami_lab::setups::RareRare1d(10, 5, l_simulationSizeX / 2);
  }
  else if (l_setupChoice == "SHOCKSHOCK1D")
  {
    l_setup = new tsunami_lab::setups::ShockShock1d(10, 5, l_simulationSizeX / 2);
  }
  else if (l_setupChoice == "SUBCRITICAL1D")
  {
    l_simulationSizeX = 10;
    l_setup = new tsunami_lab::setups::Subcritical1d(0.0001, 5);
  }
  else if (l_setupChoice == "SUPERCRITICAL1D")
  {
    l_simulationSizeX = 10;
    l_setup = new tsunami_lab::setups::Supercritical1d(0.0001, 5);
  }
  else if (l_setupChoice == "TSUNAMIEVENT1D")
  {
    l_setup = new tsunami_lab::setups::TsunamiEvent1d(l_bathymetryFilePath);
  }
  else if (l_setupChoice == "TSUNAMIEVENT2D")
  {
    tsunami_lab::io::NetCdf *l_netCdfTE2D = new tsunami_lab::io::NetCdf(l_nx,
                                                                        l_ny,
                                                                        l_simulationSizeX,
                                                                        l_simulationSizeY,
                                                                        l_offsetX,
                                                                        l_offsetY,
                                                                        l_netcdfOutputPath,
                                                                        l_checkPointFilePath);
    l_setup = new tsunami_lab::setups::TsunamiEvent2d("resources/artificialtsunami_bathymetry_1000.nc",
                                                      "resources/artificialtsunami_displ_1000.nc",
                                                      l_netCdfTE2D,
                                                      l_nx);
  }
  else if (l_setupChoice == "ARTIFICIAL2D")
  {
    l_simulationSizeX = 10000;
    l_simulationSizeY = 10000;
    l_offsetX = -5000;
    l_offsetY = -5000;
    l_setup = new tsunami_lab::setups::ArtificialTsunami2d();
  }
  else if (l_setupChoice == "CHILE")
  {
    l_simulationSizeX = 3500000;
    l_simulationSizeY = 2950000;
    l_offsetX = -2999875;
    l_offsetY = -1449875;

    tsunami_lab::io::NetCdf *l_netCdfChile = new tsunami_lab::io::NetCdf(l_nx,
                                                                         l_ny,
                                                                         l_simulationSizeX,
                                                                         l_simulationSizeY,
                                                                         l_offsetX,
                                                                         l_offsetY,
                                                                         l_netcdfOutputPath,
                                                                         l_checkPointFilePath);
    l_setup = new tsunami_lab::setups::TsunamiEvent2d("resources/chile/chile_gebco20_usgs_250m_bath_fixed.nc",
                                                      "resources/chile/chile_gebco20_usgs_250m_displ_fixed.nc",
                                                      l_netCdfChile,
                                                      l_nx);
  }
  else if (l_setupChoice == "TOHOKU")
  {
    l_simulationSizeX = 2700000;
    l_simulationSizeY = 1500000;
    l_offsetX = -199875;
    l_offsetY = -749875;

    tsunami_lab::io::NetCdf *l_netCdfTohoku = new tsunami_lab::io::NetCdf(l_nx,
                                                                          l_ny,
                                                                          l_simulationSizeX,
                                                                          l_simulationSizeY,
                                                                          l_offsetX,
                                                                          l_offsetY,
                                                                          l_netcdfOutputPath,
                                                                          l_checkPointFilePath);
    l_setup = new tsunami_lab::setups::TsunamiEvent2d("resources/tohoku/tohoku_gebco08_ucsb3_250m_bath.nc",
                                                      "resources/tohoku/tohoku_gebco08_ucsb3_250m_displ.nc",
                                                      l_netCdfTohoku,
                                                      l_nx);
  }
  else
  {
    std::cerr << "ERROR: No valid setup specified. Terminating..." << std::endl;
    exit(EXIT_FAILURE);
  }

  l_dx = l_simulationSizeX / l_nx;
  l_dy = l_simulationSizeY / l_ny;

  // construct solver
  if (l_ny == 1)
  {
    l_waveProp = new tsunami_lab::patches::WavePropagation1d(l_nx,
                                                             l_solver,
                                                             l_boundaryL,
                                                             l_boundaryR);
  }
  else
  {
    l_waveProp = new tsunami_lab::patches::WavePropagation2d(l_nx,
                                                             l_ny,
                                                             l_boundaryL,
                                                             l_boundaryR,
                                                             l_boundaryT,
                                                             l_boundaryB);
  }

  // set up stations
  std::cout << "Setting up stations..." << std::endl;
  std::cout << "Frequency for all stations is " << l_stationFrequency << std::endl;
  if (l_configData.contains("stations"))
  {
    for (json &elem : l_configData["stations"])
    {
      // location in meters
      tsunami_lab::t_real l_x = elem.at("locX");
      tsunami_lab::t_real l_y = elem.at("locY");

      // location cell
      tsunami_lab::t_idx l_cx = (l_x - l_offsetX) / l_dx;
      tsunami_lab::t_idx l_cy = (l_y - l_offsetY) / l_dy;

      l_stations.push_back(new tsunami_lab::io::Station(l_cx,
                                                        l_cy,
                                                        elem.at("name"),
                                                        l_waveProp));
      std::cout << "Added station " << elem.at("name") << " at x: " << l_x << " and y: " << l_y << std::endl;
    }
  }

  // maximum observed height in the setup
  tsunami_lab::t_real l_hMax = std::numeric_limits<tsunami_lab::t_real>::lowest();
  std::cout << "Setting up solver..." << std::endl;
  // set up solver
  if (l_setupChoice == "CHECKPOINT")
  {
    tsunami_lab::t_real *l_hCheck = new tsunami_lab::t_real[l_nx * l_ny];
    tsunami_lab::t_real *l_huCheck = new tsunami_lab::t_real[l_nx * l_ny];
    tsunami_lab::t_real *l_hvCheck = new tsunami_lab::t_real[l_nx * l_ny];
    tsunami_lab::t_real *l_bCheck = new tsunami_lab::t_real[l_nx * l_ny];
    l_netCdf->read(l_checkPointFilePath, "height", &l_hCheck);
    l_netCdf->read(l_checkPointFilePath, "momentumX", &l_huCheck);
    l_netCdf->read(l_checkPointFilePath, "momentumY", &l_hvCheck);
    l_netCdf->read(l_checkPointFilePath, "bathymetry", &l_bCheck);
    for (tsunami_lab::t_idx l_cy = 0; l_cy < l_ny; l_cy++)
    {
      for (tsunami_lab::t_idx l_cx = 0; l_cx < l_nx; l_cx++)
      {
        l_hMax = std::max(l_hCheck[l_cx + l_cy * l_nx], l_hMax);

        l_waveProp->setHeight(l_cx,
                              l_cy,
                              l_hCheck[l_cx + l_cy * l_nx]);

        l_waveProp->setMomentumX(l_cx,
                                 l_cy,
                                 l_huCheck[l_cx + l_cy * l_nx]);

        l_waveProp->setMomentumY(l_cx,
                                 l_cy,
                                 l_hvCheck[l_cx + l_cy * l_nx]);

        l_waveProp->setBathymetry(l_cx,
                                  l_cy,
                                  l_bCheck[l_cx + l_cy * l_nx]);
      }
    }
    delete[] l_hCheck;
    delete[] l_huCheck;
    delete[] l_hvCheck;
    delete[] l_bCheck;
  }
  else
  {
    for (tsunami_lab::t_idx l_cy = 0; l_cy < l_ny; l_cy++)
    {
      tsunami_lab::t_real l_y = l_cy * l_dy + l_offsetY;

      for (tsunami_lab::t_idx l_cx = 0; l_cx < l_nx; l_cx++)
      {
        tsunami_lab::t_real l_x = l_cx * l_dx + l_offsetX;

        // get initial values of the setup
        tsunami_lab::t_real l_h = l_setup->getHeight(l_x,
                                                     l_y);
        l_hMax = std::max(l_h, l_hMax);

        tsunami_lab::t_real l_hu = l_setup->getMomentumX(l_x,
                                                         l_y);
        tsunami_lab::t_real l_hv = l_setup->getMomentumY(l_x,
                                                         l_y);
        tsunami_lab::t_real l_b = l_setup->getBathymetry(l_x,
                                                         l_y);

        // set initial values in wave propagation solver
        l_waveProp->setHeight(l_cx,
                              l_cy,
                              l_h);

        l_waveProp->setMomentumX(l_cx,
                                 l_cy,
                                 l_hu);

        l_waveProp->setMomentumY(l_cx,
                                 l_cy,
                                 l_hv);

        l_waveProp->setBathymetry(l_cx,
                                  l_cy,
                                  l_b);
      }
    }
  }
  std::cout << "Done." << std::endl;

  // load bathymetry from file
  if (l_bathymetryFilePath.length() > 0)
  {
    if (l_bathymetryFilePath.compare(l_bathymetryFilePath.length() - 4, 4, ".csv") == 0)
    {
      std::cout << "Loading bathymetry from csv file: " << l_bathymetryFilePath << std::endl;
      tsunami_lab::io::BathymetryLoader *l_bathymetryLoader = new tsunami_lab::io::BathymetryLoader();
      l_bathymetryLoader->loadBathymetry(l_bathymetryFilePath);
      for (tsunami_lab::t_idx l_cy = 0; l_cy < l_ny; l_cy++)
      {
        tsunami_lab::t_real l_y = l_cy * l_dy;
        for (tsunami_lab::t_idx l_cx = 0; l_cx < l_nx; l_cx++)
        {
          tsunami_lab::t_real l_x = l_cx * l_dx;
          tsunami_lab::t_real l_b = l_bathymetryLoader->getBathymetry(l_x, l_y);
          l_waveProp->setBathymetry(l_cx,
                                    l_cy,
                                    l_b);
        }
      }
      l_waveProp->adjustWaterHeight();
      delete l_bathymetryLoader;
      std::cout << "Done loading bathymetry." << std::endl;
    }
    else if (l_bathymetryFilePath.compare(l_bathymetryFilePath.length() - 3, 3, ".nc") == 0)
    {
      // TODO
      //  std::cout << "Loading bathymetry from .nc file: " << l_bathymetryFilePath << std::endl;
      //  tsunami_lab::t_real *l_b = l_netCdf->read(l_bathymetryFilePath.c_str(),
      //                                            "bathymetry");

      // for (tsunami_lab::t_idx l_cy = 0; l_cy < l_ny; l_cy++)
      // {
      //   for (tsunami_lab::t_idx l_cx = 0; l_cx < l_nx; l_cx++)
      //   {
      //     l_waveProp->setBathymetry(l_cx,
      //                               l_cy,
      //                               l_b[l_cx + l_nx * l_cy]);
      //   }
      // }
      // std::cout << "Done loading bathymetry." << std::endl;
    }
    else
    {
      std::cerr << "Error: Don't know how to read file " << l_bathymetryFilePath << std::endl;
    }
  }

  // derive maximum wave speed in setup; the momentum is ignored
  tsunami_lab::t_real l_speedMax = std::sqrt(9.81 * l_hMax);

  // derive constant time step; changes at simulation time are ignored
  tsunami_lab::t_real l_dt = 0;
  if (l_ny == 1)
  {
    l_dt = 0.5 * l_dx / l_speedMax;
    l_dt *= 0.8;
  }
  else
  {
    l_dt = 0.45 * std::min(l_dx, l_dy) / l_speedMax;
    l_dt *= 0.5;
  }

  // derive scaling for a time step
  tsunami_lab::t_real l_scalingX = l_dt / l_dx;
  tsunami_lab::t_real l_scalingY = l_dt / l_dy;

  std::cout << "Writing every " << l_writingFrequency << " time steps" << std::endl;
  std::cout << "entering time loop" << std::endl;

  // set counts in case we load from a checkpoint file
  if (l_simTime > 0)
  {
    l_captureCount = std::floor(l_simTime / l_stationFrequency);
    l_checkpointCount = std::floor(l_simTime / l_checkpointFrequency);
  }
  // iterate over time
  while (l_simTime < l_endTime)
  {
    if (l_timeStep % l_writingFrequency == 0)
    {
      std::cout << "  simulation time / #time steps: "
                << l_simTime << " / " << l_timeStep << std::endl;

      switch (l_dataWriter)
      {
      case NETCDF:
      {
        std::cout << "  writing to netcdf " << std::endl;
        l_netCdf->write(l_waveProp->getStride(),
                        l_waveProp->getHeight(),
                        l_waveProp->getMomentumX(),
                        l_waveProp->getMomentumY(),
                        l_waveProp->getBathymetry(),
                        l_simTime);
        break;
      }
      case CSV:
      {
        std::string l_csvOutputPath = "solutions/" + l_outputFileName + "_" + std::to_string(l_nOut) + ".csv";
        std::cout << "  writing wave field to " << l_csvOutputPath << std::endl;
        std::ofstream l_file;
        l_file.open(l_csvOutputPath);
        tsunami_lab::io::Csv::write(l_dx,
                                    l_dy,
                                    l_nx,
                                    l_ny,
                                    l_waveProp->getStride(),
                                    l_waveProp->getHeight(),
                                    l_waveProp->getMomentumX(),
                                    l_waveProp->getMomentumY(),
                                    l_waveProp->getBathymetry(),
                                    l_file);
        l_file.close();
        l_nOut++;
        break;
      }
      }
    }
    l_waveProp->setGhostOutflow();
    l_waveProp->timeStep(l_scalingX, l_scalingY);
    // write stations
    if (l_simTime >= l_stationFrequency * l_captureCount)
    {
      for (tsunami_lab::io::Station *l_s : l_stations)
      {
        l_s->capture(l_simTime);
      }
      ++l_captureCount;
    }
    // write checkpoint
    if (l_checkpointFrequency > 0 && l_simTime >= l_checkpointFrequency * l_checkpointCount)
    {
      std::cout << "  saving checkpoint to " << l_checkPointFilePathString << std::endl;
      l_netCdf->writeCheckpoint(l_checkPointFilePath,
                                l_waveProp->getHeight(),
                                l_waveProp->getMomentumX(),
                                l_waveProp->getMomentumY(),
                                l_waveProp->getBathymetry(),
                                l_simTime,
                                l_timeStep);
      ++l_checkpointCount;
    }
    l_timeStep++;
    l_simTime += l_dt;
  }
  for (tsunami_lab::io::Station *l_s : l_stations)
  {
    l_s->write();
  }

  std::cout << "finished time loop" << std::endl;

  // free memory
  std::cout << "freeing memory" << std::endl;
  delete l_setup;
  delete l_waveProp;
  delete l_netCdf;
  for (tsunami_lab::io::Station *l_s : l_stations)
  {
    delete l_s;
  }

  std::cout << "finished, exiting" << std::endl;

  return EXIT_SUCCESS;
}
