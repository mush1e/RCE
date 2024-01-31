#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

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

        // At this point everything is good we've recieved the request
        // now we shall serve a basic html file

        // Reading the contents of our HTML file and storing them as a string
        std::ifstream html_fptr("../public/index.html");
        std::stringstream buffer;
        buffer << html_fptr.rdbuf();
        std::string html_content = buffer.str();

        // Creating the HTTP response string
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(html_content.length()) + "\r\n\r\n" + html_content;

        // Send the response to the client
        send(client_socket, response.c_str(), response.length(), 0);

        // Closing the client socket to free up resources related to
        // this particular request
        close(client_socket);
    }

    // close the server socket - GG
    close (server_socket);
    return 0;
}