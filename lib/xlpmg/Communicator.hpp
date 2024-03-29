/**
 * @author Luca-Philipp Grumbach
 *
 * # Description
 * Library to easily create a client-server connection and handle its communication and logging.
 **/
#ifndef XLPMG_COMMUNICATOR_H
#define XLPMG_COMMUNICATOR_H

#include <stdio.h>
#include <string.h>

// socketing
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// time stamp
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace xlpmg
{
    class Communicator
    {
    private:
        // Log data for storing communication logs
        std::string logData = "";
        // Client socket related variables
        int sockStatus, sockValread, sockClient_fd = -1;
        // Server socket related variables
        int server_fd, new_socket;

        /**
         * @enum LogType
         * @brief Defines different types of log messages that can be logged during communication.
         */
        enum LogType
        {
            SENT,     // Sent message log type
            RECEIVED, // Received message log type
            ERROR,    // Error log type
            INFO,     // Info log type
            DEBUG     // Debug log type
        };

        /**
         *  @brief Adds a string to the log with correct formatting.
         *  @param message string to add to the log.
         *  @param logtype xlpmg::Communicator::LogType
         *  @param log Whether the message should be logged or not.
         *  @param replaceLastLine Whether the last line should be replaced or not.
         *  @see xlpmg::Communicator::LogType
         */
        void logEvent(std::string message, LogType logtype, bool log, bool replaceLastLine = false)
        {
            if (!log)
            {
                return;
            }

            std::string line = "";

            auto now = std::chrono::system_clock::now();
            auto timer = std::chrono::system_clock::to_time_t(now);
            std::tm bt = *std::localtime(&timer);
            std::ostringstream oss;
            oss << "[" << std::put_time(&bt, "%H:%M:%S") << "]";
            std::string timeStamp = oss.str();
            line.append(timeStamp);
            switch (logtype)
            {
            case SENT:
                line.append(" Sent    : ");
                break;
            case RECEIVED:
                line.append(" Received: ");
                break;
            case ERROR:
                line.append(" Error   : ");
                break;
            case INFO:
                line.append(" Info    : ");
                break;
            case DEBUG:
                line.append(" Debug   : ");
                break;
            }
            line.append(message);

            if (!replaceLastLine)
            {
                std::cout << line << std::endl;
                logData.append(line);
                logData.append("\n");
            }
            else
            {
                std::cout << line << "\t\r" << std::flush;
                size_t position = logData.length() - 2;
                while (position > 0 && (logData[position] != '\n'))
                {
                    position--;
                }
                logData = logData.substr(0, position + 1);
                logData.append(line);
                logData.append("\n");
            }
        }

        /**
         * Sets the send timeout for a socket.
         * @param socket The socket to set the send timeout for.
         * @param timeout The timeout value in seconds.
         */
        void setSendTimeout(int socket, long timeout){
            struct timeval tv;
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
        }

        /**
         * Sets the receive timeout for a socket.
         * @param socket The socket to set the receive timeout for.
         * @param timeout The timeout value in seconds.
         */
        void setRecvTimeout(int socket, long timeout){
            struct timeval tv;
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
        }

    public:
        // Timeout value for normal socket operations in seconds
        static const long TIMEOUT = 2;
        //! default size of the reading buffer
        unsigned int BUFF_SIZE_READ_DEFAULT = 8096;
        //! actual size of the reading buffer
        unsigned int BUFF_SIZE_READ = BUFF_SIZE_READ_DEFAULT;
        //! default size of the sending buffer
        unsigned int BUFF_SIZE_SEND_DEFAULT = 8096;
        //! actual size of the sending buffer
        unsigned int BUFF_SIZE_SEND = BUFF_SIZE_SEND_DEFAULT;

        //! true if there is a connection
        bool isConnected = false;

        /**
         * @brief Sets the read buffer size.
         * 
         * @param newSize The new size of the read buffer.
         */
        void setReadBufferSize(unsigned int newSize)
        {
            BUFF_SIZE_READ = newSize;
        }

        /**
         * @brief Sets the send buffer size.
         * 
         * @param newSize The new size of the send buffer.
         */
        void setSendBufferSize(unsigned int newSize)
        {
            BUFF_SIZE_SEND = newSize;
        }

        ////////////////////
        //     CLIENT     //
        ////////////////////

        /**
         * @brief Creates a client socket and searches for a server.
         * @param PORT Port for communication with server.
         * @return 0 on success, -1 for errors.
         */
        int startClient(char *IPADDRESS, int PORT)
        {
            struct sockaddr_in serv_addr;

            if ((sockClient_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                logEvent("Socket creation error", ERROR, true);
                isConnected = false;
                return -1;
            }
            logEvent("Socket created.", INFO, true);

            setSendTimeout(sockClient_fd, TIMEOUT);
            setRecvTimeout(sockClient_fd, TIMEOUT);

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(PORT);

            // Convert IPv4 and IPv6 addresses from text to binary
            // form
            if (inet_pton(AF_INET, IPADDRESS, &serv_addr.sin_addr) <= 0)
            {
                logEvent("Invalid address/ Address not supported.", ERROR, true);
                isConnected = false;
                return -1;
            }

            if ((sockStatus = connect(sockClient_fd, (struct sockaddr *)&serv_addr,
                                      sizeof(serv_addr))) < 0)
            {
                logEvent("Connection failed.", ERROR, true);
                isConnected = false;
                return -1;
            }
            std::string ipString = std::string(IPADDRESS) + ":" + std::to_string(PORT);
            logEvent("Socket connected to " + ipString, INFO, true);
            isConnected = true;
            return sockStatus;
        }

        /**
         * @brief Stops all connections of the client socket.
         */
        void stopClient()
        {
            // closing the connected socket
            close(sockClient_fd);
            isConnected = false;
        }

        /**
         * @brief Receives a message from the server.
         * @return Message as string.
         * @param timeout Timeout for the operation in seconds.
         * @param log Whether the message should be logged or not.
         */
        std::string receiveFromServer(long timeout = TIMEOUT, bool log = true)
        {
            if (sockClient_fd < 0)
            {
                logEvent("Reading failed: Socket not initialized.", ERROR, log);
                isConnected = false;
                return "FAIL";
            }
            std::string message = "";
            char readBuffer[BUFF_SIZE_READ];
            bool finished = false;
            unsigned long totalBytes = 0;

            setRecvTimeout(sockClient_fd, timeout);

            logEvent(std::to_string(totalBytes) + " Bytes (" + std::to_string(totalBytes / 1000000) + " MB) received             ", DEBUG, log);

            while (!finished)
            {
                memset(readBuffer, 0, BUFF_SIZE_READ);
                sockValread = read(sockClient_fd, readBuffer,
                                   BUFF_SIZE_READ - 1); // subtract 1 for the null
                                                        // terminator at the end
                if (sockValread < 0)
                {
                    logEvent("Reading failed or timed out.", ERROR, log);
                    isConnected = false;
                    return "FAIL";
                }

                if (strlen(readBuffer) > 0)
                {
                    logEvent(std::to_string(totalBytes) + " Bytes (" + std::to_string(totalBytes / 1000000) + " MB) received             ", DEBUG, log, true);
                }

                message += std::string(readBuffer);
                totalBytes += strlen(readBuffer);

                if (message[message.length() - 2] == '#' && message[message.length() - 1] == '!')
                {
                    message.pop_back();
                    message.pop_back();
                    if (message.length() < 400)
                    {
                        logEvent(message, RECEIVED, log);
                    }
                    else
                    {
                        logEvent("Message is too long to be displayed.", RECEIVED, log);
                    }
                    finished = true;
                }
            }
            setRecvTimeout(sockClient_fd, TIMEOUT);
            isConnected = true;
            return message;
        }

        /**
         * @brief Sends a message to the server.
         * @param message String to send.
         * @param timeout Timeout for the operation in seconds.
         * @param log Whether the message should be logged or not.
         * @return 0 if successful, 1 otherwise.
         */
        int sendToServer(std::string message, long timeout = TIMEOUT, bool log = true)
        {
            if (sockClient_fd < 0)
            {
                logEvent("Sending failed: Socket not initialized.", ERROR, log);
                isConnected = false;
                return 1;
            }

            setSendTimeout(sockClient_fd, timeout);

            // terminator
            message.append("#!");

            if (message.length() < BUFF_SIZE_SEND)
            {
                send(sockClient_fd, message.c_str(), strlen(message.c_str()), 0);
                logEvent(message, SENT, log);
                std::string bytesSentStr = "=> " + std::to_string(strlen(message.c_str())) + " Bytes";
                logEvent(bytesSentStr.c_str(), DEBUG, log);
            }
            else
            {
                logEvent("Sending buffered message (" + std::to_string(strlen(message.c_str())) + " Bytes = " + std::to_string((double)strlen(message.c_str()) / 1000000) + " MB)             ", INFO, log);
                logEvent("0%", DEBUG, log);
                unsigned long bytes_total = 0;
                const char *data_ptr = message.data();
                std::size_t data_size = message.size();
                int bytes_sent;
                while (data_size > 0)
                {
                    bytes_sent = send(sockClient_fd, data_ptr, data_size, 0);
                    if (bytes_sent < 0)
                    {
                        isConnected = false;
                        return -1;
                    }

                    data_ptr += bytes_sent;
                    bytes_total += bytes_sent;
                    data_size -= bytes_sent;
                    int percentage = std::max((double)0, std::min(((double)bytes_total / strlen(message.c_str())) * 100, (double)100));
                    std::string bytesSentStr = std::to_string(percentage) + "%";
                    logEvent(bytesSentStr.c_str(), DEBUG, log, true);
                }
            }
            setSendTimeout(sockClient_fd, TIMEOUT);
            isConnected = true;
            return 0;
        }

        /**
         * @brief Gets the log data.
         * @param o_logData Pointer to the string which the log will be written into.
         */
        void getLog(std::string &o_logData)
        {
            o_logData = logData;
        }

        /**
         * @brief Clears the log data.
         */
        void clearLog()
        {
            logData.clear();
        }

        ////////////////////
        //     SERVER     //
        ////////////////////

        /**
         * @brief Creates a server socket and waits for connections.
         * @param PORT Port for communication with client.
         */
        void startServer(int PORT)
        {
            ssize_t valread;
            struct sockaddr_in address;
            int opt = 1;
            socklen_t addrlen = sizeof(address);

            // Creating socket file descriptor
            if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("socket failed");
                isConnected = false;
                exit(EXIT_FAILURE);
            }

            // Forcefully attaching socket to the port 8080
            if (setsockopt(server_fd, SOL_SOCKET,
                           SO_REUSEADDR, &opt,
                           sizeof(opt)))
            {
                perror("setsockopt");
                isConnected = false;
                exit(EXIT_FAILURE);
            }
            
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(PORT);

            setSendTimeout(new_socket, TIMEOUT);

            // Forcefully attaching socket to the port 8080
            if (bind(server_fd, (struct sockaddr *)&address,
                     sizeof(address)) < 0)
            {
                perror("bind failed");
                isConnected = false;
                exit(EXIT_FAILURE);
            }
            if (listen(server_fd, 3) < 0)
            {
                perror("listen");
                isConnected = false;
                exit(EXIT_FAILURE);
            }
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                     &addrlen)) < 0)
            {
                perror("accept");
                isConnected = false;
                exit(EXIT_FAILURE);
            }
            isConnected = true;
        }

        /**
         * @brief Stops all connections of the server.
         */
        void stopServer()
        {
            // closing the connected socket
            close(new_socket);
            // closing the listening socket
            close(server_fd);

            isConnected = false;
        }

        /**
         * @brief Receives a message from a client.
         * @return Message as string.
         * @param log Whether the message should be logged or not.
         */
        std::string receiveFromClient(bool log = true)
        {
            if (new_socket < 0)
            {
                logEvent("Reading failed: Socket not initialized.", ERROR, log);
                isConnected = false;
                return "FAIL";
            }
            std::string message = "";
            char readBuffer[BUFF_SIZE_READ];
            bool finished = false;
            unsigned long totalBytes = 0;

            while (!finished)
            {
                memset(readBuffer, 0, BUFF_SIZE_READ);
                sockValread = read(new_socket, readBuffer,
                                   BUFF_SIZE_READ - 1);
                if (sockValread < 0)
                {
                    logEvent("Reading failed or timed out.", ERROR, log);
                    isConnected = false;
                    return "FAIL";
                }

                message += std::string(readBuffer);
                totalBytes += strlen(readBuffer);

                if (message[message.length() - 2] == '#' && message[message.length() - 1] == '!')
                {
                    message.pop_back();
                    message.pop_back();
                    if (message.length() < 400)
                    {
                        logEvent(message, RECEIVED, log);
                    }
                    else
                    {
                        logEvent("Message is too long to be displayed.", RECEIVED, log);
                    }
                    finished = true;
                }
            }
            return message;
        }

        /**
         * @brief Sends a message to a client.
         * @param message Message to send.
         * @param log Whether the message should be logged or not.
         */
        void sendToClient(std::string message, bool log = true)
        {
            // terminator
            message.append("#!");

            if (strlen(message.c_str()) < BUFF_SIZE_SEND)
            {
                send(new_socket, message.c_str(), strlen(message.c_str()), 0);
                logEvent(message, SENT, log);
                std::string bytesSentStr = "=> " + std::to_string(strlen(message.c_str())) + " Bytes";
                logEvent(bytesSentStr.c_str(), DEBUG, log);
            }
            else
            {
                logEvent("Sending buffered message (" + std::to_string(strlen(message.c_str())) + " Bytes = " + std::to_string((double)strlen(message.c_str()) / 1000000) + " MB)             ", INFO, log);
                logEvent("0%", DEBUG, log);
                unsigned long bytes_total = 0;
                const char *data_ptr = message.data();
                std::size_t data_size = message.size();
                int bytes_sent;
                while (data_size > 0)
                {
                    bytes_sent = send(new_socket, data_ptr, data_size, 0);
                    if (bytes_sent < 0)
                    {
                        return;
                    }

                    data_ptr += bytes_sent;
                    bytes_total += bytes_sent;
                    data_size -= bytes_sent;
                    int percentage = std::max((double)0, std::min(((double)bytes_total / strlen(message.c_str())) * 100, (double)100));
                    std::string bytesSentStr = std::to_string(percentage) + "%";
                    logEvent(bytesSentStr.c_str(), DEBUG, log, true);
                }
            }
        }
    };
}
#endif
