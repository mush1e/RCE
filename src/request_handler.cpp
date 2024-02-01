#include "request_handler.hpp"


auto handle_request(HTTPRequest& req, int client_socket) -> void {

    if(req.method == "GET" && req.URI == "/") {
        // Reading the contents of our HTML file and storing them as a string
        std::ifstream html_fptr("../public/index.html");
        std::stringstream buffer;
        buffer << html_fptr.rdbuf();
        std::string html_content = buffer.str();

        // Creating the HTTP response string
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(html_content.length()) + "\r\n\r\n" + html_content;

        // Send the response to the client
        send(client_socket, response.c_str(), response.length(), 0);
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
            req.headers.emplace_back(key, value);
        }
    }

    // Parse body (if any)
    std::getline(iss, req.body, '\0');
}

void handle_client(int client_socket) {

    // instance of the HTTP Request struct
    HTTPRequest request {};

    // Read the request string into the buffer 
    char BUFFER[BUFSIZ];
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