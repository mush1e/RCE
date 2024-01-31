#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFF_SIZE 2048

struct HTTP_request {
    std::string  method              {};
    std::string  URI                 {};
    std::string  version             {};
    std::vector<std::pair<std::string, std::string>> headers {};
    std::string body                 {};
};

// Just a utilities function to display HTTP Request to see if the values are being captured accurately
auto display_request(HTTP_request& req) -> void {
    
    std::cout << "Headers:\n";
    for(auto x : req.headers) 
        std::cout << x.first << ": " << x.second << std::endl;
    
    std::cout << "Body:\n" << req.body;
}


// Parse the html request string 
auto parse_request(HTTP_request& req, std::string req_str) -> void {
    std::istringstream iss(req_str);

    // Reading the Request Line
    iss >> req.method >> req.URI >> req.version;
    std::string buffer {};

    // Reading the headers 
    while(std::getline(iss, buffer) && !buffer.empty()) {
        size_t index = buffer.find(":");
        if(index != std::string::npos)
            req.headers.push_back(std::make_pair(buffer.substr(0, index), buffer.substr(index+2)));
    }

    // Parse Request Body
    std::getline(iss, req.body, '\0');

    display_request(req);
    
}

auto main() -> int {

    // Creating a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // error handling - If socket could not be created
    if (server_socket == -1) {
        std::cerr << "Error: Failed to create socket!\n";
        return 1;
    }

    // Binding socket to port 8080
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);

    // error handling - If socket could not be bound
    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Error: Failed to bind socket to port!\n";
        return 1;
    }

    // Listening on the server for incoming connections
    if (listen(server_socket, 5) == -1) {
        std::cerr << "Error: Failed to listen for connections!\n";
        return 1;
    }

    // Display success message for setting up server
    std::cout << "Server started. Listening on port 8080...\n";

    // Main server loop to listen for incoming requests
    while (true) {

        // Accept incoming requests 
        int client_socket = accept(server_socket, nullptr, nullptr);

        // error handling - if unable to listen to incoming request
        if (client_socket == -1) {
            std::cerr << "Error: Failed to accept connection!\n";
            continue;
        }

        // Reading the request string from the client socket
        char BUFFER[BUFF_SIZE] {};
        ssize_t bytes_read = recv(client_socket, BUFFER, sizeof(BUFFER), 0);

        // error handling- if unable to read request string
        if(bytes_read == 0) {
            std::cerr << "Error: Client disconnected from server!\n";
            continue;    
        } 
        
        else if (bytes_read <= 0) {
            std::cerr << "Error: Error reading request string!\n";
            continue;
        }

        // Now that we know that there was no error reading the request string Let's parse it
        HTTP_request request;

        std::string http_request_string(BUFFER, bytes_read);

        parse_request(request, http_request_string);

        /*
            When making a request from something like postman the loop only runs once per request 
            but for something like safari, firefox the loop runs multiple times
        */

        // At this point everything is good we've recieved the request
        // now we shall serve a basic html file

        // Reading the contents of our HTML file and storing them as a string
        // std::ifstream html_fptr("../public/index.html");
        // std::stringstream buffer;
        // buffer << html_fptr.rdbuf();
        // std::string html_content = buffer.str();

        // Creating the HTTP response string
        // std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(html_content.length()) + "\r\n\r\n" + html_content;

        // Send the response to the client
        // send(client_socket, response.c_str(), response.length(), 0);

        // Closing the client socket to free up resources related to
        // this particular request
        close(client_socket);
    }

    // close the server socket - GG
    close (server_socket);
    return 0;
}