#include <iostream>
#include <sstream>
#include <random>
#include <functional>

#include "sqlite3.h"

#ifndef DATABASE_HPP
#define DATABASE_HPP

class Database {
    sqlite3* db;
public:
    // DB methods
    Database();
    ~Database();
    static Database& getInstance();
    void initialize_database();
    void create_tables();
    void execute_query(const char*);
    std::string sanitize_input(const std::string& input) const;

    // Query functions
    bool username_exists(const std::string& username);
    bool insert_user(const std::string& username, const std::string& password, bool is_admin);
};

std::string generate_salt();
std::string hash_password(const std::string& password);

#endif // DATABASE_HPP
