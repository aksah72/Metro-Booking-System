#pragma once
using namespace std;

class FareService {
public:
    int baseFare = 10;
    int perStopCharge = 2;
    int transferCharge = 5;

    int calculateFare(int stops, int transfers) {

        if (stops <= 1)
            return baseFare;

        return baseFare
             + (perStopCharge * (stops - 1))
             + (transferCharge * transfers);
    }
};