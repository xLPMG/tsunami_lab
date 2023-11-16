/**
 * @author Luca Grumbach, Ricahrd Hofamnn
 *
 * @section DESCRIPTION
 * Tests the 2d dam break setup.
 **/
#include <catch2/catch.hpp>
#include "CircularDamBreak2d.h"

TEST_CASE("Test the two-dimensional dam break setup.", "[DamBreak2d]")
{
  tsunami_lab::setups::CircularDamBreak2d l_damBreak;

  // sqrt(x^2 +y^2) < 10
  REQUIRE(l_damBreak.getHeight(45, 45) == 10);

  REQUIRE(l_damBreak.getMomentumX(45, 45) == 0);

  REQUIRE(l_damBreak.getMomentumY(45, 45) == 0);

  REQUIRE(l_damBreak.getHeight(50, 48) == 10);

  REQUIRE(l_damBreak.getMomentumX(50, 48) == 0);

  REQUIRE(l_damBreak.getMomentumY(50, 48) == 0);

  // sqrt(x^2 +y^2) > 10
  REQUIRE(l_damBreak.getHeight(10, 5) == 5);

  REQUIRE(l_damBreak.getMomentumX(10, 5) == 0);

  REQUIRE(l_damBreak.getMomentumY(10, 5) == 0);

  REQUIRE(l_damBreak.getHeight(15, 15) == 5);

  REQUIRE(l_damBreak.getMomentumX(15, 15) == 0);

  REQUIRE(l_damBreak.getMomentumY(15, 15) == 0);
}