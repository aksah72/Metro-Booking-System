string user = auth.getUserFromToken(token);
string bookingKey = user + "|" + source + "|" + destination;

if (bookingStore.find(bookingKey) != bookingStore.end()) {
    cout << "Duplicate booking request detected\n";
    Booking existing = bookingStore[bookingKey];

    cout << "Returning existing booking ID: "
         << existing.bookingId << "\n";
    return;
}