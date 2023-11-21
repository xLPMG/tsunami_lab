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

tsunami_lab::io::NetCdf::NetCdf(t_idx i_nx,
                                const t_idx i_ny,
                                t_idx i_stride,
                                t_real const *i_b)
{

    m_nx = i_nx;
    m_ny = i_ny;
    m_stride = i_stride;

    m_err = nc_create("tsunamiNetCdf.nc", // path
                      NC_CLOBBER,         // cmode
                      &m_ncId);           // ncidp
    checkNcErr(m_err);

    t_real m_bathymetryData[i_nx][i_ny];

    // loop for bathymetry
    for (std::size_t l_ix = 0; l_ix < i_nx; l_ix++)
    {
        for (std::size_t l_iy = 0; l_iy < i_ny; l_iy++)
        {
            m_bathymetryData[l_ix][l_iy] = i_b[l_ix + l_iy * i_stride];
        }
    }

    // define dimensions

    m_err = nc_def_dim(m_ncId,       // ncid
                       "time",       // name
                       NC_UNLIMITED, // len
                       &m_dimTId);   // idp
    checkNcErr(m_err);

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
                       m_dimIds,     // dimidsp
                       &m_varBId);   // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncId,      // ncid
                       "momentumX", // name
                       NC_FLOAT,    // xtype
                       3,           // ndims
                       m_dimTIds,   // dimidsp
                       &m_varHuId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncId,      // ncid
                       "momentumY", // name
                       NC_FLOAT,    // xtype
                       3,           // ndims
                       m_dimTIds,   // dimidsp
                       &m_varHvId); // varidp
    checkNcErr(m_err);

    // assign attributes
    m_err = nc_put_att_text(m_ncId, m_dimTId, "units",
                            strlen("seconds"), "seconds");
    checkNcErr(m_err);
    m_err = nc_put_att_text(m_ncId, m_dimXId, "units",
                            strlen("meters"), "meters");
    checkNcErr(m_err);
    m_err = nc_put_att_text(m_ncId, m_dimYId, "units",
                            strlen("meters"), "meters");
    checkNcErr(m_err);
    m_err = nc_put_att_text(m_ncId, m_varHId, "units",
                            strlen("meters"), "meters");
    checkNcErr(m_err);
    m_err = nc_put_att_text(m_ncId, m_varHuId, "units",
                            strlen("meters"), "meters");
    checkNcErr(m_err);
    m_err = nc_put_att_text(m_ncId, m_varHvId, "units",
                            strlen("meters"), "meters");
    checkNcErr(m_err);

    m_err = nc_enddef(m_ncId); // ncid
    checkNcErr(m_err);

    // write data
    m_err = nc_put_var_float(m_ncId,                   // ncid
                             m_varBId,                 // varid
                             &m_bathymetryData[0][0]); // op
    checkNcErr(m_err);
}

tsunami_lab::io::NetCdf::~NetCdf()
{
    int m_err = nc_close(m_ncId);
    checkNcErr(m_err);
}

void tsunami_lab::io::NetCdf::write(t_real const *i_h,
                                    t_real const *i_hu,
                                    t_real const *i_hv,
                                    t_idx i_t)
{
    t_idx start[] = {i_t, 0, 0};
    t_idx count[] = {1, m_nx, m_ny};

    t_real *l_h = new t_real[(m_nx + 2) * (m_ny + 2)];
    t_real *l_hu = new t_real[(m_nx + 2) * (m_ny + 2)];
    t_real *l_hv = new t_real[(m_nx + 2) * (m_ny + 2)];
    int i=0;
    for (t_idx l_x = 0; l_x < m_nx + 1; l_x++)
    {
        for (t_idx l_y = 0; l_y < m_ny + 1; l_y++)
        {
            l_h[i] = i_h[l_x + l_y * m_stride];
            l_hu[i] = i_hu[l_x + l_y * m_stride];
            l_hv[i] = i_hv[l_x + l_y * m_stride];
            i++;
        }
    }

    m_err = nc_put_vara_float(m_ncId,
                              m_varHId,
                              start,
                              count,
                              l_h);
    checkNcErr(m_err);

    m_err = nc_put_vara_float(m_ncId,
                              m_varHuId,
                              start,
                              count,
                              l_h);
    checkNcErr(m_err);

    m_err = nc_put_vara_float(m_ncId,
                              m_varHvId,
                              start,
                              count,
                              l_h);
    checkNcErr(m_err);

    delete[] l_h;
    delete[] l_hu;
    delete[] l_hv;
}
