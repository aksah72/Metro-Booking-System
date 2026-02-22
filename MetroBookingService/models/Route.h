#pragma once
#include <string>
#include <vector>
using namespace std;

class Route {
public:
    string id;
    string color;
    vector<string> stopIds;

    Route() {}
    Route(string id, string color) {
        this->id = id;
        this->color = color;
    }

    void addStop(string stopId) {
        stopIds.push_back(stopId);
    }
};