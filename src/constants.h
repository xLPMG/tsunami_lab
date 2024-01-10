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

  //! should not not induce any functionality and is only used to check if the other side responds
  inline const char* KEY_CHECK = "XCHECKX";

  //! tells the server to shutdown
  inline const char* KEY_SHUTDOWN_SERVER = "Sshutdown";

  //! tells the server to restart
  inline const char* KEY_RESTART_SERVER = "Srestart";

  //! tells the server to exit the launcher as soon as possible
  inline const char* KEY_EXIT_LAUNCHER = "Sexit_launcher";

  //! tells the server to revive the launcher
  inline const char* KEY_REVIVE_LAUNCHER = "Srevive_launcher";

  //! launcher will start its main loop
  inline const char* KEY_START_SIMULATION = "FV_START";

  //! launcher will write a checkpoint
  inline const char* KEY_WRITE_CHECKPOINT = "FV_WRITE_CHECKPOINT";

  //! launcher will load config from json data which should be sent immediately after
  inline const char* KEY_LOAD_CONFIG_JSON = "FV_LOAD_CONFIG_JSON";

  //! launcher will load data from a config .json file
  inline const char* KEY_LOAD_CONFIG_FILE = "FV_LOAD_CONFIG_FILE";

  //! launcher will toggle file i/o usage to the boolean which is sent immediately after
  inline const char* KEY_TOGGLE_FILEIO = "FV_TOGGLE_FILEIO";
}

#endif