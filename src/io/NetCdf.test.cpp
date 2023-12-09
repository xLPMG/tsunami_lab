/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Unit tests for the NETCDF_interface.
 **/

#include <catch2/catch.hpp>
#include "../constants.h"
#include <iostream>
#include <filesystem>

#define private public
#include "NetCdf.h"
#undef public

TEST_CASE("Test NetCdf reading and writing functionality", "[NetCdf], [ReadFile], [WriteFile]")
{
    // setup

    // for simplification, n_cells and simulation size match
    tsunami_lab::t_idx l_x = 10, l_y = 10;
    tsunami_lab::t_real l_simulationSizeX = l_x, l_simulationSizeY = l_y;
    const char *l_netCdfFile = "resources/netCdfTest.nc";
    std::filesystem::remove(l_netCdfFile);

    // write data
    tsunami_lab::io::NetCdf *l_netCdfWrite = new tsunami_lab::io::NetCdf(l_x,
                                                                         l_y,
                                                                         1,
                                                                         l_simulationSizeX,
                                                                         l_simulationSizeY,
                                                                         0,
                                                                         0,
                                                                         l_netCdfFile,
                                                                         "");

    tsunami_lab::t_real *l_dataToWrite = new tsunami_lab::t_real[l_x * l_y];

    int l_i = 1;
    for (tsunami_lab::t_idx l_iy = 0; l_iy < l_y; l_iy++)
    {
        for (tsunami_lab::t_idx l_ix = 0; l_ix < l_x; l_ix++)
        {
            l_dataToWrite[l_ix + l_x * l_iy] = -l_i * 100;
            l_i++;
        }
    }

    l_netCdfWrite->write(l_x,
                         l_dataToWrite,
                         nullptr,
                         nullptr,
                         nullptr,
                         0);

    delete l_netCdfWrite;
    delete[] l_dataToWrite;

    // read data
    tsunami_lab::io::NetCdf *l_netCdfRead = new tsunami_lab::io::NetCdf(l_x,
                                                                        l_y,
                                                                        1,
                                                                        l_simulationSizeX,
                                                                        l_simulationSizeY,
                                                                        0,
                                                                        0,
                                                                        l_netCdfFile,
                                                                        "");

    tsunami_lab::t_idx l_nx = 0, l_ny = 0;
    l_netCdfRead->getDimensionSize(l_netCdfFile,
                                   "x",
                                   l_nx);
    l_netCdfRead->getDimensionSize(l_netCdfFile,

                                   "y",
                                   l_ny);

    tsunami_lab::t_real *l_dataXToRead = new tsunami_lab::t_real[l_nx];
    tsunami_lab::t_real *l_dataYToRead = new tsunami_lab::t_real[l_ny];
    tsunami_lab::t_real *l_dataToRead = new tsunami_lab::t_real[l_nx * l_ny];
    
    l_netCdfRead->read(l_netCdfFile,
                       "height",
                       &l_dataXToRead,
                       &l_dataYToRead,
                       &l_dataToRead);

    REQUIRE(l_nx == l_x);
    REQUIRE(l_ny == l_y);

    l_i = 1;
    for (tsunami_lab::t_idx l_iy = 0; l_iy < l_y; l_iy++)
    {
        for (tsunami_lab::t_idx l_ix = 0; l_ix < l_x; l_ix++)
        {
            REQUIRE(l_dataToRead[l_ix + l_x * l_iy] == -l_i * 100);
            l_i++;
        }
    }
    // tear down
    std::filesystem::remove(l_netCdfFile);

    delete l_netCdfRead;
    delete[] l_dataXToRead;
    delete[] l_dataYToRead;
    delete[] l_dataToRead;
}

TEST_CASE("Test NetCdf checkpointing functionality", "[NetCdf], [Checkpoint]")
{
    // setup

    // for simplification, n_cells and simulation size match
    tsunami_lab::t_idx l_x = 10, l_y = 10;
    tsunami_lab::t_real l_simulationSizeX = l_x, l_simulationSizeY = l_y;
    tsunami_lab::t_real l_offsetX = -5, l_offsetY = -5;
    tsunami_lab::t_real l_t = static_cast<tsunami_lab::t_real>(rand()) / static_cast<float>(RAND_MAX / 10);
    tsunami_lab::t_idx l_timeStep = static_cast<tsunami_lab::t_idx>(rand()) / static_cast<float>(RAND_MAX / 1000);
    const char *l_netCdfFile = "resources/netCdfTest.nc";
    const char *l_checkpointFile = "resources/netCdfTest.nc";
    std::filesystem::remove(l_netCdfFile);
    std::filesystem::remove(l_checkpointFile);

    // write data
    tsunami_lab::io::NetCdf *l_netCdf = new tsunami_lab::io::NetCdf(l_x,
                                                                    l_y,
                                                                    1,
                                                                    l_simulationSizeX,
                                                                    l_simulationSizeY,
                                                                    l_offsetX,
                                                                    l_offsetY,
                                                                    l_netCdfFile,
                                                                    l_checkpointFile);

    tsunami_lab::t_real *l_h = new tsunami_lab::t_real[l_x * l_y];
    tsunami_lab::t_real *l_b = new tsunami_lab::t_real[l_x * l_y];
    tsunami_lab::t_real *l_hu = new tsunami_lab::t_real[l_x * l_y];
    tsunami_lab::t_real *l_hv = new tsunami_lab::t_real[l_x * l_y];

    tsunami_lab::t_idx l_i = 1;
    for (tsunami_lab::t_idx l_iy = 0; l_iy < l_y; l_iy++)
    {
        for (tsunami_lab::t_idx l_ix = 0; l_ix < l_x; l_ix++)
        {
            l_h[l_ix + l_x * l_iy] = l_i * 100;
            l_b[l_ix + l_x * l_iy] = -l_i * 100;
            l_hu[l_ix + l_x * l_iy] = l_i * 13;
            l_hv[l_ix + l_x * l_iy] = 0;
            l_i++;
        }
    }

    l_netCdf->writeCheckpoint(l_checkpointFile,
                              l_x,
                              l_h,
                              l_hu,
                              l_hv,
                              l_b,
                              l_t,
                              l_timeStep);

    // load checkpoint
    tsunami_lab::t_idx l_xRead = 0, l_yRead = 0;
    tsunami_lab::t_real l_simulationSizeXRead = 0, l_simulationSizeYRead = 0;
    tsunami_lab::t_real l_offsetXRead = 0, l_offsetYRead = 0;
    tsunami_lab::t_real l_tRead = 0;
    tsunami_lab::t_idx l_timeStepRead = 0;
    tsunami_lab::t_idx l_k= 0;

    l_netCdf->loadCheckpointDimensions(l_checkpointFile,
                                       l_xRead,
                                       l_yRead,
                                       l_k,
                                       l_simulationSizeXRead,
                                       l_simulationSizeYRead,
                                       l_offsetXRead,
                                       l_offsetYRead,
                                       l_tRead,
                                       l_timeStepRead);

    tsunami_lab::t_real *l_hRead = new tsunami_lab::t_real[l_xRead * l_yRead];
    tsunami_lab::t_real *l_bRead = new tsunami_lab::t_real[l_xRead * l_yRead];
    tsunami_lab::t_real *l_huRead = new tsunami_lab::t_real[l_xRead * l_yRead];
    tsunami_lab::t_real *l_hvRead = new tsunami_lab::t_real[l_xRead * l_yRead];
    l_netCdf->read(l_checkpointFile, "height", &l_hRead);
    l_netCdf->read(l_checkpointFile, "bathymetry", &l_bRead);
    l_netCdf->read(l_checkpointFile, "momentumX", &l_huRead);
    l_netCdf->read(l_checkpointFile, "momentumY", &l_hvRead);

    REQUIRE(l_xRead == l_x);
    REQUIRE(l_yRead == l_y);
    REQUIRE(l_simulationSizeXRead == l_simulationSizeX);
    REQUIRE(l_simulationSizeYRead == l_simulationSizeY);
    REQUIRE(l_offsetXRead == l_offsetX);
    REQUIRE(l_offsetYRead == l_offsetY);
    REQUIRE(l_tRead == l_t);
    REQUIRE(l_timeStepRead == l_timeStep);

    l_i = 0;
    for (tsunami_lab::t_idx l_iy = 0; l_iy < l_y; l_iy++)
    {
        for (tsunami_lab::t_idx l_ix = 0; l_ix < l_x; l_ix++)
        {
            REQUIRE(l_hRead[l_i] == l_h[l_i]);
            REQUIRE(l_bRead[l_i] == l_b[l_i]);
            REQUIRE(l_huRead[l_i] == l_hu[l_i]);
            REQUIRE(l_hvRead[l_i] == l_hv[l_i]);
            l_i++;
        }
    }

    // tear down
    std::filesystem::remove(l_netCdfFile);
    std::filesystem::remove(l_checkpointFile);

    delete l_netCdf;

    delete[] l_h;
    delete[] l_b;
    delete[] l_hu;
    delete[] l_hv;

    delete[] l_hRead;
    delete[] l_bRead;
    delete[] l_huRead;
    delete[] l_hvRead;
}