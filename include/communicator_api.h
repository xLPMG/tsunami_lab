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
    TYPE,
    KEY,
    ARGS
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(MessagePart, {{TYPE, "type"},
                                             {KEY, "key"},
                                             {ARGS, "arguments"}});

  enum MessageType
  {
    SERVER__CALL,
    FUNCTION_CALL,
    OTHER,
    SERVER_RESPONSE
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(MessageType, {{SERVER__CALL, "server_call"},
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
  inline const Message CHECK_MESSAGE = {MessageType::SERVER__CALL, "XCHECKX"};

  //! Tells the server to shutdown.
  inline const Message SHUTDOWN_SERVER_MESSAGE = {MessageType::SERVER__CALL, "shutdown_server"};

  //! Tells the server to restart.
  inline const Message START_SIMULATION_MESSAGE = {MessageType::SERVER__CALL, "start_simulation"};

  //! Server will stop the running simulation.
  inline const Message KILL_SIMULATION_MESSAGE = {MessageType::SERVER__CALL, "kill_simulation"};

  //! Server will recompile with provided arguments.
  inline const Message RECOMPILE_MESSAGE = {MessageType::SERVER__CALL, "recompile", ""};

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




  //inline const Message LOAD_JSON= {MessageType::SERVER__CALL, "load_json"};

}

#endif