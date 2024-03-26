#include <string>
#include <unordered_map>

#ifndef FILESYSTEM_MANAGER_HPP
#define FILESYSTEM_MANAGER_HPP

class Filesystem_Manager {
    std::string base_directory {};
    std::unordered_map<std::string, std::unordered_map<std::string, FILE*>> file_indexer {};
    
    public:
        static Filesystem_Manager& get_instance();
        Filesystem_Manager(const std::string& base_dir);
        void add_submission();
};

#endif