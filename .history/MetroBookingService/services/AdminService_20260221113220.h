#pragma once
#include "../services/MetroRouteService.h"
#include "../services/AuthenticationService.h"
#include "../utils/Logger.h"

using namespace std;

class AdminService {

public:

    bool addNewRoute(
        MetroRouteService &metro,
        AuthenticationService &auth,
        string token,
        Route route
    ) {

        if (!auth.validateToken(token)) {
            Logger::error("Unauthorized admin attempt");
            return false;
        }

        if (auth.getUserRole(token) != "ADMIN") {
            Logger::error("Access denied: Not an admin");
            return false;
        }

        metro.addRoute(route);
        metro.buildGraph();

        Logger::info("Admin added new route: " + route.id);
        return true;
    }

    bool addNewStop(
        MetroRouteService &metro,
        AuthenticationService &auth,
        string token,
        string id,
        string name
    ) {

        if (!auth.validateToken(token)) {
            Logger::error("Unauthorized admin attempt");
            return false;
        }

        if (auth.getUserRole(token) != "ADMIN") {
            Logger::error("Access denied: Not an admin");
            return false;
        }

        metro.addStop(id, name);

        Logger::info("Admin added new stop: " + id);
        return true;
    }
};