/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 *
 **/
#ifndef TSUNAMI_LAB_IO_NETCDF
#define TSUNAMI_LAB_IO_NETCDF

#include "../constants.h"
#include <cstring>
#include <iostream>
#include <vector>

namespace tsunami_lab
{
    namespace io
    {
        class NetCdf;
    }
}

class tsunami_lab::io::NetCdf
{
private:
    t_idx m_nx = 0;
    t_idx m_ny = 0;
    int m_ncId = 0;
    // error
    int m_err = 0;

    // dimension ids
    int m_dimXId = 0;
    int m_dimYId = 0;
    int m_dimTId = 0;

    // variable ids
    int m_varXId = 0;
    int m_varYId = 0;
    int m_varTId = 0;
    int m_varHId = 0;
    int m_varTHId = 0;
    int m_varBId = 0;
    int m_varHuId = 0;
    int m_varHvId = 0;

    // index for timesteps
    t_idx m_timeStepCount = 0;

    /**
     * checks for error in Cdf file
     * @param i_err error
     */
    void checkNcErr(t_idx i_err);

    /**
     * Sets up a netcdf file fr writing
     *
     */
    void setUpFile(const char *path);

public:
    /**
     *  Constructor
     * @param i_nx amount of cells in x-direction
     * @param i_ny amount of cells in y-direction
     *
     */
    NetCdf(t_idx i_nx,
           t_idx i_ny);

    /**
     * Destructor
     *
     */
    ~NetCdf();

    /**
     * Writes into cdf file
     *
     * @param path file path
     * @param i_stride stride
     * @param i_h water heights
     * @param i_hu momentum x-direction
     * @param i_hv momentum y-direction
     * @param i_b bathymetry
     * @param i_t current timestep
     *
     */
    void write(const char *path,
               t_idx i_stride,
               t_real const *i_h,
               t_real const *i_hu,
               t_real const *i_hv,
               t_real const *i_b,
               t_real i_t);

    /**
     * reads from the cdf file
     *
     * @param l_file name of the file to read from
     */
    t_real *read(const char *i_file,
                 const char *i_var);
};
#endif
