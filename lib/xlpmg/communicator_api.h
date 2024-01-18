/**
 * @author Luca-Philipp Grumbach
 *
 * # Description
 * Communicator API.
 * This file provides an interface with pre-defined
 * messages to enable a correct transmission of
 * information between client and server.
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
    TYPE,
    KEY,
    ARGS
  };

  enum MessageType
  {
    SERVER_CALL,
    FUNCTION_CALL,
    OTHER,
    SERVER_RESPONSE
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(MessageType, {{SERVER_CALL, "server_call"},
                                             {FUNCTION_CALL, "function_call"},
                                             {OTHER, "other"},
                                             {SERVER_RESPONSE, "server_response"}});

  struct Message
  {
    MessageType type = MessageType::OTHER;
    std::string key = "NONE";
    json args = "";
  };

  /**
   * Converts a Message to json object.
   *
   * @param i_message message
   * @return message as json object
   */
  json messageToJson(Message i_message)
  {
    json msg;
    msg[MessagePart::TYPE] = i_message.type;
    msg[MessagePart::KEY] = i_message.key;
    msg[MessagePart::ARGS] = i_message.args;
    return msg;
  }

  /**
   * Converts a Message to a json string.
   *
   * @param i_message message
   * @return message as json string
   */
  std::string messageToJsonString(Message message)
  {
    return messageToJson(message).dump();
  }

  /**
   * Converts a json object to a Message.
   *
   * @param i_json json object
   * @return message
   */
  Message jsonToMessage(json i_json)
  {
    Message l_message;
    l_message.type = i_json.at(MessagePart::TYPE);
    l_message.key = i_json.at(MessagePart::KEY);
    l_message.args = i_json.at(MessagePart::ARGS);
    return l_message;
  }

  //! should not not induce any functionality and is only used to check if the other side responds
  inline const Message CHECK_MESSAGE = {MessageType::SERVER_CALL, "XCHECKX"};

  //! Tells the server to shutdown.
  inline const Message SHUTDOWN_SERVER_MESSAGE = {MessageType::SERVER_CALL, "shutdown_server"};

  //! Tells the server to restart.
  inline const Message START_SIMULATION_MESSAGE = {MessageType::SERVER_CALL, "start_simulation"};

  //! Server will stop the running simulation.
  inline const Message KILL_SIMULATION_MESSAGE = {MessageType::SERVER_CALL, "kill_simulation"};

  //! Server will recompile with provided arguments.
  inline const Message COMPILE_MESSAGE = {MessageType::SERVER_CALL, "compile", ""};

  //! Server will recompile with provided arguments and run using a bash script.
  inline const Message COMPILE_RUN_BASH_MESSAGE = {MessageType::SERVER_CALL, "compile_run_bash", ""};

  //! Server will recompile with provided arguments and run using an sbatch script.
  inline const Message COMPILE_RUN_SBATCH_MESSAGE = {MessageType::SERVER_CALL, "compile_run_sbatch", ""};

  //! Simulator will write a checkpoint.
  inline const Message WRITE_CHECKPOINT_MESSAGE = {MessageType::FUNCTION_CALL, "write_checkpoint"};

  //! Simulator will load config from json data.
  inline const Message LOAD_CONFIG_JSON_MESSAGE = {MessageType::FUNCTION_CALL, "load_config_json"};

  //! Simulator will load config from .json config file.
  inline const Message LOAD_CONFIG_FILE_MESSAGE = {MessageType::FUNCTION_CALL, "load_config_file"};

  //! Simulator will toggle file i/o usage to given arg.
  inline const Message TOGGLE_FILEIO_MESSAGE = {MessageType::FUNCTION_CALL, "toggle_fileio"};

  //! Returns the current timestep from the simulator.
  inline const Message GET_TIMESTEP_MESSAGE = {MessageType::FUNCTION_CALL, "get_current_timestep"};

  //! Returns the max timesteps from the simulator.
  inline const Message GET_MAX_TIMESTEPS_MESSAGE = {MessageType::FUNCTION_CALL, "get_max_timesteps"};

  //! Tells the server to start sending height data. (buffered)
  inline const Message GET_HEIGHT_DATA_MESSAGE = {MessageType::FUNCTION_CALL, "get_height_data"};

  //! Tells the client that a buffered sending operation has finished.
  inline const Message BUFFERED_SEND_FINISHED = {MessageType::SERVER_RESPONSE, "buff_send_finished"};

  //! Tells the server to change the buffer size.
  inline const Message SET_BUFFER_SIZE = {MessageType::SERVER_CALL, "set_buffer_size"};

  //! Prepares the server for incoming bathymetry data.
  inline const Message PREPARE_BATHYMETRY_DATA = {MessageType::SERVER_CALL, "prepare_bathymetry_data"};

  //! Prepares the server for incoming displacement data.
  inline const Message PREPARE_DISPLACEMENT_DATA = {MessageType::SERVER_CALL, "prepare_displacement_data"};

  //! Sends bathymetry data to the server.
  inline const Message SET_BATHYMETRY_DATA = {MessageType::SERVER_CALL, "set_bathymetry_data"};

  //! Sends displacement data to the server.
  inline const Message SET_DISPLACEMENT_DATA = {MessageType::SERVER_CALL, "set_displacement_data"};
}

#endif