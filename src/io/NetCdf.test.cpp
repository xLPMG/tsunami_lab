#include <catch2/catch.hpp>
#include "../constants.h"
#include <sstream>
#define private public
#include "NetCdf.h"
#undef public
#include <iostream>

TEST_CASE("Test NetCdf reading functionality", "[NetCdfRead]")
{
    /**
     * Test case: view resources/netCdfTest.cdl
     */

    tsunami_lab::io::NetCdf *l_netCdf = new tsunami_lab::io::NetCdf(nullptr, 10, 10, 10);
    tsunami_lab::t_real *l_data = l_netCdf->read("resources/netCdfTest.nc",
                                                 "bathymetry");

    tsunami_lab::t_idx l_stride = 10;

    for (tsunami_lab::t_idx l_iy = 0; l_iy < 10; l_iy++)
    {
        REQUIRE(l_data[0 + l_stride * l_iy] == tsunami_lab::t_real(-1));
        REQUIRE(l_data[1 + l_stride * l_iy] == tsunami_lab::t_real(-2));
        REQUIRE(l_data[2 + l_stride * l_iy] == tsunami_lab::t_real(-3));
    }

    REQUIRE(l_data[5 + l_stride * 2] == tsunami_lab::t_real(23));
    REQUIRE(l_data[8 + l_stride * 6] == tsunami_lab::t_real(99));
}