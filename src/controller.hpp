#include <regex>

#ifndef CONTROLLER_HPP
#define CONTRILLER_HPP

#include "database.hpp"
#include "server.hpp"
#include "utils.hpp"

void handle_registration(HTTPRequest& req, int client_socket);

#endif