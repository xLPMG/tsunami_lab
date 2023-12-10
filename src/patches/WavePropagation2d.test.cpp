/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Unit tests for the two-dimensional wave propagation patch.
 **/
#include <catch2/catch.hpp>
#include "WavePropagation2d.h"

using Boundary = tsunami_lab::patches::WavePropagation::Boundary;

TEST_CASE("Test the 2d wave propagation solver using fwave.", "[WaveProp2d]")
{
  /*
   * Test case:
   *
   *   Single dam break problem between cell 49 and 50.
   *     left | right
   *       10 | 8
   *        0 | 0
   *
   *   Elsewhere steady state.
   *
   * The net-updates at the respective edge are given as
   * (see derivation in Roe solver):
   *    left          | right
   *      9.394671362 | -9.394671362
   *    -88.25985     | -88.25985
   */

  // construct solver and setup a dambreak problem
  tsunami_lab::patches::WavePropagation2d m_waveProp(100,
                                                     100,
                                                     Boundary::OUTFLOW,
                                                     Boundary::OUTFLOW,
                                                     Boundary::OUTFLOW,
                                                     Boundary::OUTFLOW);

  std::size_t stride = 100 + 2;

  for (std::size_t l_ce = 0; l_ce < 50; l_ce++)
  {
    for (std::size_t l_de = 0; l_de < 100; l_de++)
    {
      m_waveProp.setHeight(l_ce,
                           l_de,
                           10);
      m_waveProp.setMomentumX(l_ce,
                              l_de,
                              0);
      m_waveProp.setMomentumY(l_ce,
                              l_de,
                              0);
    }
  }
  for (std::size_t l_ce = 50; l_ce < 100; l_ce++)
  {
    for (std::size_t l_de = 0; l_de < 100; l_de++)
    {
      m_waveProp.setHeight(l_ce,
                           l_de,
                           8);
      m_waveProp.setMomentumX(l_ce,
                              l_de,
                              0);
      m_waveProp.setMomentumY(l_ce,
                              l_de,
                              0);
    }
  }

  // set outflow boundary condition
  m_waveProp.setGhostOutflow();

  // perform a time step
  m_waveProp.timeStep(0.1, 0.1);

  // steady state
  for (std::size_t l_ce = 0; l_ce < 49; l_ce++)
  {
    for (std::size_t l_de = 0; l_de < 100; l_de++)
    {
      REQUIRE(m_waveProp.getHeight()[l_ce + l_de * stride] == Approx(10));
      REQUIRE(m_waveProp.getMomentumX()[l_ce + l_de * stride] == Approx(0));
      REQUIRE(m_waveProp.getMomentumY()[l_ce + l_de * stride] == Approx(0));
    }
  }

  // dam-break
  for (std::size_t l_de = 0; l_de < 100; l_de++)
  {
    REQUIRE(m_waveProp.getHeight()[49 + l_de * stride] == Approx(10 - 0.1 * 9.394671362));
    REQUIRE(m_waveProp.getMomentumX()[49 + l_de * stride] == Approx(0 + 0.1 * 88.25985));
    REQUIRE(m_waveProp.getMomentumY()[49 + l_de * stride] == Approx(0));

    REQUIRE(m_waveProp.getHeight()[50 + l_de * stride] == Approx(8 + 0.1 * 9.394671362));
    REQUIRE(m_waveProp.getMomentumX()[50 + l_de * stride] == Approx(0 + 0.1 * 88.25985));
    REQUIRE(m_waveProp.getMomentumY()[50 + l_de * stride] == Approx(0));
  }
  // steady state
  for (std::size_t l_ce = 51; l_ce < 100; l_ce++)
  {
    for (std::size_t l_de = 0; l_de < 100; l_de++)
    {
      REQUIRE(m_waveProp.getHeight()[l_ce + l_de * stride] == Approx(8));
      REQUIRE(m_waveProp.getMomentumX()[l_ce + l_de * stride] == Approx(0));
      REQUIRE(m_waveProp.getMomentumY()[l_ce + l_de * stride] == Approx(0));
    }
  }
}