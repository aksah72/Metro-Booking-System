#include <iostream>
#include "utils/httplib.h"
#include "utils/json.hpp"

#include "services/MetroRouteService.h"
#include "services/BookingService.h"
#include "services/AuthenticationService.h"
#include "services/AdminService.h"

using namespace httplib;
using json = nlohmann::json;

int main() {

    Server server;

    MetroRouteService metro;
    BookingService bookingService;
    AuthenticationService auth;
    AdminService admin;

    // ROOT
    // ===============================
// GET ALL STOPS
// ===============================
server.Get("/stops", [&](const Request& req, Response& res) {

    json response = json::array();

    for (auto &s : metro.stops) {
        json stop;
        stop["id"] = s.second.id;
        stop["name"] = s.second.name;
        response.push_back(stop);
    }

    res.set_content(response.dump(), "application/json");
});
    server.Get("/", [](const Request&, Response& res) {
        res.set_content("Metro Booking Backend Running", "text/plain");
    });

    // ===============================
    // REGISTER
    // ===============================
    server.Post("/register", [&](const Request& req, Response& res) {

        json body = json::parse(req.body);

        string username = body["username"];
        string password = body["password"];
        string role = body.value("role", "USER");

        bool success = auth.registerUser(username, password, role);

        json response;

        if(success) {
            response["message"] = "User registered";
        } else {
            response["error"] = "User already exists";
        }

        res.set_content(response.dump(), "application/json");
    });

    // ===============================
    // LOGIN
    // ===============================
    server.Post("/login", [&](const Request& req, Response& res) {

        json body = json::parse(req.body);

        string username = body["username"];
        string password = body["password"];

        string token = auth.login(username, password);

        json response;

        if(token.empty()) {
            response["error"] = "Invalid credentials";
        } else {
            response["token"] = token;
        }

        res.set_content(response.dump(), "application/json");
    });

    // ===============================
    // ADMIN - ADD STOP
    // ===============================
    server.Post("/admin/stop", [&](const Request& req, Response& res) {

    string token = req.get_header_value("Authorization");

    json body = json::parse(req.body);

    string id = body["id"];
    string name = body["name"];

    admin.addNewStop(metro, auth, token, id, name);

    json response;
    response["message"] = "Stop processed";

    res.set_content(response.dump(), "application/json");
});

    // ===============================
    // CREATE BOOKING
    // ===============================
    server.Post("/booking", [&](const Request& req, Response& res) {

        string token = req.get_header_value("Authorization");

        json body = json::parse(req.body);

        string source = body["source"];
        string destination = body["destination"];

        json response =
            bookingService.createBookingAPI(
                metro, auth, token,
                source, destination
            );

        res.set_content(response.dump(), "application/json");
    });

    std::cout << "Server running at http://localhost:8080\n";

    server.listen("0.0.0.0", 8080);

    return 0;
}