/**
 * @author Luca-Philipp Grumbach
 *
 * # Description
 * Communicator api.
 **/
#ifndef XLPMG_COMMUNICATOR_API_H
#define XLPMG_COMMUNICATOR_API_H

#include <cstddef>
#include <string>

#include <nlohmann/json.hpp>

namespace xlpmg
{
  enum MessagePart
  {
    MESSAGE_TYPE,
    KEY,
    ARGS
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(MessagePart, {{MESSAGE_TYPE, "type"},
                                              {KEY, "key"},
                                              {ARGS, "arguments"}});

  enum MessageType
  {
    SERVER__CALL,
    FUNCTION_CALL,
    CHECK_CALL
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(MessageType, {{SERVER__CALL, "server_call"},
                                             {FUNCTION_CALL, "function_call"},
                                             {CHECK_CALL, "check_call"}});

  //! should not not induce any functionality and is only used to check if the other side responds
  inline const char *KEY_CHECK = "XCHECKX";

  // SERVER CALLS

  /*
  * Tells the server to shutdown.
  *
  * @MESSAGE_TYPE SERVER__CALL
  * @ARGS none
  */
  inline const char *KEY_SHUTDOWN_SERVER = "shutdown_server";

  /*
  * Tells the server to restart.
  *
  * @MESSAGE_TYPE SERVER__CALL
  * @ARGS path to config file or "".
  */
  inline const char *KEY_START_SIMULATION = "start_simulation";

  /*
  * Server will stop the running simulation.
  *
  * @MESSAGE_TYPE SERVER__CALL
  * @ARGS none.
  */
  inline const char *KEY_KILL_SIMULATION = "kill_simulation";

  /*
  * Server will recompile.
  *
  * @MESSAGE_TYPE SERVER__CALL
  * @ARGS as array: 
  *   args[ENV]: environment option for compiler (eg. "CXX=g++-13" or "").
  *   args[OPT]: compiler options (eg. "omp=gnu opt=-O2" or "")
  */
  inline const char *KEY_RECOMPILE = "recompile";

  // FUNCTION CALLS: VOID

  /*
  * Simulator will write a checkpoint.
  *
  * @MESSAGE_TYPE FUNCTION_CALL
  * @ARGS none.
  */
  inline const char *KEY_WRITE_CHECKPOINT = "write_checkpoint";

  /*
  * Simulator will load config from json data.
  *
  * @MESSAGE_TYPE FUNCTION_CALL
  * @ARGS config data in json format.
  */
  inline const char *KEY_LOAD_CONFIG_JSON = "load_config_json";

  /*
  * Simulator will load config from .json config file.
  *
  * @MESSAGE_TYPE FUNCTION_CALL
  * @ARGS path to config file.
  */
  inline const char *KEY_LOAD_CONFIG_FILE = "load_config_file";

  /*
  * Simulator will toggle file i/o usage.
  *
  * @MESSAGE_TYPE FUNCTION_CALL
  * @ARGS boolean to set file i/o to.
  */
  inline const char *KEY_TOGGLE_FILEIO = "FV_TOGGLE_FILEIO";

  // FUNCTION CALLS: RETURNING

  /*
  * Returns the current timestep from the simulator.
  *
  * @MESSAGE_TYPE FUNCTION_CALL
  * @ARGS none.
  */
  inline const char *KEY_GET_TIMESTEP = "FR_GET_TIMESTEP";

  /*
  * Returns the max timesteps from the simulator.
  *
  * @MESSAGE_TYPE FUNCTION_CALL
  * @ARGS none.
  */
  inline const char *KEY_GET_MAXTIMESTEPS = "FR_GET_MAXTIMESTEPS";
}

#endif