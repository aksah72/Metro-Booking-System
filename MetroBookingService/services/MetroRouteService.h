#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "../models/Stop.h"
#include "../models/Route.h"

using namespace std;

struct Edge {
    string neighbor;
    int travelTime;
    string routeId;
};

class MetroRouteService {
public:

    unordered_map<string, Stop> stops;
    unordered_map<string, Route> routes;
    unordered_map<string, vector<Edge>> adj;

    void addStop(string id, string name) {
        stops[id] = Stop(id, name);
    }

    void addRoute(Route route) {
        routes[route.id] = route;

        for (auto &stopId : route.stopIds) {
            if (stops.find(stopId) != stops.end())
                stops[stopId].addRoute(route.id);
        }
    }

    void buildGraph(int time = 2) {

        adj.clear();

        for (auto &entry : routes) {

            Route &route = entry.second;

            for (int i = 0; i < route.stopIds.size() - 1; i++) {

                string u = route.stopIds[i];
                string v = route.stopIds[i + 1];

                Edge e1{v, time, route.id};
                Edge e2{u, time, route.id};

                adj[u].push_back(e1);
                adj[v].push_back(e2);
            }
        }
    }

    bool stopExists(string id) {
        return stops.find(id) != stops.end();
    }
};