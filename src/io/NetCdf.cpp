/**
 * @author Luca Grumbacher, Richard Hofmann
 *
 * @section DESCRIPTION
 * interface for NetCdf
 **/
#include "NetCdf.h"
#include <iostream>
#include <netcdf.h>

void checkNcErr(tsunami_lab::t_idx i_err)
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
    m_err = nc_create( "tsunamiNetCdf.nc", // path
                      NC_CLOBBER,    // cmode
                      &m_ncId );     // ncidp
    checkNcErr( m_err );

    // define dimensions
    m_err = nc_def_dim( m_ncId,      // ncid
                        "x",         // name
                        i_nx,           // len
                        &m_dimXId ); // idp
    checkNcErr( m_err );

    m_err = nc_def_dim( m_ncId,      // ncid
                        "y",         // name
                        i_ny,          // len
                        &m_dimYId ); // idp
    checkNcErr( m_err );

    m_err = nc_def_dim( m_ncId,      // ncid
                        "time",         // name
                        NC_UNLIMITED,          // len
                        &m_dimTId ); // idp
    checkNcErr( m_err );


    //define vairables
    m_dimHIds[0] = m_dimXId;
    m_dimHIds[1] = m_dimYId;
    m_dimHIds[2] = m_dimTId;

    m_err = nc_def_var( m_ncId,     // ncid
                      "height",     // name
                      NC_FLOAT,     // xtype
                      3,          // ndims
                      m_dimHIds,   // dimidsp
                      &m_varHId ); // varidp
    checkNcErr( m_err ); 


    m_dimBIds[0] = m_dimXId;
    m_dimBIds[1] = m_dimYId;

    m_err = nc_def_var( m_ncId,     // ncid
                      "bathymetry",     // name
                      NC_INT,     // xtype
                      2,          // ndims
                      m_dimBIds,   // dimidsp
                      &m_varBId ); // varidp
    checkNcErr( m_err ); 

    m_dimHuIds[0] = m_dimXId;
    m_dimHuIds[1] = m_dimYId;
    m_dimHuIds[2] = m_dimTId;

    m_err = nc_def_var( m_ncId,     // ncid
                      "momentumX",     // name
                      NC_FLOAT,     // xtype
                      3,          // ndims
                      m_dimHuIds,   // dimidsp
                      &m_varHuId ); // varidp
    checkNcErr( m_err ); 

    m_dimHvIds[0] = m_dimXId;
    m_dimHvIds[1] = m_dimYId;
    m_dimHvIds[2] = m_dimTId;

    m_err = nc_def_var( m_ncId,     // ncid
                      "momentumY",     // name
                      NC_FLOAT,     // xtype
                      3,          // ndims
                      m_dimHvIds,   // dimidsp
                      &m_varHvId ); // varidp
    checkNcErr( m_err ); 


    // write data
    // l_err = nc_put_var_int( l_ncId,          // ncid
    //                       l_varId,         // varid
    //                       &l_data[0][0] ); // op
    // checkNcErr( l_err );

    m_err = nc_close( m_ncId ); // ncid
    checkNcErr( m_err );
}
