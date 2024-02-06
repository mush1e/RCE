#include "controller.h"

auto handle_registration(HTTPRequest req, int client_socket) -> void {
    std::string username = get_form_field(req.body, "username");
    std::string password = get_form_field(req.body, "password");
}