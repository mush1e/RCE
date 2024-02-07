#include <regex>
#include "request_handler.hpp"
#include "database.hpp"
#include "utils.hpp"

#ifndef CONTROLLER_HPP
#define CONTRILLER_HPP

void handle_registration(HTTPRequest& req, int client_socket);

#endif