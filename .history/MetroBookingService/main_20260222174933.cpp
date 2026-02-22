#include <iostream>
#include <fstream>
#include <sstream>

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

    // ================================
    // DEFAULT METRO DATA
    // ================================
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

    auth.registerUser("admin","admin","ADMIN");

    // ================================
    // SERVE UI
    // ================================
    server.Get("/", [&](const Request&, Response& res) {
        std::ifstream file("ui/index.html");
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.set_content(buffer.str(), "text/html");
    });

    // ================================
    // REGISTER
    // ================================
    server.Post("/register", [&](const Request& req, Response& res) {

        json body = json::parse(req.body);

        bool success = auth.registerUser(
            body["username"],
            body["password"],
            body.value("role","USER")
        );

        if(success)
            res.set_content(R"({"message":"User registered"})","application/json");
        else
            res.set_content(R"({"error":"User already exists"})","application/json");
    });

    // ================================
    // LOGIN
    // ================================
    server.Post("/login", [&](const Request& req, Response& res) {

        json body = json::parse(req.body);

        std::string token = auth.login(
            body["username"],
            body["password"]
        );

        if(token.empty())
            res.set_content(R"({"error":"Invalid credentials"})","application/json");
        else
            res.set_content(json({{"token",token}}).dump(),"application/json");
    });

    // ================================
    // CREATE BOOKING
    // ================================
    server.Post("/booking", [&](const Request& req, Response& res) {

        std::string token = req.get_header_value("Authorization");
        json body = json::parse(req.body);

        json result =
            bookingService.createBookingAPI(
                metro,
                auth,
                token,
                body["source"],
                body["destination"]
            );

        res.set_content(result.dump(),"application/json");
    });

    // ================================
    // PAYMENT
    // ================================
    server.Post("/pay", [&](const Request& req, Response& res) {

        std::string token = req.get_header_value("Authorization");

        if (!auth.validateToken(token)) {
            res.set_content(R"({"error":"Unauthorized"})","application/json");
            return;
        }

        json body = json::parse(req.body);
        std::string bookingId = body["bookingId"];

        for (auto &b : bookingService.bookingStore) {

            if (b.second.bookingId == bookingId) {

                b.second.status = "CONFIRMED";

                res.set_content(
                    R"({"message":"Payment Successful","status":"CONFIRMED"})",
                    "application/json"
                );
                return;
            }
        }

        res.set_content(R"({"error":"Booking not found"})","application/json");
    });

    // ================================
    // GET BOOKINGS
    // ================================
    server.Get("/bookings", [&](const Request& req, Response& res) {

        std::string token = req.get_header_value("Authorization");

        if (!auth.validateToken(token)) {
            res.set_content(R"({"error":"Unauthorized"})","application/json");
            return;
        }

        std::string user = auth.getUserFromToken(token);
        json response = json::array();

        for (auto &b : bookingService.bookingStore) {

            if (b.first.find(user + "|") == 0) {

                response.push_back({
                    {"bookingId", b.second.bookingId},
                    {"source", b.second.source},
                    {"destination", b.second.destination},
                    {"route", b.second.path},
                    {"transfers", b.second.transfers},
                    {"fare", b.second.fare},
                    {"status", b.second.status}
                });
            }
        }

        res.set_content(response.dump(),"application/json");
    });

    std::cout << "Server running at http://localhost:8080\n";

    server.listen("0.0.0.0",8080);

    return 0;
}