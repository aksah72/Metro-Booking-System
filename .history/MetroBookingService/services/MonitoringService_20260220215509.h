#pragma once
#include <atomic>
#include <iostream>
using namespace std;

class MonitoringService {
public:
    atomic<int> totalBookings{0};
    atomic<int> totalRouteSearches{0};
    atomic<int> cacheHits{0};
    atomic<int> failedRequests{0};

    void printMetrics() {
        cout << "\n----- SYSTEM METRICS -----\n";
        cout << "Total Bookings: "
             << totalBookings << "\n";
        cout << "Total Route Searches: "
             << totalRouteSearches << "\n";
        cout << "Cache Hits: "
             << cacheHits << "\n";
        cout << "Failed Requests: "
             << failedRequests << "\n";
        cout << "--------------------------\n";
    }
};