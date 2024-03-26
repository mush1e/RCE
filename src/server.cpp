#include "server.hpp"

HTTP_Server::HTTP_Server(int port) : port(port) {

    // Try creating the TCP socket, if it doesnt work exit
    if((this->server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error: Failed to create socket!\n";
        exit(1);
    }

    // Bind server socket to port
    this->server_address.sin_addr.s_addr = INADDR_ANY;
    this->server_address.sin_family = AF_INET;
    this->server_address.sin_port = htons(this->port);

    // If bind unsuccessful display error message and exit
    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Error: Failed to bind socket to port!\n";
        exit(1);
    }

}

HTTP_Server::~HTTP_Server() {
    close(this->server_socket);
}

auto HTTP_Server::start() -> void {

    // Listen for incoming connections
    if(listen(server_socket, 10) < 0) {
        std::cerr << "Error: Failed to listen for connections!\n";
        exit(1);
    }
    
    // Obtain the single instance of the Database class
    Database& database = Database::getInstance();

    // Use the database instance to perform operations
    database.initialize_database();
    database.create_tables();
    database.add_leetcode_problems();

    Filesystem_Manager& filesystem = Filesystem_Manager::get_instance();
    filesystem.init_filesystems();
    
    // Success message
    std::cout << "Server started. Listening on port " << this->port << "...\n";

    for(;;) {

        // accept incoming request
        int client_socket = accept(this->server_socket, nullptr, nullptr);

        // verify if connection was accepted
        if(client_socket < 0) {
            std::cerr << "Error: Failed to accept connection!\n";
            continue;
        }

        // Create a threat to handle this request and then detach it so it just does its own thing
        // std::thread client_thread(handle_client, client_socket);
        // client_thread.detach();
        //
        thread_pool.enqueue([client_socket]() {
                handle_client(client_socket);
            });
    }


}
