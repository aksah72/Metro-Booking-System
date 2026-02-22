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

    // ===============================
    // SERVICES
    // ===============================

    MetroRouteService metro;
    BookingService bookingService;
    AuthenticationService auth;
    AdminService admin;

    // ===============================
    // DEFAULT METRO DATA
    // ===============================

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

    // ===============================
    // SERVE UI
    // ===============================

    server.Get("/app", [&](const Request&, Response& res) {
        std::ifstream file("ui/index.html");
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.set_content(buffer.str(), "text/html");
    });

    server.Get("/app/index.html", [&](const Request&, Response& res) {
        std::ifstream file("ui/index.html");
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.set_content(buffer.str(), "text/html");
    });

    // ===============================
    // ROOT CHECK
    // ===============================

    server.Get("/", [](const Request&, Response& res) {
        res.set_content("Metro Booking Backend Running", "text/plain");
    });

    // ===============================
    // REGISTER
    // ===============================

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

    // ===============================
    // LOGIN
    // ===============================

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

    // ===============================
    // CREATE BOOKING
    // ===============================

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

    // ===============================
    // PAYMENT
    // ===============================

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

    // ===============================
    // GET BOOKINGS
    // ===============================

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
                    {"path", b.second.path},
                    {"transfers", b.second.transfers},
                    {"fare", b.second.fare},
                    {"status", b.second.status}
                });
            }
        }

        res.set_content(response.dump(),"application/json");
    });

    // ===============================
    // STOPS
    // ===============================

    server.Get("/stops", [&](const Request&, Response& res) {

        json response = json::array();

        for (auto &s : metro.stops) {
            response.push_back({
                {"id", s.second.id},
                {"name", s.second.name}
            });
        }

        res.set_content(response.dump(),"application/json");
    });

    // ===============================
    // ROUTES
    // ===============================

    server.Get("/routes", [&](const Request&, Response& res) {

        json response = json::array();

        for (auto &r : metro.routes) {
            response.push_back({
                {"id", r.second.id},
                {"color", r.second.color},
                {"stops", r.second.stopIds}
            });
        }

        res.set_content(response.dump(),"application/json");
    });

    // ===============================
    // ADMIN ANALYTICS
    // ===============================

    server.Get("/admin/analytics", [&](const Request& req, Response& res) {

        std::string token = req.get_header_value("Authorization");

        if (!auth.validateToken(token) ||
            auth.getUserRole(token) != "ADMIN") {

            res.set_content(R"({"error":"Unauthorized"})","application/json");
            return;
        }

        int totalBookings = 0;
        int confirmed = 0;
        int pending = 0;
        int totalRevenue = 0;

        for (auto &b : bookingService.bookingStore) {

            totalBookings++;

            if (b.second.status == "CONFIRMED") {
                confirmed++;
                totalRevenue += b.second.fare;
            } else {
                pending++;
            }
        }

        res.set_content(json({
            {"totalBookings", totalBookings},
            {"confirmed", confirmed},
            {"pending", pending},
            {"totalRevenue", totalRevenue}
        }).dump(),"application/json");
    });

    // ===============================

    std::cout << "Server running at http://localhost:8080\n";
    std::cout << "Open UI at: http://localhost:8080/app\n";

    server.listen("0.0.0.0",8080);

    return 0;
}