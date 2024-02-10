#include "session_management.hpp"

SessionManager& SessionManager::get_instance() {
    static SessionManager instance;
    return instance;
}

// Create a new session for a user
std::string SessionManager::createSession(const std::string& userId) {
    std::lock_guard<std::mutex> lock(mtx);
    std::string sessionId = generateSessionId();
    SessionData sessionData = {userId, time(nullptr), time(nullptr) + session_duration};
    sessions[sessionId] = sessionData;
    return sessionId;
}

