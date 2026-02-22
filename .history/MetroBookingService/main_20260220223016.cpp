#include "services/MetroRouteService.h"
#include "services/BookingService.h"
#include "services/AuthenticationService.h"
#include "services/AdminService.h"

#include "controllers/AuthController.h"
#include "controllers/BookingController.h"
#include "controllers/AdminController.h"

#include <iostream>
using namespace std;

int main() {

    MetroRouteService metro;
    BookingService bookingService;
    AuthenticationService auth;
    AdminService adminService;

    AuthController authController(auth);
    BookingController bookingController(
        bookingService, auth, metro);
    AdminController adminController(
        adminService, auth, metro);

    // Simulate API calls

    auto r1 =
        authController.registerUser(
            "admin",
            "admin123",
            "ADMIN");

    auto r2 =
        authController.registerUser(
            "aman",
            "1234",
            "USER");

    auto loginAdmin =
        authController.login(
            "admin",
            "admin123");

    auto loginUser =
        authController.login(
            "aman",
            "1234");

    string adminToken = loginAdmin.data;
    string userToken = loginUser.data;

    adminController.addStop(
        adminToken,
        "A",
        "Station A");

    adminController.addStop(
        adminToken,
        "B",
        "Station B");

    adminController.addStop(
        adminToken,
        "C",
        "Station C");

    adminController.addStop(
        adminToken,
        "D",
        "Station D");

    adminController.addStop(
        adminToken,
        "E",
        "Station E");

    metro.loadFromCSV("stops.csv",
                      "routes.csv");

    bookingController.createBooking(
        userToken,
        "A",
        "E");

    return 0;
}