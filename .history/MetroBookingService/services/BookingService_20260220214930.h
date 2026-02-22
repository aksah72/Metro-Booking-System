#pragma once

#include "../models/Booking.h"
#include "../services/PathOptimizationEngine.h"
#include "../utils/QRGenerator.h"
#include "../services/FareService.h"
#include "../services/AuthenticationService.h"

#include <iostream>
#include <unordered_map>
#include <mutex>
#include <atomic>

using namespace std;

class BookingService {
public:
    atomic<int> bookingCounter{1};
    unordered_map<string, PathResult> routeCache;

    PathOptimizationEngine engine;
    FareService fareService;

    mutex cacheMutex;

    void createBooking(
        MetroRouteService &metro,
        AuthenticationService &auth,
        string token,
        string source,
        string destination
    ) {

        // 🔐 AUTHENTICATION CHECK
        if (!auth.validateToken(token)) {
            cout << "UNAUTHORIZED ACCESS\n";
            return;
        }

        string user = auth.getUserFromToken(token);
        cout << "Booking requested by: "
             << user << "\n";

        // 🚫 VALIDATION
        if (!metro.stopExists(source) ||
            !metro.stopExists(destination)) {
            cout << "STOP NOT FOUND\n";
            return;
        }

        if (source == destination) {
            cout << "Source and Destination are same\n";
            return;
        }

        PathResult result;
        string key = source + "|" + destination;

        // 🔒 THREAD-SAFE CACHE ACCESS
        {
            lock_guard<mutex> lock(cacheMutex);

            if (routeCache.find(key) != routeCache.end()) {
                result = routeCache[key];
                cout << "Cache Hit\n";
            } else {
                result = engine.computePath(
                    metro, source, destination);

                if (result.transfers != -1)
                    routeCache[key] = result;
            }
        }

        if (result.transfers == -1) {
            cout << "NO ROUTE AVAILABLE\n";
            return;
        }

        // 🎟️ BOOKING CREATION
        string bookingId =
            "B" + to_string(bookingCounter++);

        string qr =
            QRGenerator::generate(
                bookingId,
                source,
                destination
            );

        int fare =
            fareService.calculateFare(
                result.path.size(),
                result.transfers
            );

        Booking booking(
            bookingId,
            source,
            destination,
            result.path,
            result.transfers,
            qr,
            fare
        );

        // 📤 OUTPUT
        cout << "\nBooking Created\n";
        cout << "Booking ID: "
             << booking.bookingId << "\n";

        cout << "\nRoute Details:\n";

        string currentRoute =
            result.routesUsed[0];

        cout << "Take Line "
             << currentRoute << ":\n";

        cout << result.path[0];

        for (int i = 1;
             i < result.path.size(); i++) {

            if (result.routesUsed[i-1] != currentRoute) {

                cout << "\n\nInterchange at "
                     << result.path[i-1] << "\n";

                currentRoute =
                    result.routesUsed[i-1];

                cout << "Take Line "
                     << currentRoute << ":\n";

                cout << result.path[i-1];
            }

            cout << " -> "
                 << result.path[i];
        }

        cout << "\n\nTransfers: "
             << booking.transfers << "\n";

        cout << "Fare: Rs "
             << booking.fare << "\n";

        cout << "QR String: "
             << booking.qrString << "\n";
    }
};