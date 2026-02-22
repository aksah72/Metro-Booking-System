#pragma once
#include <string>
#include <ctime>
#include <sstream>
using namespace std;

class QRGenerator {
public:
    static string generate(
        string bookingId,
        string source,
        string destination) {

        string raw = bookingId + "|" +
                     source + "|" +
                     destination + "|" +
                     to_string(time(0));

        hash<string> hasher;
        size_t hashed = hasher(raw);

        stringstream ss;
        ss << hashed;
        return ss.str();
    }
};