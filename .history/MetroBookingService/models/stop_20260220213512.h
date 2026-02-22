#pragma once
#include <string>
#include <vector>
using namespace std;

class Stop {
public:
    string id;
    string name;
    vector<string> routeIds;

    Stop() {}
    Stop(string id, string name) {
        this->id = id;
        this->name = name;
    }

    void addRoute(string routeId) {
        routeIds.push_back(routeId);
    }
};