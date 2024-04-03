#include "controller.hpp"
#include <string>
#include <sys/socket.h>

bool is_author(HTTPRequest& req, int problem_id) {
    bool is_authenticated {};
    std::string username {};
    SessionManager& session = SessionManager::get_instance();
    Database& DB = Database::getInstance();

    auto it = std::find_if(req.cookies.begin(), req.cookies.end(),
                           [](const std::pair<std::string, std::string>& pair) {
                               return pair.first == "session_id";
                           });

    is_authenticated = it != req.cookies.end() && session.isValidSession(it->second);

    if(!is_authenticated)
        return false;

    username = session.getUserId(it->second);

    std::string query = "SELECT users.username "
                        "FROM questions "
                        "INNER JOIN users ON questions.admin_id = users.user_id "
                        "WHERE questions.question_id = " + std::to_string(problem_id);

        sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(DB.getDBHandle(), query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement" << std::endl;
        return false;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        std::cerr << "No rows returned" << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    std::string author(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));

    if (username.compare(author) == 0)
        return true;

    return false;
}

void handle_registration(HTTPRequest& req, int client_socket) {

    std::string http_response {};
    HTTPResponse response {};

    std::string username = get_form_field(req.body, "username");
    std::string password = get_form_field(req.body, "password");
    std::string confirm_password = get_form_field(req.body, "confirm_password");
    std::string admin_checkbox = get_form_field(req.body, "admin");

    Database& db = Database::getInstance();

    auto send_bad_request = [&](int client_socket) {
        response.status_code = 400;
        response.status_message = "Bad Request";
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(), http_response.length(), 0);
    };

    auto send_internal_server_error = [&](int client_socket) {
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(), http_response.length(), 0);
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
        response.status_code = 409;
        response.status_message = "Conflict";
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(), http_response.length(), 0);
        return;
    }

    bool is_admin = (admin_checkbox == "on");
    if (!db.insert_user(username, password, is_admin)) {
        std::cerr << "Error: Failed to insert user into database" << std::endl;
        send_internal_server_error(client_socket);
        return;
    }

    // Successful registration response
    response.status_code = 200;
    response.status_message = "OK";
    http_response = response.generate_response();
    send(client_socket, http_response.c_str(), http_response.length(), 0);
}

void handle_authentication(HTTPRequest& req, int client_socket){
    HTTPResponse response {};
    std::string http_response {};

    std::string username = get_form_field(req.body, "username");
    std::string password = get_form_field(req.body, "password");

    Database& db = Database::getInstance();

    auto send_bad_request = [&](int client_socket) {
        response.status_code = 400;
        response.status_message = "Bad Request";
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(), http_response.length(), 0);
    };

    if (username.empty() || password.empty()) {
        send_bad_request(client_socket);
        std::cerr << "Error: Empty Fields!\n";
        return;
    }

    // Check if the username and password match
    if (!db.login(username, password)) {
        // If the credentials don't match, send unauthorized response
        response.status_code = 401;
        response.status_message = "Unauthorized";
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(), http_response.length(), 0);
        return;
    }

    /// Generate a session ID
    SessionManager& sessionManager = SessionManager::get_instance();
    std::string sessionId = sessionManager.createSession(username);

    // Send successful authentication response with session ID
    response.status_code = 200;
    response.status_message = "OK";
    response.cookies = {"session_id", sessionId};

    std::string success_response = response.generate_response();
    send(client_socket, success_response.c_str(), success_response.length(), 0);
}

void handle_get_problems(HTTPRequest& req, int client_socket) {
    HTTPResponse response {};

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

        if (stepResult == SQLITE_ROW)
            json_response += ",";
    }
    sqlite3_finalize(stmt);
    json_response += "]";

    // Construct HTTP response headers
    response.status_code = 200;
    response.status_message = "OK";
    response.set_JSON_content(json_response);
    std::string http_response = response.generate_response();

    // Send response to the client
    send(client_socket, http_response.c_str(), http_response.length(), 0);

    // Close the client socket
    close(client_socket);
}

void handle_view_problem(HTTPRequest &req, int client_socket, int problem_id) {
    HTTPResponse response {};
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
    response.status_code = 200;
    response.status_message = "OK";
    response.set_JSON_content(json_response);

    std::string http_response = response.generate_response();

    // Send the HTTP response
    send(client_socket, http_response.c_str(), http_response.length(), 0);

    // Finalize the prepared statement
    sqlite3_finalize(stmt);
    close(client_socket);
}

void handle_is_auth(HTTPRequest& req, int client_socket) {
    std::string http_response {};
    HTTPResponse response {};

    SessionManager& session = SessionManager::get_instance();

    auto it = std::find_if(req.cookies.begin(), req.cookies.end(),
                           [](const std::pair<std::string, std::string>& pair) {
                               return pair.first == "session_id";
                           });

    if(it != req.cookies.end() && session.isValidSession(it->second)){
        response.status_code = 200;
        response.status_message = "OK";
        response.body = "Session ID is valid";
        http_response = response.generate_response();
    }
    else {
        response.status_code = 403;
        response.status_message = "Forbidden";
        response.body = "Forbidden: Invalid session ID.";
        http_response = response.generate_response();
    }
    send(client_socket, http_response.c_str(), http_response.length(), 0);
}

// TODO
void handle_submission(HTTPRequest& req, int client_socket) {

}

void handle_logout(HTTPRequest& req, int client_socket) {
    std::string http_response {};

    HTTPResponse response {};
    SessionManager& session_manager = SessionManager::get_instance();

    auto it = std::find_if(req.cookies.begin(), req.cookies.end(),
                           [](const std::pair<std::string, std::string>& pair) {
                               return pair.first == "session_id";
                           });

    if (it != req.cookies.end()) {
        session_manager.logout(it->first);
    }

    response.status_code = 302;
    response.status_message = "Found";
    response.location = "/";

    http_response = response.generate_response();

    send(client_socket, http_response.c_str(), http_response.length(), 0);

}

void handle_search(HTTPRequest& req, int client_socket, std::string search_query) {

    Database& db = Database::getInstance();
    HTTPResponse response {};

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
    response.status_code = 200;
    response.status_message = "OK";
    response.set_JSON_content(json_response);
    std::string http_response = response.generate_response();

    // Send response to the client
    send(client_socket, http_response.c_str(), http_response.length(), 0);

    // Close the client socket
    close(client_socket);
}

void handle_add_problem(HTTPRequest& req, int client_socket) {
    std::string http_response {};
    std::string title = get_form_field(req.body, "question_title");
    std::string text = get_form_field(req.body, "question_text");

    title = escape_string(title);
    text = escape_string(text);

    HTTPResponse response {};
    Database& DB = Database::getInstance();
    SessionManager& session = SessionManager::get_instance();
    Filesystem_Manager& filesystem = Filesystem_Manager::get_instance();

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
            response.status_code = 400;
            response.status_message = "Bad Request";
            response.body = "Problem with title '" + title + "' already exists.";
            http_response = response.generate_response();
        } else {
            query = "INSERT INTO questions (question_title, question_text, date_posted, admin_id) VALUES (?, ?, CURRENT_DATE, ?)";

            sqlite3_stmt* insertStmt;
            if (sqlite3_prepare_v2(DB.getDBHandle(), query.c_str(), -1, &insertStmt, NULL) != SQLITE_OK) {
                std::cerr << "Error preparing SQL statement" << std::endl;
            } else {
                sqlite3_bind_text(insertStmt, 1, title.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(insertStmt, 2, text.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(insertStmt, 3, user_id.c_str(), -1, SQLITE_STATIC);

                int result = sqlite3_step(insertStmt);
                if (result != SQLITE_DONE) {
                    response.status_code = 500;
                    response.status_message = "Internal Server Error";
                    response.body = "Failed to add problem: " + title;
                    http_response = response.generate_response();
                } else {
                    // add directory for the new problem
                    int question_id = sqlite3_last_insert_rowid(DB.getDBHandle());
                    filesystem.add_problem_dir(std::to_string(question_id));

                    response.status_code = 200;
                    response.status_message = "OK";
                    response.body = "Question has been added!";
                    http_response = response.generate_response();
                }
                sqlite3_finalize(insertStmt);
            }
        }
    } else {
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.body = "Invalid Session";
        http_response = response.generate_response();
    }
    send(client_socket, http_response.c_str(), http_response.length(), 0);
}


void handle_is_author(HTTPRequest& req, int client_socket, int problem_id) {
    bool is_authenticated {};
    std::string username {};
    HTTPResponse response {};

    SessionManager& session = SessionManager::get_instance();
    Database& DB = Database::getInstance();

    auto it = std::find_if(req.cookies.begin(), req.cookies.end(),
                           [](const std::pair<std::string, std::string>& pair) {
                               return pair.first == "session_id";
                           });

    is_authenticated = it != req.cookies.end() && session.isValidSession(it->second);
    if (is_authenticated) {
        username = session.getUserId(it->second);

        std::string query = "SELECT users.username "
                            "FROM questions "
                            "INNER JOIN users ON questions.admin_id = users.user_id "
                            "WHERE questions.question_id = " + std::to_string(problem_id);

         sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(DB.getDBHandle(), query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            std::cerr << "Error preparing SQL statement" << std::endl;
            return;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW) {
            std::cerr << "No rows returned" << std::endl;
            sqlite3_finalize(stmt);
            return;
        }

        std::string author(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));

        if(username.compare(author) == 0) {
            response.status_code = 200;
            response.status_message = "OK";
            response.body = "Author has been validated.\r\n";

            std::string http_response = response.generate_response();

            send(client_socket, http_response.c_str(), http_response.length(), 0);
        }
    }
}

void handle_delete_problem(HTTPRequest& req, int client_socket, int problem_id) {
    int count {};

    HTTPResponse response {};
    std::string http_response {};

    Database& DB = Database::getInstance();
    SessionManager& session = SessionManager::get_instance();
    Filesystem_Manager& filesystem = Filesystem_Manager::get_instance();

    auto it = std::find_if(req.cookies.begin(), req.cookies.end(),
                            [](const std::pair<std::string, std::string>& pair) {
                                return pair.first == "session_id";
                         });

    bool is_authenticated = it != req.cookies.end() && session.isValidSession(it->second);

    if (!is_authenticated)
        return;

    std::string query = "SELECT * FROM questions WHERE question_id = " + std::to_string(problem_id);
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(DB.getDBHandle(), query.c_str(), -1, &stmt, NULL) != SQLITE_OK)
        std::cerr << "Error preparing SQL statement" << std::endl;

    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);

    if (count == 0) {
        response.status_code = 400;
        response.status_message = "Bad Request";
        http_response = response.generate_response();
    } else {
        query = "DELETE FROM questions WHERE question_id = " + std::to_string(problem_id);
        if (sqlite3_prepare_v2(DB.getDBHandle(), query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            http_response = response.generate_response();
        } else {
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                response.status_code = 500;
                response.status_message = "Internal Server Error: Failed to delete the question";
                http_response = response.generate_response();
            } else {
                filesystem.delete_problem_dir(std::to_string(problem_id));

                sqlite3_finalize(stmt);
                response.status_code = 302;
                response.status_message = "Question Deleted";
                response.location = "/";
                http_response = response.generate_response();
            }
        }
    }
    send(client_socket, http_response.c_str(), http_response.length(), 0);
}

void handle_update_problem(HTTPRequest& req, int client_socket, int problem_id) {

    HTTPResponse response {};
    std::string http_response {};

    bool is_valid = is_author(req, problem_id);

    Database& DB = Database::getInstance();

    std::string title = get_form_field(req.body, "question_title");
    std::string text = get_form_field(req.body, "question_text");

    title = escape_string(title);
    text = escape_string(text);

    if (!is_valid) {
        response.status_code = 403;
        response.status_message = "Forbidden";
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(),http_response.length() , 0);
        return;
    }

    std::string update_query = "UPDATE questions SET question_title = ?, question_text = ? WHERE question_id = ?";
    sqlite3_stmt* stmt {};

    int prepare_result = sqlite3_prepare_v2(DB.getDBHandle(), update_query.c_str(), -1, &stmt, nullptr);
    if (prepare_result != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement" << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, text.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, problem_id);

    int step_result = sqlite3_step(stmt);

    if (step_result == SQLITE_DONE) {
        response.status_code = 200;
        response.status_message = "OK";
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(),http_response.length() , 0);
    }
    else {
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        http_response = response.generate_response();
        send(client_socket, http_response.c_str(),http_response.length() , 0);
    }
    sqlite3_finalize(stmt);
}


void handle_submit_solution(HTTPRequest& req, int client_socket, int problem_id) {
    HTTPResponse response {};
    std::string http_response {};
    SessionManager& session = SessionManager::get_instance();
    req.body = decode_file_data(req);

    auto it = std::find_if(req.cookies.begin(), req.cookies.end(),
                            [](const std::pair<std::string, std::string>& pair) {
                                return pair.first == "session_id";
                         });

    std::string username = session.getUserId(it->second);
    std::string file_path = "./script_storage/" + std::to_string(problem_id)
                           + "/" + username + ".py";

    std::fstream file(file_path, std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        http_response = response.generate_response();
    }
    else {
        file << req.body;
        response.status_code = 200;
        response.status_message = "OK";
        http_response = response.generate_response();
    }
    send(client_socket, http_response.c_str(), http_response.length(), 0);
}

void handle_run_submission(HTTPRequest& req, int client_socket, int problem_id) {
    // TODO
}