#include "request_handler.hpp"
#include <string>
#include <sstream>


#ifndef UTILS_HPP
#define UTILS_HPP

void display_request(HTTPRequest& req);
auto get_form_field(const std::string& body, const std::string& field_name) -> std::string;
std::string decode_file_data(HTTPRequest& req);
auto url_decode(const std::string& encoded) -> std::string;
std::string escape_string(const std::string&);
#endif
