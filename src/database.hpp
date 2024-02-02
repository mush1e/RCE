#include <iostream>
#include "sqlite3.h"

#ifndef DATABASE_HPP
#define DATABASE_HPP

class Database {
    sqlite3* db;
public:
    Database();
    ~Database();
    void initialize_database();
    void create_tables();
};

#endif // DATABASE_HPP
