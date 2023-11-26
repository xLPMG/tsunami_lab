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

void tsunami_lab::io::NetCdf::setUpFile(const char *path)
{
    m_err = nc_create(path,       // path
                      NC_CLOBBER, // cmode
                      &m_ncId);   // ncidp
    checkNcErr(m_err);
    m_outputFileOpened = true;

    t_real *l_x = new t_real[m_nx];
    t_real *l_y = new t_real[m_ny];
    for (t_idx l_ix = 0; l_ix < m_nx; l_ix++)
    {
        l_x[l_ix] = (l_ix + 0.5) * (m_simulationSizeX / m_nx) + m_offsetX;
    }
    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++)
    {
        l_y[l_iy] = (l_iy + 0.5) * (m_simulationSizeY / m_ny) + m_offsetY;
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
                       m_nx,       // len
                       &m_dimXId); // idp
    checkNcErr(m_err);

    m_err = nc_def_dim(m_ncId,     // ncid
                       "y",        // name
                       m_ny,       // len
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

    m_err = nc_put_att_text(m_ncId, m_varHuId, "units",
                            strlen("square meters per second"), "square meters per second");
    checkNcErr(m_err);
    m_err = nc_put_att_text(m_ncId, m_varHvId, "units",
                            strlen("square meters per second"), "square meters per second");
    checkNcErr(m_err);

    m_err = nc_enddef(m_ncId); // ncid
    checkNcErr(m_err);

    // write data
    m_err = nc_put_var_float(m_ncId,   // ncid
                             m_varXId, // varid
                             &l_x[0]); // op
    checkNcErr(m_err);
    m_err = nc_put_var_float(m_ncId,   // ncid
                             m_varYId, // varid
                             &l_y[0]); // op
    checkNcErr(m_err);

    delete[] l_x;
    delete[] l_y;
}

tsunami_lab::io::NetCdf::NetCdf(t_idx i_nx,
                                t_idx i_ny,
                                t_real i_simulationSizeX,
                                t_real i_simulationSizeY,
                                t_real i_offsetX,
                                t_real i_offsetY)
{
    m_nx = i_nx;
    m_ny = i_ny;
    m_simulationSizeX = i_simulationSizeX;
    m_simulationSizeY = i_simulationSizeY;
    m_offsetX = i_offsetX;
    m_offsetY = i_offsetY;
}

tsunami_lab::io::NetCdf::~NetCdf()
{
    if(m_outputFileOpened) checkNcErr(nc_close(m_ncId));
    m_outputFileOpened = false;
}

void tsunami_lab::io::NetCdf::write(const char *path,
                                    t_idx i_stride,
                                    t_real const *i_h,
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
            l_h[l_i] = i_h == nullptr ? 0 : i_h[l_x + l_y * i_stride];
            l_tH[l_i] = (i_h == nullptr ? 0 : i_h[l_x + l_y * i_stride]) + (i_b == nullptr ? 0 : i_b[l_x + l_y * i_stride]);
            l_hu[l_i] = i_hu == nullptr ? 0 : i_hu[l_x + l_y * i_stride];
            l_hv[l_i] = i_hv == nullptr ? 0 : i_hv[l_x + l_y * i_stride];
            l_i++;
        }
    }

    // set up file and write bathymetry on first call
    if (m_timeStepCount == 0)
    {
        setUpFile(path);

        t_real *l_b = new t_real[m_nx * m_ny]{0};
        int l_i = 0;
        for (t_idx l_x = 0; l_x < m_nx; l_x++)
        {
            for (t_idx l_y = 0; l_y < m_ny; l_y++)
            {
                l_b[l_i++] = i_b == nullptr ? 0 : i_b[l_x + l_y * i_stride];
            }
        }

        // we did not use m_err in this function in hopes of better performance
        checkNcErr(nc_put_var_float(m_ncId,
                                    m_varBId,
                                    l_b));
        delete[] l_b;
    }
    // write values
    checkNcErr(nc_put_var1_float(m_ncId,
                                 m_varTId,
                                 &m_timeStepCount,
                                 &i_t));

    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varHId,
                                 start,
                                 count,
                                 l_h));

    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varTHId,
                                 start,
                                 count,
                                 l_tH));

    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varHuId,
                                 start,
                                 count,
                                 l_hu));

    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varHvId,
                                 start,
                                 count,
                                 l_hv));

    m_timeStepCount++;

    delete[] l_h;
    delete[] l_tH;
    delete[] l_hu;
    delete[] l_hv;
}

void tsunami_lab::io::NetCdf::read(const char *i_file,
                                   const char *i_var,
                                   t_idx &o_nx,
                                   t_idx &o_ny,
                                   t_real **o_xData,
                                   t_real **o_yData,
                                   t_real **o_data)

{
    int l_ncIdRead = 0, l_varXIdRead = 0, l_varYIdRead = 0, l_varDataIdRead = 0;
    t_idx l_nx = 0, l_ny = 0;

    checkNcErr(nc_open(i_file, NC_NOWRITE, &l_ncIdRead));
    // get dimension ids
    checkNcErr(nc_inq_dimid(l_ncIdRead, "x", &l_varXIdRead));
    checkNcErr(nc_inq_dimid(l_ncIdRead, "y", &l_varYIdRead));
    // read dimension size
    checkNcErr(nc_inq_dimlen(l_ncIdRead, l_varXIdRead, &l_nx));
    checkNcErr(nc_inq_dimlen(l_ncIdRead, l_varYIdRead, &l_ny));
    // get var id of desired variable
    checkNcErr(nc_inq_varid(l_ncIdRead, i_var, &l_varDataIdRead));
    // read data
    t_real *l_xData = new t_real[l_nx];
    checkNcErr(nc_get_var_float(l_ncIdRead, l_varXIdRead, l_xData));

    t_real *l_yData = new t_real[l_ny];
    checkNcErr(nc_get_var_float(l_ncIdRead, l_varYIdRead, l_yData));

    t_real *l_data = new t_real[l_nx * l_ny];
    checkNcErr(nc_get_var_float(l_ncIdRead, l_varDataIdRead, l_data));

    checkNcErr(nc_close(l_ncIdRead));
    // convert to strided array
    t_real *l_stridedArray = new t_real[l_nx * l_ny];
    int l_i = 0;
    for (std::size_t l_ix = 0; l_ix < l_nx; l_ix++)
    {
        for (std::size_t l_iy = 0; l_iy < l_ny; l_iy++)
        {
            l_stridedArray[l_ix + l_iy * l_nx] = l_data[l_i++];
        }
    }

    // set other outputs
    o_nx = l_nx;
    o_ny = l_ny;
    *o_xData = l_xData;
    *o_yData = l_yData;
    *o_data = l_stridedArray;
}