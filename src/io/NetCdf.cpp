/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 * Interface for NetCdf
 **/
#include "NetCdf.h"
#include <iostream>
#include <netcdf.h>
#include <filesystem>

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
        l_x[l_ix] = l_ix * (m_simulationSizeX / m_nx) + m_offsetX;
    }
    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++)
    {
        l_y[l_iy] = l_iy * (m_simulationSizeY / m_ny) + m_offsetY;
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
                             l_x);     // op
    checkNcErr(m_err);
    m_err = nc_put_var_float(m_ncId,   // ncid
                             m_varYId, // varid
                             l_y);     // op
    checkNcErr(m_err);

    delete[] l_x;
    delete[] l_y;
}

void tsunami_lab::io::NetCdf::setUpCheckpointFile(const char *path)
{
    if (std::filesystem::exists(path))
        std::filesystem::remove(path);

    m_err = nc_create(path,          // path
                      NC_CLOBBER,    // cmode
                      &m_ncCheckId); // ncidp
    checkNcErr(m_err);

    // define dimensions
    int m_dimCheckIds[2];

    m_err = nc_def_dim(m_ncCheckId,     // ncid
                       "x",             // name
                       m_nx,            // len
                       &m_dimCheckXId); // idp
    checkNcErr(m_err);

    m_err = nc_def_dim(m_ncCheckId,     // ncid
                       "y",             // name
                       m_ny,            // len
                       &m_dimCheckYId); // idp
    checkNcErr(m_err);

    // define variables
    m_dimCheckIds[0] = m_dimCheckXId;
    m_dimCheckIds[1] = m_dimCheckYId;

    m_err = nc_def_var(m_ncCheckId,     // ncid
                       "height",        // name
                       NC_FLOAT,        // xtype
                       2,               // ndims
                       m_dimCheckIds,   // dimidsp
                       &m_varCheckHId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncCheckId,      // ncid
                       "momentumX",      // name
                       NC_FLOAT,         // xtype
                       2,                // ndims
                       m_dimCheckIds,    // dimidsp
                       &m_varCheckHuId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncCheckId,      // ncid
                       "momentumY",      // name
                       NC_FLOAT,         // xtype
                       2,                // ndims
                       m_dimCheckIds,    // dimidsp
                       &m_varCheckHvId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncCheckId,     // ncid
                       "time",          // name
                       NC_FLOAT,        // xtype
                       0,               // ndims
                       m_dimCheckIds,   // dimidsp
                       &m_varCheckTId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncCheckId,            // ncid
                       "timeStep",             // name
                       NC_FLOAT,               // xtype
                       0,                      // ndims
                       m_dimCheckIds,          // dimidsp
                       &m_varCheckTimeStepId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncCheckId,                 // ncid
                       "timeStepCount",             // name
                       NC_FLOAT,                    // xtype
                       0,                           // ndims
                       m_dimCheckIds,               // dimidsp
                       &m_varCheckTimeStepCountId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncCheckId,            // ncid
                       "simulationSizeX",      // name
                       NC_FLOAT,               // xtype
                       0,                      // ndims
                       m_dimCheckIds,          // dimidsp
                       &m_varCheckSimSizeXId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncCheckId,            // ncid
                       "simulationSizeY",      // name
                       NC_FLOAT,               // xtype
                       0,                      // ndims
                       m_dimCheckIds,          // dimidsp
                       &m_varCheckSimSizeYId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncCheckId,           // ncid
                       "offsetX",             // name
                       NC_FLOAT,              // xtype
                       0,                     // ndims
                       m_dimCheckIds,         // dimidsp
                       &m_varCheckOffsetXId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncCheckId,           // ncid
                       "offsetY",             // name
                       NC_FLOAT,              // xtype
                       0,                     // ndims
                       m_dimCheckIds,         // dimidsp
                       &m_varCheckOffsetYId); // varidp
    checkNcErr(m_err);

    m_err = nc_enddef(m_ncCheckId); // ncid
    checkNcErr(m_err);

    t_real *l_x = new t_real[m_nx];
    t_real *l_y = new t_real[m_ny];
    for (t_idx l_ix = 0; l_ix < m_nx; l_ix++)
    {
        l_x[l_ix] = l_ix;
    }
    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++)
    {
        l_y[l_iy] = l_iy;
    }
    // write data
    m_err = nc_put_var_float(m_ncCheckId,   // ncid
                             m_varCheckXId, // varid
                             l_x);          // op
    checkNcErr(m_err);
    m_err = nc_put_var_float(m_ncCheckId,   // ncid
                             m_varCheckYId, // varid
                             l_y);          // op
    checkNcErr(m_err);

    delete[] l_x;
    delete[] l_y;
}

tsunami_lab::io::NetCdf::NetCdf(t_idx i_nx,
                                t_idx i_ny,
                                t_real i_simulationSizeX,
                                t_real i_simulationSizeY,
                                t_real i_offsetX,
                                t_real i_offsetY,
                                const char *i_checkpointFile,
                                const char *i_path)
{
    m_nx = i_nx;
    m_ny = i_ny;
    m_simulationSizeX = i_simulationSizeX;
    m_simulationSizeY = i_simulationSizeY;
    m_offsetX = i_offsetX;
    m_offsetY = i_offsetY;
    m_path = i_path;
    m_checkpointFile = i_checkpointFile;
}

tsunami_lab::io::NetCdf::NetCdf(const char *i_path,
                                const char *i_checkpointFile)
{
    m_path = i_path;
}

tsunami_lab::io::NetCdf::~NetCdf()
{
    if (m_outputFileOpened)
        checkNcErr(nc_close(m_ncId));
    m_outputFileOpened = false;
}

void tsunami_lab::io::NetCdf::write(t_idx i_stride,
                                    t_real const *i_h,
                                    t_real const *i_hu,
                                    t_real const *i_hv,
                                    t_real const *i_b,
                                    t_real i_t)
{
    t_idx start[] = {m_timeStepCount, 0, 0};
    t_idx count[] = {1, m_nx, m_ny};

    t_real *l_data = new t_real[m_nx * m_ny];
    int l_i = 0;

    // set up file and write bathymetry on first call
    if (m_timeStepCount == 0)
    {
        setUpFile(m_path);
        t_real *l_b = new t_real[m_nx * m_ny];
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
    // WRITE TIME
    checkNcErr(nc_put_var1_float(m_ncId,
                                 m_varTId,
                                 &m_timeStepCount,
                                 &i_t));

    // WRITE HEIGHT
    l_i = 0;
    for (t_idx l_x = 0; l_x < m_nx; l_x++)
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            l_data[l_i++] = i_h == nullptr ? 0 : i_h[l_x + l_y * i_stride];
        }
    }
    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varHId,
                                 start,
                                 count,
                                 l_data));
    // WRITE TOTAL HEIGHT
    l_i = 0;
    for (t_idx l_x = 0; l_x < m_nx; l_x++)
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            l_data[l_i++] = (i_h == nullptr ? 0 : i_h[l_x + l_y * i_stride]) + (i_b == nullptr ? 0 : i_b[l_x + l_y * i_stride]);
        }
    }
    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varTHId,
                                 start,
                                 count,
                                 l_data));
    // WRITE MOMENTUM X
    l_i = 0;
    for (t_idx l_x = 0; l_x < m_nx; l_x++)
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            l_data[l_i++] = i_hu == nullptr ? 0 : i_hu[l_x + l_y * i_stride];
        }
    }
    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varHuId,
                                 start,
                                 count,
                                 l_data));

    // WRITE MOMENTUM Y
    l_i = 0;
    for (t_idx l_x = 0; l_x < m_nx; l_x++)
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            l_data[l_i++] = i_hv == nullptr ? 0 : i_hv[l_x + l_y * i_stride];
        }
    }
    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varHvId,
                                 start,
                                 count,
                                 l_data));

    m_timeStepCount++;

    delete[] l_data;
}

void tsunami_lab::io::NetCdf::getDimensionSize(const char *i_file,
                                               t_idx &o_n,
                                               const char *i_dimName)
{
    int l_ncIdRead = 0, l_varRead = 0;
    checkNcErr(nc_open(i_file, NC_NOWRITE, &l_ncIdRead));
    // get dimension id
    checkNcErr(nc_inq_dimid(l_ncIdRead, i_dimName, &l_varRead));
    // read dimension size
    checkNcErr(nc_inq_dimlen(l_ncIdRead, l_varRead, &o_n));

    checkNcErr(nc_close(l_ncIdRead));
}

void tsunami_lab::io::NetCdf::read(const char *i_file,
                                   const char *i_var,
                                   t_real **o_xData,
                                   t_real **o_yData,
                                   t_real **o_data)

{
    int l_ncIdRead = 0, l_varXIdRead = 0, l_varYIdRead = 0, l_varDataIdRead = 0;
    std::size_t l_nx = 0, l_ny = 0;
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
    checkNcErr(nc_get_var_float(l_ncIdRead, l_varXIdRead, *o_xData));

    checkNcErr(nc_get_var_float(l_ncIdRead, l_varYIdRead, *o_yData));

    t_real *l_data = new t_real[l_nx * l_ny];
    checkNcErr(nc_get_var_float(l_ncIdRead, l_varDataIdRead, l_data));

    checkNcErr(nc_close(l_ncIdRead));

    // convert to strided array
    int l_i = 0;
    for (std::size_t l_iy = 0; l_iy < l_ny; l_iy++)
    {
        for (std::size_t l_ix = 0; l_ix < l_nx; l_ix++)
        {
            *(*o_data + (l_ix + l_nx * l_iy)) = l_data[l_i++];
        }
    }
    delete[] l_data;
}

void tsunami_lab::io::NetCdf::writeCheckpoint(const char *path,
                                              t_real const *i_h,
                                              t_real const *i_hu,
                                              t_real const *i_hv,
                                              t_real i_t,
                                              t_real i_timeStep)
{
    setUpCheckpointFile(path);
    
    t_idx start[] = {0, 0, 0};
    t_idx count[] = {1, m_nx, m_ny};
    int l_i = 0;
    t_real *l_data = new t_real[m_nx * m_ny];
    // WRITE SIMULATION SIZE X
    checkNcErr(nc_put_vara_float(m_ncCheckId, m_varCheckSimSizeXId, start, count, &m_simulationSizeX));
    // WRITE SIMULATION SIZE Y
    checkNcErr(nc_put_vara_float(m_ncCheckId, m_varCheckSimSizeYId, start, count, &m_simulationSizeY));
    // WRITE OFFSET X
    checkNcErr(nc_put_vara_float(m_ncCheckId, m_varCheckOffsetXId, start, count, &m_offsetX));
    // WRITE OFFSET Y
    checkNcErr(nc_put_vara_float(m_ncCheckId, m_varCheckOffsetYId, start, count, &m_offsetY));
    // WRITE TIME STEP COUNT
    float l_timeStepFloat = float(m_timeStepCount);
    checkNcErr(nc_put_vara_float(m_ncCheckId, m_varCheckTimeStepCountId, start, count, &l_timeStepFloat));


    // WRITE TIME STEP
    checkNcErr(nc_put_vara_float(m_ncCheckId, m_varCheckTimeStepId, start, count, &i_timeStep));
    // WRITE TIME
    checkNcErr(nc_put_vara_float(m_ncCheckId, m_varCheckTId, start, count, &i_t));

    // WRITE HEIGHT
    l_i = 0;
    for (t_idx l_x = 0; l_x < m_nx; l_x++)
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            l_data[l_i++] = i_h == nullptr ? 0 : i_h[l_x + l_y * m_nx];
        }
    }
    checkNcErr(nc_put_vara_float(m_ncCheckId,
                                 m_varCheckHId,
                                 start,
                                 count,
                                 l_data));
    // WRITE MOMENTUM X
    l_i = 0;
    for (t_idx l_x = 0; l_x < m_nx; l_x++)
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            l_data[l_i++] = i_hu == nullptr ? 0 : i_hu[l_x + l_y * m_nx];
        }
    }
    checkNcErr(nc_put_vara_float(m_ncCheckId,
                                 m_varCheckHuId,
                                 start,
                                 count,
                                 l_data));

    // WRITE MOMENTUM Y
    l_i = 0;
    for (t_idx l_x = 0; l_x < m_nx; l_x++)
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            l_data[l_i++] = i_hv == nullptr ? 0 : i_hv[l_x + l_y * m_nx];
        }
    }
    checkNcErr(nc_put_vara_float(m_ncCheckId,
                                 m_varCheckHvId,
                                 start,
                                 count,
                                 l_data));

    delete[] l_data;
    checkNcErr(nc_close(m_ncCheckId));
}

void tsunami_lab::io::NetCdf::loadCheckpoint(const char *path,
                                             t_real **o_h,
                                             t_real **o_hu,
                                             t_real **o_hv,
                                             t_idx &o_nx,
                                             t_idx &o_ny,
                                             t_real &o_simulationSizeX,
                                             t_real &o_simulationSizeY,
                                             t_real &o_offsetX,
                                             t_real &o_offsetY,
                                             t_real &o_t,
                                             t_real &o_timeStep)
{
    int l_ncIdCheckRead = 0, l_dimCheckXIdRead = 0, l_dimCheckYIdRead = 0, l_varDataIdRead = 0, l_i = 0;
    checkNcErr(nc_open(path, NC_NOWRITE, &l_ncIdCheckRead));
    // get dimension ids
    checkNcErr(nc_inq_dimid(l_ncIdCheckRead, "x", &l_dimCheckXIdRead));
    checkNcErr(nc_inq_dimid(l_ncIdCheckRead, "y", &l_dimCheckYIdRead));
    // read dimension size
    checkNcErr(nc_inq_dimlen(l_ncIdCheckRead, l_dimCheckXIdRead, &m_nx));
    checkNcErr(nc_inq_dimlen(l_ncIdCheckRead, l_dimCheckYIdRead, &m_ny));
    o_nx = m_nx;
    o_ny = m_ny;
    // READ VARIABLES : //
    t_real *l_data = new t_real[m_nx * m_ny];

    // READ HEIGHT
    checkNcErr(nc_inq_varid(l_varDataIdRead, "height", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_varDataIdRead, l_varDataIdRead, l_data));
    l_i = 0;
    for (std::size_t l_iy = 0; l_iy < m_ny; l_iy++)
    {
        for (std::size_t l_ix = 0; l_ix < m_nx; l_ix++)
        {
            *(*o_h + (l_ix + m_nx * l_iy)) = l_data[l_i++];
        }
    }
    // READ MOMENTUM X
    checkNcErr(nc_inq_varid(l_varDataIdRead, "momentumX", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_varDataIdRead, l_varDataIdRead, l_data));
    l_i = 0;
    for (std::size_t l_iy = 0; l_iy < m_ny; l_iy++)
    {
        for (std::size_t l_ix = 0; l_ix < m_nx; l_ix++)
        {
            *(*o_hu + (l_ix + m_nx * l_iy)) = l_data[l_i++];
        }
    }

    // READ MOMENTUM Y
    checkNcErr(nc_inq_varid(l_varDataIdRead, "momentumY", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_varDataIdRead, l_varDataIdRead, l_data));
    l_i = 0;
    for (std::size_t l_iy = 0; l_iy < m_ny; l_iy++)
    {
        for (std::size_t l_ix = 0; l_ix < m_nx; l_ix++)
        {
            *(*o_hv + (l_ix + m_nx * l_iy)) = l_data[l_i++];
        }
    }
    // READ SIMULATION SIZE X
    checkNcErr(nc_inq_varid(l_varDataIdRead, "simulationSizeX", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_varDataIdRead, l_varDataIdRead, &m_simulationSizeX));
    o_simulationSizeX = m_simulationSizeX;

    // READ SIMULATION SIZE Y
    checkNcErr(nc_inq_varid(l_varDataIdRead, "simulationSizeY", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_varDataIdRead, l_varDataIdRead, &m_simulationSizeY));
    o_simulationSizeY = m_simulationSizeY;

    // READ OFFSET X
    checkNcErr(nc_inq_varid(l_varDataIdRead, "offsetX", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_varDataIdRead, l_varDataIdRead, &m_offsetX));
    o_offsetX = m_offsetX;

    // READ OFFSET Y
    checkNcErr(nc_inq_varid(l_varDataIdRead, "offsetY", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_varDataIdRead, l_varDataIdRead, &m_offsetY));
    o_offsetY = m_offsetY;

    // READ TIME
    checkNcErr(nc_inq_varid(l_varDataIdRead, "time", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_varDataIdRead, l_varDataIdRead, &o_t));

    // READ TIME STEP
    checkNcErr(nc_inq_varid(l_varDataIdRead, "timeStep", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_varDataIdRead, l_varDataIdRead, &o_timeStep));

    // READ TIME STEP COUNT
    checkNcErr(nc_inq_varid(l_varDataIdRead, "timeStepCount", &l_varDataIdRead));
    checkNcErr(nc_get_var_int(l_varDataIdRead, l_varDataIdRead, &l_i));
    m_timeStepCount = size_t(l_i);

    delete[] l_data;
    checkNcErr(nc_close(l_ncIdCheckRead));

    // set up netcdf file
    if (m_outputFileOpened)
    {
        checkNcErr(nc_close(m_ncId));
    }
    m_err = nc_create(m_checkpointFile, // path
                      NC_CLOBBER,       // cmode
                      &m_ncId);         // ncidp
    checkNcErr(m_err);
    m_outputFileOpened = true;
    checkNcErr(nc_inq_dimid(m_ncId, "x", &m_dimXId));
    checkNcErr(nc_inq_dimid(m_ncId, "y", &m_dimYId));
    checkNcErr(nc_inq_dimid(m_ncId, "time", &m_dimTId));
    checkNcErr(nc_inq_varid(m_ncId, "height", &m_varHId));
    checkNcErr(nc_inq_varid(m_ncId, "totalHeight", &m_varTHId));
    checkNcErr(nc_inq_varid(m_ncId, "bathymetry", &m_varBId));
    checkNcErr(nc_inq_varid(m_ncId, "momentumX", &m_varHuId));
    checkNcErr(nc_inq_varid(m_ncId, "momentumY", &m_varHvId));
}