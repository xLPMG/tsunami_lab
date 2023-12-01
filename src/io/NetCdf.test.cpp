#include <catch2/catch.hpp>
#include "../constants.h"
#define private public
#include "NetCdf.h"
#undef public
#include <iostream>

TEST_CASE("Test NetCdf reading and writing functionality", "[NetCdf]")
{
    // for simplification, n_cells and simulation size match
    tsunami_lab::t_idx l_x = 10, l_y = 10;

    // write data
    tsunami_lab::io::NetCdf *l_netCdfWrite = new tsunami_lab::io::NetCdf(l_x, l_y, l_x, l_y, 0, 0);

    tsunami_lab::t_real *l_dataToWrite = new tsunami_lab::t_real[l_x * l_y];
    for (tsunami_lab::t_idx l_ix = 0; l_ix < l_x; l_ix++)
    {
        for (tsunami_lab::t_idx l_iy = 0; l_iy < l_y; l_iy++)
        {
            l_dataToWrite[l_ix + l_x * l_iy] = -int(l_iy);
        }
    }
    l_netCdfWrite->write("resources/netCdfTest.nc",
                         l_x,
                         l_dataToWrite,
                         nullptr,
                         nullptr,
                         nullptr,
                         0);
    delete l_netCdfWrite;
    delete[] l_dataToWrite;

    // read data
    tsunami_lab::io::NetCdf *l_netCdfRead = new tsunami_lab::io::NetCdf(l_x, l_y, l_x, l_y, 0, 0);

    tsunami_lab::t_real *l_dataXToRead = new tsunami_lab::t_real[l_x];
    tsunami_lab::t_real *l_dataYToRead = new tsunami_lab::t_real[l_y];
    tsunami_lab::t_real *l_dataToRead = new tsunami_lab::t_real[l_x*l_y];

    tsunami_lab::t_idx l_nx = 0, l_ny = 0;
    l_netCdfRead->getDimensionSize("resources/netCdfTest.nc",
                                   l_nx,
                                   "x");
    l_netCdfRead->getDimensionSize("resources/netCdfTest.nc",
                                   l_ny,
                                   "y");
    l_netCdfRead->read("resources/netCdfTest.nc",
                       "height",
                       &l_dataXToRead,
                       &l_dataYToRead,
                       &l_dataToRead);

    REQUIRE(l_nx == l_x);
    REQUIRE(l_ny == l_y);

    int i=0;
    for (tsunami_lab::t_idx l_ix = 0; l_ix < l_x; l_ix++)
    {
        for (tsunami_lab::t_idx l_iy = 0; l_iy < l_y; l_iy++)
        {
            REQUIRE(l_dataToRead[i++] == -int(l_iy));
        }
    }

    delete l_netCdfRead;
    delete[] l_dataXToRead;
    delete[] l_dataYToRead;
    delete[] l_dataToRead;
}