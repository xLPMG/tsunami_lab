/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * IO-routines for writing a snapshot as Comma Separated Values (CSV).
 **/
#ifndef TSUNAMI_LAB_IO_NetCdf
#define TSUNAMI_LAB_IO_NetCdf

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
    /**
     * checks for error in Cdf file
     * @param i_err error
     */
static void checkNcErr(int i_err);

public:
    /**
     * writes into cdf file
     */
static void write(t_real i_dx,
               t_real i_dy,
               t_idx i_nx,
               t_idx i_ny,
               t_idx i_stride,
               t_real const *i_h,
               t_real const *i_hu,
               t_real const *i_hv,
               t_real const *i_b,
               std::ostream &io_stream);
};
#endif
