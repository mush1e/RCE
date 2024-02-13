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

auto handle_authentication(HTTPRequest& req, int client_socket) -> void {

    std::string username = get_form_field(req.body, "username");
    std::string password = get_form_field(req.body, "password");

    Database& db = Database::getInstance();

    auto send_bad_request = [](int client_socket) {
        std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response.c_str(), response.length(), 0);
    };

    auto send_internal_server_error = [](int client_socket) {
       std::string response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response.c_str(), response.length(), 0);
    };

    if (username.empty() || password.empty()) {
        send_bad_request(client_socket);
        std::cerr << "Error: Empty Fields!\n";
        return;
    }

    // Check if the username and password match
    if (!db.login(username, password)) {
        // If the credentials don't match, send unauthorized response
        std::string response = "HTTP/1.1 401 Unauthorized\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response.c_str(), response.length(), 0);
        return;
    }

    /// Generate a session ID
    SessionManager& sessionManager = SessionManager::get_instance();
    std::string sessionId = sessionManager.createSession(username);

    // Send successful authentication response with session ID
    std::string success_response = "HTTP/1.1 200 OK\r\n";
    success_response += "Set-Cookie: session_id=" + sessionId + "; SameSite=None; Secure; HttpOnly\r\n";
    success_response += "Content-Length: 0\r\n\r\n";
    send(client_socket, success_response.c_str(), success_response.length(), 0);
}

void handle_get_problems(HTTPRequest& req, int client_socket) {
    // Get instance of your Database
    Database& db = Database::getInstance();

    // Execute SQL query to retrieve problems data from the database
    std::string query = "SELECT questions.question_id, questions.question_title, users.username "
                        "FROM questions "
                        "INNER JOIN users ON questions.admin_id = users.user_id";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db.getDBHandle(), query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement" << std::endl;
        return;
    }

    // Define a string to hold the JSON response
    std::string json_response = "[";

    // Iterate over the results and construct JSON-like string

    for (auto stepResult = sqlite3_step(stmt); stepResult == SQLITE_ROW;) {

        std::string author(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        std::string title(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        
        std::string problem_json = "{";
        problem_json += "\"author\":\"" + author + "\",";
        problem_json += "\"title\":\"" + title + "\",";
        problem_json += "\"id\":" + std::to_string(sqlite3_column_int(stmt, 0));
        problem_json += "}";

        json_response += problem_json;
        stepResult = sqlite3_step(stmt);

        // Add comma if not the last row
        if (stepResult == SQLITE_ROW) {
            json_response += ",";
        }

    }

    sqlite3_finalize(stmt);
    json_response += "]";

    // Construct HTTP response headers
    std::string http_response = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: application/json\r\n"
                                "Content-Length: " + std::to_string(json_response.length()) + "\r\n"
                                "\r\n" + json_response;

    // Send response to the client
    send(client_socket, http_response.c_str(), http_response.length(), 0);

    // Close the client socket
    close(client_socket);
}
