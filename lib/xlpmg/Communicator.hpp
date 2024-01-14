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

namespace xlpmg
{
    class Communicator
    {
    private:
        int TIMEOUT = 2;
        std::string clientLog = "";
        int sockStatus, sockValread, sockClient_fd = -1;
        int server_fd, new_socket;

        enum LogType
        {
            SENT,
            RECEIVED,
            ERROR,
            INFO
        };

        /// @brief Adds a string to the client log with correct formatting.
        /// @param message string to add to the log.
        /// @param logtype type of message.
        void logToClient(std::string message, LogType logtype)
        {
            auto now = std::chrono::system_clock::now();
            auto timer = std::chrono::system_clock::to_time_t(now);
            std::tm bt = *std::localtime(&timer);
            std::ostringstream oss;
            oss << "[" << std::put_time(&bt, "%H:%M:%S") << "]";
            std::string timeStamp = oss.str();
            clientLog.append(timeStamp);
            switch (logtype)
            {
            case SENT:
                clientLog.append(" Sent    : ");
                break;
            case RECEIVED:
                clientLog.append(" Received: ");
                break;
            case ERROR:
                clientLog.append(" Error   : ");
                break;
            case INFO:
                clientLog.append(" Info    : ");
                break;
            }
            clientLog.append(message);
            clientLog.append("\n");
        }

    public:
        //! size of the reading buffer
        const unsigned int BUFF_SIZE_DEFAULT = 8096;
        unsigned int BUFF_SIZE = BUFF_SIZE_DEFAULT;

        void setReadBufferSize(unsigned int newSize)
        {
            BUFF_SIZE = newSize;
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
                logToClient("Socket creation error", ERROR);
                return -1;
            }
            logToClient("Socket created.", INFO);

            setsockopt(sockClient_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
            setsockopt(sockClient_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(PORT);

            // Convert IPv4 and IPv6 addresses from text to binary
            // form
            if (inet_pton(AF_INET, IPADDRESS, &serv_addr.sin_addr) <= 0)
            {
                logToClient("Invalid address/ Address not supported.", ERROR);
                return -1;
            }

            if ((sockStatus = connect(sockClient_fd, (struct sockaddr *)&serv_addr,
                                      sizeof(serv_addr))) < 0)
            {
                logToClient("Connection failed.", ERROR);
                return -1;
            }
            std::string ipString = std::string(IPADDRESS) + ":" + std::to_string(PORT);
            logToClient("Socket connected to " + ipString, INFO);
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
                logToClient("Reading failed: Socket not initialized.", ERROR);
                return "FAIL";
            }
            char readBuffer[BUFF_SIZE];
            memset(readBuffer, 0, BUFF_SIZE);
            sockValread = read(sockClient_fd, readBuffer,
                               BUFF_SIZE - 1); // subtract 1 for the null
                                               // terminator at the end
            if (sockValread < 0)
            {
                logToClient("Reading failed or timed out.", ERROR);
                return "FAIL";
            }
            else
            {
                logToClient(std::string(readBuffer), RECEIVED);
                return std::string(readBuffer);
            }
        }

        /// @brief Sends a message to the server.
        /// @param message String to send.
        int sendToServer(std::string message)
        {
            if (sockClient_fd < 0)
            {
                clientLog.append("Error   : Sending failed: Socket not initialized \n");
                return 1;
            }
            send(sockClient_fd, message.c_str(), strlen(message.c_str()), MSG_NOSIGNAL);
            logToClient(message, SENT);

            return strcmp(receiveFromServer().c_str(), "OK");
        }

        /// @brief Gets the log data of the client.
        /// @param o_clientLog Pointer to the string which the log will be written into.
        void getClientLog(std::string &o_clientLog)
        {
            o_clientLog = clientLog;
        }

        /// @brief Clears the log data of the client.
        void clearClientLog()
        {
            clientLog.clear();
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
            char readBuffer[BUFF_SIZE];
            memset(readBuffer, 0, BUFF_SIZE);
            sockValread = read(new_socket, readBuffer,
                               BUFF_SIZE - 1); // subtract 1 for the null
                                               // terminator at the end
            send(new_socket, "OK", strlen("OK"), MSG_NOSIGNAL);
            return std::string(readBuffer);
        }

        /// @brief Sends a message to a client.
        /// @param message Message to send.
        void sendToClient(std::string message)
        {
            send(new_socket, message.c_str(), strlen(message.c_str()), MSG_NOSIGNAL);
        }
    };
}
#endif