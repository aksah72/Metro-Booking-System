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
    string qrString;
    int fare;

    Booking() {}   // 🔥 important for unordered_map

    Booking(
    string id,
    string src,
    string dest,
    vector<string> p,
    int t,
    string qr,
    int f,
    string s = "PENDING"
)
: bookingId(id),
  source(src),
  destination(dest),
  path(p),
  transfers(t),
  qr(qr),
  fare(f),
  status(s) {}
};