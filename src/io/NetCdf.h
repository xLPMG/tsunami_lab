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

    const char *m_netcdfOutputPath = nullptr;
    const char *m_checkpointFilePath = nullptr;

    bool m_hasCheckpointFileBeenSetup = false;
    bool m_doesSolutionExist = false;

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
    int m_varCheckBId = 0;
    int m_varCheckTimeStepId = 0;
    int m_varCheckWritingStepsCountId = 0;
    int m_varCheckSimSizeXId = 0;
    int m_varCheckSimSizeYId = 0;
    int m_varCheckOffsetXId = 0;
    int m_varCheckOffsetYId = 0;

    // index for timesteps
    t_idx m_writingStepsCount = 0;

    // tracks if file was opened for writing
    bool m_outputFileOpened = false;

    /**
     * checks for error in Cdf file
     * @param i_err error
     */
    void checkNcErr(t_idx i_err);

    /**
     * Sets up a netcdf file for writing.
     *
     * @param i_netcdfOutputPath path of the netcdf output file
     */
    void setUpFile(const char *i_file);

    /**
     * Sets up a netcdf file for checkpointing.
     *
     * @param i_checkpointFile path of the checkpoint file
     */
    void setUpCheckpointFile(const char *i_checkpointFile);

    /**
     * Loads dimension and variable ids from an existing netcdf file.
     * 
     */
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
     * @param i_netcdfOutputPath path of the netcdf output file
     * @param i_checkpointFile path of the checkpoint file
     */
    NetCdf(t_idx i_nx,
           t_idx i_ny,
           t_real i_simulationSizeX,
           t_real i_simulationSizeY,
           t_real i_offsetX,
           t_real i_offsetY,
           const char *i_netcdfOutputPath,
           const char *i_checkpointFilePath);

    /**
     *  Constructor for when we have a checkpoint file to read from
     * @param i_netcdfOutputPath path of the netcdf output file
     * @param i_checkpointFile path of the checkpoint file
     */
    NetCdf(const char *i_netcdfOutputPath,
           const char *i_checkpointFilePath);

    /**
     * Destructor
     *
     */
    ~NetCdf();

    /**
     * Writes data into netcdf file
     *
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
     * @param i_file path of the file to read from
     * @param i_dimName name of the dimension
     * @param o_n number of entries along that axis
     */
    void getDimensionSize(const char *i_file,
                          const char *i_dimName,
                          t_idx &o_n);

    /**
     * Reads from the cdf file
     *
     * @param i_file path of the file to read from
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

    /** Reads from the cdf file
     *
     * @param i_file path of the file to read from
     * @param i_var variable to be read
     * @param o_data output data
     */
    void read(const char *i_file,
              const char *i_var,
              t_real **o_data);

    /** Writes checkpoint data to a file.
     *
     * @param i_checkpointFile path of the checkpoint file
     * @param i_h water heights
     * @param i_hu momentum x-direction
     * @param i_hv momentum y-direction
     * @param i_b bathymetry
     * @param i_t simulation time
     * @param i_timestep timestep
     */
    void writeCheckpoint(const char *i_checkpointFile,
                         t_real const *i_h,
                         t_real const *i_hu,
                         t_real const *i_hv,
                         t_real const *i_b,
                         t_real i_t,
                         t_real i_timeStep);

    /** Loads checkpoint data from a file.
     *
     * @param i_checkpointFile path of the checkpoint file
     * @param o_nx amount of cells in x-direction
     * @param o_ny amount of cells in y-direction
     * @param o_simulationSizeX simulation size in x-direction
     * @param o_simulationSizeY simulation size in y-direction
     * @param o_offsetX offset in x-direction
     * @param o_offsetY offset in y-direction
     * @param o_t simulation time
     * @param o_timestep timestep
     */
    void loadCheckpointDimensions(const char *i_checkpointFile,
                                  t_idx &o_nx,
                                  t_idx &o_ny,
                                  t_real &o_simulationSizeX,
                                  t_real &o_simulationSizeY,
                                  t_real &o_offsetX,
                                  t_real &o_offsetY,
                                  t_real &o_t,
                                  t_idx &o_timeStep);
};
#endif
