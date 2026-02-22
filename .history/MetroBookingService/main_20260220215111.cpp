#include "services/MetroRouteService.h"
#include "services/BookingService.h"
#include "services/AuthenticationService.h"

using namespace std;

int main() {

    MetroRouteService metro;
    BookingService bookingService;
    AuthenticationService auth;

    // 🔐 Register & Login
    auth.registerUser("aman", "1234");
    string token = auth.login("aman", "1234");

    // 🚇 Add Stops
    metro.addStop("A","Station A");
    metro.addStop("B","Station B");
    metro.addStop("C","Station C");
    metro.addStop("D","Station D");
    metro.addStop("E","Station E");

    // 🚇 Add Routes
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

    // 🎟️ Create Booking
    bookingService.createBooking(
        metro,
        auth,
        token,
        "A",
        "E"
    );

    return 0;
}