#include <iostream>
#include <string>
#include <ctime>
#include <unordered_map>
#include <random>
#include <mutex>

#ifndef SESSION_MANAGEMENT_HPP
#define SESSION_MANAGEMENT_HPP

struct SessionData {
    std::string user_id  {};
    time_t creation_time {};
    time_t expiry_time   {};
};

class SessionManager {
    std::unordered_map<std::string, SessionData> sessions;
    const int session_duration = 3600;
    std::mutex mtx;

    SessionManager() = default;

    public:
        static SessionManager& get_instance();
        bool isValidSession(const std::string& sessionId);
        std::string createSession(const std::string& userId); 
        std::string generateSessionId();
        bool logout(const std::string& sessionId);
        std::string getUserId(const std::string& sessionID) const;
};

#endif