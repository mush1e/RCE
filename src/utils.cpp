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
    std::cout << field_name << " : " << field_value << std::endl;
    std::cout << body << std::endl;

    return field_value;
}

auto url_decode(const std::string& encoded) -> std::string {
    std::stringstream decoded;
    decoded.fill('0');

    size_t len = encoded.length();
    size_t index = 0;
    while (index < len) {
        if (encoded[index] == '%' && index + 2 < len && isxdigit(encoded[index + 1]) && isxdigit(encoded[index + 2])) {
            // Found an encoded character
            char decoded_char = static_cast<char>(std::stoi(encoded.substr(index + 1, 2), nullptr, 16));
            decoded << decoded_char;
            index += 3; // Move to the next encoded character
        } else if (encoded[index] == '+') {
            // '+' is decoded to space ' ' in URL encoding
            decoded << ' ';
            index++;
        } else {
            // Normal character
            decoded << encoded[index];
            index++;
        }
    }

    return decoded.str();
}