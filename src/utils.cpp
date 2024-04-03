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

std::string decode_file_data(HTTPRequest& req) {
    std::string boundary;
    for (const auto& header : req.headers) {
        if (header.first == "Content-Type") {
            size_t pos = header.second.find("boundary=");
            if (pos != std::string::npos) {
                boundary = header.second.substr(pos + 9); // Length of "boundary="
                break;
            }
        }
    }

    if (boundary.empty()) {
        std::cerr << "Boundary not found in Content-Type header" << std::endl;
        return "";
    }

    std::string decoded_data;
    size_t pos = 0;

    while ((pos = req.body.find("--" + boundary, pos)) != std::string::npos) {
        size_t start_pos = pos + boundary.length() + 2;
        size_t end_pos = req.body.find("\r\n--" + boundary, start_pos);
        if (end_pos == std::string::npos)
            end_pos = req.body.length();

        size_t header_end = req.body.find("\r\n\r\n", start_pos);
        if (header_end != std::string::npos)
            start_pos = header_end + 4;

        std::string part_content = req.body.substr(start_pos, end_pos - start_pos - 2);
        decoded_data += part_content;
        pos = end_pos;
    }
    pos = decoded_data.find_last_of('\n');

    if (pos != std::string::npos)
        decoded_data.erase(pos);

    return decoded_data;
}
