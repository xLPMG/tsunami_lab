/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Interface for NetCdf
 **/
#include "NetCdf.h"
#include <iostream>
#include <netcdf.h>
#ifndef BENCHMARK
#include <filesystem>
#endif

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

void tsunami_lab::io::NetCdf::setUpFile(const char *i_file)
{
    if (m_doesSolutionExist)
    {
        m_err = nc_open(i_file,   // path
                        NC_WRITE, // cmode
                        &m_ncId); // ncidp
        checkNcErr(m_err);
        m_outputFileOpened = true;

        int l_dimensions = 0;
        int l_variables = 0;
        int l_attributes = 0;
        checkNcErr(nc_inq(m_ncId, &l_dimensions, &l_variables, &l_attributes, &m_dimTId));

        if (l_dimensions != 3)
            std::cerr << "Error in " << i_file << " file. Dimension size is invalid." << std::endl;
        if (l_variables != 8)
            std::cerr << "Error in " << i_file << " file. Variable size is invalid." << std::endl;

        checkNcErr(nc_inq_dimid(m_ncId, "x", &m_dimXId));
        checkNcErr(nc_inq_dimid(m_ncId, "y", &m_dimYId));
        checkNcErr(nc_inq_dimid(m_ncId, "time", &m_dimTId));
        checkNcErr(nc_inq_varid(m_ncId, "height", &m_varHId));
        checkNcErr(nc_inq_varid(m_ncId, "totalHeight", &m_varTHId));
        checkNcErr(nc_inq_varid(m_ncId, "bathymetry", &m_varBId));
        checkNcErr(nc_inq_varid(m_ncId, "momentumX", &m_varHuId));
        checkNcErr(nc_inq_varid(m_ncId, "momentumY", &m_varHvId));
    }
    else
    {
        m_err = nc_create(i_file,                       // path
                          NC_CLOBBER | NC_64BIT_OFFSET, // cmode
                          &m_ncId);                     // ncidp
        checkNcErr(m_err);
        m_outputFileOpened = true;
        int l_i = 0;
        t_real *l_y = new t_real[m_nky]{0};
        t_real *l_x = new t_real[m_nkx]{0};
        t_real l_averagingFactor = 1 / m_k;
        for (t_idx l_gy = 0; l_gy < m_ny; l_gy += m_k)
        {
            for (t_idx l_iy = 0; l_iy < m_k; l_iy++)
            {
                l_y[l_i] += (l_gy + l_iy) * (m_simulationSizeY / m_ny) + m_offsetY;
            }
            l_y[l_i] *= l_averagingFactor;
            l_i++;
        }
        l_i = 0;
        for (t_idx l_gx = 0; l_gx < m_nx; l_gx += m_k)
        {
            for (t_idx l_ix = 0; l_ix < m_k; l_ix++)
            {
                l_x[l_i] += (l_gx + l_ix) * (m_simulationSizeX / m_nx) + m_offsetX;
            }
            l_x[l_i] *= l_averagingFactor;
            l_i++;
        }
        // define dimensions
        int m_dimIds[3];

        m_err = nc_def_dim(m_ncId,       // ncid
                           "time",       // name
                           NC_UNLIMITED, // len
                           &m_dimTId);   // idp
        checkNcErr(m_err);

        m_err = nc_def_dim(m_ncId,     // ncid
                           "y",        // name
                           m_nky,      // len
                           &m_dimYId); // idp
        checkNcErr(m_err);

        m_err = nc_def_dim(m_ncId,     // ncid
                           "x",        // name
                           m_nkx,      // len
                           &m_dimXId); // idp
        checkNcErr(m_err);

        // define variables
        m_dimIds[0] = m_dimTId;
        m_dimIds[1] = m_dimYId;
        m_dimIds[2] = m_dimXId;

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

        m_err = nc_put_att_text(m_ncId, m_varYId, "units",
                                strlen("meters"), "meters");
        checkNcErr(m_err);
        m_err = nc_put_att_text(m_ncId, m_varYId, "axis",
                                strlen("Y"), "Y");
        checkNcErr(m_err);

        m_err = nc_put_att_text(m_ncId, m_varXId, "units",
                                strlen("meters"), "meters");
        checkNcErr(m_err);
        m_err = nc_put_att_text(m_ncId, m_varXId, "axis",
                                strlen("X"), "X");
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
                                 m_varYId, // varid
                                 l_y);     // op
        checkNcErr(m_err);

        m_err = nc_put_var_float(m_ncId,   // ncid
                                 m_varXId, // varid
                                 l_x);     // op
        checkNcErr(m_err);

        delete[] l_y;
        delete[] l_x;
    }
}

void tsunami_lab::io::NetCdf::setUpCheckpointFile(const char *i_checkpointFile)
{
#ifndef BENCHMARK
    if (std::filesystem::exists(i_checkpointFile))
        std::filesystem::remove(i_checkpointFile);
#endif
    m_err = nc_create(i_checkpointFile,             // path
                      NC_CLOBBER | NC_64BIT_OFFSET, // cmode
                      &m_ncCheckId);                // ncidp
    checkNcErr(m_err);

    // define dimensions
    int m_dimCheckIds[2];

    m_err = nc_def_dim(m_ncCheckId,     // ncid
                       "y",             // name
                       m_ny,            // len
                       &m_dimCheckYId); // idp
    checkNcErr(m_err);

    m_err = nc_def_dim(m_ncCheckId,     // ncid
                       "x",             // name
                       m_nx,            // len
                       &m_dimCheckXId); // idp
    checkNcErr(m_err);

    // define variables
    m_dimCheckIds[0] = m_dimCheckYId;
    m_dimCheckIds[1] = m_dimCheckXId;

    m_err = nc_def_var(m_ncCheckId,     // ncid
                       "x",             // name
                       NC_FLOAT,        // xtype
                       1,               // ndims
                       &m_dimCheckXId,  // dimidsp
                       &m_varCheckXId); // varidp
    checkNcErr(m_err);

    m_err = nc_def_var(m_ncCheckId,     // ncid
                       "y",             // name
                       NC_FLOAT,        // xtype
                       1,               // ndims
                       &m_dimCheckYId,  // dimidsp
                       &m_varCheckYId); // varidp
    checkNcErr(m_err);

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
                       "bathymetry",    // name
                       NC_FLOAT,        // xtype
                       2,               // ndims
                       m_dimCheckIds,   // dimidsp
                       &m_varCheckBId); // varidp
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

    m_err = nc_def_var(m_ncCheckId,                     // ncid
                       "writingStepsCount",             // name
                       NC_FLOAT,                        // xtype
                       0,                               // ndims
                       m_dimCheckIds,                   // dimidsp
                       &m_varCheckWritingStepsCountId); // varidp
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

    m_err = nc_def_var(m_ncCheckId,     // ncid
                       "k",             // name
                       NC_INT,          // xtype
                       0,               // ndims
                       m_dimCheckIds,   // dimidsp
                       &m_varCheckKId); // varidp
    checkNcErr(m_err);

    m_err = nc_enddef(m_ncCheckId); // ncid
    checkNcErr(m_err);
    t_real *l_y = new t_real[m_ny];
    t_real *l_x = new t_real[m_nx];

    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++)
    {
        l_y[l_iy] = l_iy;
    }
    for (t_idx l_ix = 0; l_ix < m_nx; l_ix++)
    {
        l_x[l_ix] = l_ix;
    }

    // write data
    m_err = nc_put_var_float(m_ncCheckId,   // ncid
                             m_varCheckYId, // varid
                             l_y);          // op
    checkNcErr(m_err);
    m_err = nc_put_var_float(m_ncCheckId,   // ncid
                             m_varCheckXId, // varid
                             l_x);          // op
    checkNcErr(m_err);

    delete[] l_y;
    delete[] l_x;
}

tsunami_lab::io::NetCdf::NetCdf(t_idx i_nx,
                                t_idx i_ny,
                                t_idx i_nk,
                                t_real i_simulationSizeX,
                                t_real i_simulationSizeY,
                                t_real i_offsetX,
                                t_real i_offsetY,
                                const char *i_netcdfOutputFile,
                                const char *i_checkpointFile)
{
    m_nx = i_nx;
    m_ny = i_ny;
    m_k = i_nk;
    m_nkx = i_nx / i_nk;
    m_nky = i_ny / i_nk;
    m_simulationSizeX = i_simulationSizeX;
    m_simulationSizeY = i_simulationSizeY;
    m_offsetX = i_offsetX;
    m_offsetY = i_offsetY;
    m_netcdfOutputFile = i_netcdfOutputFile;
    m_checkpointFile = i_checkpointFile;
#ifndef BENCHMARK
    m_doesSolutionExist = std::filesystem::exists(i_netcdfOutputFile);
#endif
}

tsunami_lab::io::NetCdf::NetCdf(const char *i_netcdfOutputFile,
                                const char *i_checkpointFile)
{
    m_netcdfOutputFile = i_netcdfOutputFile;
    m_checkpointFile = i_checkpointFile;

    // those parameters are only needed to satisfy the function call parameters
    t_real l_t = 0;
    t_idx l_timeStep = 0;

    loadCheckpointDimensions(i_checkpointFile,
                             m_nx,
                             m_ny,
                             m_k,
                             m_simulationSizeX,
                             m_simulationSizeY,
                             m_offsetX,
                             m_offsetY,
                             l_t,
                             l_timeStep);

    m_nkx = m_nx / m_k;
    m_nky = m_ny / m_k;
#ifndef BENCHMARK
    m_doesSolutionExist = std::filesystem::exists(i_netcdfOutputFile);
#endif
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
    t_idx start[] = {m_writingStepsCount, 0, 0};
    t_idx count[] = {1, m_nky, m_nkx};

    t_real *l_data = new t_real[m_nkx * m_nky];
    int l_i = 0;

    t_real l_averagingFactor = 1 / m_k * m_k;

    // set up file and write bathymetry on first call
    if (!m_outputFileOpened)
    {
        setUpFile(m_netcdfOutputFile);
    }
    if (m_writingStepsCount == 0)
    {
        t_real *l_b = new t_real[m_nkx * m_nky]{0};
        l_i = 0;
        if (i_b != nullptr)
        {
            for (t_idx l_gy = 0; l_gy < m_ny; l_gy += m_k)
            {
                for (t_idx l_gx = 0; l_gx < m_nx; l_gx += m_k)
                {
                    for (t_idx l_y = 0; l_y < m_k; l_y++)
                    {
                        for (t_idx l_x = 0; l_x < m_k; l_x++)
                        {
                            l_b[l_i] += i_b[l_gx + l_x + (l_y + l_gy) * i_stride];
                        }
                    }
                    l_b[l_i] *= l_averagingFactor;
                    l_i++;
                }
            }
        }
        checkNcErr(nc_put_var_float(m_ncId,
                                    m_varBId,
                                    l_b));
        delete[] l_b;
    }
    // WRITE TIME
    checkNcErr(nc_put_var1_float(m_ncId,
                                 m_varTId,
                                 &m_writingStepsCount,
                                 &i_t));
    // WRITE HEIGHT
    l_i = 0;
    memset(l_data, 0, m_nkx * m_nky * sizeof(t_real));
    if (i_h != nullptr)
    {
        int l_i = 0;
        for (t_idx l_gy = 0; l_gy < m_ny; l_gy += m_k)
        {
            for (t_idx l_gx = 0; l_gx < m_nx; l_gx += m_k)
            {
                for (t_idx l_y = 0; l_y < m_k; l_y++)
                {
                    for (t_idx l_x = 0; l_x < m_k; l_x++)
                    {
                        l_data[l_i] += i_h[l_gx + l_x + (l_y + l_gy) * i_stride];
                    }
                }
                l_data[l_i] *= l_averagingFactor;
                l_i++;
            }
        }
    }
    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varHId,
                                 start,
                                 count,
                                 l_data));
    // WRITE TOTAL HEIGHT
    l_i = 0;
    memset(l_data, 0, m_nkx * m_nky * sizeof(t_real));
    if (i_h != nullptr && i_b != nullptr)
    {
        int l_i = 0;
        for (t_idx l_gy = 0; l_gy < m_ny; l_gy += m_k)
        {
            for (t_idx l_gx = 0; l_gx < m_nx; l_gx += m_k)
            {
                for (t_idx l_y = 0; l_y < m_k; l_y++)
                {
                    for (t_idx l_x = 0; l_x < m_k; l_x++)
                    {
                        l_data[l_i] += i_h[l_gx + l_x + (l_y + l_gy) * i_stride] + i_b[l_gx + l_x + (l_y + l_gy) * i_stride];
                    }
                }
                l_data[l_i] *= l_averagingFactor;
                l_i++;
            }
        }
    }
    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varTHId,
                                 start,
                                 count,
                                 l_data));
    // WRITE MOMENTUM X
    l_i = 0;
    memset(l_data, 0, m_nkx * m_nky * sizeof(t_real));
    if (i_hu != nullptr)
    {
        int l_i = 0;
        for (t_idx l_gy = 0; l_gy < m_ny; l_gy += m_k)
        {
            for (t_idx l_gx = 0; l_gx < m_nx; l_gx += m_k)
            {
                for (t_idx l_y = 0; l_y < m_k; l_y++)
                {
                    for (t_idx l_x = 0; l_x < m_k; l_x++)
                    {
                        l_data[l_i] += i_hu[l_gx + l_x + (l_y + l_gy) * i_stride];
                    }
                }
                l_data[l_i] *= l_averagingFactor;
                l_i++;
            }
        }
    }
    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varHuId,
                                 start,
                                 count,
                                 l_data));

    // WRITE MOMENTUM Y
    l_i = 0;
    memset(l_data, 0, m_nkx * m_nky * sizeof(t_real));
    if (i_hv != nullptr)
    {
        int l_i = 0;
        for (t_idx l_gy = 0; l_gy < m_ny; l_gy += m_k)
        {
            for (t_idx l_gx = 0; l_gx < m_nx; l_gx += m_k)
            {
                for (t_idx l_y = 0; l_y < m_k; l_y++)
                {
                    for (t_idx l_x = 0; l_x < m_k; l_x++)
                    {
                        l_data[l_i] += i_hv[l_gx + l_x + (l_y + l_gy) * i_stride];
                    }
                }
                l_data[l_i] *= l_averagingFactor;
                l_i++;
            }
        }
    }
    checkNcErr(nc_put_vara_float(m_ncId,
                                 m_varHvId,
                                 start,
                                 count,
                                 l_data));

    m_writingStepsCount++;

    delete[] l_data;
}

void tsunami_lab::io::NetCdf::getDimensionSize(const char *i_file,
                                               const char *i_dimName,
                                               t_idx &o_n)
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

void tsunami_lab::io::NetCdf::read(const char *i_file,
                                   const char *i_var,
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

void tsunami_lab::io::NetCdf::writeCheckpoint(const char *i_checkpointFile,
                                              t_idx i_stride,
                                              t_real const *i_h,
                                              t_real const *i_hu,
                                              t_real const *i_hv,
                                              t_real const *i_b,
                                              t_real i_t,
                                              t_real i_timeStep)
{
    setUpCheckpointFile(i_checkpointFile);

    t_idx start[] = {0, 0};
    t_idx count[] = {m_ny, m_nx};

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
    // WRITE WRITING STEP COUNT
    float l_writingStepsFloat = float(m_writingStepsCount);
    checkNcErr(nc_put_vara_float(m_ncCheckId, m_varCheckWritingStepsCountId, start, count, &l_writingStepsFloat));
    // WRITE TIME STEP
    checkNcErr(nc_put_vara_float(m_ncCheckId, m_varCheckTimeStepId, start, count, &i_timeStep));
    // WRITE TIME
    checkNcErr(nc_put_vara_float(m_ncCheckId, m_varCheckTId, start, count, &i_t));
    // WRITE K
    float l_kFloat = float(m_k);
    checkNcErr(nc_put_vara_float(m_ncCheckId, m_varCheckKId, start, count, &l_kFloat));

    // WRITE HEIGHT
    l_i = 0;
    if (i_h == nullptr)
    {
        memset(l_data, 0, m_nx * m_ny * sizeof(t_real));
    }
    else
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            for (t_idx l_x = 0; l_x < m_nx; l_x++)
            {
                l_data[l_i++] = i_h[l_x + l_y * i_stride];
            }
        }
    }
    checkNcErr(nc_put_vara_float(m_ncCheckId,
                                 m_varCheckHId,
                                 start,
                                 count,
                                 l_data));
    // WRITE MOMENTUM X
    l_i = 0;
    if (i_hu == nullptr)
    {
        memset(l_data, 0, m_nx * m_ny * sizeof(t_real));
    }
    else
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            for (t_idx l_x = 0; l_x < m_nx; l_x++)
            {
                l_data[l_i++] = i_hu[l_x + l_y * i_stride];
            }
        }
    }
    checkNcErr(nc_put_vara_float(m_ncCheckId,
                                 m_varCheckHuId,
                                 start,
                                 count,
                                 l_data));

    // WRITE MOMENTUM Y
    l_i = 0;
    if (i_hv == nullptr)
    {
        memset(l_data, 0, m_nx * m_ny * sizeof(t_real));
    }
    else
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            for (t_idx l_x = 0; l_x < m_nx; l_x++)
            {
                l_data[l_i++] = i_hv[l_x + l_y * i_stride];
            }
        }
    }
    checkNcErr(nc_put_vara_float(m_ncCheckId,
                                 m_varCheckHvId,
                                 start,
                                 count,
                                 l_data));

    // WRITE BATHYMETRY
    l_i = 0;
    if (i_b == nullptr)
    {
        memset(l_data, 0, m_nx * m_ny * sizeof(t_real));
    }
    else
    {
        for (t_idx l_y = 0; l_y < m_ny; l_y++)
        {
            for (t_idx l_x = 0; l_x < m_nx; l_x++)
            {
                l_data[l_i++] = i_b[l_x + l_y * i_stride];
            }
        }
    }
    checkNcErr(nc_put_vara_float(m_ncCheckId,
                                 m_varCheckBId,
                                 start,
                                 count,
                                 l_data));

    delete[] l_data;

    // flush all data
    nc_sync(m_ncCheckId);
    if (m_outputFileOpened)
    {
        nc_sync(m_ncId);
    }

    checkNcErr(nc_close(m_ncCheckId));
}

void tsunami_lab::io::NetCdf::loadCheckpointDimensions(const char *i_checkpointFile,
                                                       t_idx &o_nx,
                                                       t_idx &o_ny,
                                                       t_idx &o_nk,
                                                       t_real &o_simulationSizeX,
                                                       t_real &o_simulationSizeY,
                                                       t_real &o_offsetX,
                                                       t_real &o_offsetY,
                                                       t_real &o_t,
                                                       t_idx &o_timeStep)
{
    int l_ncIdCheckRead = 0, l_dimCheckXIdRead = 0, l_dimCheckYIdRead = 0, l_varDataIdRead = 0, l_i = 0;
    checkNcErr(nc_open(i_checkpointFile, NC_NOWRITE, &l_ncIdCheckRead));
    // get dimension ids
    checkNcErr(nc_inq_dimid(l_ncIdCheckRead, "x", &l_dimCheckXIdRead));
    checkNcErr(nc_inq_dimid(l_ncIdCheckRead, "y", &l_dimCheckYIdRead));
    // read dimension size
    checkNcErr(nc_inq_dimlen(l_ncIdCheckRead, l_dimCheckXIdRead, &m_nx));
    checkNcErr(nc_inq_dimlen(l_ncIdCheckRead, l_dimCheckYIdRead, &m_ny));
    o_nx = m_nx;
    o_ny = m_ny;
    // READ VARIABLES : //

    // READ SIMULATION SIZE X
    checkNcErr(nc_inq_varid(l_ncIdCheckRead, "simulationSizeX", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_ncIdCheckRead, l_varDataIdRead, &m_simulationSizeX));
    o_simulationSizeX = m_simulationSizeX;

    // READ SIMULATION SIZE Y
    checkNcErr(nc_inq_varid(l_ncIdCheckRead, "simulationSizeY", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_ncIdCheckRead, l_varDataIdRead, &m_simulationSizeY));
    o_simulationSizeY = m_simulationSizeY;

    // READ OFFSET X
    checkNcErr(nc_inq_varid(l_ncIdCheckRead, "offsetX", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_ncIdCheckRead, l_varDataIdRead, &m_offsetX));
    o_offsetX = m_offsetX;

    // READ OFFSET Y
    checkNcErr(nc_inq_varid(l_ncIdCheckRead, "offsetY", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_ncIdCheckRead, l_varDataIdRead, &m_offsetY));
    o_offsetY = m_offsetY;

    // READ TIME
    checkNcErr(nc_inq_varid(l_ncIdCheckRead, "time", &l_varDataIdRead));
    checkNcErr(nc_get_var_float(l_ncIdCheckRead, l_varDataIdRead, &o_t));

    // READ TIME STEP
    checkNcErr(nc_inq_varid(l_ncIdCheckRead, "timeStep", &l_varDataIdRead));
    int l_timeStepInt = 0;
    checkNcErr(nc_get_var_int(l_ncIdCheckRead, l_varDataIdRead, &l_timeStepInt));
    o_timeStep = t_idx(l_timeStepInt);

    // READ WRITING STEPS COUNT
    checkNcErr(nc_inq_varid(l_ncIdCheckRead, "writingStepsCount", &l_varDataIdRead));
    checkNcErr(nc_get_var_int(l_ncIdCheckRead, l_varDataIdRead, &l_i));
    m_writingStepsCount = t_idx(l_i);

    // READ K
    checkNcErr(nc_inq_varid(l_ncIdCheckRead, "k", &l_varDataIdRead));
    int l_k = 0;
    checkNcErr(nc_get_var_int(l_ncIdCheckRead, l_varDataIdRead, &l_k));
    m_k = t_idx(l_k);
    o_nk = t_idx(l_k);
    m_nkx = m_nx / m_k;
    m_nky = m_ny / m_k;

    // start over in case the solution got lost
    if (!m_doesSolutionExist)
        m_writingStepsCount = 0;

    checkNcErr(nc_close(l_ncIdCheckRead));
}