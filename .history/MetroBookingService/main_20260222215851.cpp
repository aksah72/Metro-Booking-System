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
    AuthenticationService auth;
    BookingService bookingService;
    AdminService admin;

    // ======================
    // DEFAULT METRO DATA
    // ======================
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

    // Default admin
    auth.registerUser("admin@metro.com","admin123","ADMIN");

    // ======================
    // SERVE UI
    // ======================
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

    // ======================
    // REGISTER (EMAIL)
    // ======================
    server.Post("/register", [&](const Request& req, Response& res) {

        json body = json::parse(req.body);

        bool success = auth.registerUser(
            body["email"],
            body["password"],
            "USER"
        );

        if(success)
            res.set_content(R"({"message":"User registered"})","application/json");
        else
            res.set_content(R"({"error":"User already exists"})","application/json");
    });

    // ======================
    // LOGIN (EMAIL)
    // ======================
    server.Post("/login", [&](const Request& req, Response& res){

        json body = json::parse(req.body);

        std::string token = auth.login(
            body["email"],
            body["password"]
        );

        if(token==""){
            res.set_content(R"({"error":"Invalid login"})","application/json");
            return;
        }

        json response;
        response["token"] = token;
        res.set_content(response.dump(),"application/json");
    });

    // ======================
    // BOOKING
    // ======================
    server.Post("/booking",[&](const Request& req, Response& res){

    string token=req.get_header_value("Authorization");

    if(!auth.validateToken(token)){
        res.set_content(R"({"error":"Unauthorized"})","application/json");
        return;
    }

    int userId=auth.getUserIdFromToken(token);

    json body=json::parse(req.body);

    json result=bookingService.createBookingAPI(
        metro,
        auth,
        token,
        body["source"],
        body["destination"]
    );

    res.set_content(result.dump(),"application/json");
});

    // ======================
    // PAYMENT (QR FIXED)
    // ======================
    server.Post("/pay", [&](const Request& req, Response& res) {

        std::string token = req.get_header_value("Authorization");

        if (!auth.validateToken(token)) {
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})","application/json");
            return;
        }

        json body = json::parse(req.body);
        std::string bookingId = body["bookingId"];
        std::string user = auth.getUserFromToken(token);
        std::string key = user + "|" + bookingId;

        if(bookingService.bookingStore.find(key) ==
           bookingService.bookingStore.end()){

            res.set_content(R"({"error":"Booking not found"})","application/json");
            return;
        }

        bookingService.bookingStore[key].status = "CONFIRMED";

        json response;
        response["message"] = "Payment Successful";
        response["status"] = "CONFIRMED";
        response["qr"] = bookingService.bookingStore[key].qr;

        res.set_content(response.dump(),"application/json");
    });

    std::cout << "Server running at http://localhost:9090\n";

    server.listen("0.0.0.0",9090);

    return 0;
}