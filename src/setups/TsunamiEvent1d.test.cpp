/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Test for the implementation of TsunamiEvent1d
 **/

#include "TsunamiEvent1d.h"
#include <catch2/catch.hpp>

TEST_CASE("Test the one-dimensional TsunamiEvent setup.", "[TsunamiEvent1d]")
{
  tsunami_lab::setups::TsunamiEvent1d l_TsunamiEvent1d("resources/tsunamiEvent1dTest.csv");
    /*
     * Test csv file:
     * 0,0,0,-1
     * 0,0,1,-2
     * 0,0,2,-3
     * 0,0,3,-25
     * 0,0,4,-5
     * 0,0,5,5
     * 
     * Calculate height for x=2:
     * x*=250 < 175000 -> computeD(x) returns 0
     * m_delta = 20 and -bathymetry for x=2 is 3, thus smaller than m_delta
     * therefore we get m_delta as the result, which is 20
     * 
     * Calculate height for x=3:
     * x*=250 < 175000 -> computeD(x) returns 0
     * m_delta = 20 and -bathymetry for x=3 is 25, thus greater than m_delta
     * therefore we get -bathymetry as the result, which is 25
     * 
     */

  //test inside specified bathymetry area
  REQUIRE(l_TsunamiEvent1d.getHeight(2, 0) == tsunami_lab::t_real(20));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(2, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(2, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getHeight(2, 4) == tsunami_lab::t_real(20));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(2, 4) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(2, 4) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getHeight(3, 0) == tsunami_lab::t_real(25));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(3, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(3, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getHeight(3, 4) == tsunami_lab::t_real(25));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(3, 4) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(3, 4) == tsunami_lab::t_real(0));

  //bathymetry > 0
  REQUIRE(l_TsunamiEvent1d.getHeight(5, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(5, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(5, 0) == tsunami_lab::t_real(0));

  //test outside specified bathymetry area
  REQUIRE(l_TsunamiEvent1d.getHeight(10, 0) == tsunami_lab::t_real(8));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(10, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(10, 0) == tsunami_lab::t_real(0));
}