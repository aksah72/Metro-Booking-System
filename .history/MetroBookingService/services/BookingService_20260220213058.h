#pragma once
#include "../models/Booking.h"
#include "../services/PathOptimizationEngine.h"
#include "../utils/QRGenerator.h"
#include "../services/FareService.h"
#include <iostream>
#include <unordered_map>
using namespace std;

class BookingService {
public:
    int bookingCounter = 1;
    unordered_map<string, pair<vector<string>, int>> routeCache;

    PathOptimizationEngine engine;

    void createBooking(
        MetroRouteService &metro,
        string source,
        string destination
    ) {

        if (!metro.stopExists(source) ||
            !metro.stopExists(destination)) {
            cout << "STOP NOT FOUND\n";
            return;
        }

        if (source == destination) {
            cout << "Source and Destination are same\n";
            return;
        }

        pair<vector<string>, int> result;

        string key = source + "|" + destination;

        if(routeCache.find(key) != routeCache.end()) {
            result = routeCache[key];
            cout << "Cache Hit\n";
        } else {
            result = engine.computePath(metro, source, destination);

            if(result.second != -1) {
                routeCache[key] = result;
            }
        }

        if (result.second == -1) {
            cout << "NO ROUTE AVAILABLE\n";
            return;
        }

        string bookingId = "B" + to_string(bookingCounter++);
        string qr = QRGenerator::generate(
                        bookingId, source, destination);

        Booking booking(bookingId, source,
                        destination,
                        result.first,
                        result.second,
                        qr);

        cout << "\nBooking Created\n";
        cout << "Booking ID: " << booking.bookingId << "\n";

        cout << "Path: ";
        for (auto &s : booking.path)
            cout << s << " ";

        cout << "\nTransfers: "
             << booking.transfers << "\n";

        cout << "QR String: "
             << booking.qrString << "\n";
    }
};