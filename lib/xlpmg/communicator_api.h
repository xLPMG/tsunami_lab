/**
 * @author Luca-Philipp Grumbach
 *
 * @brief Communicator API.
 *
 * # Description
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
  /**
   * @brief Enum representing the different parts of a message.
   *
   * The MessagePart enum defines the different parts of a message, including the expectation, urgency, key, and arguments.
   */
  enum MessagePart
  {
    EXPECTATION,
    URGENCY,
    KEY,
    ARGS
  };

  enum MessageExpectation
  {
    NO_RESPONSE,
    EXPECT_RESPONSE
  };

  enum MessageUrgency
  {
    CRITICAl,
    HIGH,
    MEDIUM,
    LOW
  };

  /**
   * @brief Macro to briefly define a mapping between MessageExpectation enum and JSON
   */
  NLOHMANN_JSON_SERIALIZE_ENUM(MessageExpectation, {{NO_RESPONSE, "no_response"},
                                                    {EXPECT_RESPONSE, "expect_response"}});

  /**
   * @brief Macro to briefly define a mapping between MessageUrgency enum and JSON
   */
  NLOHMANN_JSON_SERIALIZE_ENUM(MessageUrgency, {{CRITICAl, "critical"},
                                                {HIGH, "high"},
                                                {MEDIUM, "medium"},
                                                {LOW, "low"}});

  /**
   * Struct representing a message in the communicator API.
   *
   * # Description
   * This struct contains information about the urgency, key, and arguments of a message.
   */
  struct Message
  {
    // The expectation of the message.
    MessageExpectation expectation = MessageExpectation::NO_RESPONSE;
    // The urgency of the message.
    MessageUrgency urgency = MessageUrgency::MEDIUM;
    // The key associated with the message.
    std::string key = "NONE";
    // The arguments of the message.
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
    msg[MessagePart::EXPECTATION] = i_message.expectation;
    msg[MessagePart::URGENCY] = i_message.urgency;
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
    l_message.expectation = i_json.at(MessagePart::EXPECTATION);
    l_message.urgency = i_json.at(MessagePart::URGENCY);
    l_message.key = i_json.at(MessagePart::KEY);
    l_message.args = i_json.at(MessagePart::ARGS);
    return l_message;
  }

  /////////////////////////////////
  //         NO_RESPONSE         //
  /////////////////////////////////

  // CRITICAL

  //! Server will stop the running simulation.
  inline const Message KILL_SIMULATION = {MessageExpectation::NO_RESPONSE, MessageUrgency::CRITICAl, "kill_simulation"};
  //! Tells the Simulator to write a checkpoint.
  inline const Message WRITE_CHECKPOINT = {MessageExpectation::NO_RESPONSE, MessageUrgency::CRITICAl, "write_checkpoint"};
  //! Pauses a simulation.
  inline const Message PAUSE_SIMULATION = {MessageExpectation::NO_RESPONSE, MessageUrgency::CRITICAl, "pause_simulation"};
  //! Tells the server to shutdown.
  inline const Message SHUTDOWN_SERVER = {MessageExpectation::NO_RESPONSE, MessageUrgency::CRITICAl, "shutdown_server"};

  // HIGH

  //! Should not not induce any functionality and is only used to check if the other side responds
  inline const Message CHECK = {MessageExpectation::NO_RESPONSE, MessageUrgency::HIGH, "CHECK"};
  //! Tells the server to start the simulator.
  inline const Message START_SIMULATION = {MessageExpectation::NO_RESPONSE, MessageUrgency::HIGH, "start_simulation"};
    //! Continues a simulation.
  inline const Message CONTINUE_SIMULATION = {MessageExpectation::NO_RESPONSE, MessageUrgency::HIGH, "continue_simulation"};
  //! Tells the Simulator to reset.
  inline const Message RESET_SIMULATOR = {MessageExpectation::NO_RESPONSE, MessageUrgency::HIGH, "reset_simulator"};
  //! Tells the Simulator to toggle file i/o usage to given argument.
  inline const Message TOGGLE_FILEIO = {MessageExpectation::NO_RESPONSE, MessageUrgency::HIGH, "toggle_fileio"};

  // MEDIUM

  //! Server will recompile with provided arguments.
  inline const Message COMPILE = {MessageExpectation::NO_RESPONSE, MessageUrgency::MEDIUM, "compile", ""};
  //! Server will recompile with provided arguments and run using a bash script.
  inline const Message COMPILE_RUN_BASH = {MessageExpectation::NO_RESPONSE, MessageUrgency::MEDIUM, "compile_run_bash", ""};
  //! Server will recompile with provided arguments and run using an sbatch script.
  inline const Message COMPILE_RUN_SBATCH = {MessageExpectation::NO_RESPONSE, MessageUrgency::MEDIUM, "compile_run_sbatch", ""};
    //! Deletes checkpoints.
  inline const Message DELETE_CHECKPOINTS = {MessageExpectation::NO_RESPONSE, MessageUrgency::MEDIUM, "delete_checkpoints"};
  //! Deletes stations.
  inline const Message DELETE_STATIONS = {MessageExpectation::NO_RESPONSE, MessageUrgency::MEDIUM, "delete_stations"};
  //! Sets the cell amount of the simulation.
  inline const Message SET_OFFSET = {MessageExpectation::NO_RESPONSE, MessageUrgency::MEDIUM, "set_offset"};
  //! Sets the offset of the simulation.
  inline const Message SET_CELL_AMOUNT = {MessageExpectation::NO_RESPONSE, MessageUrgency::MEDIUM, "set_cell_amount"};

  // LOW

  //! For sending a file to the server
  inline const Message SEND_FILE = {MessageExpectation::NO_RESPONSE, MessageUrgency::LOW, "send_file"};
  //! Tells the Simulator to load config from json data.
  inline const Message LOAD_CONFIG_JSON = {MessageExpectation::NO_RESPONSE, MessageUrgency::LOW, "load_config_json"};
  //! Tells the Simulator to load config from .json config file.
  inline const Message LOAD_CONFIG_FILE = {MessageExpectation::NO_RESPONSE, MessageUrgency::LOW, "load_config_file"};
  //! Tells the server to change the read buffer size.
  inline const Message SET_READ_BUFFER_SIZE = {MessageExpectation::NO_RESPONSE, MessageUrgency::MEDIUM, "set_read_buffer_size"};
  //! Tells the server to change the send buffer size.
  inline const Message SET_SEND_BUFFER_SIZE = {MessageExpectation::NO_RESPONSE, MessageUrgency::MEDIUM, "set_send_buffer_size"};

  ////////////////////////////////
  //      EXPECT_RESPONSE       //
  ////////////////////////////////

  // CRITICAL

  //! Returns the current timestep from the simulator.
  inline const Message GET_TIME_VALUES = {MessageExpectation::EXPECT_RESPONSE, MessageUrgency::CRITICAl, "get_time_values"};
  //! Gets system info such as CPU and RAM usage.
  inline const Message GET_SYSTEM_INFORMATION = {MessageExpectation::EXPECT_RESPONSE, MessageUrgency::CRITICAl, "get_system_information"};
  //! Returns the current simulation sizes from the simulator.
  inline const Message GET_SIMULATION_SIZES = {MessageExpectation::EXPECT_RESPONSE, MessageUrgency::CRITICAl, "get_simulation_sizes"};

  // HIGH

  //! Tells the server to start sending height data. (buffered)
  inline const Message GET_HEIGHT_DATA = {MessageExpectation::EXPECT_RESPONSE, MessageUrgency::MEDIUM, "get_height_data"};
  //! Tells the server to start sending bathymetry data. (buffered)
  inline const Message GET_BATHYMETRY_DATA = {MessageExpectation::EXPECT_RESPONSE, MessageUrgency::MEDIUM, "get_bathymetry_data"};

  // MEDIUM

  //! For receiving a file from the server
  inline const Message RECV_FILE = {MessageExpectation::EXPECT_RESPONSE, MessageUrgency::LOW, "recv_file"};

  ////////////////////////////////
  //      SERVER_RESPONSE       //
  ////////////////////////////////

  //! Tells the client that a buffered sending operation has finished.
  inline const Message BUFFERED_SEND_FINISHED = {MessageExpectation::NO_RESPONSE, MessageUrgency::CRITICAl, "buff_send_finished"};
  //! Server response template
  inline const Message SERVER_RESPONSE = {MessageExpectation::NO_RESPONSE, MessageUrgency::CRITICAl, "server_response"};
}

#endif