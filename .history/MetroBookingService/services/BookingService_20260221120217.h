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
    // LOAD BOOKINGS FROM FILE (On Startup)
    // =========================================
    void loadBookingsFromFile() {

        ifstream file("data/bookings.json");
        if (!file.is_open()) return;

        json data;
        try {
            file >> data;
        } catch (...) {
            return;
        }

        int maxId = 0;

        for (auto &item : data) {

            Booking booking(
                item["bookingId"],
                item["source"],
                item["destination"],
                item["path"].get<vector<string>>(),
                item["transfers"],
                item["qr"],
                item["fare"]
            );

            string key =
                item["user"].get<string>() + "|" +
                item["source"].get<string>() + "|" +
                item["destination"].get<string>();

            bookingStore[key] = booking;

            // Update counter to avoid duplicate IDs
            string id = item["bookingId"];
            if (id.length() > 1) {
                int numeric = stoi(id.substr(1));
                maxId = max(maxId, numeric);
            }
        }

        bookingCounter = maxId + 1;
    }

    // =========================================
    // SAVE BOOKING TO FILE
    // =========================================
    void saveBookingToFile(string user, Booking &booking) {

        json data = json::array();

        ifstream in("data/bookings.json");
        if (in.is_open()) {
            try {
                in >> data;
            } catch (...) {
                data = json::array();
            }
            in.close();
        }

        json item;
        item["user"] = user;
        item["bookingId"] = booking.bookingId;
        item["source"] = booking.source;
        item["destination"] = booking.destination;
        item["path"] = booking.path;
        item["transfers"] = booking.transfers;
        item["qr"] = booking.;
        item["fare"] = booking.fare;

        data.push_back(item);

        ofstream out("data/bookings.json");
        out << data.dump(4);
    }

    // =========================================
    // REST API BOOKING (FINAL VERSION)
    // =========================================
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

        string user = auth.getUserFromToken(token);

        if (!metro.stopExists(source) ||
            !metro.stopExists(destination)) {

            response["error"] = "Invalid Stops";
            return response;
        }

        if (source == destination) {
            response["error"] = "Source and Destination are same";
            return response;
        }

        string bookingKey =
            user + "|" + source + "|" + destination;

        {
            lock_guard<mutex> lock(bookingMutex);

            if (bookingStore.find(bookingKey) != bookingStore.end()) {
                response["error"] = "Duplicate booking";
                return response;
            }
        }

        PathResult result =
            engine.computePath(metro, source, destination);

        if (result.transfers == -1) {
            response["error"] = "No route available";
            return response;
        }

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

        {
            lock_guard<mutex> lock(bookingMutex);
            bookingStore[bookingKey] = booking;
        }

        saveBookingToFile(user, booking);

        response["bookingId"] = bookingId;
        response["route"] = result.path;
        response["transfers"] = result.transfers;
        response["fare"] = fare;
        response["qr"] = qr;

        return response;
    }
};