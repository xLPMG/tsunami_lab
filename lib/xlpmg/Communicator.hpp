/**
 * @author Luca-Philipp Grumbach
 *
 * # Description
 * Library to easily create a client-server connection and handle its communication and logging.
 **/
#ifndef COMMUNICATOR
#define COMMUNICATOR

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>

namespace xlpmg
{
    const int BUFF_SIZE = 8096;
    class Communicator
    {
    private:
        char buffer[BUFF_SIZE] = {0};
        int TIMEOUT = 2;
        std::string clientLog = "";
        int sockStatus, sockValread, sockClient_fd = -1;
        int server_fd, new_socket;

    public:
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
                clientLog.append("Error   : Socket creation error \n");
                return -1;
            }
            clientLog.append("Socket created.\n");

            setsockopt(sockClient_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
            setsockopt(sockClient_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(PORT);

            // Convert IPv4 and IPv6 addresses from text to binary
            // form
            if (inet_pton(AF_INET, IPADDRESS, &serv_addr.sin_addr) <= 0)
            {
                clientLog.append("Error   : Invalid address/ Address not supported \n");
                return -1;
            }

            if ((sockStatus = connect(sockClient_fd, (struct sockaddr *)&serv_addr,
                                      sizeof(serv_addr))) < 0)
            {
                clientLog.append("Error   : Connection Failed \n");
                return -1;
            }
            clientLog.append("Socket connected to ");
            clientLog.append(IPADDRESS);
            clientLog.append(":"+std::to_string(PORT)+".\n");
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
                clientLog.append("Error   : Reading failed: Socket not initialized. \n");
                return "FAIL";
            }
            memset(buffer, 0, BUFF_SIZE);
            sockValread = read(sockClient_fd, buffer,
                               BUFF_SIZE - 1); // subtract 1 for the null
                                               // terminator at the end
            if (sockValread < 0)
            {
                clientLog.append("Error   : Read failed or timed out. \n");
                return "FAIL";
            }
            else
            {
                clientLog.append("Received: ");
                clientLog.append(buffer);
                clientLog.append("\n");
                return std::string(buffer);
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
            clientLog.append("Sent    : ");
            clientLog.append(message);
            clientLog.append("\n");

            return strcmp(receiveFromServer().c_str(), "OK");
        }

        /// @brief Gets the log data of the client.
        /// @param o_clientLog Pointer to the string which the log will be written into.
        void getClientLog(std::string &o_clientLog)
        {
            o_clientLog = clientLog;
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
            memset(buffer, 0, BUFF_SIZE);
            sockValread = read(new_socket, buffer,
                               BUFF_SIZE - 1); // subtract 1 for the null
                                               // terminator at the end
            send(new_socket, "OK", strlen("OK"), MSG_NOSIGNAL);
            return std::string(buffer);
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