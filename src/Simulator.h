/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description
 * Class that launches and controls the simulation.
 **/

#ifndef TSUNAMI_LAB_Simulator_H
#define TSUNAMI_LAB_Simulator_H

#ifdef USEOMP
#include <omp.h>
#endif

// waveprop patches
#include "patches/WavePropagation1d.h"
#include "patches/WavePropagation2d.h"

// setups
#include "setups/DamBreak1d.h"
#include "setups/CircularDamBreak2d.h"
#include "setups/RareRare1d.h"
#include "setups/ShockShock1d.h"
#include "setups/Subcritical1d.h"
#include "setups/Supercritical1d.h"
#include "setups/GeneralDiscontinuity1d.h"
#include "setups/TsunamiEvent1d.h"
#include "setups/TsunamiEvent2d.h"
#include "setups/ArtificialTsunami2d.h"

// io
#include "io/Csv.h"
#include "io/BathymetryLoader.h"
#include "io/Station.h"
#include "io/NetCdf.h"

// external libraries
#include <nlohmann/json.hpp>
#include <netcdf.h>

#include <string>
using json = nlohmann::json;
using Boundary = tsunami_lab::patches::WavePropagation::Boundary;

namespace tsunami_lab
{
    class Simulator;
}

class tsunami_lab::Simulator
{
private:
    //------------------------------------------//
    //----------------VARIABLES-----------------//
    //------------------------------------------//
    // config parameters
    std::string m_configFilePath = "configs/config.json";
    json m_configData;

    bool m_useFileIO = true;

    // input parameters
    std::string m_bathymetryFilePath = "";
    std::string m_displacementFilePath = "";

    // output parameters
    std::string m_outputFileName = "";
    tsunami_lab::t_real m_stationFrequency = 0;
    tsunami_lab::t_idx m_writingFrequency = 0;
    std::string m_netCdfOutputPathString = "";
    const char *m_netcdfOutputPath = "";
    enum DataWriter
    {
        NETCDF = 0,
        CSV = 1
    };
    DataWriter m_dataWriter = NETCDF;
    tsunami_lab::io::NetCdf *m_netCdf;

    // checkpointing
    bool m_checkpointExists = false;
    tsunami_lab::t_real m_checkpointFrequency = -1;
    std::string m_checkPointFilePathString = "";
    const char *m_checkPointFilePath = "";

    // setup parameters
    std::string m_setupChoice = "";
    tsunami_lab::setups::Setup *m_setup;

    // simulation parameters
    std::string m_solver = "";
    tsunami_lab::patches::WavePropagation *m_waveProp;
    tsunami_lab::t_idx m_nx = 0;
    tsunami_lab::t_idx m_ny = 0;
    tsunami_lab::t_idx m_nk = 1;
    tsunami_lab::t_real m_simulationSizeX = 0;
    tsunami_lab::t_real m_simulationSizeY = 0;
    tsunami_lab::t_real m_offsetX = 0;
    tsunami_lab::t_real m_offsetY = 0;
    tsunami_lab::t_real m_dx = 0;
    tsunami_lab::t_real m_dy = 0;
    tsunami_lab::t_real m_endTime = 0;

    // boundary conditions
    Boundary m_boundaryL = Boundary::OUTFLOW;
    Boundary m_boundaryR = Boundary::OUTFLOW;
    Boundary m_boundaryT = Boundary::OUTFLOW;
    Boundary m_boundaryB = Boundary::OUTFLOW;

    // stations
    std::vector<tsunami_lab::io::Station *> m_stations;

    // time and print control
    tsunami_lab::t_idx m_timeStep = 0;
    tsunami_lab::t_idx m_timeStepMax = 0;
    tsunami_lab::t_real m_simTime = 0;
    tsunami_lab::t_idx m_nOut = 0;
    tsunami_lab::t_idx m_captureCount = 0;

    // simulation variables
    tsunami_lab::t_real m_hMax = std::numeric_limits<tsunami_lab::t_real>::lowest();
    tsunami_lab::t_real m_dt = 0;
    tsunami_lab::t_real m_scalingX;
    tsunami_lab::t_real m_scalingY;

    //------------------------------------------//
    //-------------END OF VARIABLES-------------//
    //------------------------------------------//

/**                                                           \
 *  Determines if a string ends with another string.          \
 *                                                            \
 * @param i_str input string to check                         \
 * @param i_suffix possible suffix of i_str                   \
 * @return true if i_str ends with i_suffix, otherwise false. \
 */                                                           \
    bool endsWith(std::string const &i_str, std::string const &i_suffix);

    /**
     *  Helper method that sets up the required folder structure.
     *
     *  @return void
     */
    void setupFolders();

    /**
     *  Helper method that sets up the required file structure and looks for a checkpoint file.
     *
     *  @return void
     */
    void configureFiles();

    /**
     *  Helper method that loads configuration data from a config file.
     *
     *  @return void
     */
    void loadConfiguration();

    /**
     *  Helper method that constructs a setup from the active setup choice.
     *
     *  @return void
     */
    void constructSetup();

    /**
     *  Helper method that sets up the netcdf I/O.
     *  @return void
     */
    void setUpNetCdf();

    /**
     *  Helper method that constructs the solver.
     *
     *  @return void
     */
    void constructSolver();

    /**
     *  Helper method that loads bathymetry from a .csv file into the wave propagation patch.
     *
     *  @param i_file path of the .csv bathymetry file
     *  @return void
     */
    void loadBathymetry(std::string *i_file);

    /**
     *  Helper method that sets up the stations from the config data.
     *
     *  @return void
     */
    void loadStations();

    /**
     *  Helper method that writes out station data to files.
     *
     *  @return void
     */
    void writeStations();

    /**
     *  Helper method for the derivation of a time step.
     *
     *  @return void
     */
    void deriveTimeStep();

    /**
     *  Helper method that frees the allocated memory.
     *
     *  @return void
     */
    void freeMemory();

public:
    //------------------------------------------//
    //-----------------GETTERS------------------//
    //------------------------------------------//

    tsunami_lab::patches::WavePropagation *getWaveProp()
    {
        return m_waveProp;
    }

    void getSetupChoice(std::string &o_setupChoice)
    {
        o_setupChoice = m_setupChoice;
    }

    tsunami_lab::t_idx getTimeStep()
    {
        return m_timeStep;
    }

    //------------------------------------------//
    //-----------------SETTERS------------------//
    //------------------------------------------//

    void setSetupChoice(std::string i_setupChoice)
    {
        m_setupChoice = i_setupChoice;
    }

    //------------------------------------------//
    //----------------FUNCTIONS-----------------//
    //------------------------------------------//

    /**
     *  Creates a WavePropagation object.
     *
     *  @return void
     */
    void createWaveProp();

    /**
     *  Calls the netcdf writeCheckpoint function with necessary parameters.
     *
     *  @return void
     */
    void writeCheckpoint();

    /**
     *  Loads the config data from a file
     *
     *  @param i_configFilePath path of the config file
     *  @return void
     */
    void loadConfigDataFromFile(std::string i_configFilePath);

    /**
     *  Loads the config data from an input json string.
     *
     *  @param i_config config data in json format
     *  @return void
     */
    void loadConfigDataJsonString(std::string i_config);

    /**
     *  Sets up a station.
     *
     *  @param i_locationX location of the station in x-direction
     *  @param i_locationX location of the station in y-direction
     *  @param i_stationName name of the station
     *  @return void
     */
    void addStation(tsunami_lab::t_real i_locationX,
                    tsunami_lab::t_real i_locationY,
                    std::string i_stationName);

    /**
     *  Starts the calculation with the set parameters.
     *
     *  @param i_config path to the config file
     *  @return void
     */
    void runCalculation();

    /**
     *  Enables or disables file I/O.
     *
     *  @param i_useFileIO true if file I/O should be enabled.
     *  @return void
     */
    void toggleFileIO(bool i_useFileIO)
    {
        m_useFileIO = i_useFileIO;
        std::cout << "toggled file I/O" << std::endl;
    }

    int start(std::string i_config);
};

#endif
