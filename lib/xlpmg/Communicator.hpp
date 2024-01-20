/**
 * @author Luca-Philipp Grumbach
 *
 * # Description
 * Library to easily create a client-server connection and handle its communication and logging.
 **/
#ifndef COMMUNICATOR
#define COMMUNICATOR

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
        int TIMEOUT = 2;
        std::string logData = "";
        int sockStatus, sockValread, sockClient_fd = -1;
        int server_fd, new_socket;

        enum LogType
        {
            SENT,
            RECEIVED,
            ERROR,
            INFO,
            DEBUG
        };

        /// @brief Adds a string to the log with correct formatting.
        /// @param message string to add to the log.
        /// @param logtype type of message.
        void logEvent(std::string message, LogType logtype, bool replaceLastLine = false)
        {
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
                logData = logData.substr(0, position+1);
                logData.append(line);
                logData.append("\n");
            }
        }

    public:
        //! default size of the reading buffer
        const unsigned int BUFF_SIZE_READ_DEFAULT = 8096;
        //! actual size of the reading buffer
        unsigned int BUFF_SIZE_READ = BUFF_SIZE_READ_DEFAULT;

        //! default size of the sending buffer
        const unsigned int BUFF_SIZE_SEND_DEFAULT = 8096;
        //! actual size of the sending buffer
        unsigned int BUFF_SIZE_SEND = BUFF_SIZE_SEND_DEFAULT;

        void setReadBufferSize(unsigned int newSize)
        {
            BUFF_SIZE_READ = newSize;
        }

        void setSendBufferSize(unsigned int newSize)
        {
            BUFF_SIZE_SEND = newSize;
        }

        ////////////////////
        //     CLIENT     //
        ////////////////////

        /// @brief Creates a client socket and searches for a server.
        /// @param PORT Port for communication with server.
        /// @return 0 on success, -1 for errors.
        int startClient(char *IPADDRESS, int PORT)
        {
            struct sockaddr_in serv_addr;
            struct timeval tv;
            tv.tv_sec = TIMEOUT;
            tv.tv_usec = 0;

            if ((sockClient_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                logEvent("Socket creation error", ERROR);
                return -1;
            }
            logEvent("Socket created.", INFO);

            setsockopt(sockClient_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
            setsockopt(sockClient_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(PORT);

            // Convert IPv4 and IPv6 addresses from text to binary
            // form
            if (inet_pton(AF_INET, IPADDRESS, &serv_addr.sin_addr) <= 0)
            {
                logEvent("Invalid address/ Address not supported.", ERROR);
                return -1;
            }

            if ((sockStatus = connect(sockClient_fd, (struct sockaddr *)&serv_addr,
                                      sizeof(serv_addr))) < 0)
            {
                logEvent("Connection failed.", ERROR);
                return -1;
            }
            std::string ipString = std::string(IPADDRESS) + ":" + std::to_string(PORT);
            logEvent("Socket connected to " + ipString, INFO);
            return sockStatus;
        }

        /// @brief Stops all connections of the client socket.
        void stopClient()
        {
            // closing the connected socket
            close(sockClient_fd);
        }

        /// @brief Receives a message from the server.
        /// @return Message as string.
        std::string receiveFromServer()
        {
            if (sockClient_fd < 0)
            {
                logEvent("Reading failed: Socket not initialized.", ERROR);
                return "FAIL";
            }
            std::string message = "";
            char readBuffer[BUFF_SIZE_READ];
            bool finished = false;
            unsigned long totalBytes = 0;

            logEvent(std::to_string(totalBytes) + " Bytes (" + std::to_string(totalBytes / 1000000) + " MB) received", DEBUG);

            while (!finished)
            {
                memset(readBuffer, 0, BUFF_SIZE_READ);
                sockValread = read(sockClient_fd, readBuffer,
                                   BUFF_SIZE_READ - 1); // subtract 1 for the null
                                                        // terminator at the end
                if (sockValread < 0)
                {
                    logEvent("Reading failed or timed out.", ERROR);
                    return "FAIL";
                }
                totalBytes += strlen(readBuffer);
                logEvent(std::to_string(totalBytes) + " Bytes (" + std::to_string(totalBytes / 1000000) + " MB) received", DEBUG, true);

                if (strncmp("DONE", readBuffer, 4) == 0)
                {
                    if (message.length() < BUFF_SIZE_READ)
                    {
                        logEvent(message, RECEIVED);
                    }
                    else
                    {
                        logEvent("Message is bigger than buffer and wont be displayed.", RECEIVED);
                    }
                    finished = true;
                }
                else
                {
                    message += std::string(readBuffer);
                }
            }
            return message;
        }

        /// @brief Sends a message to the server.
        /// @param message String to send.
        int sendToServer(std::string message)
        {
            if (sockClient_fd < 0)
            {
                logEvent("Sending failed: Socket not initialized.", ERROR);
                return 1;
            }

            if (strlen(message.c_str()) < BUFF_SIZE_SEND)
            {
                send(sockClient_fd, message.c_str(), strlen(message.c_str()), MSG_NOSIGNAL);
                logEvent(message, SENT);
                std::string bytesSentStr = "=> " + std::to_string(strlen(message.c_str())) + " Bytes";
                logEvent(bytesSentStr.c_str(), DEBUG);
            }
            else
            {
                logEvent("Sending buffered message (" + std::to_string(strlen(message.c_str())) + " Bytes = " + std::to_string((double)strlen(message.c_str()) / 1000000) + " MB)", INFO);
                logEvent("0%", DEBUG);
                unsigned long bytes_total = 0;
                while (bytes_total < strlen(message.c_str()))
                {
                    unsigned long bytes_sent = send(sockClient_fd, message.c_str() + bytes_total, BUFF_SIZE_SEND - 1, 0);
                    bytes_total += bytes_sent;
                    int percentage = std::max((double)0, std::min(((double)bytes_total / strlen(message.c_str())) * 100, (double)100));
                    std::string bytesSentStr = std::to_string(percentage) + "%";
                    logEvent(bytesSentStr.c_str(), DEBUG, true);
                }
            }
            send(sockClient_fd, "DONE", 4, MSG_NOSIGNAL);

            return 0;
        }

        /// @brief Gets the log data.
        /// @param o_logData Pointer to the string which the log will be written into.
        void getLog(std::string &o_logData)
        {
            o_logData = logData;
        }

        /// @brief Clears the log data.
        void clearLog()
        {
            logData.clear();
        }

        ////////////////////
        //     SERVER     //
        ////////////////////

        /// @brief Creates a server socket and waits for connections.
        /// @param PORT Port for communication with client.
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
                exit(EXIT_FAILURE);
            }

            // Forcefully attaching socket to the port 8080
            if (setsockopt(server_fd, SOL_SOCKET,
                           SO_REUSEADDR, &opt,
                           sizeof(opt)))
            {
                perror("setsockopt");
                exit(EXIT_FAILURE);
            }
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(PORT);

            // Forcefully attaching socket to the port 8080
            if (bind(server_fd, (struct sockaddr *)&address,
                     sizeof(address)) < 0)
            {
                perror("bind failed");
                exit(EXIT_FAILURE);
            }
            if (listen(server_fd, 3) < 0)
            {
                perror("listen");
                exit(EXIT_FAILURE);
            }
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                     &addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
        }

        /// @brief Stops all connections of the server.
        void stopServer()
        {
            // closing the connected socket
            close(new_socket);
            // closing the listening socket
            close(server_fd);
        }

        /// @brief Receives a message froma  client.
        /// @return Message as string.
        std::string receiveFromClient()
        {
            if (new_socket < 0)
            {
                logEvent("Reading failed: Socket not initialized.", ERROR);
                return "FAIL";
            }
            std::string message = "";
            char readBuffer[BUFF_SIZE_READ];
            bool finished = false;
            unsigned long totalBytes = 0;

            logEvent(std::to_string(totalBytes) + " Bytes (" + std::to_string(totalBytes / 1000000) + " MB) received", DEBUG);
            while (!finished)
            {
                memset(readBuffer, 0, BUFF_SIZE_READ);
                sockValread = read(new_socket, readBuffer,
                                   BUFF_SIZE_READ - 1); // subtract 1 for the null
                                                        // terminator at the end
                if (sockValread < 0)
                {
                    logEvent("Reading failed or timed out.", ERROR);
                    return "FAIL";
                }
                totalBytes += strlen(readBuffer);
                logEvent(std::to_string(totalBytes) + " Bytes (" + std::to_string(totalBytes / 1000000) + " MB) received", DEBUG, true);

                if (strncmp("DONE", readBuffer, 4) == 0)
                {
                    if (message.length() < BUFF_SIZE_READ)
                    {
                        logEvent(message, RECEIVED);
                    }
                    else
                    {
                        logEvent("Message is bigger than buffer and wont be displayed.", RECEIVED);
                    }
                    finished = true;
                }
                else
                {
                    message += std::string(readBuffer);
                }
            }
            return message;
        }

        /// @brief Sends a message to a client.
        /// @param message Message to send.
        void sendToClient(std::string message)
        {
            if (strlen(message.c_str()) < BUFF_SIZE_SEND)
            {
                send(new_socket, message.c_str(), strlen(message.c_str()), MSG_NOSIGNAL);
                logEvent(message, SENT);
                std::string bytesSentStr = "=> " + std::to_string(strlen(message.c_str())) + " Bytes";
                logEvent(bytesSentStr.c_str(), DEBUG);
            }
            else
            {
                logEvent("Sending buffered message (" + std::to_string(strlen(message.c_str())) + " Bytes = " + std::to_string((double)strlen(message.c_str()) / 1000000) + " MB)", INFO);
                logEvent("0%", DEBUG);
                unsigned long bytes_total = 0;
                while (bytes_total < strlen(message.c_str()))
                {
                    unsigned long bytes_sent = send(new_socket, message.c_str() + bytes_total, BUFF_SIZE_SEND - 1, 0);
                    bytes_total += bytes_sent;
                    int percentage = std::max((double)0, std::min(((double)bytes_total / strlen(message.c_str())) * 100, (double)100));
                    std::string bytesSentStr = std::to_string(percentage) + "%";
                    logEvent(bytesSentStr.c_str(), DEBUG, true);
                }
            }
            send(new_socket, "DONE", 4, MSG_NOSIGNAL);
        }
    };
}
#endif