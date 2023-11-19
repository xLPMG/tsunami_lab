/**
 * @author Luca-Philipp Grumbach & Richard Hofmann
 *
 * @section DESCRIPTION
 * Test of the bathymetry loader
 **/

#include <catch2/catch.hpp>
#include "BathymetryLoader.h"

TEST_CASE("Test of the bathymetry loader", "[BathymetryLoader]")
{
    /**
     * Test csv file:
     *
     * #dimensions
     * DIM,10,1
     * #x,y,bathymetry
     * 0,0,0
     * 1,0,1
     * 2,0,2
     * 3,0,3
     * 4,0,4
     * 0,1,5
     * 1,1,6
     * 2,1,7
     * 3,1,8
     * 4,1,9
     *
     */

    tsunami_lab::io::BathymetryLoader m_bathymetryLoader;
    m_bathymetryLoader.loadBathymetry("resources/bathymetryLoaderTest.csv");

    REQUIRE(m_bathymetryLoader.getBathymetry(0, 0) == 0);
    REQUIRE(m_bathymetryLoader.getBathymetry(1, 0) == 1);
    REQUIRE(m_bathymetryLoader.getBathymetry(3, 0) == 3);

    REQUIRE(m_bathymetryLoader.getBathymetry(0, 1) == 5);
    REQUIRE(m_bathymetryLoader.getBathymetry(1, 1) == 6);
    REQUIRE(m_bathymetryLoader.getBathymetry(3, 1) == 8);

    REQUIRE(m_bathymetryLoader.getBathymetry(9, 9) == 0);
}