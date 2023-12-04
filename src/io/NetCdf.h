/**
 * @author Luca-Philipp Grumbach, Richard Hofmann
 *
 * @section DESCRIPTION
 *
 **/
#ifndef TSUNAMI_LAB_IO_NETCDF
#define TSUNAMI_LAB_IO_NETCDF

#include "../constants.h"
#include <cstring>
#include "Station.h"

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
    // amount of cells in x direction
    t_idx m_nx = 0;
    // amount of cells in y direction
    t_idx m_ny = 0;
    // simulation size in x direction
    t_real m_simulationSizeX = 0;
    // simulation size in y direction
    t_real m_simulationSizeY = 0;
    // offset in x direction
    t_real m_offsetX = 0;
    // offset in y direction
    t_real m_offsetY = 0;
    // Id for nc file
    int m_ncId = 0;
    // error
    int m_err = 0;

    const char *m_path = nullptr;
    const char *m_checkpointFile = nullptr;

    bool m_hasCheckpointFileBeenSetup = false;

    // dimension ids
    int m_dimXId = 0;
    int m_dimYId = 0;
    int m_dimTId = 0;

    // variable ids
    int m_varXId = 0;
    int m_varYId = 0;
    int m_varTId = 0;
    int m_varHId = 0;
    int m_varTHId = 0;
    int m_varBId = 0;
    int m_varHuId = 0;
    int m_varHvId = 0;

    int m_ncCheckId = 0;

    // dimension ids for checkpointing
    int m_dimCheckXId = 0;
    int m_dimCheckYId = 0;
    int m_dimCheckTId = 0;

    // variable ids for checkpointing
    int m_varCheckXId = 0;
    int m_varCheckYId = 0;
    int m_varCheckTId = 0;
    int m_varCheckHId = 0;
    int m_varCheckHuId = 0;
    int m_varCheckHvId = 0;
    int m_varCheckTimeStepId = 0;
    int m_varCheckTimeStepCountId = 0;
    int m_varCheckSimSizeXId = 0;
    int m_varCheckSimSizeYId = 0;
    int m_varCheckOffsetXId = 0;
    int m_varCheckOffsetYId = 0;

    // index for timesteps
    t_idx m_timeStepCount = 0;

    // tracks if file was opened for writing
    bool m_outputFileOpened = false;

    /**
     * checks for error in Cdf file
     * @param i_err error
     */
    void checkNcErr(t_idx i_err);

    /**
     * Sets up a netcdf file for writing
     *
     */
    void setUpFile(const char *path);

    /**
     * Sets up a netcdf file for checkpointing
     *
     */
    void setUpCheckpointFile(const char *i_checkpointFile);

    void loadNetCdfIds();

public:
    /**
     *  Constructor for initialization
     *
     * @param i_nx amount of cells in x-direction
     * @param i_ny amount of cells in y-direction
     * @param i_simulationSizeX simulation size in x-direction
     * @param i_simulationSizeY simulation size in y-direction
     * @param i_offsetX offset in x-direction
     * @param i_offsetY offset in y-direction
     */
    NetCdf(t_idx i_nx,
           t_idx i_ny,
           t_real i_simulationSizeX,
           t_real i_simulationSizeY,
           t_real i_offsetX,
           t_real i_offsetY,
           const char *path,
           const char *i_checkpointFile);

    /**
     *  Constructor for when we have  checkoint file to read from
     * @param i_checkpointFile path of a checkpoint file
     */
    NetCdf(const char *path,
           const char *i_checkpointFile);

    /**
     * Destructor
     *
     */
    ~NetCdf();

    /**
     * Writes into cdf file
     *
     * @param path file path
     * @param i_stride stride
     * @param i_h water heights
     * @param i_hu momentum x-direction
     * @param i_hv momentum y-direction
     * @param i_b bathymetry
     * @param i_t current timestep
     *
     */
    void write(t_idx i_stride,
               t_real const *i_h,
               t_real const *i_hu,
               t_real const *i_hv,
               t_real const *i_b,
               t_real i_t);

    /**
     * Gets the size of a dimension
     *
     * @param i_file name of the file to read from
     * @param o_n number of entries along that axis
     * @param i_dimName name of the dimension
     */
    void getDimensionSize(const char *i_file,
                          t_idx &o_n,
                          const char *i_dimName);

    /**
     * Reads from the cdf file
     *
     * @param i_file name of the file to read from
     * @param i_var variable to be read
     * @param o_xData data of x dimension
     * @param o_yData data of y dimension
     * @param o_data output data
     */
    void read(const char *i_file,
              const char *i_var,
              t_real **o_xData,
              t_real **o_yData,
              t_real **o_data);

    void writeCheckpoint(const char *path,
                         t_real const *i_h,
                         t_real const *i_hu,
                         t_real const *i_hv,
                         t_real i_t,
                         t_real i_timeStep);

    void loadCheckpoint(const char *path,
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
                        t_real &o_timeStep);
};
#endif
