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
                                         "username TEXT,"
                                         "password TEXT,"
                                         "is_admin INTEGER"
                                         ");";

    // Execute SQL queries to create tables
    execute_query(create_questions_table_sql);
    execute_query(create_submissions_table_sql);
    execute_query(create_users_table_sql);
}

void Database::execute_query(const char* sql_query) {
    int result = sqlite3_exec(db, sql_query, nullptr, nullptr, nullptr);
    if (result != SQLITE_OK) {
        std::cerr << "Error: SQL error: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Table created successfully" << std::endl;
    }
}