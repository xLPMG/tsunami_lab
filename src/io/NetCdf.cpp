/**
 * @author Luca Grumbacher, Richard Hofmann
 *
 * @section DESCRIPTION
 * interface for NetCdf
 **/
#include "NetCdf.h"
#include <iostream>
#include <netcdf.h>

void tsunami_lab::io::NetCdf::checkNcErr(tsunami_lab::t_idx i_err)
{
    if (i_err)
    {
        std::cerr << "Error: "
                  << nc_strerror(i_err)
                  << std::endl;
        exit(2);
    }
}

tsunami_lab::io::NetCdf::NetCdf(t_real i_dx,
                               t_real i_dy,
                               t_idx  i_nx,
                               const t_idx  i_ny,
                               t_idx i_stride,
                               t_real const *i_b)
{

    m_nx = i_dx;
    m_ny = i_dy;
    m_stride = i_stride;

     m_err = nc_create("tsunamiNetCdf.nc", // path
                      NC_CLOBBER,         // cmode
                      &m_ncId);           // ncidp
    checkNcErr(m_err);

    t_real m_bathymetryData[i_nx][i_ny];

    //loop for bathymetry
    for (std::size_t l_ix = 0; l_ix < i_nx; l_ix++)
    {
        for (std::size_t l_iy = 0; l_iy < i_ny; l_iy++)
        {
            m_bathymetryData[l_ix][l_iy] = i_b[l_ix + l_iy * i_stride];
        }
    }

    // define dimensions

    m_err = nc_def_dim(m_ncId,     // ncid
                       "x",        // name
                       i_nx,       // len
                       &m_dimXId); // idp
    checkNcErr(m_err);

    m_err = nc_def_dim(m_ncId,     // ncid
                       "y",        // name
                       i_ny,       // len
                       &m_dimYId); // idp
    checkNcErr(m_err);
 
    m_err = nc_def_dim(m_ncId,       // ncid
                       "time",       // name
                       NC_UNLIMITED, // len
                       &m_dimTId);   // idp
    checkNcErr(m_err);




    // define variables
    m_dimTIds[0] = m_dimTId;
    m_dimTIds[1] = m_dimXId;
    m_dimTIds[2] = m_dimYId;

    m_dimIds[0] = m_dimXId;
    m_dimIds[1] = m_dimYId;

    m_err = nc_def_var(m_ncId,     // ncid
                       "height",   // name
                       NC_FLOAT,   // xtype
                       3,          // ndims
                       m_dimTIds,  // dimidsp
                       &m_varHId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncId,       // ncid
                       "bathymetry", // name
                       NC_FLOAT,     // xtype
                       2,            // ndims
                       m_dimIds,    // dimidsp
                       &m_varBId);   // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncId,      // ncid
                       "momentumX", // name
                       NC_FLOAT,    // xtype
                       3,           // ndims
                       m_dimTIds,  // dimidsp
                       &m_varHuId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncId,      // ncid
                       "momentumY", // name
                       NC_FLOAT,    // xtype
                       3,           // ndims
                       m_dimTIds,  // dimidsp
                       &m_varHvId); // varidp
    checkNcErr(m_err);

    m_err = nc_enddef( m_ncId ); // ncid
  checkNcErr( m_err );

    // write data
    m_err = nc_put_var_float(m_ncId,                   // ncid
                             m_varBId,                 // varid
                             &m_bathymetryData[0][0]); // op
    checkNcErr(m_err);

    m_err = nc_close(m_ncId); // ncid
    checkNcErr(m_err);
}

void tsunami_lab::io::NetCdf::write(t_real const *i_h,
                                    t_real const *i_hu,
                                    t_real const *i_hv,
                                    t_real i_t)
{

    size_t start[3], count[3];

    count[0] = 1;
    count[1] = m_nx;
    count[2] = m_ny;

    start[0] = i_t;
    start[1] = 0;
    start[2] = 0;

    ptrdiff_t stride[] = {0,m_stride, m_stride};//stride


   m_err = nc_put_vars_float(m_ncId,                   // ncid
                             m_varHId, 
                             start,
                             count,
                             stride,
                             i_h); // op
    checkNcErr(m_err);
}
