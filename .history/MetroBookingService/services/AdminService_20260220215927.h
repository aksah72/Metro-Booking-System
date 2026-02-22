#pragma once
#include "../services/MetroRouteService.h"
#include "../services/AuthenticationService.h"
#include "../utils/Logger.h"

using namespace std;

class AdminService {

public:

    void addNewRoute(
        MetroRouteService &metro,
        AuthenticationService &auth,
        string token,
        Route route
    ) {

        if (!auth.validateToken(token)) {
            Logger::error("Unauthorized admin attempt");
            return;
        }

        if (auth.getUserRole(token) != "ADMIN") {
            Logger::error("Access denied: Not an admin");
            return;
        }

        metro.addRoute(route);
        metro.buildGraph();

        Logger::info("Admin added new route: " + route.id);
    }

    void addNewStop(
        MetroRouteService &metro,
        AuthenticationService &auth,
        string token,
        string id,
        string name
    ) {

        if (!auth.validateToken(token)) {
            Logger::error("Unauthorized admin attempt");
            return;
        }

        if (auth.getUserRole(token) != "ADMIN") {
            Logger::error("Access denied: Not an admin");
            return;
        }

        metro.addStop(id, name);

        Logger::info("Admin added new stop: " + id);
    }
};