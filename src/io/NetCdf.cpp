/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Interface for NetCdf
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

    t_real *m_bathymetryData = new t_real[i_nx * i_ny];
    t_real *m_x = new t_real[i_nx];
    t_real *m_y = new t_real[i_ny];

    // loop for bathymetry
    int i = 0;
    for (std::size_t l_ix = 0; l_ix < i_nx; l_ix++)
    {
        for (std::size_t l_iy = 0; l_iy < i_ny; l_iy++)
        {
            m_bathymetryData[i++] = i_b[l_ix + l_iy * i_stride];
            m_y[l_iy] = l_iy;
        }
        m_x[l_ix] = l_ix;
    }

    // define dimensions
    int m_dimIds[3];

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
    m_dimIds[0] = m_dimTId;
    m_dimIds[1] = m_dimXId;
    m_dimIds[2] = m_dimYId;

    m_err = nc_def_var(m_ncId,     // ncid
                       "time",     // name
                       NC_FLOAT,   // xtype
                       1,          // ndims
                       &m_dimTId,  // dimidsp
                       &m_varTId); // varidp
    checkNcErr(m_err);
    m_err = nc_def_var(m_ncId,     // ncid
                       "x",        // name
                       NC_FLOAT,   // xtype
                       1,          // ndims
                       &m_dimXId,  // dimidsp
                       &m_varXId); // varidp
    checkNcErr(m_err);
    m_err = nc_def_var(m_ncId,     // ncid
                       "y",        // name
                       NC_FLOAT,   // xtype
                       1,          // ndims
                       &m_dimYId,  // dimidsp
                       &m_varYId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncId,     // ncid
                       "height",   // name
                       NC_FLOAT,   // xtype
                       3,          // ndims
                       m_dimIds,   // dimidsp
                       &m_varHId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncId,        // ncid
                       "totalHeight", // name
                       NC_FLOAT,      // xtype
                       3,             // ndims
                       m_dimIds,      // dimidsp
                       &m_varTHId);   // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncId,       // ncid
                       "bathymetry", // name
                       NC_FLOAT,     // xtype
                       2,            // ndims
                       m_dimIds + 1, // dimidsp
                       &m_varBId);   // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncId,      // ncid
                       "momentumX", // name
                       NC_FLOAT,    // xtype
                       3,           // ndims
                       m_dimIds,    // dimidsp
                       &m_varHuId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncId,      // ncid
                       "momentumY", // name
                       NC_FLOAT,    // xtype
                       3,           // ndims
                       m_dimIds,    // dimidsp
                       &m_varHvId); // varidp
    checkNcErr(m_err);

    // assign attributes
    m_err = nc_put_att_text(m_ncId, m_varTId, "units",
                            strlen("seconds"), "seconds");
    checkNcErr(m_err);

    m_err = nc_put_att_text(m_ncId, m_varXId, "units",
                            strlen("meters"), "meters");
    checkNcErr(m_err);
    m_err = nc_put_att_text(m_ncId, m_varXId, "axis",
                            strlen("X"), "X");
    checkNcErr(m_err);

    m_err = nc_put_att_text(m_ncId, m_varYId, "units",
                            strlen("meters"), "meters");
    checkNcErr(m_err);
    m_err = nc_put_att_text(m_ncId, m_varYId, "axis",
                            strlen("Y"), "Y");
    checkNcErr(m_err);

    m_err = nc_put_att_text(m_ncId, m_varHId, "units",
                            strlen("meters"), "meters");
    checkNcErr(m_err);
    m_err = nc_put_att_text(m_ncId, m_varTHId, "units",
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
    m_err = nc_put_var_float(m_ncId,            // ncid
                             m_varBId,          // varid
                             m_bathymetryData); // op
    checkNcErr(m_err);

    m_err = nc_put_var_float(m_ncId,
                             m_varXId,
                             &m_x[0]);
    checkNcErr(m_err);
    m_err = nc_put_var_float(m_ncId,
                             m_varYId,
                             &m_y[0]);
    checkNcErr(m_err);

    delete[] m_bathymetryData;
    delete[] m_x;
    delete[] m_y;
}

tsunami_lab::io::NetCdf::~NetCdf()
{
    int m_err = nc_close(m_ncId);
    checkNcErr(m_err);
}

void tsunami_lab::io::NetCdf::write(t_real const *i_h,
                                    t_real const *i_hu,
                                    t_real const *i_hv,
                                    t_real const *i_b,
                                    t_real i_t)
{
    t_idx start[] = {m_timeStepCount, 0, 0};
    t_idx count[] = {1, m_nx, m_ny};

    t_real *l_h = new t_real[m_nx * m_ny];
    t_real *l_tH = new t_real[m_nx * m_ny];
    t_real *l_hu = new t_real[m_nx * m_ny];
    t_real *l_hv = new t_real[m_nx * m_ny];
    int l_i = 0;
    for (t_idx l_x = 0; l_x < m_nx; l_x++)
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            l_h[l_i] = i_h[l_x + l_y * m_stride];
            l_tH[l_i] = i_h[l_x + l_y * m_stride] + i_b[l_x + l_y * m_stride];
            l_hu[l_i] = i_hu[l_x + l_y * m_stride];
            l_hv[l_i] = i_hv[l_x + l_y * m_stride];
            l_i++;
        }
    }
    m_err = nc_put_var1_float(m_ncId,
                              m_varTId,
                              &m_timeStepCount,
                              &i_t);
    checkNcErr(m_err);

    m_err = nc_put_vara_float(m_ncId,
                              m_varHId,
                              start,
                              count,
                              l_h);
    checkNcErr(m_err);
    m_err = nc_put_vara_float(m_ncId,
                              m_varTHId,
                              start,
                              count,
                              l_tH);
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

    m_timeStepCount++;

    delete[] l_h;
    delete[] l_tH;
    delete[] l_hu;
    delete[] l_hv;
}
