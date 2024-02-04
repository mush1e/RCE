#include <iostream>
#include "sqlite3.h"

#ifndef DATABASE_HPP
#define DATABASE_HPP

class Database {
    sqlite3* db;
public:
    Database();
    ~Database();
    static Database& getInstance();
    void initialize_database();
    void create_tables();
    void execute_query(const char*);
};

#endif // DATABASE_HPP
