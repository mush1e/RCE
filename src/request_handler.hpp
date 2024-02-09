#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>

#ifndef REQUEST_HANDLER_HPP
#define REQUEST_HANDLER_HPP

#include "database.hpp"

struct HTTPRequest {
    std::string method   {};
    std::string URI      {};
    std::string version  {};
    std::vector<std::pair<std::string, std::string>> headers {};
    std::string body     {};
};

void handle_client(int client_socket);
void parse_request(HTTPRequest& req, const std::string& req_str);
void handle_request(HTTPRequest& req, int client_socket);
void parse_form_data(const std::string& form_data, HTTPRequest& req);
void serveStaticFile(const std::string& filePath, int client_socket);
void sendNotFoundResponse(int client_socket);

#endif // REQUEST_HANDLER_HPP
