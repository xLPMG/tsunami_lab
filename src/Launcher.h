/**
 * @author Luca-Philipp Grumbach
 * @author Richard Hofmann
 *
 * # Description 
 * Class that launches and controls the simulation.
 **/

#ifndef TSUNAMI_LAB_LAUNCHER_H
#define TSUNAMI_LAB_LAUNCHER_H

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
    class Launcher;
}

class tsunami_lab::Launcher
{
private:
    /**
     *  Determines if a string ends with another string.
     *
     * @param i_str input string to check
     * @param i_suffix possible suffix of i_str
     * @return true if i_str ends with i_suffix, otherwise false.
     */
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
    /**
     *  Loads the config data from a file
     *
     *  @param i_configFilePath path of the config file
     *  @return void
     */
    void loadConfigDataFromFile(std::string i_configFilePath);

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

    int start(std::string i_config);
};

#endif
