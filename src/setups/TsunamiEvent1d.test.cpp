/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
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
     * 0,0,250,-2
     * 0,0,500,-3
     * 0,0,750,-25
     * 0,0,1000,-5
     * 0,0,1250,5
     * 
     * Calculate height for x=500:
     * x*=250 < 175000 -> computeD(x) returns 0
     * m_delta = 20 and -bathymetry for x=2 is 3, thus smaller than m_delta
     * therefore we get m_delta as the result, which is 20
     * 
     * Calculate height for x=750:
     * x*=250 < 175000 -> computeD(x) returns 0
     * m_delta = 20 and -bathymetry for x=3 is 25, thus greater than m_delta
     * therefore we get -bathymetry as the result, which is 25
     * 
     */

  //test inside specified bathymetry area
  REQUIRE(l_TsunamiEvent1d.getHeight(500, 0) == tsunami_lab::t_real(20));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(500, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(500, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getHeight(500, 4) == tsunami_lab::t_real(20));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(500, 4) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(500, 4) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getHeight(750, 0) == tsunami_lab::t_real(25));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(750, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(750, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getHeight(750, 4) == tsunami_lab::t_real(25));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(750, 4) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(750, 4) == tsunami_lab::t_real(0));

  //bathymetry > 0
  REQUIRE(l_TsunamiEvent1d.getHeight(1250, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(1250, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(1250, 0) == tsunami_lab::t_real(0));

  //test outside specified bathymetry area
  REQUIRE(l_TsunamiEvent1d.getHeight(10000, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumX(10000, 0) == tsunami_lab::t_real(0));

  REQUIRE(l_TsunamiEvent1d.getMomentumY(100000, 0) == tsunami_lab::t_real(0));

  //test computeD
  REQUIRE(l_TsunamiEvent1d.computeD(800, 0) == Approx(-8.6602));
}