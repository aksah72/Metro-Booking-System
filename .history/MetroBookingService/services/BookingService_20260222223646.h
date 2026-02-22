#pragma once

#include "../services/PathOptimizationEngine.h"
#include "../services/FareService.h"
#include "../services/AuthenticationService.h"
#include "../utils/json.hpp"

#include <atomic>
#include <mutex>

using namespace std;
using json = nlohmann::json;

class BookingService {
public:

    atomic<int> bookingCounter{1};

    PathOptimizationEngine engine;
    FareService fareService;

    mutex cacheMutex;

    json createBookingAPI(
        MetroRouteService &metro,
        AuthenticationService &auth,
        string token,
        string source,
        string destination
    )
    {
        json response;

        // ======================
        // AUTH CHECK
        // ======================
        if (!auth.validateToken(token)) {
            response["error"] = "Unauthorized";
            return response;
        }

        int userId = auth.getUserIdFromToken(token);

        // ======================
        // VALIDATION
        // ======================
        if (!metro.stopExists(source) ||
            !metro.stopExists(destination) ||
            source == destination) {

            response["error"] = "Invalid stops";
            return response;
        }

        // ======================
        // COMPUTE PATH
        // ======================
        PathResult result =
            engine.computePath(metro, source, destination);

        if (result.path.empty()) {
            response["error"] = "No route available";
            return response;
        }

        // ======================
        // CREATE BOOKING
        // ======================
        string bookingId = to_string(bookingCounter++);

        int fare = fareService.calculateFare(
            result.path.size(),
            result.transfers
        );

        string qr = "BOOKING_" + bookingId;

        // ======================
        // INSERT INTO SQLITE
        // ======================
        string sql =
            "INSERT INTO bookings(user_id,source,destination,fare,status,qr)"
            " VALUES(" + to_string(userId) + ",'" +
            source + "','" + destination + "'," +
            to_string(fare) + ",'PENDING','" +
            qr + "');";

        auth.db.execute(sql);

        // ======================
        // RESPONSE
        // ======================
        response["bookingId"] = bookingId;
        response["route"] = result.path;

        response["segments"] = json::array();
        for (auto &seg : result.segments) {
            response["segments"].push_back({
                {"line", seg.line},
                {"stops", seg.stops}
            });
        }

        response["transfers"] = result.transfers;
        response["duration"] = result.totalTime;
        response["fare"] = fare;
        response["qr"] = qr;
        response["status"] = "PENDING";

        return response;
    }
};