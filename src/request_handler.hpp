#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <sstream>
#include <sstream>
#include <fstream>
#include <unordered_map>

#ifndef REQUEST_HANDLER_HPP
#define REQUEST_HANDLER_HPP

#include "database.hpp"

struct HTTPRequest {
    std::string method   {};
    std::string URI      {};
    std::string version  {};
    std::vector<std::pair<std::string, std::string>> headers {};
    std::vector<std::pair<std::string, std::string>> cookies {};
    std::string body     {};
};

struct HTTPResponse {
    int status_code {};
    std::string status_message {};
    std::string content_type = "text/plain";
    std::string body {};
    std::string location {};
    std::pair<std::string, std::string> cookies {};

    std::string generate_response() const;
    void set_JSON_content(const std::string& json_data);
};

void handle_client(int client_socket);
void parse_request(HTTPRequest& req, const std::string& req_str);
void handle_request(HTTPRequest& req, int client_socket);
void parse_form_data(const std::string& form_data, HTTPRequest& req);
void serveStaticFile(const std::string& filePath, int client_socket);
void sendNotFoundResponse(int client_socket);
std::unordered_map<std::string, std::string> parse_parameters(std::string uri);

#endif // REQUEST_HANDLER_HPP
