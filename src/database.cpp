#include "database.hpp"

Database& Database::getInstance() {
    static Database instance; // Guaranteed to be initialized once
    return instance;
}


Database::Database() {

    if(sqlite3_open("./db/model.sql", &db) != SQLITE_OK) {
        std::cerr << "Error: Can't open database: " 
                  << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }
}

Database::~Database() {
    sqlite3_close(db);
}

void Database::initialize_database() {
    create_tables(); 
}

void Database::create_tables() {
    // Define SQL queries to create tables
    const char* create_questions_table_sql = "CREATE TABLE IF NOT EXISTS questions ("
                                             "question_id INTEGER PRIMARY KEY,"
                                             "question_title TEXT,"
                                             "question_text TEXT,"
                                             "solution_script TEXT,"
                                             "date_posted DATE,"
                                             "admin_id INTEGER,"
                                             "FOREIGN KEY (admin_id) REFERENCES users(user_id)"
                                             ");";

    const char* create_submissions_table_sql = "CREATE TABLE IF NOT EXISTS submissions ("
                                               "submission_id INTEGER PRIMARY KEY,"
                                               "question_id INTEGER,"
                                               "user_id INTEGER,"
                                               "submission_script TEXT,"
                                               "date_submitted DATE,"
                                               "FOREIGN KEY (question_id) REFERENCES questions(question_id),"
                                               "FOREIGN KEY (user_id) REFERENCES users(user_id)"
                                               ");";

    const char* create_users_table_sql = "CREATE TABLE IF NOT EXISTS users ("
                                         "user_id INTEGER PRIMARY KEY,"
                                         "username TEXT UNIQUE,"
                                         "password_hash TEXT,"
                                         "password_salt TEXT,"
                                         "is_admin INTEGER"
                                         ");";


    // Execute SQL queries to create tables
    execute_query(create_questions_table_sql);
    execute_query(create_submissions_table_sql);
    execute_query(create_users_table_sql);
}

bool Database::execute_query(const char* sql_query) {
    int result = sqlite3_exec(db, sql_query, nullptr, nullptr, nullptr);

    // error handling
    if (result != SQLITE_OK) {
        std::cerr << "Error: SQL error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    } 
    else 
        return true;

}

// Method to sanitize input by escaping special characters
std::string Database::sanitize_input(const std::string& input) const{
    std::ostringstream sanitized;
    for (char c : input) {
        if (c == '\'') {
            sanitized << "''"; // Escape single quotes
        } else {
            sanitized << c;
        }
    }
    return sanitized.str();
}

// Method to check if a username exists in the database
bool Database::username_exists(const std::string& username) {
    std::string sanitized_username = sanitize_input(username);
    std::string query = "SELECT COUNT(*) FROM users WHERE username = '" + sanitized_username + "'";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement" << std::endl;
        return false;
    }
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count > 0;
}

std::string generate_salt() {
    const std::string char_list = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    const size_t salt_length = 16; // Adjust length as needed
    std::string salt;

    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, char_list.length() - 1);

    for (auto i = 0u; i < salt_length; ++i) 
        salt += char_list[dis(gen)];

    return salt;
}

std::string hash_password(const std::string& password) {
    std::hash<std::string> hash_fn;
    return std::to_string(hash_fn(password));
}

// Method to insert a new user into the users table
bool Database::insert_user(const std::string& username, const std::string& password, bool is_admin) {
    std::string sanitized_username = sanitize_input(username);
    std::string sanitized_password = sanitize_input(password);
    
    // Salting and hashing the passwords before inserting into db
    std::string salt = generate_salt();
    std::string salted_password = salt + sanitized_password;
    std::string hashed_password = hash_password(salted_password);


    std::string query = "INSERT INTO users (username, password_hash, password_salt, is_admin) VALUES ('" 
                            + sanitized_username + "', '" 
                            + hashed_password + "', '" 
                            + salt + "', " 
                            + (is_admin ? "1" : "0") 
                            + ")";

    return execute_query(query.c_str());
}