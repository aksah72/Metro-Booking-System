#include "services/MetroRouteService.h"
#include "services/BookingService.h"
using namespace std;

int main() {

    MetroRouteService metro;

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

    BookingService bookingService;

    bookingService.createBooking(metro,"A","E");

    return 0;
}