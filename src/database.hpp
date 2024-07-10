#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <iostream>
#include <sstream>
#include <vector>
#include <random>
#include <sqlite3.h>

class Database {
private:
    sqlite3* db;

    bool execute_query(const char* sql_query);


public:
    Database();
    ~Database();
    static Database& getInstance();
    void initialize_database();
    void create_tables();
    std::string sanitize_input(const std::string& input) const;

    bool insert_post(const std::string& title, const std::string& content, int author_id);
    std::vector<std::pair<std::string, std::string>> get_all_posts();
    std::vector<std::pair<std::string, std::string>> get_posts_by_author(int author_id);

    // Additional methods for user management (if needed)
    bool username_exists(const std::string& username);
    bool login(const std::string& username, const std::string& password);
    bool insert_user(const std::string& username, const std::string& password, bool is_admin);
    std::string Database::get_user(const std::string& username) ;

    sqlite3* getDBHandle() { return db; } 

    // Optional: Method to hash passwords and generate salts
    static std::string generate_salt();
    static std::string hash_password(const std::string& password);
};

#endif // DATABASE_HPP
