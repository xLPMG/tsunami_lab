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

tsunami_lab::io::NetCdf::NetCdf(const char *path,
                                t_idx i_nx,
                                t_idx i_ny,
                                t_idx i_stride)
{

    m_nx = i_nx;
    m_ny = i_ny;
    m_stride = i_stride;

    if (path == nullptr)
        return;

    m_err = nc_create(path,       // path
                      NC_CLOBBER, // cmode
                      &m_ncId);   // ncidp
    checkNcErr(m_err);

    t_real *m_x = new t_real[i_nx];
    t_real *m_y = new t_real[i_ny];

    // set x and y
    for (std::size_t l_ix = 0; l_ix < i_nx; l_ix++)
    {
        m_x[l_ix] = l_ix;
    }
    for (std::size_t l_iy = 0; l_iy < i_ny; l_iy++)
    {
        m_y[l_iy] = l_iy;
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
                            strlen("seconds since the earthquake event"),
                            "seconds since the earthquake event");
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

    m_err = nc_enddef(m_ncId); // ncid
    checkNcErr(m_err);

    // write data
    m_err = nc_put_var_float(m_ncId,   // ncid
                             m_varXId, // varid
                             &m_x[0]); // op
    checkNcErr(m_err);
    m_err = nc_put_var_float(m_ncId,   // ncid
                             m_varYId, // varid
                             &m_y[0]); // op
    checkNcErr(m_err);

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

    // write bathymetry on first call
    if (m_timeStepCount == 0)
    {
        t_real *l_b = new t_real[m_nx * m_ny];
        for (t_idx l_x = 0; l_x < m_nx; l_x++)
        {
            for (t_idx l_y = 0; l_y < m_ny; l_y++)
            {
                l_b[l_i] = i_b[l_x + l_y * m_stride];
                l_i++;
            }
        }

        m_err = nc_put_var_float(m_ncId,
                                 m_varBId,
                                 l_b);
        checkNcErr(m_err);
        delete[] l_b;
    }

    // write values
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

tsunami_lab::t_real *tsunami_lab::io::NetCdf::read(const char *i_file,
                                                   const char *i_var)
{
    std::cout << "Loading " << i_var << " from .nc file: " << i_file << std::endl;

    int l_ncIdRead = 0, l_varXIdRead = 0, l_varYIdRead = 0, l_varDataIdRead = 0;
    t_idx l_nx = 0, l_ny = 0;

    m_err = nc_open(i_file, NC_NOWRITE, &l_ncIdRead);
    checkNcErr(m_err);

    // get dimension ids
    m_err = nc_inq_dimid(l_ncIdRead, "x", &l_varXIdRead);
    checkNcErr(m_err);
    m_err = nc_inq_dimid(l_ncIdRead, "y", &l_varYIdRead);
    checkNcErr(m_err);
    // read dimension size
    m_err = nc_inq_dimlen(l_ncIdRead, l_varXIdRead, &l_nx);
    checkNcErr(m_err);
    m_err = nc_inq_dimlen(l_ncIdRead, l_varYIdRead, &l_ny);
    checkNcErr(m_err);
    // get var id of desired variable
    m_err = nc_inq_varid(l_ncIdRead, i_var, &l_varDataIdRead);
    checkNcErr(m_err);
    // read data
    t_real *l_data = new t_real[l_nx * l_ny];
    m_err = nc_get_var_float(l_ncIdRead, l_varDataIdRead, l_data);
    checkNcErr(m_err);

    m_err = nc_close(l_ncIdRead);
    checkNcErr(m_err);

    // choose smaller value to not reach out of bounds situations
    l_nx = std::min(l_nx, m_nx);
    l_ny = std::min(l_ny, m_ny);
    // convert to strided array
    t_real *l_stridedArray = new t_real[m_nx * m_ny]{0};
    int l_i = 0;
    for (std::size_t l_ix = 0; l_ix < l_nx; l_ix++)
    {
        for (std::size_t l_iy = 0; l_iy < l_ny; l_iy++)
        {
            l_stridedArray[l_ix + l_iy * l_nx] = l_data[l_i++];
        }
    }
    std::cout << "Done loading " << i_var << std::endl;
    return l_stridedArray;
}