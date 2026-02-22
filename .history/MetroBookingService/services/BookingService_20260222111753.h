#pragma once

#include "../models/Booking.h"
#include "../services/PathOptimizationEngine.h"
#include "../utils/QRGenerator.h"
#include "../services/FareService.h"
#include "../services/AuthenticationService.h"
#include "../utils/json.hpp"

#include <iostream>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <fstream>

using namespace std;
using json = nlohmann::json;

class BookingService {
public:

    atomic<int> bookingCounter{1};

    unordered_map<string, PathResult> routeCache;
    unordered_map<string, Booking> bookingStore;

    PathOptimizationEngine engine;
    FareService fareService;

    mutex cacheMutex;
    mutex bookingMutex;

    // =========================================
    // REST API BOOKING (FINAL CLEAN VERSION)
    // =========================================
    json createBookingAPI(
        MetroRouteService &metro,
        AuthenticationService &auth,
        string token,
        string source,
        string destination
    ) {

        json response;

        // 1. AUTH CHECK
        if (!auth.validateToken(token)) {
            response["error"] = "Unauthorized";
            return response;
        }

        // 2. STOP VALIDATION
        if (!metro.stopExists(source) ||
            !metro.stopExists(destination)) {

            response["error"] = "Invalid Stop";
            return response;
        }

        if (source == destination) {
            response["error"] = "Source and destination cannot be same";
            return response;
        }

        // 3. COMPUTE PATH
        PathResult result =
            engine.computePath(metro, source, destination);

        if (result.transfers == -1 || result.path.empty()) {
            response["error"] = "No route available";
            return response;
        }

        // 4. CREATE BOOKING
        string bookingId = "B" + to_string(bookingCounter++);

        int fare = fareService.calculateFare(
            result.path.size(),
            result.transfers
        );

        string qr = QRGenerator::generate(
            bookingId,
            source,
            destination
        );

        Booking booking(
            bookingId,
            source,
            destination,
            result.path,
            result.transfers,
            qr,
            fare,
            "PENDING"
        );

        string user = auth.getUserFromToken(token);

        string bookingKey =
            user + "|" + bookingId;

        {
            lock_guard<mutex> lock(bookingMutex);
            bookingStore[bookingKey] = booking;
        }

        // 5. RETURN RESPONSE
        response["bookingId"] = bookingId;
        response["route"] = result.path;
        response["fare"] = fare;
        response["transfers"] = result.transfers;
        response["qr"] = qr;
        response["status"] = "PENDING";

        return response;
    }
};