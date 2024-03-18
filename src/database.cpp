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

bool Database::login(const std::string& username, const std::string& password) {
    std::string sanitized_username = sanitize_input(username);
    std::string sanitized_password = sanitize_input(password);

    // Retrieve the salt and hashed password from the database for the provided username
    std::string query = "SELECT password_salt, password_hash FROM users WHERE username = '" + sanitized_username + "'";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Error preparing SQL statement" << std::endl;
        return false;
    }

    std::string stored_salt;
    std::string stored_hash;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        stored_salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        stored_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    }
    sqlite3_finalize(stmt);

    // Hash the provided password with the retrieved salt and compare it with the stored hash
    std::string salted_password = stored_salt + sanitized_password;
    std::string hashed_password = hash_password(salted_password);

    return hashed_password == stored_hash;
}

void Database::add_leetcode_problems() {
    // Array of LeetCode problem titles and descriptions
    std::vector<std::pair<std::string, std::string>> problems = {
        {"Two Sum", "Given an array of integers, return indices of the two numbers such that they add up to a specific target."},
        {"Reverse Integer", "Reverse digits of an integer."},
        {"Palindrome Number", "Determine whether an integer is a palindrome. An integer is a palindrome when it reads the same backward as forward."},
        {"Valid Parentheses", "Given a string containing just the characters '(', ')', '{', '}', '[' and ']', determine if the input string is valid."},
        {"Merge Two Sorted Lists", "Merge two sorted linked lists and return it as a new sorted list."},
        {"Maximum Subarray", "Find the contiguous subarray within an array (containing at least one number) which has the largest sum."},
        {"Container With Most Water", "Given n non-negative integers a1, a2, ..., an , where each represents a point at coordinate (i, ai). n vertical lines are drawn such that the two endpoints of the line i is at (i, ai) and (i, 0). Find two lines, which, together with the x-axis forms a container, such that the container contains the most water."},
        {"Longest Common Prefix", "Write a function to find the longest common prefix string amongst an array of strings. If there is no common prefix, return an empty string."},
        {"Roman to Integer", "Given a roman numeral, convert it to an integer. Input is guaranteed to be within the range from 1 to 3999."},
        {"Valid Parentheses", "Given a string s containing just the characters '(', ')', '{', '}', '[' and ']', determine if the input string is valid."},
        {"Merge k Sorted Lists", "Merge k sorted linked lists and return it as one sorted list. Analyze and describe its complexity."},
        {"Generate Parentheses", "Given n pairs of parentheses, write a function to generate all combinations of well-formed parentheses."},
        {"Merge Intervals", "Given an array of intervals where intervals[i] = [starti, endi], merge all overlapping intervals."},
        {"Group Anagrams", "Given an array of strings strs, group the anagrams together. You can return the answer in any order."},
        {"Pow(x, n)", "Implement pow(x, n), which calculates x raised to the power n (i.e., x^n)."},
        {"Remove Nth Node From End of List", "Given the head of a linked list, remove the nth node from the end of the list and return its head."},
        {"Next Permutation", "Implement next permutation, which rearranges numbers into the lexicographically next greater permutation of numbers."},
        {"Search in Rotated Sorted Array", "There is an integer array nums sorted in ascending order (with distinct values). Prior to being passed to your function, nums is rotated at an unknown pivot index k (0 <= k < nums.length) such that the resulting array is [nums[k], nums[k+1], ..., nums[n-1], nums[0], nums[1], ..., nums[k-1]] (0-indexed). For example, [0,1,2,4,5,6,7] might be rotated at pivot index 3 and become [4,5,6,7,0,1,2]. Given the array nums after the rotation and an integer target, return the index of target if it is in nums, or -1 if it is not in nums."},
        {"Search Insert Position", "Given a sorted array of distinct integers and a target value, return the index if the target is found. If not, return the index where it would be if it were inserted in order."},
        {"Combination Sum", "Given an array of distinct integers candidates and a target integer target, return a list of all unique combinations of candidates where the chosen numbers sum to target. You may return the combinations in any order."}
    };

    // Iterate over each problem and insert it into the database
    for (const auto& problem : problems) {
        // Sanitize input
        std::string sanitized_title = sanitize_input(problem.first);
        std::string sanitized_description = sanitize_input(problem.second);
        
        // Check if the problem already exists in the database
        std::string check_query = "SELECT COUNT(*) FROM questions WHERE question_title = '" + sanitized_title + "'";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, check_query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            std::cerr << "Error preparing SQL statement" << std::endl;
            continue; // Skip to the next problem
        }

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);

        // If the problem already exists, skip adding it
        if (count > 0) {
            continue; // Skip to the next problem
        }

        // Construct the SQL query to insert the problem
        std::string query = "INSERT INTO questions (question_title, question_text, date_posted, admin_id) VALUES ('" 
                            + sanitized_title + "', '" 
                            + sanitized_description + "', " 
                            + "CURRENT_DATE, 1)";  
        
        // Execute the query
        if (!execute_query(query.c_str())) {
            std::cerr << "Failed to add problem: " << sanitized_title << std::endl;
        }
    }
}

std::string Database::get_user(const std::string& username) {
    std::string user_id;
    std::string sanitized_username = sanitize_input(username);

    std::string query = "SELECT user_id FROM users WHERE username = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return user_id; 
    }

    sqlite3_bind_text(stmt, 1, sanitized_username.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    if (result == SQLITE_ROW) {
        user_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    } else if (result != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cerr << "No user found with the specified ID" << std::endl;
    }

    sqlite3_finalize(stmt);
    return user_id;
}
