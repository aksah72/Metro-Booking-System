#pragma once
#include <queue>
#include <unordered_map>
#include <climits>
#include "../services/MetroRouteService.h"
using namespace std;

class PathOptimizationEngine {
public:
    int transferPenalty = 5;

    pair<vector<string>, int> computePath(
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

            if (node == destination) break;

            for (auto &edge : metro.adj[node]) {
                int newCost = cost + edge.travelTime;
                int transfer = 0;

                if (prevRoute != "" && prevRoute != edge.routeId)
                    transfer = transferPenalty;

                newCost += transfer;

                if (newCost < dist[edge.neighbor]) {
                    dist[edge.neighbor] = newCost;
                    parent[edge.neighbor] = node;
                    parentRoute[edge.neighbor] = edge.routeId;
                    pq.push({newCost, {edge.neighbor, edge.routeId}});
                }
            }
        }

        if (dist[destination] == INT_MAX)
            return {{}, -1};

        vector<string> path;
        string curr = destination;

        while (curr != source) {
            path.push_back(curr);
            curr = parent[curr];
        }
        path.push_back(source);
        sort(path.begin(), path.end());

        int transfers = 0;
        string prev = "";

        for (int i = 1; i < path.size(); i++) {
            string route = parentRoute[path[i]];
            if (prev != "" && route != prev)
                transfers++;
            prev = route;
        }

        return {path, transfers};
    }
};