#include <string>
#include <functional>
#include <unordered_map>

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "request_handler.hpp"

class Router {
    std::unordered_map<std::string, std::unordered_map<std::string, std::function<void()>>> router {};
};

#endif
