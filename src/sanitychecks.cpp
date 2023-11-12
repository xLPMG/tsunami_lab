/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Entry-point for sanity checks.
 **/

#include "constants.h"
#include "io/Csv.h"
#include "patches/WavePropagation1d.h"
#include "setups/GeneralDiscontinuity1d.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <limits>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#undef CATCH_CONFIG_RUNNER

int main(int i_argc,
         char *i_argv[])
{
  std::cout << "Testing middle states.." << std::endl;
  int l_result = Catch::Session().run(i_argc, i_argv);

  return (l_result < 0xff ? l_result : 0xff);
}

TEST_CASE("Middle states sanity check using middle_states.csv", "[MiddleStates]")
{

  // START TEST CONDITIONS

  // allowed margin of error when comapring given hStar to calculated value
  tsunami_lab::t_real l_accuracy = 0.00489;

  // amount of tests to run
  tsunami_lab::t_real l_tests = 500;

  // csv file path
  std::ifstream l_inputFile("resources/middle_states.csv");

  // END TEST CONDITIONS
  // START SIMULATION CONDITIONS

  // cell count
  tsunami_lab::t_idx l_nx = 10;
  tsunami_lab::t_idx l_ny = 1;

  // simulation size in metres
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
      tsunami_lab::io::Csv::splitLine(std::stringstream(l_line), ',', l_row);

      // START CALCULATION

      tsunami_lab::t_real l_hStar = std::stof(l_row[4]);
      tsunami_lab::t_real l_hStarApproximation = 0;

      // construct setup
      tsunami_lab::setups::Setup *l_setup;
      l_setup = new tsunami_lab::setups::GeneralDiscontinuity1d(std::stof(l_row[0]),
                                                                std::stof(l_row[1]),
                                                                std::stof(l_row[2]),
                                                                std::stof(l_row[3]),
                                                                l_xdis);
      // construct solver
      tsunami_lab::patches::WavePropagation *l_waveProp;
      l_waveProp = new tsunami_lab::patches::WavePropagation1d(l_nx, 
                                                               l_solver,
                                                               false,
                                                               false);

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
      tsunami_lab::t_real l_steps = 0;

      while (l_simTime < l_endTime)
      {
        l_waveProp->setGhostOutflow();
        l_waveProp->timeStep(l_scaling);
        l_simTime += l_dt;
        l_steps++;
      }
      // retrieve hStar as the height of the cell at the discontinuity location
      l_hStarApproximation = l_waveProp->getHeight()[tsunami_lab::t_idx(l_xdis / l_dxy)];

      // END CALCULATION

      // compare calculated and given values
      if (abs(l_hStarApproximation - l_hStar) <= l_accuracy)
      {
        ++l_passedTests;
      }
      else
      {
        std::cout << "TEST #" << l_executedTests << " (" << l_steps << " steps) FAILED! Missed target by " << abs(l_hStarApproximation - l_hStar) << std::endl;
      }
      ++l_executedTests;
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
  REQUIRE(l_passedTests / static_cast<double>(l_executedTests) >= 0.99);
}
