#pragma once
#include "../services/AdminService.h"
#include "../models/ApiResponse.h"

class AdminController {

private:
    AdminService &adminService;
    AuthenticationService &auth;
    MetroRouteService &metro;

public:
    AdminController(
        AdminService &a,
        AuthenticationService &authSvc,
        MetroRouteService &m)
        : adminService(a),
          auth(authSvc),
          metro(m) {}

    ApiResponse addStop(
        string token,
        string id,
        string name
    ) {

        if (!auth.validateToken(token))
            return ApiResponse(false,
                "Unauthorized");

        if (auth.getUserRole(token)
            != "ADMIN")
            return ApiResponse(false,
                "Access denied");

        adminService.addNewStop(
            metro,
            auth,
            token,
            id,
            name);

        return ApiResponse(true,
            "Stop added");
    }
};