#include "utils.hpp"

auto display_request(HTTPRequest& req) -> void {
    std::cout << req.URI << req.method << std::endl;
}