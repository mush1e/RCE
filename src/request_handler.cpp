#include "request_handler.hpp"
#include "controller.hpp"
#include "utils.hpp"

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
        
        else 
            sendNotFoundResponse(client_socket);

    else if (req.method == "POST") 
        
        if (req.URI == "/register") 
            handle_registration(req, client_socket);
        
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

    // display_request(req);
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