/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Test for the implementation of Supercritical1d setup
 **/

#include <catch2/catch.hpp>
#include "Supercritical1d.h"

TEST_CASE("Test the one-dimensional Supercritical1d setup.", "[Supercritical1d]")
{
  tsunami_lab::setups::Supercritical1d l_Supercritical1d(0,0);

  // left side of  
  // 
  REQUIRE(l_Supercritical1d.getHeight(2, 0) == tsunami_lab::t_real(0.33));

  REQUIRE(l_Supercritical1d.getMomentumX(0, 0) == tsunami_lab::t_real(0.18));

  REQUIRE(l_Supercritical1d.getMomentumY(2, 0) == 0);

  REQUIRE(l_Supercritical1d.getBathymetry(2, 0) == tsunami_lab::t_real(-0.33));



  // x element of (8,12); x=10
  //bathymetry= -0.13-0.05(10-10)^2 = -0.13
  REQUIRE(l_Supercritical1d.getHeight(10, 0) == tsunami_lab::t_real(0.13));

  REQUIRE(l_Supercritical1d.getMomentumX(10, 0) == tsunami_lab::t_real(0.18));

  REQUIRE(l_Supercritical1d.getMomentumY(10, 0) == 0);

  REQUIRE(l_Supercritical1d.getBathymetry(10, 0) == tsunami_lab::t_real(-0.13));


  //right side
  REQUIRE(l_Supercritical1d.getHeight(20, 0) == tsunami_lab::t_real(0.33));

  REQUIRE(l_Supercritical1d.getMomentumX(20, 0) == tsunami_lab::t_real(0.18));

  REQUIRE(l_Supercritical1d.getMomentumY(20, 0) == 0);

  REQUIRE(l_Supercritical1d.getBathymetry(20, 0) == tsunami_lab::t_real(-0.33));
}

