#include "request_handler.hpp"

#ifndef UTILS_HPP
#define UTILS_HPP

void display_request(HTTPRequest& req);
auto get_form_field(const std::string& body, const std::string& field_name) -> std::string;

#endif