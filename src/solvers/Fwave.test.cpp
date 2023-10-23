/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
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
  float eigenvalue_roe_1 = 0;
  float eigenvalue_roe_2 = 0;
  tsunami_lab::solvers::Fwave::computeEigenvalues( 10,
                                         9,
                                         -3,
                                         3,
                                         eigenvalue_roe_1,
                                         eigenvalue_roe_2 );

  REQUIRE( eigenvalue_roe_1 == Approx( -9.7311093998375095 ) );
  REQUIRE( eigenvalue_roe_2 == Approx(  9.5731051658991654 ) );
}

TEST_CASE( "Test the computation of the Eigencoefficients.", "[Eigencoefficients]" ) {
  /*
   * Test case:
   *  h:   10 | 9
   *  u:   -3 | 3
   *  hu: -30 | 27
   *
   * The derivation of the Roe speeds (s1, s2) is given above.
   *
   *  Matrix of right eigenvectors:
   *
   *      | 1   1 |
   *  R = |       |
   *      | s1 s2 |
   *
   * Inversion yields:
   *
   * wolframalpha.com query: invert {{1, 1}, {-9.7311093998375095, 9.5731051658991654}}
   *
   *        | 0.49590751974393229 -0.051802159398648326 |
   * Rinv = |                                           |
   *        | 0.50409248025606771  0.051802159398648326 |
   *
   *
   * F-diff[0] = huR - huL = 27--30
   * F-diff[1] = (9 * (3)^2 + 1/2 * 9.80665 * 9^2) - (10 * (-3)^2 + 1/2 * 9.80665 * 10^2) = 478.168 - 580.333 = -102.163
   *
   * wolframalpha.com query: {{0.49590751974393229, -0.051802159398648326}, {0.50409248025606771, 0.051802159398648326}} * {27--30, -102.163}
   * -> alpha = (33.559, 23.441)
   */
  float alpha_1 = 0;
  float alpha_2 = 0;

  tsunami_lab::solvers::Fwave::computeEigencoefficients( 10,
                                            9,
                                            -30,
                                            27,
                                            -9.7311093998375095,
                                            9.5731051658991654,
                                            alpha_1,
                                            alpha_2 );

  REQUIRE( alpha_1 == Approx(33.559) );
  REQUIRE( alpha_2 == Approx(23.441) );
}
