#include <regex>
#include <unistd.h>
#include <fstream>
#include "request_handler.hpp"
#include "session_management.hpp"
#include "filesystem_manager.hpp"
#include "database.hpp"
#include "utils.hpp"

#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

// Controller methods for handling HTTP requests related to blog posts and user management
void handle_registration(HTTPRequest& req, int client_socket);
void handle_authentication(HTTPRequest& req, int client_socket);
void handle_get_posts(HTTPRequest& req, int client_socket);
void handle_view_post(HTTPRequest& req, int client_socket, int post_id);
void handle_is_auth(HTTPRequest& req, int client_socket);
void handle_submission(HTTPRequest& req, int client_socket);
void handle_logout(HTTPRequest& req, int client_socket);
void handle_search(HTTPRequest& req, int client_socket, std::string query);
void handle_add_post(HTTPRequest& req, int client_socket);
void handle_delete_post(HTTPRequest& req, int client_socket, int post_id);
void handle_update_post(HTTPRequest& req, int client_socket, int post_id);

// Helper methods
bool is_author(HTTPRequest& req, int post_id);

#endif
