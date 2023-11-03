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

int middleStatesSanityCheck()
{

  // START TEST CONDITIONS

  // accuracy when comapring given hStar to calculated value
  tsunami_lab::t_real l_accuracy = 0.001;

  // amount of tests to run
  tsunami_lab::t_real l_tests = 10000;

  // csv file path
  std::ifstream l_inputFile("resources/middle_states.csv");

  // END TEST CONDITIONS
  // START SIMULATION CONDITIONS

  // cell amount in x and y direction
  tsunami_lab::t_idx l_nx = 100;
  tsunami_lab::t_idx l_ny = 1;
  // simulation size
  tsunami_lab::t_real l_dxy = 10.0 / l_nx;
  // solver choice
  std::string l_solver = "fwave";

  // END SIMULATION CONDITIONS

  std::cout << "Started middle states sanity check." << std::endl;
  std::vector<std::string> l_row;
  std::string l_line;
  //counters for executed and passed tests
  tsunami_lab::t_idx l_executedTests = 0;
  tsunami_lab::t_idx l_passedTests = 0;

  while (getline(l_inputFile, l_line) && l_executedTests < l_tests)
  {
    // ignore lines starting with #
    if (l_line.substr(0, 1) != "#")
    {
      // extract data from csv line
      l_row = tsunami_lab::io::Csv::splitLine(std::stringstream(l_line), ',');

      // START CALCULATION

      tsunami_lab::t_real l_hStar = 0;

      // construct setup
      tsunami_lab::setups::Setup *l_setup;
      l_setup = new tsunami_lab::setups::MiddleStates(std::stof(l_row[0]),
                                                      std::stof(l_row[1]),
                                                      std::stof(l_row[2]),
                                                      std::stof(l_row[3]),
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
      tsunami_lab::t_real l_dt = tsunami_lab::t_real(0.5) * l_dxy / l_speedMax;

      // derive scaling for a time step
      tsunami_lab::t_real l_scaling = l_dt / l_dxy;

      // set up time and print control
      tsunami_lab::t_real l_endTime = 1.25;
      tsunami_lab::t_real l_simTime = 0;

      while (l_simTime < l_endTime)
      {
        l_waveProp->setGhostOutflow();
        l_waveProp->timeStep(l_scaling);
        l_simTime += l_dt;
        l_hStar = l_waveProp->getHeight()[(int)(l_dxy / 2)];
        //end early if we have reached the target
        if (abs(l_hStar - std::stof(l_row[4])) <= 0.001)
          break;
      }

      // END CALCULATION

      // compare calculated and given values
      if (abs(l_hStar - std::stof(l_row[4])) > l_accuracy)
      {
        std::cout << "TEST #" << l_executedTests << " FAILED! Missed target by " << abs(l_hStar - std::stof(l_row[4])) - l_accuracy << std::endl;
      }
      else
      {
        l_passedTests++;
      }
      l_executedTests++;
    }
  }
  // test evaluation
  std::cout << l_passedTests << " / " << l_executedTests << " PASSED" << std::endl;
  std::cout << "Accuracy: " << l_accuracy << std::endl;
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
