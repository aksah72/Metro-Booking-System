#pragma once
#include "../services/BookingService.h"
#include "../models/ApiResponse.h"

class BookingController {

private:
    BookingService &bookingService;
    AuthenticationService &auth;
    MetroRouteService &metro;

public:
    BookingController(
        BookingService &b,
        AuthenticationService &a,
        MetroRouteService &m)
        : bookingService(b),
          auth(a),
          metro(m) {}

    ApiResponse createBooking(
        string token,
        string source,
        string destination
    ) {

        if (!auth.validateToken(token))
            return ApiResponse(false,
                "Unauthorized");

        bookingService.createBooking(
            metro,
            auth,
            token,
            source,
            destination);

        return ApiResponse(true,
            "Booking processed");
    }
};