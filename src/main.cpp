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
  // set up stations folder for stations to save their data in
  if (std::filesystem::exists("stations"))
    std::filesystem::remove_all("stations");

  std::filesystem::create_directory("stations");

  // clean solutions folder
  if (std::filesystem::exists("solutions"))
    std::filesystem::remove_all("solutions");

  std::filesystem::create_directory("solutions");
}

int main(int i_argc,
         char *i_argv[])
{
  // config file path
  std::string l_configFilePath = "config.json";
  // setup choice
  std::string l_setupChoice = "";
  // number of cells in x- and y-direction
  tsunami_lab::t_idx l_nx = 0;
  tsunami_lab::t_idx l_ny = 1;
  // set simulation size in metres
  tsunami_lab::t_real l_simulationSizeX = 10.0;
  tsunami_lab::t_real l_simulationSizeY = 10.0;
  // set offset in metres
  tsunami_lab::t_real l_offsetX = 0;
  tsunami_lab::t_real l_offsetY = 0;
  // set cell size
  tsunami_lab::t_real l_dx = 1;
  tsunami_lab::t_real l_dy = 1;
  // solver default
  std::string l_solver = "";
  bool l_hasBoundaryL = false;
  bool l_hasBoundaryR = false;
  bool l_hasBoundaryT = false;
  bool l_hasBoundaryB = false;
  // input file paths
  std::string l_bathymetryFilePath = "";
  std::string l_displacementFilePath = "";
  // simulation time limit
  tsunami_lab::t_real l_endTime = 20;
  // keep track of all stations
  std::vector<tsunami_lab::io::Station *> l_stations;
  tsunami_lab::t_real l_stationFrequency = 1;
  // data writer choice
  enum DataWriter
  {
    NETCDF = 0,
    CSV = 1
  };
  DataWriter l_dataWriter = NETCDF;

  std::cout << "####################################" << std::endl;
  std::cout << "### Tsunami Lab                  ###" << std::endl;
  std::cout << "###                              ###" << std::endl;
  std::cout << "### https://scalable.uni-jena.de ###" << std::endl;
  std::cout << "####################################" << std::endl;

  if (i_argc == 2)
    l_configFilePath = i_argv[1];
  std::cout << "runtime configuration file: " << l_configFilePath << std::endl;

  // read configuration data from file
  std::ifstream l_configFile(l_configFilePath);
  json l_configData = json::parse(l_configFile);

  l_setupChoice = l_configData.value("setup", "CIRCULARDAMBREAK2D");
  l_solver = l_configData.value("solver", "fwave");
  l_nx = l_configData.value("nx", 1);
  l_ny = l_configData.value("ny", 1);
  l_simulationSizeX = l_configData.value("simulationSizeX", 1);
  l_simulationSizeY = l_configData.value("simulationSizeY", 1);
  l_offsetX = l_configData.value("offsetX", 0);
  l_offsetY = l_configData.value("offsetY", 0);
  l_hasBoundaryL = l_configData.value("hasBoundaryL", false);
  l_hasBoundaryR = l_configData.value("hasBoundaryR", false);
  l_hasBoundaryT = l_configData.value("hasBoundaryT", false);
  l_hasBoundaryB = l_configData.value("hasBoundaryB", false);
  l_bathymetryFilePath = l_configData.value("bathymetry", "");
  l_displacementFilePath = l_configData.value("displacement", "");
  l_endTime = l_configData.value("endTime", 20);
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

  // construct setup
  /**
   * note: switch statement was not feasible because the string from the json
   * would have needed to be converted into an enum using if-statements anyway
   **/
  tsunami_lab::setups::Setup *l_setup;
  if (l_setupChoice == "GENERALDISCONTINUITY1D")
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
                                                                        l_offsetY);
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
  else
  {
    std::cerr << "ERROR: No valid setup specified. Terminating..." << std::endl;
    exit(EXIT_FAILURE);
  }

  l_dx = l_simulationSizeX / l_nx;
  l_dy = l_simulationSizeY / l_ny;

  // construct solver
  tsunami_lab::patches::WavePropagation *l_waveProp;
  if (l_ny == 1)
  {
    l_waveProp = new tsunami_lab::patches::WavePropagation1d(l_nx,
                                                             l_solver,
                                                             l_hasBoundaryL,
                                                             l_hasBoundaryR);
  }
  else
  {
    l_waveProp = new tsunami_lab::patches::WavePropagation2d(l_nx,
                                                             l_ny,
                                                             l_hasBoundaryL,
                                                             l_hasBoundaryR,
                                                             l_hasBoundaryT,
                                                             l_hasBoundaryB);
  }

  // set up stations
  std::cout << "Setting up stations..." << std::endl;
  std::cout << "Frequency for all stations is " << l_stationFrequency << std::endl;
  if (l_configData.contains("stations"))
  {
    for (json &elem : l_configData["stations"])
    {
      tsunami_lab::t_real l_x = elem.at("locX");
      tsunami_lab::t_real l_y = elem.at("locY");
      l_stations.push_back(new tsunami_lab::io::Station(l_x,
                                                        l_y,
                                                        elem.at("name"),
                                                        l_waveProp));
      std::cout << "Added station " << elem.at("name") << " at x: " << l_x << " and y: " << l_y << std::endl;
    }
  }

  // set up folders
  setupFolders();

  // maximum observed height in the setup
  tsunami_lab::t_real l_hMax = std::numeric_limits<tsunami_lab::t_real>::lowest();

  // set up solver
  for (tsunami_lab::t_idx l_cy = 0; l_cy < l_ny; l_cy++)
  {
    tsunami_lab::t_real l_y = (l_cy + 0.5) * l_dy + l_offsetX;

    for (tsunami_lab::t_idx l_cx = 0; l_cx < l_nx; l_cx++)
    {
      tsunami_lab::t_real l_x = (l_cx + 0.5) * l_dx + l_offsetY;

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
  // set up netCdf I/O
  tsunami_lab::io::NetCdf *l_netCdf = new tsunami_lab::io::NetCdf(l_nx,
                                                                  l_ny,
                                                                  l_simulationSizeX,
                                                                  l_simulationSizeY,
                                                                  l_offsetX,
                                                                  l_offsetY);

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

  // set up time and print control
  tsunami_lab::t_idx l_timeStep = 0;
  tsunami_lab::t_idx l_nOut = 0;
  tsunami_lab::t_real l_simTime = 0;
  tsunami_lab::t_idx l_captureCount = 0;
  tsunami_lab::t_idx l_writingFrequency = (l_endTime * 0.04) + 50;
  l_writingFrequency = (l_endTime/4) < l_writingFrequency ? (l_endTime/4) : l_writingFrequency;
  std::cout << "Writing every " << l_writingFrequency << " time steps" << std::endl;
  std::cout << "entering time loop" << std::endl;

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
        l_netCdf->write("solutions/solution.nc",
                        l_waveProp->getStride(),
                        l_waveProp->getHeight(),
                        l_waveProp->getMomentumX(),
                        l_waveProp->getMomentumY(),
                        l_waveProp->getBathymetry(),
                        l_simTime);
        break;
      }
      case CSV:
      {
        std::string l_path = "solutions/solution_" + std::to_string(l_nOut) + ".csv";
        std::cout << "  writing wave field to " << l_path << std::endl;
        std::ofstream l_file;
        l_file.open(l_path);
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
    if (l_simTime >= l_stationFrequency * l_captureCount)
    {
      for (tsunami_lab::io::Station *l_s : l_stations)
      {
        l_s->capture(l_simTime);
      }
      ++l_captureCount;
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
