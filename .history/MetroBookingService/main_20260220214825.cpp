#include "services/AuthenticationService.h"

int main() {

    MetroRouteService metro;
    BookingService bookingService;
    AuthenticationService auth;

    // Register and Login
    auth.registerUser("aman", "1234");
    string token = auth.login("aman", "1234");

    metro.addStop("A","Station A");
    metro.addStop("B","Station B");
    metro.addStop("C","Station C");
    metro.addStop("D","Station D");
    metro.addStop("E","Station E");

    Route yellow("Y","Yellow");
    yellow.addStop("A");
    yellow.addStop("B");
    yellow.addStop("C");

    Route blue("B","Blue");
    blue.addStop("C");
    blue.addStop("D");
    blue.addStop("E");

    metro.addRoute(yellow);
    metro.addRoute(blue);
    metro.buildGraph();

    bookingService.createBooking(
        metro,
        auth,
        token,
        "A",
        "E"
    );

    return 0;
}