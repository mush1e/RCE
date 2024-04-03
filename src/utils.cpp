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

    return url_decode(field_value);
}

std::string url_decode(const std::string& str) {
    int i = 0;
    std::stringstream decoded;

    while (i < str.length()) {
        if (str[i] == '%') {
            if (i + 2 < str.length()) {
                int hexValue;
                std::istringstream(str.substr(i + 1, 2)) >> std::hex >> hexValue;
                decoded << static_cast<char>(hexValue);
                i += 3;
            } else {
                // If '%' is at the end of the string, leave it unchanged
                decoded << '%';
                i++;
            }
        } else if (str[i] == '+') {
            decoded << ' ';
            i++;
        } else {
            decoded << str[i];
            i++;
        }
    }
    return decoded.str();
}

std::string escape_string(const std::string& input) {
    std::string output;
    for (char c : input) {
        if (c == '\'') {
            output += '\'';
        } else if (c == '\n') {
            output += "<br>\\n";
        } else {
            output += c;
        }
    }
    return output;
}
