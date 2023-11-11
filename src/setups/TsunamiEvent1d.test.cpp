// /**
//  * @author Luca-Philipp Grumbach & Richard Hofmann
//  *
//  * @section DESCRIPTION
//  * Test for the implementation of TsunamiEvent1d
//  **/

// #include "TsunamiEvent1d.h"
// #include <catch2/catch.hpp>

// TEST_CASE("Test the one-dimensional TsunamiEvent setup.", "[TsunamiEvent1d]")
// {
//   tsunami_lab::setups::TsunamiEvent1d l_TsunamiEvent1d("../../resources/dem.csv");

//   // left side
//   REQUIRE(l_ShockShock.getHeight(2, 0) == 20);

//   REQUIRE(l_ShockShock.getMomentumX(2, 0) == 10);

//   REQUIRE(l_ShockShock.getMomentumY(2, 0) == 0);

//   REQUIRE(l_ShockShock.getHeight(2, 5) == 20);

//   REQUIRE(l_ShockShock.getMomentumX(2, 5) == 10);

//   REQUIRE(l_ShockShock.getMomentumY(2, 2) == 0);
// }