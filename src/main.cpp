#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#define BUFF_SIZE 4096

struct HTTP_request {
    std::string  method              {};
    std::string  URI                 {};
    std::string  version             {};
    std::vector<std::pair<std::string, std::string>> headers {};
    std::string body                 {};
};

// A function to serve different html pages based on the request type
auto handle_request(HTTP_request& req, int client_socket) -> void {
    
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

// Parse the html request string 
void parse_request(HTTP_request& req, const std::string& req_str) {
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
    std::getline(iss, req.body);
}


// A function to serve different html pages based on the request type
void handle_client(int client_socket) {
    HTTP_request request;

    // Reading the request string from the client socket
    char BUFFER[BUFF_SIZE] {};
    ssize_t bytes_read = recv(client_socket, BUFFER, sizeof(BUFFER), 0);

    // error handling- if unable to read request string
    if(bytes_read == 0) {
        std::cerr << "Error: Client disconnected from server!\n";
        close(client_socket);
        return;
    } 
        
    else if (bytes_read <= 0) {
        std::cerr << "Error: Error reading request string!\n";
        close(client_socket);
        return;
    }

    // Now that we know that there was no error reading the request string Let's parse it
    std::string http_request_string(BUFFER, bytes_read);
    parse_request(request, http_request_string);

    // At this point everything is good we've recieved the request
    // now we shall serve a basic html file
    handle_request(request, client_socket);

    // Closing the client socket to free up resources related to
    // this particular request
    close(client_socket);
}

auto main() -> int {

    // Creating a socket IPV4, Stream socket with TCP
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // error handling - If socket could not be created
    if (server_socket == -1) {
        std::cerr << "Error: Failed to create socket!\n";
        return 1;
    }

    // Binding socket to port 8080
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY; // recieve connections from any address

    // Host to network, short (basically converts byte order for 8080 fo network standard)
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

        // Create a new thread to handle the client connection
        std::thread client_thread(handle_client, client_socket);
        client_thread.detach(); // Detach the thread to allow it to run independently
    }

    // close the server socket - GG
    close (server_socket);
    return 0;
}