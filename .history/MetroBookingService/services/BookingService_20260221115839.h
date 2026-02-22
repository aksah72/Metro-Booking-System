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


    void loadBookingsFromFile() {

    ifstream file("data/bookings.json");

    if (!file.is_open()) return;

    json data;
    file >> data;

    for (auto &item : data) {

        Booking booking(
            item["bookingId"],
            item["source"],
            item["destination"],
            item["path"],
            item["transfers"],
            item["qr"],
            item["fare"]
        );

        string key =
            item["user"].get<string>() + "|" +
            item["source"].get<string>() + "|" +
            item["destination"].get<string>();

        bookingStore[key] = booking;
    }
}

    // ================================
    // Console Booking (Old Version)
    // ================================

    void saveBookingToFile(
    string user,
    Booking &booking
) {

    json data;

    ifstream in("data/bookings.json");
    if (in.is_open()) {
        in >> data;
        in.close();
    }

    json item;
    item["user"] = user;
    item["bookingId"] = booking.bookingId;
    item["source"] = booking.source;
    item["destination"] = booking.destination;
    item["path"] = booking.path;
    item["transfers"] = booking.transfers;
    item["qr"] = booking.qrCode;
    item["fare"] = booking.fare;

    data.push_back(item);

    ofstream out("data/bookings.json");
    out << data.dump(4);
}
    void createBooking(
        MetroRouteService &metro,
        AuthenticationService &auth,
        string token,
        string source,
        string destination
    ) {

        if (!auth.validateToken(token)) {
            cout << "UNAUTHORIZED ACCESS\n";
            return;
        }

        string user = auth.getUserFromToken(token);

        if (!metro.stopExists(source) ||
            !metro.stopExists(destination)) {
            cout << "STOP NOT FOUND\n";
            return;
        }

        if (source == destination) {
            cout << "Source and Destination are same\n";
            return;
        }

        string bookingKey =
            user + "|" + source + "|" + destination;

        {
            lock_guard<mutex> lock(bookingMutex);
            if (bookingStore.find(bookingKey) != bookingStore.end()) {
                cout << "\nDuplicate booking detected\n";
                return;
            }
        }

        PathResult result =
            engine.computePath(metro, source, destination);

        if (result.transfers == -1) {
            cout << "NO ROUTE AVAILABLE\n";
            return;
        }

        string bookingId =
            "B" + to_string(bookingCounter++);

        string qr =
            QRGenerator::generate(
                bookingId,
                source,
                destination);

        int fare =
            fareService.calculateFare(
                result.path.size(),
                result.transfers);

        Booking booking(
            bookingId,
            source,
            destination,
            result.path,
            result.transfers,
            qr,
            fare);

        {
            lock_guard<mutex> lock(bookingMutex);
            bookingStore[bookingKey] = booking;
        }

        cout << "\nBooking Created\n";
        cout << "Booking ID: " << bookingId << "\n";
        cout << "Fare: Rs " << fare << "\n";
    }

    // =================================
    // REST API Booking (JSON Version)
    // =================================
    json createBookingAPI(
        MetroRouteService &metro,
        AuthenticationService &auth,
        string token,
        string source,
        string destination
    ) {

        json response;

        if (!auth.validateToken(token)) {
            response["error"] = "Unauthorized";
            return response;
        }

        if (!metro.stopExists(source) ||
            !metro.stopExists(destination)) {

            response["error"] = "Invalid Stops";
            return response;
        }

        PathResult result =
            engine.computePath(metro, source, destination);

        if (result.transfers == -1) {
            response["error"] = "No route available";
            return response;
        }

        int fare =
            fareService.calculateFare(
                result.path.size(),
                result.transfers);

        response["route"] = result.path;
        response["transfers"] = result.transfers;
        response["fare"] = fare;

        return response;
    }
};