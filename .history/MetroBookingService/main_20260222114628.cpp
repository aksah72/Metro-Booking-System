#include <iostream>
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
    BookingService bookingService;
    AuthenticationService auth;

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

    server.set_mount_point("/", "./ui");

    server.Post("/register",[&](const Request& req, Response& res){
        json body = json::parse(req.body);
        bool ok = auth.registerUser(
            body["username"],
            body["password"],
            "USER"
        );

        if(ok)
            res.set_content(R"({"message":"Registered"})","application/json");
        else
            res.set_content(R"({"error":"User exists"})","application/json");
    });

    server.Post("/login",[&](const Request& req, Response& res){
        json body = json::parse(req.body);
        string token = auth.login(
            body["username"],
            body["password"]
        );

        if(token.empty())
            res.set_content(R"({"error":"Invalid"})","application/json");
        else
            res.set_content(json({{"token",token}}).dump(),"application/json");
    });

    server.Post("/booking",[&](const Request& req, Response& res){
        json body = json::parse(req.body);

        json result =
            bookingService.createBookingAPI(
                metro,
                auth,
                req.get_header_value("Authorization"),
                body["source"],
                body["destination"]
            );

        res.set_content(result.dump(),"application/json");
    });

    server.Post("/pay",[&](const Request& req, Response& res){

        string token = req.get_header_value("Authorization");

        if(!auth.validateToken(token)){
            res.set_content(R"({"error":"Unauthorized"})","application/json");
            return;
        }

        res.set_content(
            R"({"message":"Payment Successful"})",
            "application/json"
        );
    });

    server.Get("/bookings",[&](const Request& req, Response& res){

        string token = req.get_header_value("Authorization");

        if(!auth.validateToken(token)){
            res.set_content(R"({"error":"Unauthorized"})","application/json");
            return;
        }

        json arr = json::array();

        for(auto &b : bookingService.bookingStore){
            arr.push_back({
                {"bookingId", b.second.bookingId},
                {"source", b.second.source},
                {"destination", b.second.destination},
                {"fare", b.second.fare},
                {"status", b.second.status}
            });
        }

        res.set_content(arr.dump(),"application/json");
    });

    std::cout << "Server running at http://localhost:8080\n";
    server.listen("0.0.0.0",8080);
}