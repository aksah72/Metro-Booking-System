#pragma once

#include "../services/PathOptimizationEngine.h"
#include "../services/FareService.h"
#include "../services/AuthenticationService.h"
#include "../utils/json.hpp"

#include <mutex>

using namespace std;
using json = nlohmann::json;

class BookingService {
public:

    PathOptimizationEngine engine;
    FareService fareService;

    mutex bookingMutex;

    json createBookingAPI(
        MetroRouteService &metro,
        AuthenticationService &auth,
        string token,
        string source,
        string destination,
        string finalTime
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

        int fare = fareService.calculateFare(
            result.path.size(),
            result.transfers
        );

        string qr = "TEMP_QR"; // temporary before insert

        // ======================
        // INSERT INTO SQLITE
        // ======================
        string sql =
            "INSERT INTO bookings(user_id,source,destination,fare,status,qr)"
            " VALUES(" + to_string(userId) + ",'" +
            source + "','" + destination + "'," +
            to_string(fare) + ",'PENDING','TEMP');";

        lock_guard<mutex> lock(bookingMutex);

        if (!auth.db.execute(sql)) {
            response["error"] = "Database error";
            return response;
        }

        // ======================
        // GET LAST INSERT ID
        // ======================
        int bookingId = sqlite3_last_insert_rowid(auth.db.db);

        qr = "BOOKING_" + to_string(bookingId);

        // Update QR with correct value
        string updateSql =
            "UPDATE bookings SET qr='" + qr +
            "' WHERE id=" + to_string(bookingId) + ";";

        auth.db.execute(updateSql);

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
        response["Time"] = finalTime
        response["qr"] = qr;
        response["status"] = "PENDING";

        return response;
    }
};