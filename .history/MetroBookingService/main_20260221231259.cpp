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

    server.set_default_headers({
    {"Access-Control-Allow-Origin", "*"},
    {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
    {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
});

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

    // ===============================
    // ROOT
    // ===============================



    server.Get("/", [](const Request&, Response& res) {
        res.set_content("Metro Booking Backend Running", "text/plain");
    });

    // ===============================
    // GET ALL STOPS
    // ===============================
    server.Get("/stops", [&](const Request&, Response& res) {

        json response = json::array();

        for (auto &s : metro.stops) {
            response.push_back({
                {"id", s.second.id},
                {"name", s.second.name}
            });
        }

        res.set_content(response.dump(), "application/json");
    });

    // ===============================
    // GET ALL ROUTES
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

        res.set_content(response.dump(), "application/json");
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

        string token = auth.login(
            body["username"],
            body["password"]
        );

        if(token.empty())
            res.set_content(R"({"error":"Invalid credentials"})","application/json");
        else
            res.set_content(json({{"token",token}}).dump(),"application/json");
    });

    // ===============================
    // ADMIN - ADD STOP
    // ===============================
    server.Post("/admin/stop", [&](const Request& req, Response& res) {

        string token = req.get_header_value("Authorization");

        if(!auth.validateToken(token) ||
           auth.getUserRole(token) != "ADMIN") {

            res.set_content(R"({"error":"Unauthorized"})","application/json");
            return;
        }

        json body = json::parse(req.body);

        admin.addNewStop(
            metro,
            auth,
            token,
            body["id"],
            body["name"]
        );

        res.set_content(R"({"message":"Stop added"})","application/json");
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

        res.set_content(result.dump(),"application/json");
    });

    // ===============================
    // PAYMENT API
    // ===============================
    server.Post("/pay", [&](const Request& req, Response& res) {

        string token = req.get_header_value("Authorization");

        if (!auth.validateToken(token)) {
            res.set_content(R"({"error":"Unauthorized"})","application/json");
            return;
        }

        json body = json::parse(req.body);
        string bookingId = body["bookingId"];

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
    // GET USER BOOKINGS
    // ===============================
    server.Get("/bookings", [&](const Request& req, Response& res) {

        string token = req.get_header_value("Authorization");

        if (!auth.validateToken(token)) {
            res.set_content(R"({"error":"Unauthorized"})","application/json");
            return;
        }

        string user = auth.getUserFromToken(token);

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
    // ADMIN ANALYTICS
    // ===============================
    server.Get("/admin/analytics", [&](const Request& req, Response& res) {

        string token = req.get_header_value("Authorization");

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


    std::cout << "Server running at http://localhost:8080\n";
    std::cout << "Serving UI from: ui\n";
    server.listen("0.0.0.0",8080);

    return 0;
}