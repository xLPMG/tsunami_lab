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
#include "setups/RareRare1d.h"
#include "setups/ShockShock1d.h"
#include "setups/Subcritical1d.h"
#include "setups/Supercritical1d.h"
#include "setups/GeneralDiscontinuity1d.h"
#include "setups/GeneralDiscontinuity2d.h"
#include "setups/TsunamiEvent1d.h"
#include "io/Csv.h"
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <fstream>
#include <limits>
#include <filesystem>

using json = nlohmann::json;

int main()
{
  // number of cells in x- and y-direction
  tsunami_lab::t_idx l_nx = 0;
  tsunami_lab::t_idx l_ny = 1;

  // set simulation size in metres
  tsunami_lab::t_real l_simulationSizeX = 10.0;
    tsunami_lab::t_real l_simulationSizeY = 10.0;

  // set cell size
  tsunami_lab::t_real l_dxy = 1;

  // solver default
  std::string l_solver = "fwave";
  bool l_hasBoundaryL = false;
  bool l_hasBoundaryR = false;

  //dimension choice
  int l_dimension = 1;

  std::cout << "####################################" << std::endl;
  std::cout << "### Tsunami Lab                  ###" << std::endl;
  std::cout << "###                              ###" << std::endl;
  std::cout << "### https://scalable.uni-jena.de ###" << std::endl;
  std::cout << "####################################" << std::endl;

  // read configuration data from file
  std::ifstream l_configFile("config.json");
  json l_configData = json::parse(l_configFile);

  if (l_configData.contains("solver"))
    l_solver = l_configData["solver"];
  if (l_configData.contains("nx"))
    l_nx = l_configData["nx"];
  if (l_configData.contains("ny"))
    l_ny = l_configData["ny"];
  if (l_configData.contains("l_simulationSizeX"))
    l_simulationSizeX = l_configData["l_simulationSizeX"];
  if (l_configData.contains("hasBoundaryL"))
    l_hasBoundaryL = l_configData["hasBoundaryL"];
  if (l_configData.contains("hasBoundaryR"))
    l_hasBoundaryR = l_configData["hasBoundaryR"];
  if (l_configData.contains("dimension"))
    l_dimension = l_configData["dimension"];

  l_dxy = l_simulationSizeX / l_nx;

  std::cout << "runtime configuration" << std::endl;
  std::cout << "  number of cells in x-direction: " << l_nx << std::endl;
  std::cout << "  number of cells in y-direction: " << l_ny << std::endl;
  std::cout << "  simulation size in x-direction: " << l_simulationSizeX << std::endl;
  std::cout << "  cell size:                      " << l_dxy << std::endl;
  std::cout << "  selected solver:                " << l_solver << std::endl;
  std::cout << "  has boundary <left> <right>?:   " << l_hasBoundaryL << " " << l_hasBoundaryR << std::endl;
  // construct setup
  tsunami_lab::setups::Setup *l_setup;
  l_setup = new tsunami_lab::setups::GeneralDiscontinuity2d(5,10,0,0,0,0,5,5);
  // construct solver
  tsunami_lab::patches::WavePropagation *l_waveProp;
  l_waveProp = new tsunami_lab::patches::WavePropagation2d(l_nx,
                                                           l_ny,
                                                           l_solver,
                                                           l_hasBoundaryL,
                                                           l_hasBoundaryR,
                                                           l_hasBoundaryL,
                                                           l_hasBoundaryR);

  // maximum observed height in the setup
  tsunami_lab::t_real l_hMax = std::numeric_limits<tsunami_lab::t_real>::lowest();

  // set up solver
  for (tsunami_lab::t_idx l_cy = 0; l_cy < l_ny; l_cy++)
  {
    tsunami_lab::t_real l_y = l_cy * l_dxy;

    for (tsunami_lab::t_idx l_cx = 0; l_cx < l_nx; l_cx++)
    {
      tsunami_lab::t_real l_x = l_cx * l_dxy;

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

  // l_waveProp->adjustWaterHeight();

  // derive maximum wave speed in setup; the momentum is ignored
  tsunami_lab::t_real l_speedMax = std::sqrt(9.81 * l_hMax);

  // derive constant time step; changes at simulation time are ignored
  tsunami_lab::t_real l_dt = 0.5 * l_dxy / l_speedMax;

  // derive scaling for a time step
  tsunami_lab::t_real l_scaling = l_dt / l_dxy;

  // set up time and print control
  tsunami_lab::t_idx l_timeStep = 0;
  tsunami_lab::t_idx l_nOut = 0;
  tsunami_lab::t_real l_endTime = 20;
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
      tsunami_lab::io::Csv::write(l_dxy,
                                  l_nx,
                                  1,
                                  1,
                                  l_waveProp->getHeight(),
                                  l_waveProp->getMomentumX(),
                                  l_waveProp->getMomentumY(),
                                  l_waveProp->getBathymetry(),
                                  l_file);
      l_file.close();
      l_nOut++;
    }

    l_waveProp->setGhostOutflow();
    l_waveProp->timeStep(l_scaling);

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
