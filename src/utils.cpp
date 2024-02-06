#include "utils.hpp"

auto display_request(HTTPRequest& req) -> void {
    std::cout << req.URI << req.method << std::endl;
}

std::string get_form_field(const std::string& body, const std::string& field_name) {
    std::string field_value;
    size_t pos = body.find(field_name + "=");

    if (pos != std::string::npos) {
        pos += field_name.length() + 1;
        size_t end_pos = body.find("&", pos);
        
        end_pos = (end_pos == std::string::npos) 
                    ?  body.length()
                    :  end_pos;
        
        field_value = body.substr(pos, end_pos - pos);
    }
    return field_value;
}
