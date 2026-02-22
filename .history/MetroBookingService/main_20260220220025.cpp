#include "services/MetroRouteService.h"
#include "services/BookingService.h"
#include "services/AuthenticationService.h"
#include "services/AdminService.h"

using namespace std;

int main() {

    MetroRouteService metro;
    BookingService bookingService;
    AuthenticationService auth;
    AdminService adminService;

    // 🔐 Register Users with Roles
    auth.registerUser("aman", "1234", "USER");
    auth.registerUser("admin", "admin123", "ADMIN");

    // 🔑 Login
    string userToken = auth.login("aman", "1234");
    string adminToken = auth.login("admin", "admin123");

    // 🚇 Admin adds stops
    adminService.addNewStop(
        metro, auth, adminToken, "A", "Station A");
    adminService.addNewStop(
        metro, auth, adminToken, "B", "Station B");
    adminService.addNewStop(
        metro, auth, adminToken, "C", "Station C");
    adminService.addNewStop(
        metro, auth, adminToken, "D", "Station D");
    adminService.addNewStop(
        metro, auth, adminToken, "E", "Station E");

    // 🚇 Admin adds routes
    Route yellow("Y","Yellow");
    yellow.addStop("A");
    yellow.addStop("B");
    yellow.addStop("C");

    Route blue("B","Blue");
    blue.addStop("C");
    blue.addStop("D");
    blue.addStop("E");

    adminService.addNewRoute(
        metro, auth, adminToken, yellow);

    adminService.addNewRoute(
        metro, auth, adminToken, blue);

    metro.buildGraph();

    // 🎟️ USER creates booking
    bookingService.createBooking(
        metro,
        auth,
        userToken,
        "A",
        "E"
    );

    return 0;
}