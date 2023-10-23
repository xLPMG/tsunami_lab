/**
 * @author Luca-Philipp Gurmbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Unit tests of the Fwave solver.
 **/
#include <catch2/catch.hpp>
#define private public
#include "Fwave.h"
#undef public

TEST_CASE( "Test the derivation of the Eigenvalues.", "[Eigenvalues]" ) {
   /*
    * Test case:
    *  h: 10 | 9
    *  u: -3 | 3
    *
    * roe height: 9. 
    * roe velocity: (sqrt(10) * -3 + 3 * 3) / ( sqrt(10) + sqrt(9) )
    *               = -0.0790021169691720
    * roe speeds: s1 = -0.079002116969172024 - sqrt(9.80665 * 9.5) = -9.7311093998375095
    *             s2 = -0.079002116969172024 + sqrt(9.80665 * 9.5) =  9.5731051658991654
    */
  float l_waveSpeedL = 0;
  float l_waveSpeedR = 0;
  tsunami_lab::solvers::Fwave::computeEigenvalues( 10,
                                         9,
                                         -3,
                                         3,
                                         l_waveSpeedL,
                                         l_waveSpeedR );

  REQUIRE( l_waveSpeedL == Approx( -9.7311093998375095 ) );
  REQUIRE( l_waveSpeedR == Approx(  9.5731051658991654 ) );
}
