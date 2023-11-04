/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Entry-point for sanity checks.
 **/
#include "io/Csv.h"
#include "patches/WavePropagation1d.h"
#include "setups/GeneralDiscontinuity.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <cmath>

int middleStatesSanityCheck()
{

  // START TEST CONDITIONS

  // allowed margin of error when comapring given hStar to calculated value
  tsunami_lab::t_real l_accuracy = 0.005;

  // amount of tests to run
  tsunami_lab::t_real l_tests = 1000;

  // csv file path
  std::ifstream l_inputFile("resources/middle_states.csv");

  // END TEST CONDITIONS
  // START SIMULATION CONDITIONS

  // cell count
  tsunami_lab::t_idx l_nx = 100;
  tsunami_lab::t_idx l_ny = 1;

  // simulation size
  tsunami_lab::t_real l_size = 10;

  // cell size
  tsunami_lab::t_real l_dxy = l_size / l_nx;

  // solver choice
  std::string l_solver = "fwave";

  // discontinuity location x
  tsunami_lab::t_real l_xdis = l_size / 2;

  // END SIMULATION CONDITIONS

  std::cout << "Started middle states sanity check." << std::endl;
  std::vector<std::string> l_row;
  std::string l_line;
  // counters for executed and passed tests
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
      l_setup = new tsunami_lab::setups::GeneralDiscontinuity(std::stof(l_row[0]),
                                                              std::stof(l_row[1]),
                                                              std::stof(l_row[2]),
                                                              std::stof(l_row[3]),
                                                              l_xdis);
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
      tsunami_lab::t_real l_endTime = 1.5;
      tsunami_lab::t_real l_simTime = 0;
      tsunami_lab::t_real l_steps = 0;

      while (l_simTime < l_endTime)
      {
        l_waveProp->setGhostOutflow();
        l_waveProp->timeStep(l_scaling);
        l_simTime += l_dt;
        l_steps++;
      }
      l_hStar = l_waveProp->getHeightAt(l_nx / 2);
      // END CALCULATION

      // compare calculated and given values
      if (abs(l_hStar - std::stof(l_row[4])) > l_accuracy)
      {
        std::cout << "#" << l_executedTests << " (" << l_steps << " steps) FAILED! Missed target by " << abs(l_hStar - std::stof(l_row[4])) - l_accuracy << std::endl;
      }
      else
      {
        l_passedTests++;
      }
      l_executedTests++;
      delete l_setup;
      delete l_waveProp;
    }
  }
  // test evaluation
  tsunami_lab::t_real ratio = tsunami_lab::t_real(l_passedTests) / tsunami_lab::t_real(l_executedTests);
  std::cout << l_passedTests << " / " << l_executedTests << " PASSED (" << ratio * 100 << "%)" << std::endl;
  std::cout << "Solver: " << l_solver << std::endl;
  std::cout << "Accuracy: " << l_accuracy << std::endl;
  std::cout << "Cells x y: " << l_nx << " " << l_ny << std::endl;
  // check if at least 99% of the tests passed
  if (l_passedTests >= 0.99 * l_executedTests)
  {
    std::cout << "MIDDLE STATES SANITY CHECK PASSED (99% PASSED TESTS REQUIRED)" << std::endl;
    return 0;
  }
  else
  {
    std::cout << "MIDDLE STATES SANITY CHECK FAILED (99% PASSED TESTS REQUIRED)" << std::endl;
    return EXIT_FAILURE;
  }
}

int main()
{
  if (middleStatesSanityCheck() != 0)
    return EXIT_FAILURE;
  return 0;
}
