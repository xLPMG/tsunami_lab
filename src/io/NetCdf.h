/**
 * @author Luca Grumbach, Richard Hofmann
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

    t_real m_nx = 0;
    t_real m_ny = 0;
    t_real m_stride = 0;
    int m_ncId = 0;
    // error
     int m_err = 0;

     int m_dimXId = 0;

     int m_dimYId = 0;

     int m_dimTId = 0;

     int m_varHId = 0;

     int m_varBId = 0;

     int m_varHuId = 0;

     int m_varHvId = 0;

     int m_dimIds[2] = {0};

     int m_dimTIds[3] = {0};


    /**
     * checks for error in Cdf file
     * @param i_err error
     */
    void checkNcErr(t_idx i_err);

public:
    /**
     *
     *
     */
    NetCdf(t_real i_dx,
           t_real i_dy,
           t_idx i_nx,
           t_idx i_ny,
           t_idx i_stride,
           t_real const *i_b);

    /**
     * writes into cdf file
     *
     * @param *i_h pointer for height
     * @param *i_hu mementum x
     * @param *i_hv momnetum y
     * 
     */
    void write(t_real const *i_h,
                      t_real const *i_hu,
                      t_real const *i_hv,
                      t_real i_t);
};
#endif
