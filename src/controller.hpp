#include <regex>
#include "request_handler.hpp"

#ifndef CONTROLLER_HPP
#define CONTRILLER_HPP

#include "database.hpp"
#include "utils.hpp"

void handle_registration(HTTPRequest& req, int client_socket);

#endif