#pragma once
#include <vector>
using namespace std;

class FareService {
public:
    int baseFare = 10;
    int perStopCharge = 2;
    int transferCharge = 5;

    int calculateFare(int totalStops, int transfers) {

        if (totalStops <= 1)
            return baseFare;

        int fare = baseFare
                 + (perStopCharge * (totalStops - 1))
                 + (transferCharge * transfers);

        return fare;
    }
};