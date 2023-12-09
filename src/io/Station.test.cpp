/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Test of stations
 **/

#include <catch2/catch.hpp>
#include "Station.h"
#include "../setups/DamBreak1d.h"
#include "../patches/WavePropagation1d.h"
#include "../patches/WavePropagation2d.h"

using Boundary = tsunami_lab::patches::WavePropagation::Boundary;

TEST_CASE("1D test of the station implementation", "[Station], [WavePropagation1d]")
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

    std::vector<tsunami_lab::io::Station *> m_stations;
    tsunami_lab::patches::WavePropagation1d m_waveProp(100,
                                                       "fwave",
                                                       Boundary::OUTFLOW,
                                                       Boundary::OUTFLOW);

    for (std::size_t l_ce = 0; l_ce < 50; l_ce++)
    {
        m_waveProp.setHeight(l_ce,
                             0,
                             10);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                0);
    }
    for (std::size_t l_ce = 50; l_ce < 100; l_ce++)
    {
        m_waveProp.setHeight(l_ce,
                             0,
                             8);
        m_waveProp.setMomentumX(l_ce,
                                0,
                                0);
    }

    // set outflow boundary condition
    m_waveProp.setGhostOutflow();

    // initialize stations
    m_stations.push_back(new tsunami_lab::io::Station(10,
                                                      0,
                                                      "station_left",
                                                      &m_waveProp));
    m_stations.push_back(new tsunami_lab::io::Station(49,
                                                      0,
                                                      "station_middleLeft",
                                                      &m_waveProp));
    m_stations.push_back(new tsunami_lab::io::Station(50,
                                                      0,
                                                      "station_middleRight",
                                                      &m_waveProp));
    m_stations.push_back(new tsunami_lab::io::Station(90,
                                                      0,
                                                      "station_right",
                                                      &m_waveProp));
    // perform a time step
    m_waveProp.timeStep(0.1, 0);
    for (tsunami_lab::io::Station *l_s : m_stations)
    {
        l_s->capture(0.1);
    }

    // steady state
    REQUIRE(m_stations[0]->getData()[0][1] == Approx(10)); // height
    REQUIRE(m_stations[0]->getData()[0][2] == Approx(0));  // momentum x

    // dam-break
    REQUIRE(m_stations[1]->getData()[0][1] == Approx(10 - 0.1 * 9.394671362)); // height
    REQUIRE(m_stations[1]->getData()[0][2] == Approx(0 + 0.1 * 88.25985));     // momentum x

    REQUIRE(m_stations[2]->getData()[0][1] == Approx(8 + 0.1 * 9.394671362)); // height
    REQUIRE(m_stations[2]->getData()[0][2] == Approx(0 + 0.1 * 88.25985));    // momentum x

    // steady state
    REQUIRE(m_stations[3]->getData()[0][1] == Approx(8)); // height
    REQUIRE(m_stations[3]->getData()[0][2] == Approx(0)); // momentum x

    for (tsunami_lab::io::Station *l_s : m_stations)
    {
        delete l_s;
    }
}

TEST_CASE("2D test of the station implementation", "[Station],[WavePropagation2d]")
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
    std::vector<tsunami_lab::io::Station *> m_stations;
    tsunami_lab::patches::WavePropagation2d m_waveProp(100,
                                                       100,
                                                       Boundary::OUTFLOW,
                                                       Boundary::OUTFLOW,
                                                       Boundary::OUTFLOW,
                                                       Boundary::OUTFLOW);

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

    // initialize stations
    m_stations.push_back(new tsunami_lab::io::Station(10,
                                                      50,
                                                      "station_left",
                                                      &m_waveProp));
    m_stations.push_back(new tsunami_lab::io::Station(49,
                                                      50,
                                                      "station_middleLeft",
                                                      &m_waveProp));
    m_stations.push_back(new tsunami_lab::io::Station(50,
                                                      50,
                                                      "station_middleRight",
                                                      &m_waveProp));
    m_stations.push_back(new tsunami_lab::io::Station(90,
                                                      50,
                                                      "station_right",
                                                      &m_waveProp));
    // perform a time step and capture data
    m_waveProp.timeStep(0.1, 0.1);
    for (tsunami_lab::io::Station *l_s : m_stations)
    {
        l_s->capture(0.1);
    }

    // steady state
    REQUIRE(m_stations[0]->getData()[0][1] == Approx(10)); // height
    REQUIRE(m_stations[0]->getData()[0][2] == Approx(0));  // momentum x
    REQUIRE(m_stations[0]->getData()[0][3] == Approx(0));  // momentum y

    // dam-break
    REQUIRE(m_stations[1]->getData()[0][1] == Approx(10 - 0.1 * 9.394671362)); // height
    REQUIRE(m_stations[1]->getData()[0][2] == Approx(0 + 0.1 * 88.25985));     // momentum x
    REQUIRE(m_stations[1]->getData()[0][3] == Approx(0));                      // momentum y

    REQUIRE(m_stations[2]->getData()[0][1] == Approx(8 + 0.1 * 9.394671362)); // height
    REQUIRE(m_stations[2]->getData()[0][2] == Approx(0 + 0.1 * 88.25985));    // momentum x
    REQUIRE(m_stations[2]->getData()[0][3] == Approx(0));                     // momentum y

    // steady state
    REQUIRE(m_stations[3]->getData()[0][1] == Approx(8)); // height
    REQUIRE(m_stations[3]->getData()[0][2] == Approx(0)); // momentum x
    REQUIRE(m_stations[3]->getData()[0][3] == Approx(0)); // momentum y

    for (tsunami_lab::io::Station *l_s : m_stations)
    {
        delete l_s;
    }
}