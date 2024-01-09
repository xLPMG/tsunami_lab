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

  //! keywords for client-server communication
  inline static const char* KEY_SHUTDOWN_SERVER = "shutdown";
  inline static const char* KEY_RESTART_SERVER = "restart";
  inline static const char* KEY_EXIT_LAUNCHER = "exit_launcher";
  inline static const char* KEY_REVIVE_LAUNCHER = "revive_launcher";
  inline static const char* KEY_START_SIMULATION = "FV_START";
  inline static const char* KEY_WRITE_CHECKPOINT = "FV_WRITE_CHECKPOINT";
  inline static const char* KEY_LOAD_CONFIG_JSON = "FV_LOAD_CONFIG_JSON";
  inline static const char* KEY_LOAD_CONFIG_FILE = "FV_LOAD_CONFIG_FILE";
}

#endif