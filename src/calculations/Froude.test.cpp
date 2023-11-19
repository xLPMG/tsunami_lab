/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Test of the Froude number calculation
 **/

#include <catch2/catch.hpp>
#include "Froude.h"
#include "../setups/Setup.h"
#include "../setups/Supercritical1d.h"

TEST_CASE("Test of the Froude number calculator using the Supercritical1d setup", "[Froude], [Supercritical1d]")
{
    /**
     * Test case:
     * i_x = 5
     * -> h = 0.33
     * -> hu = 0.18
     * -> u = 0.18 / 0.33 = 0.5454545454545
     * -> F = 0.5454545454545 / sqrt(0.33 * 9.80665) = 0.3032084201066
     */
    tsunami_lab::setups::Setup *l_setup = new tsunami_lab::setups::Supercritical1d(10, 5);
    tsunami_lab::t_real l_froudeNumber = 0;
    tsunami_lab::calculations::Froude::getFroude(l_setup,
                                                 5,
                                                 l_froudeNumber);
    REQUIRE(l_froudeNumber == Approx(0.3032084201066));
}

TEST_CASE("Test of the maximum Froude number calculator using the Supercritical1d setup", "[MaxFroude], [Supercritical1d]")
{
    /**
     * Test case:
     * i_x in [0, 25]
     * -> for max F, u needs to be maximized and h minimized
     * -> hu is the same for all i_x, hu = 0.18
     * froude(x) = (0.18 / height(x)) / sqrt(height(x) * 9.80665)
     * -> calculate extrema using geogebra (define functions as seen above)
     * -> maximum at (10, 1.2263011593719)
     */
    tsunami_lab::setups::Setup *l_setup = new tsunami_lab::setups::Supercritical1d(10, 5);
    tsunami_lab::t_real l_maxFroudeNumber = 0;
    tsunami_lab::t_real l_maxFroudeNumberPosition = 0;
    tsunami_lab::calculations::Froude::getMaxFroude(l_setup,
                                                    25,
                                                    0.0001,
                                                    l_maxFroudeNumber,
                                                    l_maxFroudeNumberPosition);
    REQUIRE(l_maxFroudeNumber == Approx(1.2263011593719));
    REQUIRE(l_maxFroudeNumberPosition == Approx(10.0).margin(0.001));

    delete l_setup;
}