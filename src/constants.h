/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * # Description
 * Constants / typedefs used throughout the code.
 **/
#ifndef TSUNAMI_LAB_CONSTANTS_H
#define TSUNAMI_LAB_CONSTANTS_H

#include <cstddef>
#include <string>

namespace tsunami_lab
{
  //! integral type for cell-ids, pointer arithmetic, etc.
  typedef std::size_t t_idx;

  //! floating point type
  typedef float t_real;

  //------------------------------------------//
  //-------------Communicator API-------------//
  //------------------------------------------//

  /**
   * To ensure faster decoding, the commands need to follow some rules:
   * 1. Commands need to be unique.
   * 2. Commands regarding the server start with an 'S'
   * 3. Commands that call a function within the simulator, start with an 'F'
   * 3.1 Void functions start with 'FV': the client will not expect a reply
   * 3.2 Non-void functions start with 'FR': the client will expect a reply
   * 
   * */ 


  //! should not not induce any functionality and is only used to check if the other side responds
  inline const char* KEY_CHECK = "XCHECKX";

  // SERVER CALLS

  //! tells the server to shutdown
  inline const char* KEY_SHUTDOWN_SERVER = "Sshutdown";

  //! tells the server to restart
  inline const char* KEY_RESTART_SERVER = "Srestart";

  //! launcher will start its main loop
  inline const char* KEY_KILL_SIMULATION = "Skill_simulation";

  /*
  * Server will recompile with the options that are sent immediately after.
  *
  * Protocol:
  * 1. Send the command.
  * 2. Send environment compiler option (eg. "CXX=icpc" or "").
  * 3. Send the compiler parameters (eg. "opt=-O2 omp=gnu").
  */
  inline const char* KEY_RECOMPILE = "Srecompile";

  // FUNCTION CALLS: VOID

  /*
  * Simulator will start its main loop.
  *
  * Protocol:
  * 1. Send the command.
  * 2. Send the path to a config file (located on the server) or "".
  */
  inline const char* KEY_START_SIMULATION = "FV_START";

  //! simulator will write a checkpoint
  inline const char* KEY_WRITE_CHECKPOINT = "FV_WRITE_CHECKPOINT";

  /*
  * Simulator will load config from json data.
  *
  * Protocol:
  * 1. Send the command.
  * 2. Send the config data in json format.
  */
  inline const char* KEY_LOAD_CONFIG_JSON = "FV_LOAD_CONFIG_JSON";

  /*
  * Simulator will load config from .json config file.
  *
  * Protocol:
  * 1. Send the command.
  * 2. Send the path to a config file (located on the server).
  */
  inline const char* KEY_LOAD_CONFIG_FILE = "FV_LOAD_CONFIG_FILE";

  /*
  * Simulator will toggle file i/o usage to the boolean which is sent immediately after.
  *
  * Protocol:
  * 1. Send the command.
  * 2. Send the path to a config file (located on the server).
  */
  inline const char* KEY_TOGGLE_FILEIO = "FV_TOGGLE_FILEIO";

  // FUNCTION CALLS: RETURNING

  //! gets the current timestep from the simulator
  inline const char* KEY_GET_TIMESTEP = "FR_GET_TIMESTEP";

  //! gets the max timesteps from the simulator
  inline const char* KEY_GET_MAXTIMESTEPS = "FR_GET_MAXTIMESTEPS";
}

#endif