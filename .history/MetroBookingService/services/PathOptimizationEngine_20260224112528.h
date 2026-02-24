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
    int totalTime;
};

class PathOptimizationEngine {
public:

    int transferPenalty = 5;
    string time = 

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

            for (auto &edge : metro.adj[node]) {

                int newCost = cost + edge.travelTime;

                if (prevRoute != "" && prevRoute != edge.routeId)
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
            return {{}, {}, -1, -1};

        vector<string> path;
        vector<string> routeUsed;

        string curr = destination;

        while (curr != source) {
            path.push_back(curr);
            routeUsed.push_back(parentRoute[curr]);
            curr = parent[curr];
        }

        path.push_back(source);

        reverse(path.begin(), path.end());
        reverse(routeUsed.begin(), routeUsed.end());

        vector<RouteSegment> segments;

        if (!routeUsed.empty()) {

            RouteSegment segment;
            segment.line = routeUsed[0];
            segment.stops.push_back(path[0]);

            for (int i = 1; i < path.size(); i++) {

                if (routeUsed[i - 1] != segment.line) {
                    segments.push_back(segment);
                    segment = RouteSegment();
                    segment.line = routeUsed[i - 1];
                }

                segment.stops.push_back(path[i]);
            }

            segments.push_back(segment);
        }

        int transfers = segments.size() > 0 ?
                        segments.size() - 1 : 0;

        int totalTime = dist[destination];

        return {path, segments, transfers, totalTime};
    }
};