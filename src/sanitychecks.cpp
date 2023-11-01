/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Entry-point for sanity checks.
 **/
#include "io/Csv.h"
#include "patches/WavePropagation1d.h"
#include "setups/MiddleStates.h"
#include "setups/RareRare1d.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

tsunami_lab::t_real calculateHStar(tsunami_lab::t_real hLeft, tsunami_lab::t_real hRight, tsunami_lab::t_real huLeft, tsunami_lab::t_real huRight)
{
  //cell amount in x and y direction
  tsunami_lab::t_idx l_nx = 100;
  tsunami_lab::t_idx l_ny = 1;

  //simulation size
  tsunami_lab::t_real l_dxy = 10.0 / l_nx;

  //amount of calculated steps (more steps -> higher accuracy)
  tsunami_lab::t_idx l_steps = 1000;

  //solver choice
  std::string solver = "fwave";

  // construct setup
  tsunami_lab::setups::Setup *l_setup;
  l_setup = new tsunami_lab::setups::MiddleStates(hLeft, hRight, huLeft, huRight, l_dxy/2);

  // construct solver
  tsunami_lab::patches::WavePropagation *l_waveProp;
  l_waveProp = new tsunami_lab::patches::WavePropagation1d(l_nx, solver);

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
    }
  }

  // derive maximum wave speed in setup; the momentum is ignored
  tsunami_lab::t_real l_speedMax = std::sqrt(9.81 * l_hMax);

  // derive constant time step; changes at simulation time are ignored
  tsunami_lab::t_real l_dt = 0.5 * l_dxy / l_speedMax;

  // derive scaling for a time step
  tsunami_lab::t_real l_scaling = l_dt / l_dxy;

  // iterate given amount of steps
  tsunami_lab::t_idx l_i;
  for(l_i=0;l_i<l_steps;l_i++){
    l_waveProp->setGhostOutflow();
    l_waveProp->timeStep(l_scaling);
  }
  //return height at the middle of the simulation
  return l_waveProp->getHeight()[(int)(l_dxy / 2)];
}

int middleStatesSanityCheck()
{
  //accuracy when comapring given hStar to calculated value
  tsunami_lab::t_real accuracy = 0.002;
  //amount of tests to run
  tsunami_lab::t_real tests = 10000;

  std::ifstream inputFile("resources/middle_states.csv");

  std::cout << "Started middle states sanity check." << std::endl;
  std::vector<std::string> row;
  std::string line;
  tsunami_lab::t_idx executedTests=0;
  while (getline(inputFile, line) && executedTests<tests)
  {
    // ignore lines starting with #
    if (line.substr(0, 1) != "#")
    {
      //extract data from csv line
      row = tsunami_lab::io::Csv::splitLine(std::stringstream(line), ',');
      //calculate hStar
      tsunami_lab::t_real hStar = calculateHStar(std::stof(row[0]), std::stof(row[1]), std::stof(row[2]), std::stof(row[3]));
      //compare calculated and given values
      std::cout << "hStar diff: " << abs(hStar - std::stof(row[4])) << std::endl;
      if (abs(hStar - std::stof(row[4])) > accuracy)
      {
        std::cout << "Middle states sanity check failed." << std::endl;
        return EXIT_FAILURE;
      }
      executedTests++;
    }
  }
  return 0;
}

int main()
{
  if (middleStatesSanityCheck() != 0) return EXIT_FAILURE;
  return 0;
}
