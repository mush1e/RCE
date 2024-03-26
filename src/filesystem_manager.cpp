#include "filesystem_manager.hpp"

Filesystem_Manager::Filesystem_Manager(const std::string& base_dir) : base_directory(base_dir) {};

Filesystem_Manager& Filesystem_Manager::get_instance() {
    static Filesystem_Manager filesystem;
    return filesystem;
}

void Filesystem_Manager::init_filesystems() {

    Database& DB = Database::getInstance();
    
    std::string query = "SELECT question_id FROM questions";
    sqlite3_stmt* stmt {};

    int prepare_result = sqlite3_prepare_v2(DB.getDBHandle(), query.c_str(), -1, &stmt, nullptr);

    if (prepare_result != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement" << std::endl;
        return;
    }

    while(sqlite3_step(stmt) == SQLITE_ROW) {
        const char* problem_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        
        std::string directory_name = base_directory + "/" + problem_id;

        if (mkdir(directory_name.c_str(), 0777) == -1)
            std::cerr << "Error :  " << strerror(errno) << problem_id << std::endl; 
        else
            std::cout << "Directory created" << " " << problem_id << std::endl;
    }
    
}