#include <string>
#include <mutex>
#include <sys/stat.h>
#include <sys/types.h>
#include <unordered_map>

#ifndef FILESYSTEM_MANAGER_HPP
#define FILESYSTEM_MANAGER_HPP

#include "database.hpp"

class Filesystem_Manager {
    std::string base_directory {};
    std::unordered_map<std::string, std::unordered_map<std::string, FILE*>> file_indexer {};
    std::mutex mtx {};

    Filesystem_Manager(const std::string& base_dir = "./script_storage");
    
    public:
        static Filesystem_Manager& get_instance();
        void add_submission();
        void init_filesystems();
};

#endif