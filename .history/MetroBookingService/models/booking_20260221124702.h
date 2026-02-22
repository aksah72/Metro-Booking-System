#pragma once
#include <string>
#include <vector>

using namespace std;

class Booking {
public:
    string bookingId;
    string source;
    string destination;
    vector<string> path;
    int transfers;
    string qr;        // ✅ add this
    int fare;
    string status;    // ✅ add this

    Booking() {}

    Booking(
        string id,
        string src,
        string dest,
        vector<string> p,
        int t,
        string qrCode,
        int f,
        string s = "PENDING"
    )
    : bookingId(id),
      source(src),
      destination(dest),
      path(p),
      transfers(t),
      qr(qrCode),      // ✅ now valid
      fare(f),
      status(s) {}     // ✅ now valid
};