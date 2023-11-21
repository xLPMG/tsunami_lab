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
    //
    static int m_ncId;
    // error
    static int m_err;

    static int m_dimXId; 

    static int m_dimYId;

    static int m_dimTId;

    static int m_varHId;

    static int m_varBId;

    static int  m_varHuId;

    static int  m_varHvId;

    static int m_dimBIds[2];

    static int m_dimHIds[3];

    static int m_dimHuIds[3];

    static int m_dimHvIds[3];

    //static t_real l_data;


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
