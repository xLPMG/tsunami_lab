/**
 * @author Luca-Philipp Grumbach
 *
 * @brief Communicator API.
 * 
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
  inline const Message CHECK = {MessageType::SERVER_CALL, "XCHECKX"};

  //! Will provide information on the simulation
  inline const Message GET_SIMULATION_STATS= {MessageType::SERVER_CALL, "get_simulation_stats"};

  //! Tells the server to shutdown.
  inline const Message SHUTDOWN_SERVER = {MessageType::SERVER_CALL, "shutdown_server"};

  //! Tells the server to restart.
  inline const Message START_SIMULATION = {MessageType::SERVER_CALL, "start_simulation"};

  //! Server will stop the running simulation.
  inline const Message KILL_SIMULATION = {MessageType::SERVER_CALL, "kill_simulation"};

  //! Server will recompile with provided arguments.
  inline const Message COMPILE = {MessageType::SERVER_CALL, "compile", ""};

  //! Server will recompile with provided arguments and run using a bash script.
  inline const Message COMPILE_RUN_BASH = {MessageType::SERVER_CALL, "compile_run_bash", ""};

  //! Server will recompile with provided arguments and run using an sbatch script.
  inline const Message COMPILE_RUN_SBATCH = {MessageType::SERVER_CALL, "compile_run_sbatch", ""};

  //! For sending a file to the server
  inline const Message SEND_FILE = {MessageType::SERVER_CALL, "send_file"};

  //! Tells the server to change the read buffer size.
  inline const Message SET_READ_BUFFER_SIZE = {MessageType::SERVER_CALL, "set_read_buffer_size"};

    //! Tells the server to change the send buffer size.
  inline const Message SET_SEND_BUFFER_SIZE = {MessageType::SERVER_CALL, "set_send_buffer_size"};

  //! Simulator will reset.
  inline const Message RESET_SIMULATOR = {MessageType::FUNCTION_CALL, "reset_simulator"};

  //! Simulator will write a checkpoint.
  inline const Message WRITE_CHECKPOINT = {MessageType::FUNCTION_CALL, "write_checkpoint"};

  //! Simulator will load config from json data.
  inline const Message LOAD_CONFIG_JSON = {MessageType::FUNCTION_CALL, "load_config_json"};

  //! Simulator will load config from .json config file.
  inline const Message LOAD_CONFIG_FILE = {MessageType::FUNCTION_CALL, "load_config_file"};

  //! Simulator will toggle file i/o usage to given arg.
  inline const Message TOGGLE_FILEIO = {MessageType::FUNCTION_CALL, "toggle_fileio"};

  //! Returns the current timestep from the simulator.
  inline const Message GET_TIME_VALUES = {MessageType::FUNCTION_CALL, "get_time_values"};

  //! Returns the current simulation sizes from the simulator.
  inline const Message GET_SIMULATION_SIZES = {MessageType::FUNCTION_CALL, "get_simulation_sizes"};

  //! Tells the server to start sending height data. (buffered)
  inline const Message GET_HEIGHT_DATA = {MessageType::FUNCTION_CALL, "get_height_data"};

    //! Tells the server to start sending bathymetry data. (buffered)
  inline const Message GET_BATHYMETRY_DATA = {MessageType::FUNCTION_CALL, "get_bathymetry_data"};

  //! Sets the cell amount of the simulation.
  inline const Message SET_OFFSET = {MessageType::FUNCTION_CALL, "set_offset"};

  //! Sets the offset of the simulation.
  inline const Message SET_CELL_AMOUNT = {MessageType::FUNCTION_CALL, "set_cell_amount"};

  //! Tells the client that a buffered sending operation has finished.
  inline const Message BUFFERED_SEND_FINISHED = {MessageType::SERVER_RESPONSE, "buff_send_finished"};

  //! Deletes checkpoints.
  inline const Message DELETE_CHECKPOINTS = {MessageType::SERVER_CALL, "delete_checkpoints"};

  //! Deletes stations.
  inline const Message DELETE_STATIONS = {MessageType::SERVER_CALL, "delete_stations"};

  //! pause simulation
  inline const Message PAUSE_SIMULATION = {MessageType::SERVER_CALL, "pause_simulation"};

  //! continue simulation
  inline const Message CONTINUE_SIMULATION = {MessageType::SERVER_CALL, "continue_simulation"};

  //! gets the system info such as CPU and RAM usage
  inline const Message GET_SYSTEM_INFORMATION = {MessageType::SERVER_CALL, "get_system_information"};
}

#endif