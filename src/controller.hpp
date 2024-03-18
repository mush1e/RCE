#include <regex>
#include "request_handler.hpp"
#include "session_management.hpp"
#include "database.hpp"
#include "utils.hpp"

#ifndef CONTROLLER_HPP
#define CONTRILLER_HPP

void handle_registration(HTTPRequest& req, int client_socket);
void handle_authentication(HTTPRequest& req, int client_socket);
void handle_get_problems(HTTPRequest& req, int client_socket);
void handle_view_problem(HTTPRequest& req, int client_socket, int problem_id);
void handle_is_auth(HTTPRequest& req, int client_socket);
void handle_is_author(HTTPRequest& req, int client_socket);
void handle_submission(HTTPRequest& req, int client_socket);
void handle_run(HTTPRequest& req, int client_socket);
void handle_logout(HTTPRequest& req, int client_socket);
void handle_search(HTTPRequest& req, int client_socket, std::string query);
void handle_add_problem(HTTPRequest& req, int client_socket);


#endif
