/**
 * @author Luca Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Tests the discontinuity setup setup.
 **/
#include <catch2/catch.hpp>
#include "GeneralDiscontinuity1d.h"

TEST_CASE("Test the one-dimensional discontinuity setup.", "[Discontinuity1d]")
{
    tsunami_lab::setups::GeneralDiscontinuity1d l_discontinuity(25,
                                                                55,
                                                                10,
                                                                5,
                                                                3);

    // left side
    REQUIRE(l_discontinuity.getHeight(2, 0) == 25);

    REQUIRE(l_discontinuity.getMomentumX(2, 0) == 10);

    REQUIRE(l_discontinuity.getMomentumY(2, 0) == 0);

    REQUIRE(l_discontinuity.getHeight(2, 5) == 25);

    REQUIRE(l_discontinuity.getMomentumX(2, 5) == 10);

    REQUIRE(l_discontinuity.getMomentumY(2, 2) == 0);

    // right side
    REQUIRE(l_discontinuity.getHeight(4, 0) == 55);

    REQUIRE(l_discontinuity.getMomentumX(4, 0) == 5);

    REQUIRE(l_discontinuity.getMomentumY(4, 0) == 0);

    REQUIRE(l_discontinuity.getHeight(4, 5) == 55);

    REQUIRE(l_discontinuity.getMomentumX(4, 5) == 5);

    REQUIRE(l_discontinuity.getMomentumY(4, 2) == 0);
}