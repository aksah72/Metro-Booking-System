#include <iostream>
#include <fstream>
#include <sstream>

#include "utils/httplib.h"
#include "utils/json.hpp"

#include "services/MetroRouteService.h"
#include "services/BookingService.h" 
#include "services/AuthenticationService.h"

using namespace httplib;
using json = nlohmann::json;

int main() {

    Server server;

    MetroRouteService metro;
    AuthenticationService auth;
    BookingService bookingService;

    // =====================================
    // METRO NETWORK SETUP
    // =====================================

    Route red("R","Red");
    red.addStop("North City");
    red.addStop("Tech Park");
    red.addStop("Central Market");
    red.addStop("City Hospital");
    red.addStop("Grand Junction");
    red.addStop("Museum");
    red.addStop("Old Town");
    red.addStop("South Terminal");

    Route blue("B","Blue");
    blue.addStop("Airport");
    blue.addStop("IT Hub");
    blue.addStop("Lake View");
    blue.addStop("Grand Junction");
    blue.addStop("City Center");
    blue.addStop("Stadium");
    blue.addStop("University");
    blue.addStop("West End");

    Route yellow("Y","Yellow");
    yellow.addStop("Industrial Area");
    yellow.addStop("Science Park");
    yellow.addStop("City Center");
    yellow.addStop("Metro Plaza");
    yellow.addStop("Railway Station");
    yellow.addStop("East Market");
    yellow.addStop("Old Port");

    vector<string> allStops = {
        "North City","Tech Park","Central Market","City Hospital",
        "Grand Junction","Museum","Old Town","South Terminal",
        "Airport","IT Hub","Lake View","City Center",
        "Stadium","University","West End",
        "Industrial Area","Science Park","Metro Plaza",
        "Railway Station","East Market","Old Port"
    };

    for(auto &s : allStops)
        metro.addStop(s,s);

    metro.addRoute(red);
    metro.addRoute(blue);
    metro.addRoute(yellow);
    metro.buildGraph();

    // =====================================
    // SERVE UI
    // =====================================

    server.Get("/", [](const Request&, Response& res) {

        std::ifstream file("ui/index.html");

        if (!file.is_open()) {
            res.status = 500;
            res.set_content("UI file not found", "text/plain");
            return;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        res.set_content(buffer.str(), "text/html");
    });

    // =====================================
    // REGISTER
    // =====================================

    server.Post("/register", [&](const Request& req, Response& res) {

        try {
            json body = json::parse(req.body);

            if(!body.contains("email") || !body.contains("password")) {
                res.set_content(R"({"error":"Missing fields"})","application/json");
                return;
            }

            bool success = auth.registerUser(
                body["email"].get<string>(),
                body["password"].get<string>()
            );

            if(success)
                res.set_content(R"({"message":"User registered"})","application/json");
            else
                res.set_content(R"({"error":"User already exists"})","application/json");

        } catch (...) {
            res.set_content(R"({"error":"Invalid JSON"})","application/json");
        }
    });

    // =====================================
    // LOGIN
    // =====================================

    server.Post("/login", [&](const Request& req, Response& res){

        try {
            json body = json::parse(req.body);

            string token = auth.login(
                body["email"].get<string>(),
                body["password"].get<string>()
            );

            if(token.empty()){
                res.set_content(R"({"error":"Invalid login"})","application/json");
                return;
            }

            json response;
            response["token"] = token;
            res.set_content(response.dump(),"application/json");

        } catch (...) {
            res.set_content(R"({"error":"Invalid JSON"})","application/json");
        }
    });

    // =====================================
    // BOOKING
    // =====================================

    server.Post("/booking",[&](const Request& req, Response& res){

        string token = req.get_header_value("Authorization");

        if(!auth.validateToken(token)){
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})","application/json");
            return;
        }

        try {
            json body = json::parse(req.body);

            json result = bookingService.createBookingAPI(
                metro,
                auth,
                token,
                body["source"].get<string>(),
                body["destination"].get<string>()
            );

            res.set_content(result.dump(),"application/json");

        } catch (...) {
            res.set_content(R"({"error":"Invalid JSON"})","application/json");
        }
    });

    // =====================================
    // PAYMENT
    // =====================================

    server.Post("/pay",[&](const Request& req, Response& res){

        string token = req.get_header_value("Authorization");

        if(!auth.validateToken(token)){
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})","application/json");
            return;
        }

        try {
            int userId = auth.getUserIdFromToken(token);
            json body = json::parse(req.body);

            int bookingId = body["bookingId"].get<int>();

            string sql =
                "UPDATE bookings SET status='CONFIRMED' "
                "WHERE id=" + to_string(bookingId) +
                " AND user_id=" + to_string(userId) + ";";

            if(!auth.db.execute(sql)){
                res.set_content(R"({"error":"Payment failed"})","application/json");
                return;
            }

            json response;
            response["status"] = "CONFIRMED";
            response["qr"] = "BOOKING_" + to_string(bookingId);

            res.set_content(response.dump(),"application/json");

        } catch (...) {
            res.set_content(R"({"error":"Invalid request"})","application/json");
        }
    });

    // =====================================
    // START SERVER
    // =====================================
    server.set_mount_point("/ui", "./ui");
    std::cout << "Server running at http://localhost:9090\n";
    server.listen("0.0.0.0",9090);

    return 0;
}