/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Test for the implementation of 2 artificial tsunami event
 **/

#include <catch2/catch.hpp>
#include "ArtificialTsunami2d.h"

TEST_CASE("Test the two-dimensional Artificial Tsunami event.", "[ArtificialTsunami2d]")
{
  tsunami_lab::setups::ArtificialTsunami2d l_ArtificialTsunami2d;

    /**min x=y=500  -->right in the center 
     * i_x-=500 ; i_y-=500 
     * f(0)= sin((0/500+1)* pi) = 0
     * g(0) = -(y/500)^2 +1 = 1
     * d(0)(0) = 5*1*0 = 0
     */
    REQUIRE(l_ArtificialTsunami2d.computeF(0) == 0.0);
    REQUIRE(l_ArtificialTsunami2d.computeG(0) == 1);
    REQUIRE(l_ArtificialTsunami2d.computeD(500,500) == tsunami_lab::t_real(0));

    /**min x=800 , y=700  
     * i_x-=500 ; i_y-=500 --> x=300, y=200
     * f(0)= sin((0/500+1)* pi) = -0,95106
     * g(0) = -(200/500)^2 +1 = 0,84
     * d(0)(0) = 5*-0.95106 * 0,84 =-3,994
     */
    REQUIRE(l_ArtificialTsunami2d.computeF(300) == Approx(-0.95106));
    REQUIRE(l_ArtificialTsunami2d.computeG(200) == Approx(0.84));
    REQUIRE(l_ArtificialTsunami2d.computeD(800,700) == Approx(-3.994));


    

}