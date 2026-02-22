#include "services/MetroRouteService.h"
#include "services/BookingService.h"
#include "services/AuthenticationService.h"
#include "services/AdminService.h"

#include "utils/httplib.h"

#include <iostream>
#include "utils/json.hpp"

using namespace httplib;
using json = nlohmann::json;
using namespace std;

int main() {

    Server server;

    MetroRouteService metro;
    BookingService bookingService;
    AuthenticationService auth;
    AdminService adminService;

    // ---------------- REGISTER ----------------
    server.Post("/register", [&](const Request& req, Response& res) {

        auto body = json::parse(req.body);

        string username = body["username"];
        string password = body["password"];
        string role = body.value("role", "USER");

        bool ok = auth.registerUser(username, password, role);

        if(!ok) {
            res.status = 400;
            res.set_content("{\"message\":\"User exists\"}", "application/json");
            return;
        }

        res.set_content("{\"message\":\"Registered\"}", "application/json");
    });

    // ---------------- LOGIN ----------------
    server.Post("/login", [&](const Request& req, Response& res) {

        auto body = json::parse(req.body);

        string username = body["username"];
        string password = body["password"];

        string token = auth.login(username, password);

        if(token == "") {
            res.status = 401;
            res.set_content("{\"message\":\"Invalid credentials\"}", "application/json");
            return;
        }

        json response;
        response["token"] = token;

        res.set_content(response.dump(), "application/json");
    });

    // ---------------- ADMIN ADD STOP ----------------
    server.Post("/admin/stop", [&](const Request& req, Response& res) {

        string token = req.get_header_value("Authorization");

        if(!auth.validateToken(token) ||
           auth.getUserRole(token) != "ADMIN") {

            res.status = 403;
            res.set_content("{\"message\":\"Access denied\"}", "application/json");
            return;
        }

        auto body = json::parse(req.body);

        string id = body["id"];
        string name = body["name"];

        metro.addStop(id, name);

        res.set_content("{\"message\":\"Stop added\"}", "application/json");
    });

    // ---------------- BOOKING ----------------
    server.Post("/booking", [&](const Request& req, Response& res) {

        string token = req.get_header_value("Authorization");

        if(!auth.validateToken(token)) {
            res.status = 401;
            res.set_content("{\"message\":\"Unauthorized\"}", "application/json");
            return;
        }

        auto body = json::parse(req.body);

        string source = body["source"];
        string destination = body["destination"];

        bookingService.createBooking(
            metro,
            auth,
            token,
            source,
            destination);

        res.set_content("{\"message\":\"Booking processed\"}", "application/json");
    });

    cout << "Server running at http://localhost:8080\n";

    server.listen("0.0.0.0", 8080);
}