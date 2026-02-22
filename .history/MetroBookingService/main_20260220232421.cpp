#include <iostream>
#include "utils/httplib.h"
#include "utils/json.hpp"

using namespace httplib;
using json = nlohmann::json;

int main() {

    Server server;

    // ROOT ROUTE
    server.Get("/", [](const Request&, Response& res) {
        res.set_content("Metro Backend Running", "text/plain");
    });

    // REGISTER
    server.Post("/register", [](const Request& req, Response& res) {
        json body = json::parse(req.body);

        json response;
        response["message"] = "User registered successfully";

        res.set_content(response.dump(), "application/json");
    });

    // LOGIN
    server.Post("/login", [](const Request& req, Response& res) {
        json body = json::parse(req.body);

        json response;
        response["token"] = "dummy_token_123";

        res.set_content(response.dump(), "application/json");
    });

    std::cout << "Server running at http://localhost:8080\n";

    server.listen("0.0.0.0", 8080);

    return 0;
}