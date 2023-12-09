/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Unit tests of the Fwave solver.
 **/
#include <catch2/catch.hpp>
#define private public
#include "Fwave.h"
#undef public

TEST_CASE("Test the computation of the eigenvalues.", "[Eigenvalues]")
{
  /*
   * Test case:
   *  h: 10 | 9
   *  u: -3 | 3
   *
   * roe height: 9.
   * roe velocity: (sqrt(10) * -3 + 3 * 3) / ( sqrt(10) + sqrt(9) )
   *               = -0.0790021169691720
   * roe eigenvalues:  s1 = -0.079002116969172024 - sqrt(9.80665 * 9.5) = -9.7311093998375095
   *                   s2 = -0.079002116969172024 + sqrt(9.80665 * 9.5) =  9.5731051658991654
   */
  float eigenvalue_roe_1 = 0;
  float eigenvalue_roe_2 = 0;
  tsunami_lab::solvers::Fwave::computeEigenvalues(10,
                                                  9,
                                                  -3,
                                                  3,
                                                  eigenvalue_roe_1,
                                                  eigenvalue_roe_2);

  REQUIRE(eigenvalue_roe_1 == Approx(-9.7311093998375095));
  REQUIRE(eigenvalue_roe_2 == Approx(9.5731051658991654));
}

TEST_CASE("Test the computation of the eigencoefficients.", "[Eigencoefficients]")
{
  /*
   * Test case:
   *  h:   10 | 9
   *  u:   -3 | 3
   *  hu: -30 | 27
   *
   * The derivation of the Roe eigenvalues (s1, s2) is given above.
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
   * f_delta[0] = huR - huL = 27--30
   * f_delta[1] = (9 * (3)^2 + 1/2 * 9.80665 * 9^2) - (10 * (-3)^2 + 1/2 * 9.80665 * 10^2) = 478.168 - 580.333 = -102.163
   *
   * wolframalpha.com query: {{0.49590751974393229, -0.051802159398648326}, {0.50409248025606771, 0.051802159398648326}} * {27--30, -102.163}
   * -> alpha = (33.558992636048, 23.441007363952)
   */
  float alpha_1 = 0;
  float alpha_2 = 0;

  tsunami_lab::solvers::Fwave::computeEigencoefficients(10,
                                                        9,
                                                        -30,
                                                        27,
                                                        0,
                                                        0,
                                                        -9.7311093998375095,
                                                        9.5731051658991654,
                                                        alpha_1,
                                                        alpha_2);

  REQUIRE(alpha_1 == Approx(33.558992636048));
  REQUIRE(alpha_2 == Approx(23.441007363952));
}

TEST_CASE("Test the computation of the F-wave net-updates.", "[FWaveUpdates]")
{
  /*
   * Test case:
   *
   *      left | right
   *  h:    10 | 9
   *  u:    -3 | 3
   *  hu:  -30 | 27
   *
   * The derivation of the Roe eigenvalues (s1, s2) and eigencoefficients (a1, a2) is given above.
   *
   * The net-updates are given through (eigencoefficient * eigenvector)
   *
   *                 |  1 |   |  33.558992636048 |
   * update #1: a1 * |    | = |                  |
   *                 | s1 |   | -326.56622868972 |
   *
   *                 |  1 |   | 23.441007363952 |
   * update #2: a2 * |    | = |                           |
   *                 | s2 |   | 224.403141905910928927533 |
   */
  float l_netUpdatesL[2] = {-5, 3};
  float l_netUpdatesR[2] = {4, 7};

  tsunami_lab::solvers::Fwave::netUpdates(10,
                                          9,
                                          -30,
                                          27,
                                          0,
                                          0,
                                          l_netUpdatesL,
                                          l_netUpdatesR);

  REQUIRE(l_netUpdatesL[0] == Approx(33.558992636048));
  REQUIRE(l_netUpdatesL[1] == Approx(-326.56622868972));

  REQUIRE(l_netUpdatesR[0] == Approx(23.441007363952));
  REQUIRE(l_netUpdatesR[1] == Approx(224.40322868973));

  /*
   * Test case (dam break):
   *
   *     left | right
   *   h:  10 | 8
   *   hu:  0 | 0
   *
   * Roe eigenvalues are given as:
   *
   *   s1 = -sqrt(9.80665 * 9)
   *   s2 =  sqrt(9.80665 * 9)
   *
   * Inversion of the matrix of right Eigenvectors:
   *
   *   wolframalpha.com query: invert {{1, 1}, {-sqrt(9.80665 * 9), sqrt(9.80665 * 9)}}
   *
   *          | 0.5 -0.0532217 |
   *   Rinv = |                |
   *          | 0.5 -0.0532217 |
   *
   * f_delta[0] = huR - huL = 0
   * f_delta[1] = (0*8 + 0.5*9.80665*64) - (0*10 + 0.5*9.80665*100) = -176.5197
   *
   * Multiplicaton with delta f gives the eigencoefficients:
   *
   *        |   0        |   | 9.39467851749 |   | a1 |
   * Rinv * |            | = |               | = |    |
   *        |  -176.5197 |   | 9.39467851749 |   | a2 |
   *
   * The net-updates are given through (eigencoefficient * eigenvector)
   *
   *                 |  1 |   |   9.39467851749   |
   * update #1: a1 * |    | = |                   |
   *                 | s1 |   | -88.2599172234684 |
   *
   *                 |  1 |   |  9.39467851749   |
   * update #2: a2 * |    | = |                  |
   *                 | s2 |   | 88.259917223468  |
   */
  tsunami_lab::solvers::Fwave::netUpdates(10,
                                          8,
                                          0,
                                          0,
                                          0,
                                          0,
                                          l_netUpdatesL,
                                          l_netUpdatesR);

  REQUIRE(l_netUpdatesL[0] == Approx(9.39467851749));
  REQUIRE(l_netUpdatesL[1] == -Approx(88.2599172234684));

  REQUIRE(l_netUpdatesR[0] == -Approx(9.39467851749));
  REQUIRE(l_netUpdatesR[1] == -Approx(88.259917223468));

  /*
   * Test case (trivial steady state):
   *
   *     left | right
   *   h:  10 | 10
   *  hu:   0 |  0
   */
  tsunami_lab::solvers::Fwave::netUpdates(10,
                                          10,
                                          0,
                                          0,
                                          0,
                                          0,
                                          l_netUpdatesL,
                                          l_netUpdatesR);

  REQUIRE(l_netUpdatesL[0] == Approx(0));
  REQUIRE(l_netUpdatesL[1] == Approx(0));

  REQUIRE(l_netUpdatesR[0] == Approx(0));
  REQUIRE(l_netUpdatesR[1] == Approx(0));
}
