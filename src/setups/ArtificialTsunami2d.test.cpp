/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Test for the implementation of 2 artificial tsunami event
 **/

#include <catch2/catch.hpp>
#include "ArtificialTsunami2d.h"
#include <iostream>

TEST_CASE("Test the two-dimensional Artificial Tsunami event.", "[ArtificialTsunami2d]")
{
  tsunami_lab::setups::ArtificialTsunami2d l_ArtificialTsunami2d;

  tsunami_lab::t_real margin = 0.001;

    /** x=y=0 
     *  f(0)= sin((0/500+1)* pi) = 0
     *  g(0) = -(y/500)^2 +1 = 1
     *  d(0)(0) = 5*1*0 = 0
     */

    REQUIRE_THAT(l_ArtificialTsunami2d.computeF(0), Catch::Matchers::WithinAbs(0, margin));
    REQUIRE_THAT(l_ArtificialTsunami2d.computeG(0), Catch::Matchers::WithinAbs(1, margin));
    REQUIRE_THAT(l_ArtificialTsunami2d.computeD(500,500), Catch::Matchers::WithinAbs(0, margin));

    /** x=300 , y=200  
     *  f(300)= sin((300/500+1) * pi) = -0,95106
     *  g(0) = -(200/500)^2 +1 = 0,84
     *  d(0)(0) = 5*-0.95106 * 0,84 =-3,994
     */
    REQUIRE_THAT(l_ArtificialTsunami2d.computeF(300), Catch::Matchers::WithinAbs(-0.95106, margin));
    REQUIRE_THAT(l_ArtificialTsunami2d.computeG(200), Catch::Matchers::WithinAbs(0.84, margin));
    REQUIRE_THAT(l_ArtificialTsunami2d.computeD(300,200), Catch::Matchers::WithinAbs(-3.994, margin));

    /** x=700 , y=800  
     * = 0 bc out of range
     */
    REQUIRE_THAT(l_ArtificialTsunami2d.computeD(700,800), Catch::Matchers::WithinAbs(0, margin));
}