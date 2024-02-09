#include "controller.hpp"

auto handle_registration(HTTPRequest& req, int client_socket) -> void {
    std::string username = get_form_field(req.body, "username");
    std::string password = get_form_field(req.body, "password");
    std::string confirm_password = get_form_field(req.body, "confirm_password");
    std::string admin_checkbox = get_form_field(req.body, "admin");

    Database& db = Database::getInstance();

    auto send_bad_request = [](int client_socket) {
        std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response.c_str(), response.length(), 0);
    };

    auto send_internal_server_error = [](int client_socket) {
       std::string response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response.c_str(), response.length(), 0);
    };

    std::regex passwordRegex("^(?=.*\\d)(?=.*[a-z])(?=.*[A-Z]).{6,}$");

    if (username.empty() || password.empty() || confirm_password.empty()) {
        send_bad_request(client_socket);
        std::cerr << "Error: Empty Fields!\n";
        return;
    }

    if (!std::regex_match(password, passwordRegex)) {
        send_bad_request(client_socket);
        std::cerr << "Error: Invalid Password!\n";
        return;
    }

    if (password != confirm_password) {
        send_bad_request(client_socket);
        std::cerr << "Error: Password mismatch!\n";
        return;
    }

    if (db.username_exists(username)) {
        std::string response = "HTTP/1.1 409 Conflict\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response.c_str(), response.length(), 0);
        return;
    }



    bool is_admin = (admin_checkbox == "on");
    if (!db.insert_user(username, password, is_admin)) {
        std::cerr << "Error: Failed to insert user into database" << std::endl;
        send_internal_server_error(client_socket);
        return;
    }

    // Successful registration response
    std::string success_response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    send(client_socket, success_response.c_str(), success_response.length(), 0);
}
