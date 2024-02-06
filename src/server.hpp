#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <sys/select.h>

#ifndef SERVER_HPP
#define SERVER_HPP

#include "database.hpp"
#include "request_handler.hpp"


class HTTP_Server {
    int server_socket {};
    int port {};
    sockaddr_in server_address;

    public:
        HTTP_Server(int port);
        ~HTTP_Server();
        void start();
};

#endif // SERVER_HPP