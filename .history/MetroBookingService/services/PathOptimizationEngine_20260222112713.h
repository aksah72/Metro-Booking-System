#pragma once
#include <queue>
#include <unordered_map>
#include <climits>
#include <vector>
#include <algorithm>
#include "../services/MetroRouteService.h"
using namespace std;

struct RouteSegment {
    string line;
    vector<string> stops;
};

struct PathResult {
    vector<string> path;
    vector<RouteSegment> segments;
    int transfers;
};

class PathOptimizationEngine {
public:
    int transferPenalty = 5;

    PathResult computePath(
        MetroRouteService &metro,
        string source,
        string destination
    ) {

        unordered_map<string, int> dist;
        unordered_map<string, string> parent;
        unordered_map<string, string> parentRoute;

        for (auto &s : metro.stops)
            dist[s.first] = INT_MAX;

        priority_queue<
            pair<int, pair<string, string>>,
            vector<pair<int, pair<string, string>>>,
            greater<>
        > pq;

        dist[source] = 0;
        pq.push({0, {source, ""}});

        while (!pq.empty()) {
            auto top = pq.top();
            pq.pop();

            int cost = top.first;
            string node = top.second.first;
            string prevRoute = top.second.second;

            if (node == destination)
                break;

            for (auto &edge : metro.adj[node]) {

                int newCost = cost + edge.travelTime;

                if (prevRoute != "" &&
                    prevRoute != edge.routeId)
                    newCost += transferPenalty;

                if (newCost < dist[edge.neighbor]) {
                    dist[edge.neighbor] = newCost;
                    parent[edge.neighbor] = node;
                    parentRoute[edge.neighbor] = edge.routeId;
                    pq.push({newCost,
                             {edge.neighbor, edge.routeId}});
                }
            }
        }

        if (dist[destination] == INT_MAX)
            return {{}, {}, -1};

        vector<string> path;
        vector<string> routes;

        string curr = destination;

        while (curr != source) {
            path.push_back(curr);
            routes.push_back(parentRoute[curr]);
            curr = parent[curr];
        }

        path.push_back(source);

        reverse(path.begin(), path.end());
        reverse(routes.begin(), routes.end());

        int transfers = 0;
        for (int i = 1; i < routes.size(); i++)
            if (routes[i] != routes[i - 1])
                transfers++;

        return {path, routes, transfers};
    }
};