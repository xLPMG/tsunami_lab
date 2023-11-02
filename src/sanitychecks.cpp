/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Entry-point for sanity checks.
 **/
#include "io/Csv.h"
#include "patches/WavePropagation1d.h"
#include "setups/MiddleStates.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <cmath>

tsunami_lab::t_real calculateHStar(tsunami_lab::t_real i_hLeft, 
                                   tsunami_lab::t_real i_hRight, 
                                   tsunami_lab::t_real i_huLeft, 
                                   tsunami_lab::t_real i_huRight)
{
  // cell amount in x and y direction
  tsunami_lab::t_idx l_nx = 100;
  tsunami_lab::t_idx l_ny = 1;

  // simulation size
  tsunami_lab::t_real l_dxy = 10.0 / l_nx;

  // amount of calculated steps (more steps -> higher accuracy)
  tsunami_lab::t_idx l_steps = 1000;

  // solver choice
  std::string l_solver = "fwave";

  // construct setup
  tsunami_lab::setups::Setup *l_setup;
  l_setup = new tsunami_lab::setups::MiddleStates(i_hLeft, 
                                                  i_hRight, 
                                                  i_huLeft, 
                                                  i_huRight, 
                                                  l_dxy / 2);

  // construct solver
  tsunami_lab::patches::WavePropagation *l_waveProp;
  l_waveProp = new tsunami_lab::patches::WavePropagation1d(l_nx, l_solver);

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
  for (l_i = 0; l_i < l_steps; l_i++)
  {
    l_waveProp->setGhostOutflow();
    l_waveProp->timeStep(l_scaling);
  }
  // return height at the location of the discontinuity
  return l_waveProp->getHeight()[(int)(l_dxy / 2)];
}

int middleStatesSanityCheck()
{
  // accuracy when comapring given hStar to calculated value
  tsunami_lab::t_real l_accuracy = 0.001;
  // amount of tests to run
  tsunami_lab::t_real l_tests = 10000;

  std::ifstream l_inputFile("resources/middle_states.csv");

  std::cout << "Started middle states sanity check." << std::endl;
  std::vector<std::string> l_row;
  std::string l_line;
  tsunami_lab::t_idx l_executedTests = 0;
  tsunami_lab::t_idx l_passedTests = 0;
  while (getline(l_inputFile, l_line) && l_executedTests < l_tests)
  {
    // ignore lines starting with #
    if (l_line.substr(0, 1) != "#")
    {
      // extract data from csv line
      l_row = tsunami_lab::io::Csv::splitLine(std::stringstream(l_line), ',');

      // calculate hStar
      tsunami_lab::t_real l_hStar = calculateHStar(std::stof(l_row[0]), 
                                                   std::stof(l_row[1]), 
                                                   std::stof(l_row[2]), 
                                                   std::stof(l_row[3]));

      // compare calculated and given values
      std::cout << "hStar diff: " << abs(l_hStar - std::stof(l_row[4])) << std::endl;
      if (abs(l_hStar - std::stof(l_row[4])) > l_accuracy)
      {
        std::cout << "TEST FAILED" << std::endl;
      }
      else
      {
        l_passedTests++;
      }
      l_executedTests++;
    }
  }
  // check if at least 99% of the tests passed
  if (l_passedTests >= 0.99 * l_executedTests)
  {
    std::cout << "MIDDLE STATES TEST PASSED" << std::endl;
    return 0;
  }
  else
  {
    std::cout << "MIDDLE STATES TEST FAILED" << std::endl;
    return EXIT_FAILURE;
  }
}

int main()
{
  if (middleStatesSanityCheck() != 0)
    return EXIT_FAILURE;
  return 0;
}
