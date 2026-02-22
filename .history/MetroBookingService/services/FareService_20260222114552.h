#pragma once
using namespace std;

class FareService {
public:

    int calculateFare(int stops, int transfers){

        int base;

        if(stops <= 3)
            base = 10;
        else if(stops <= 6)
            base = 20;
        else if(stops <= 10)
            base = 30;
        else
            base = 40;

        return base + (transfers * 5);
    }
};