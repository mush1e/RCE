#include "request_handler.hpp"

#ifndef UTILS_HPP
#define UTILS_HPP

auto display_request(HTTPRequest& req) -> void;
auto get_form_field(const std::string& body, const std::string& field_name) -> std::string;

#endif