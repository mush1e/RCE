#include "request_handler.hpp"
#include "controller.hpp"
#include "utils.hpp"
#include <unordered_map>

void serveStaticFile(const std::string& filePath, int client_socket) {

    std::ifstream file(filePath);

    if (file.good()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: "
                                + std::to_string(content.length())
                                + "\r\n\r\n"
                                + content;

        send(client_socket, response.c_str(), response.length(), 0);
    }
    else
        sendNotFoundResponse(client_socket);
}

void sendNotFoundResponse(int client_socket) {

    std::string notFoundContent = "<h1>404 Not Found</h1>";

    std::string response = "HTTP/1.1 404 Not Found\r\nContent-Length: "
                            + std::to_string(notFoundContent.length())
                            + "\r\n\r\n"
                            + notFoundContent;

    send(client_socket, response.c_str(), response.length(), 0);
}

auto handle_request(HTTPRequest& req, int client_socket) -> void {
    if (req.method == "GET")

        if (req.URI == "/")
            serveStaticFile("./public/index.html", client_socket);

        else if (req.URI == "/login")
            serveStaticFile("./public/login.html", client_socket);

        else if (req.URI == "/register")
            serveStaticFile("./public/register.html", client_socket);

        else if (req.URI == "/get_problems")
            handle_get_problems(req, client_socket);

        else if(req.URI == "/is_auth")
            handle_is_auth(req, client_socket);

        else if (req.URI.find("/view_problem") == 0) {
            std::unordered_map<std::string, std::string> params = parse_parameters(req.URI);
            std::cout << params["id"] << std::endl;
            if (params.find("id") != params.end())
                serveStaticFile("./public/view_problem.html", client_socket);
            else
                sendNotFoundResponse(client_socket);
        }

        else if (req.URI.find("/get_problem") == 0) {
            std::unordered_map<std::string, std::string> params = parse_parameters(req.URI);
            if (params.find("id") != params.end())
                handle_view_problem(req, client_socket, std::atoi(params["id"].c_str()));
            else
                sendNotFoundResponse(client_socket);
        }

        else
            sendNotFoundResponse(client_socket);

    else if (req.method == "POST")

        if (req.URI == "/register")
            handle_registration(req, client_socket);

        else if (req.URI == "/login")
            handle_authentication(req, client_socket);
        
        //TODO 
        else if (req.URI == "/submit") 
            handle_authentication(req, client_socket);

        //TODO
        else if (req.URI == "/run") 
            handle_run(req, client_socket);

         else
            sendNotFoundResponse(client_socket);

    else
        sendNotFoundResponse(client_socket);

}

void sendMethodNotAllowedResponse(int client_socket) {
    std::string notAllowedContent = "<h1>405 Method Not Allowed</h1>";
    std::string response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: " + std::to_string(notAllowedContent.length()) + "\r\n\r\n" + notAllowedContent;
    send(client_socket, response.c_str(), response.length(), 0);
}

auto parse_form_data(const std::string& form_data, HTTPRequest& req) -> void {
    std::istringstream iss(form_data);
    std::string pair;
    while (std::getline(iss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);

            // Decode URL-encoded key and value
            key = url_decode(key);
            value = url_decode(value);
            std::cout << key << ": " << value << std::endl;
            req.body += key + ": " + value + "\n"; // Assuming you want to store key-value pairs in the body
        }
    }
}

auto parse_request(HTTPRequest& req, const std::string& req_str) -> void {
    std::istringstream iss(req_str);
    std::string line;

    // Parse request line
    std::getline(iss, line);
    std::istringstream line_stream(line);
    line_stream >> req.method >> req.URI >> req.version;

    // Parse headers
    while (std::getline(iss, line) && line != "\r") {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Trim leading and trailing spaces from key and value
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            req.headers.emplace_back(key, value);
            if (key == "Cookie") {
                // Parse cookies
                std::istringstream cookie_stream(value);
                std::string cookie_pair;
                while (std::getline(cookie_stream, cookie_pair, ';')) {
                    size_t eq_pos = cookie_pair.find('=');
                    if (eq_pos != std::string::npos) {
                        std::string cookie_name = cookie_pair.substr(0, eq_pos);
                        std::string cookie_value = cookie_pair.substr(eq_pos + 1);
                        req.cookies.emplace_back(cookie_name, cookie_value);
                    }
                }
            } else {
                req.headers.emplace_back(key, value);
            }
        }
    }

    // Parse body based on Content-Length header (if present)
    for (const auto& header : req.headers) {
        if (header.first == "Content-Length") {
            int content_length = std::stoi(header.second);

            if (content_length > 0) {
                std::string body_content(content_length, '\0');
                if (iss.read(&body_content[0], content_length)) {

                    // URL decoding for form data
                    if (req.headers[0].second == "application/x-www-form-urlencoded")
                        parse_form_data(body_content, req);

                    else
                        req.body = body_content;
                }
            }
            break; // Stop after finding Content-Length header
        }
    }
}


void handle_client(int client_socket) {

    // instance of the HTTP Request struct
    HTTPRequest request {};

    // Read the request string into the buffer
    char BUFFER[8192];
    ssize_t bytes_read = recv(client_socket, BUFFER, sizeof(BUFFER), 0);

    // Handle for errors i.e if number of bytes read is invalid
    if (bytes_read == 0) {
        std::cerr << "Error: Client disconnected from server!\n";
        close(client_socket);
        return;
    }
    else if (bytes_read <= 0) {
        std::cerr << "Error: Error reading request string!\n";
        close(client_socket);
        return;
    }

    // parse the http request string and split it into our struct
    std::string http_request_string(BUFFER, bytes_read);

    parse_request(request, http_request_string);

    // Handle the request according to its parameters
    handle_request(request, client_socket);

    // once done close this socket
    close(client_socket);
}

auto parse_parameters(std::string uri) -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> params_map;
    size_t pos = uri.find('?');

    if(pos == std::string::npos)
        return params_map;

    std::string param_str = uri.substr(pos+1);
    size_t start = 0;
       while (start < param_str.size()) {
           // Find the position of the '&' character
           size_t end = param_str.find('&', start);
           if (end == std::string::npos) {
               end = param_str.size();
           }

           // Extract the key-value pair
           std::string keyValue = param_str.substr(start, end - start);

           // Find the position of the '=' character to split key and value
           size_t separator = keyValue.find('=');
           if (separator != std::string::npos) {
               std::string key = keyValue.substr(0, separator);
               std::string value = keyValue.substr(separator + 1);
               // URL decoding may be necessary here depending on your requirements
               params_map[key] = value;
           }

           // Move to the next key-value pair
           start = end + 1;
       }

    return params_map;
}
