#include "controller.hpp"

void handle_registration(HTTPRequest& req, int client_socket) {

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

void handle_authentication(HTTPRequest& req, int client_socket){

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

void handle_view_problem(HTTPRequest &req, int client_socket, int problem_id) {
    Database& db = Database::getInstance();

    // Construct the SQL query with proper syntax
    std::string query = "SELECT questions.question_title, questions.question_text, users.username "
                        "FROM questions "
                        "INNER JOIN users ON questions.admin_id = users.user_id "
                        "WHERE questions.question_id = " + std::to_string(problem_id);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db.getDBHandle(), query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement" << std::endl;
        return;
    }

    // Execute the prepared statement
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        std::cerr << "No rows returned" << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    // Extract data from the result set
    std::string author(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
    std::string text(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    std::string title(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));

    // Build the JSON response string
    std::string json_response = "{";
    json_response += "\"author\":\"" + author + "\",";
    json_response += "\"title\":\"" + title + "\",";
    json_response += "\"text\":\"" + text + "\""; // Enclose text within double quotes
    json_response += "}";

    // Build the HTTP response
    std::string http_response = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: application/json\r\n"
                                "Content-Length: " + std::to_string(json_response.length()) + "\r\n"
                                "\r\n" + json_response;

    // Send the HTTP response
    send(client_socket, http_response.c_str(), http_response.length(), 0);

    // Finalize the prepared statement
    sqlite3_finalize(stmt);
    close(client_socket);
}

void handle_is_auth(HTTPRequest& req, int client_socket) {
    SessionManager& session = SessionManager::get_instance();
    std::string http_response {};

    auto it = std::find_if(req.cookies.begin(), req.cookies.end(),
                           [](const std::pair<std::string, std::string>& pair) {
                               return pair.first == "session_id";
                           });

    if(it != req.cookies.end() && session.isValidSession(it->second)){
        http_response = "HTTP/1.1 200 OK\r\n";
        http_response += "Content-Type: text/plain\r\n";
        http_response += "\r\n";
        http_response += "Session ID is valid.\r\n";
    }
    else {
        http_response = "HTTP/1.1 403 Forbidden\r\n";
        http_response += "Content-Type: text/plain\r\n";
        http_response += "\r\n";
        http_response += "Forbidden: Invalid session ID.\r\n";
    }
    send(client_socket, http_response.c_str(), http_response.length(), 0);
}

// TODO
void handle_submission(HTTPRequest& req, int client_socket) {

}

void handle_logout(HTTPRequest& req, int client_socket) {
    std::string http_response {};
    SessionManager& session_manager = SessionManager::get_instance();
    auto it = std::find_if(req.cookies.begin(), req.cookies.end(),
                           [](const std::pair<std::string, std::string>& pair) {
                               return pair.first == "session_id";
                           });

    if (it != req.cookies.end()) {
        session_manager.logout(it->first);
    }

    http_response = "HTTP/1.1 302 Found\r\n";
    http_response += "Location: /\r\n"; // Redirect to the root URL
    http_response += "Content-Length: 0\r\n";
    http_response += "\r\n";

    send(client_socket, http_response.c_str(), http_response.length(), 0);

}

void handle_search(HTTPRequest& req, int client_socket, std::string search_query) {

    Database& db = Database::getInstance();
    search_query = db.sanitize_input(search_query);
    search_query = url_decode(search_query);

    std::string query = "SELECT questions.question_id, questions.question_title, users.username "
                            "FROM questions "
                            "INNER JOIN users ON questions.admin_id = users.user_id "
                            "WHERE LOWER(questions.question_title) LIKE LOWER('%" + search_query + "%')";


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

void handle_add_problem(HTTPRequest& req, int client_socket) {

    std::string title = get_form_field(req.body, "question_title");
    std::string text = get_form_field(req.body, "question_text");

    Database& DB = Database::getInstance();
    SessionManager& session = SessionManager::get_instance();

    auto it = std::find_if(req.cookies.begin(), req.cookies.end(),
                           [](const std::pair<std::string, std::string>& pair) {
                               return pair.first == "session_id";
                           });

    if(it != req.cookies.end() && session.isValidSession(it->second)) {
        int count {};
        std::string username = session.getUserId(it->second);
        std::string user_id = DB.get_user(username);

        sqlite3_stmt* stmt;
        std::string query = "SELECT COUNT(*) FROM questions WHERE question_title = '" + title + "'";

        if (sqlite3_prepare_v2(DB.getDBHandle(), query.c_str(), -1, &stmt, NULL) != SQLITE_OK)
            std::cerr << "Error preparing SQL statement" << std::endl;

        if (sqlite3_step(stmt) == SQLITE_ROW)
            count = sqlite3_column_int(stmt, 0);

        sqlite3_finalize(stmt);

        if (count > 0) {

            std::cerr << "Problem already exists" << std::endl;

            std::string http_response = "HTTP/1.1 400 Bad Request\r\n";
            http_response += "Content-Type: text/plain\r\n";
            http_response += "\r\n";
            http_response += "Problem with title '" + title + "' already exists.\r\n";
            send(client_socket, http_response.c_str(), http_response.length(), 0);


        } else {
            query = "INSERT INTO questions (question_title, question_text, date_posted, admin_id) VALUES ('"
                                + title + "', '"
                                + text + "', "
                                + "CURRENT_DATE, " + user_id
                                + ")";
            if (!DB.execute_query(query.c_str())) {
                std::cerr << "Failed to add problem: " << title << std::endl;

                std::string http_response = "HTTP/1.1 500 Internal Server Error\r\n";
                http_response += "Content-Type: text/plain\r\n";
                http_response += "\r\n";
                http_response += "Failed to add problem: " + title + "\r\n";
                send(client_socket, http_response.c_str(), http_response.length(), 0);

            } else {
                std::string http_response = "HTTP/1.1 200 OK\r\n";
                            http_response += "Content-Type: text/plain\r\n";
                            http_response += "\r\n";
                            http_response += "Question has been added.\r\n";

                send(client_socket, http_response.c_str(), http_response.length(), 0);
            }
        }
    } else {
        std::string http_response = "HTTP/1.1 400 Bad Request\r\n";
        http_response += "Content-Type: text/plain\r\n";
        http_response += "\r\n";
        http_response += "Invalid session.\r\n";
        send(client_socket, http_response.c_str(), http_response.length(), 0);
    }
}
