/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Entry-point for simulations.
 **/

#include <nlohmann/json.hpp>
#include "patches/WavePropagation1d.h"
#include "patches/WavePropagation2d.h"
#include "setups/DamBreak1d.h"
#include "setups/CircularDamBreak2d.h"
#include "setups/RareRare1d.h"
#include "setups/ShockShock1d.h"
#include "setups/Subcritical1d.h"
#include "setups/Supercritical1d.h"
#include "setups/GeneralDiscontinuity1d.h"
#include "setups/TsunamiEvent1d.h"
#include "io/Csv.h"
#include "io/Station.h"
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <fstream>
#include <limits>
#include <filesystem>

using json = nlohmann::json;

int main(int   i_argc,
         char *i_argv[])
{
  // config file path
  std::string l_configFilePath = "config.json";

  // setup choice
  std::string l_setupChoice = "CIRCULARDAMBREAK2D";

  // number of cells in x- and y-direction
  tsunami_lab::t_idx l_nx = 0;
  tsunami_lab::t_idx l_ny = 1;

  // set simulation size in metres
  tsunami_lab::t_real l_simulationSizeX = 10.0;
  tsunami_lab::t_real l_simulationSizeY = 10.0;

  // set cell size
  tsunami_lab::t_real l_dx = 1;
  tsunami_lab::t_real l_dy = 1;

  // solver default
  std::string l_solver = "fwave";
  bool l_hasBoundaryL = false;
  bool l_hasBoundaryR = false;
  bool l_hasBoundaryU = false;
  bool l_hasBoundaryD = false;

  // dimension choice
  int l_dimension = 2;

  //bathymetry file path
  std::string l_bathymetryFilePath = "";

  // simulation time limit
  tsunami_lab::t_real l_endTime = 20;

  std::cout << "####################################" << std::endl;
  std::cout << "### Tsunami Lab                  ###" << std::endl;
  std::cout << "###                              ###" << std::endl;
  std::cout << "### https://scalable.uni-jena.de ###" << std::endl;
  std::cout << "####################################" << std::endl;

  if( i_argc == 2 ) l_configFilePath = i_argv[1];
  std::cout << "runtime configuration file: " << l_configFilePath << std::endl;

  // read configuration data from file
  std::ifstream l_configFile(l_configFilePath);
  json l_configData = json::parse(l_configFile);

  if (l_configData.contains("setup"))
    l_setupChoice = l_configData["setup"];
  if (l_configData.contains("solver"))
    l_solver = l_configData["solver"];
  if (l_configData.contains("nx"))
    l_nx = l_configData["nx"];
  if (l_configData.contains("ny"))
    l_ny = l_configData["ny"];
  if (l_configData.contains("simulationSizeX"))
    l_simulationSizeX = l_configData["simulationSizeX"];
  if (l_configData.contains("simulationSizeY"))
    l_simulationSizeY = l_configData["simulationSizeY"];
  if (l_configData.contains("hasBoundaryL"))
    l_hasBoundaryL = l_configData["hasBoundaryL"];
  if (l_configData.contains("hasBoundaryR"))
    l_hasBoundaryR = l_configData["hasBoundaryR"];
  if (l_configData.contains("hasBoundaryU"))
    l_hasBoundaryU = l_configData["hasBoundaryU"];
  if (l_configData.contains("hasBoundaryD"))
    l_hasBoundaryD = l_configData["hasBoundaryD"];
  if (l_configData.contains("dimension"))
    l_dimension = l_configData["dimension"];
  if (l_configData.contains("bathymetry"))
    l_bathymetryFilePath = l_configData["bathymetry"];
  if (l_configData.contains("endTime"))
    l_endTime = l_configData["endTime"];

  l_dx = l_simulationSizeX / l_nx;
  l_dy = l_simulationSizeY / l_ny;

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
    l_setup = new tsunami_lab::setups::DamBreak1d(50, 10, l_simulationSizeX / 2);
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
    l_setup = new tsunami_lab::setups::Subcritical1d(10, 5);
  }
  else if (l_setupChoice == "SUPERCRITICAL1D")
  {
    l_setup = new tsunami_lab::setups::Supercritical1d(10, 5);
  }
  else if (l_setupChoice == "TSUNAMIEVENT1D")
  {
    l_setup = new tsunami_lab::setups::TsunamiEvent1d(l_bathymetryFilePath);
  }
  else
  {
    std::cout << "ERROR: No valid setup specified. Terminating..." << std::endl;
    exit(EXIT_FAILURE);
  }

  // construct solver
  tsunami_lab::patches::WavePropagation *l_waveProp;
  if (l_dimension == 1)
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
                                                             l_solver,
                                                             l_hasBoundaryL,
                                                             l_hasBoundaryR,
                                                             l_hasBoundaryU,
                                                             l_hasBoundaryD);
  }

  // maximum observed height in the setup
  tsunami_lab::t_real l_hMax = std::numeric_limits<tsunami_lab::t_real>::lowest();

  // set up solver
  for (tsunami_lab::t_idx l_cy = 0; l_cy < l_ny; l_cy++)
  {
    tsunami_lab::t_real l_y = l_cy * l_dy;

    for (tsunami_lab::t_idx l_cx = 0; l_cx < l_nx; l_cx++)
    {
      tsunami_lab::t_real l_x = l_cx * l_dx;

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

  l_waveProp->setBathymetry(17, 15, 30);
  l_waveProp->setBathymetry(18, 15, 30);
  l_waveProp->setBathymetry(19, 15, 30);
  l_waveProp->setBathymetry(20, 15, 30);
  l_waveProp->setBathymetry(21, 15, 30);
  l_waveProp->setBathymetry(22, 15, 30);
  l_waveProp->setBathymetry(23, 15, 30);

  l_waveProp->setBathymetry(27, 15, 30);
  l_waveProp->setBathymetry(28, 15, 30);
  l_waveProp->setBathymetry(29, 15, 30);
  l_waveProp->setBathymetry(30, 15, 30);
  l_waveProp->setBathymetry(31, 15, 30);
  l_waveProp->setBathymetry(32, 15, 30);
  l_waveProp->setBathymetry(33, 15, 30);

  l_waveProp->adjustWaterHeight();

  // instantiation stations
  tsunami_lab::io::Station *station1 = new tsunami_lab::io::Station(0, 0, "dawd", 0, 0);
  station1->setHeight(14);
  station1->setBathymetry(67);
  tsunami_lab::t_idx count = 0;
  std::string l_station_path = "station/station_" + std::to_string(count) + ".csv";
  std::ofstream l_station_file;
  l_station_file.open(l_station_path);
  l_station_file << station1->getBathymetry() << " height: " << station1->getHeight();

  l_station_file.close();

  // derive maximum wave speed in setup; the momentum is ignored
  tsunami_lab::t_real l_speedMax = std::sqrt(9.81 * l_hMax);

  // derive constant time step; changes at simulation time are ignored
  tsunami_lab::t_real l_dt = 0.45 * std::min(l_dx, l_dy) / l_speedMax;
  l_dt *= 0.3;

  // derive scaling for a time step
  tsunami_lab::t_real l_scalingX = l_dt / l_dx;
  tsunami_lab::t_real l_scalingY = l_dt / l_dy;

  // set up time and print control
  tsunami_lab::t_idx l_timeStep = 0;
  tsunami_lab::t_idx l_nOut = 0;
  tsunami_lab::t_real l_simTime = 0;

  // clean solutions folder
  if (std::filesystem::exists("solutions"))
    std::filesystem::remove_all("solutions");

  std::filesystem::create_directory("solutions");

  std::cout << "entering time loop" << std::endl;

  // iterate over time
  while (l_simTime < l_endTime)
  {
    if (l_timeStep % 25 == 0)
    {
      std::cout << "  simulation time / #time steps: "
                << l_simTime << " / " << l_timeStep << std::endl;

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
    }

    l_waveProp->setGhostOutflow();
    l_waveProp->timeStep(l_scalingX, l_scalingY);

    l_timeStep++;
    l_simTime += l_dt;
  }

  std::cout << "finished time loop" << std::endl;

  // free memory
  std::cout << "freeing memory" << std::endl;
  delete l_setup;
  delete l_waveProp;

  std::cout << "finished, exiting" << std::endl;

  return EXIT_SUCCESS;
}
