#include "session_management.hpp"

SessionManager& SessionManager::get_instance() {
    static SessionManager instance;
    return instance;
}

// Verify if session is valid
bool SessionManager::isValidSession(const std::string& sessionId){
    std::lock_guard<std::mutex> lock(mtx);
    for(auto session: sessions)
        if (session.first.compare(sessionId)) {
            time_t currentTime = time(nullptr);
            if (currentTime < session.second.expiry_time) {
                return true; // Session is valid
            } else {
                sessions.erase(session.first); // Session has expired, remove it
            }
        }
    return false; // Session not found or expired
}

std::string SessionManager::generateSessionId() {
    const std::string char_list = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    const size_t session_id_length = 32; 
    std::string session_id;

    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, char_list.length() - 1);

    for (auto i = 0u; i < session_id_length; ++i) 
        session_id += char_list[dis(gen)];

    return session_id;
}

// Create a new session for a user
std::string SessionManager::createSession(const std::string& userId) {
    std::lock_guard<std::mutex> lock(mtx);
    std::string sessionId = generateSessionId();
    SessionData sessionData = {userId, time(nullptr), time(nullptr) + session_duration};
    sessions[sessionId] = sessionData;
    return sessionId;
}

bool SessionManager::logout(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(mtx);
    for(auto session: sessions)
        if (session.first.compare(sessionId)) {
            sessions.erase(session.first); 
            return true; // Logout successful
        }

    return false; // Session ID not found
}