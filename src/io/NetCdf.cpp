/**
 * @author Luca Grumbacher, Richard Hofmann
 *
 * @section DESCRIPTION
 * interface for NetCdf
 **/
#include "NetCdf.h"
#include <iostream>
#include <netcdf.h>

void checkNcErr(int i_err)
{
    if (i_err)
    {
        std::cerr << "Error: "
                  << nc_strerror(i_err)
                  << std::endl;
        exit(2);
    }
}

void tsunami_lab::io::NetCdf::write(t_real i_dx,
                                    t_real i_dy,
                                    t_idx i_nx,
                                    t_idx i_ny,
                                    t_idx i_stride,
                                    t_real const *i_h,
                                    t_real const *i_hu,
                                    t_real const *i_hv,
                                    t_real const *i_b,
                                    std::ostream &io_stream)
{
    
}
