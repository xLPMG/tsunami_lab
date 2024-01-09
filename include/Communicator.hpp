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
    const int BUFF_SIZE = 4096;
    class Communicator
    {
    private:
        char buffer[BUFF_SIZE] = {0};
        int sockStatus, sockValread, sockClient_fd;
        int server_fd, new_socket;

    public:
        int startClient(int PORT)
        {
            struct sockaddr_in serv_addr;
            if ((sockClient_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                printf("\n Socket creation error \n");
                return -1;
            }

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(PORT);

            // Convert IPv4 and IPv6 addresses from text to binary
            // form
            if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
            {
                printf(
                    "\nInvalid address/ Address not supported \n");
                return -1;
            }

            if ((sockStatus = connect(sockClient_fd, (struct sockaddr *)&serv_addr,
                                      sizeof(serv_addr))) < 0)
            {
                printf("\nConnection Failed \n");
                return -1;
            }
            return sockStatus;
        }

        void stopClient()
        {
            // closing the connected socket
            close(sockClient_fd);
        }

        std::string receiveFromServer()
        {
            memset(buffer, 0, BUFF_SIZE);
            sockValread = read(sockClient_fd, buffer,
                               BUFF_SIZE - 1); // subtract 1 for the null
                                               // terminator at the end
            return std::string(buffer);
        }

        void sendToServer(std::string message)
        {
            send(sockClient_fd, message.c_str(), strlen(message.c_str()), 0);
        }

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

        void stopServer()
        {
            // closing the connected socket
            close(new_socket);
            // closing the listening socket
            close(server_fd);
        }

        std::string receiveFromClient()
        {
            memset(buffer, 0, BUFF_SIZE);
            sockValread = read(new_socket, buffer,
                               BUFF_SIZE - 1); // subtract 1 for the null
                                               // terminator at the end
            return std::string(buffer);
        }

        void sendToClient(std::string message)
        {
            send(new_socket, message.c_str(), strlen(message.c_str()), 0);
        }
    };
}
#endif