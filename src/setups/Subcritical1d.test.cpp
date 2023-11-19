/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Test for the implementation of Subcritical1d setup
 **/

#include <catch2/catch.hpp>
#include "Subcritical1d.h"

TEST_CASE("Test the one-dimensional Subcritical1d setup.", "[Subcritical1d]")
{
  tsunami_lab::setups::Subcritical1d l_Subcritical1d(0, 0);

  // left side
  REQUIRE(l_Subcritical1d.getHeight(2, 0) == 2);

  REQUIRE(l_Subcritical1d.getMomentumX(2, 0) == tsunami_lab::t_real(4.42));

  REQUIRE(l_Subcritical1d.getMomentumY(2, 0) == 0);

  REQUIRE(l_Subcritical1d.getBathymetry(2, 0) == -2);

  // x element (8,12) ; x=10
  // bathymetry= -1.8-0.05(10-10)^2 = -1.8
  REQUIRE(l_Subcritical1d.getHeight(10, 0) == tsunami_lab::t_real(1.8));

  REQUIRE(l_Subcritical1d.getMomentumX(10, 0) == tsunami_lab::t_real(4.42));

  REQUIRE(l_Subcritical1d.getMomentumY(10, 0) == 0);

  REQUIRE(l_Subcritical1d.getBathymetry(10, 0) == tsunami_lab::t_real(-1.8));

  // right side
  REQUIRE(l_Subcritical1d.getHeight(20, 0) == 2);

  REQUIRE(l_Subcritical1d.getMomentumX(20, 0) == tsunami_lab::t_real(4.42));

  REQUIRE(l_Subcritical1d.getMomentumY(20, 0) == 0);

  REQUIRE(l_Subcritical1d.getBathymetry(20, 0) == -2);
}