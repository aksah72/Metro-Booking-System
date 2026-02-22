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

    // ==================================
// AUTO LOAD DEFAULT METRO NETWORK
// ==================================

metro.addStop("A","Station A");
metro.addStop("B","Station B");
metro.addStop("C","Station C");
metro.addStop("D","Station D");
metro.addStop("E","Station E");

Route yellow("Y","Yellow");
yellow.addStop("A");
yellow.addStop("B");
yellow.addStop("C");

Route blue("B","Blue");
blue.addStop("C");
blue.addStop("D");
blue.addStop("E");

metro.addRoute(yellow);
metro.addRoute(blue);

metro.buildGraph();

auth.registerUser("admin", "admin", "ADMIN");

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

    // ===============================
// GET ALL ROUTES
// ===============================
server.Get("/routes", [&](const Request& req, Response& res) {

    json response = json::array();

    for (auto &r : metro.routes) {
        json route;
        route["id"] = r.second.id;
        route["color"] = r.second.color;
        route["stops"] = r.second.stopIds;
        response.push_back(route);
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

    json result =
        bookingService.createBookingAPI(
            metro,
            auth,
            token,
            body["source"],
            body["destination"]
        );

    res.set_content(result.dump(), "application/json");
});

    std::cout << "Server running at http://localhost:8080\n";

    server.listen("0.0.0.0", 8080);

    return 0;
}