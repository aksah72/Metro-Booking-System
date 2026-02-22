#pragma once

#include "../models/Booking.h"
#include "../services/PathOptimizationEngine.h"
#include "../services/FareService.h"
#include "../services/AuthenticationService.h"
#include "../utils/json.hpp"

#include <unordered_map>
#include <mutex>
#include <atomic>

using namespace std;
using json = nlohmann::json;

class BookingService {
public:

    atomic<int> bookingCounter{1};

    unordered_map<string, Booking> bookingStore;
    unordered_map<string, PathResult> routeCache;

    PathOptimizationEngine engine;
    FareService fareService;

    mutex bookingMutex;
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

        if (!auth.validateToken(token)) {
            response["error"] = "Unauthorized";
            return response;
        }

        if (!metro.stopExists(source) ||
            !metro.stopExists(destination) ||
            source == destination) {

            response["error"] = "Invalid stops";
            return response;
        }

        PathResult result;

        string cacheKey = source + "|" + destination;

        {
            lock_guard<mutex> lock(cacheMutex);

            if(routeCache.find(cacheKey) != routeCache.end())
                result = routeCache[cacheKey];
            else {
                result = engine.computePath(metro, source, destination);
                routeCache[cacheKey] = result;
            }
        }

        if(result.path.empty()){
            response["error"] = "No route available";
            return response;
        }

        string bookingId = "B" + to_string(bookingCounter++);

        int fare = fareService.calculateFare(
            result.path.size(),
            result.transfers
        );

        string rawData = bookingId + source + destination;
        string qr = to_string(hash<string>{}(rawData));

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
        string key = user + "|" + bookingId;

        {
            lock_guard<mutex> lock(bookingMutex);
            bookingStore[key] = booking;
        }

        response["bookingId"] = bookingId;
        response["route"] = result.path;
        response["segments"] = json::array();

        for(auto &seg : result.segments){
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