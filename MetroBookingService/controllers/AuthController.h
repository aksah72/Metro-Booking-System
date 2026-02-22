#pragma once
#include "../services/AuthenticationService.h"
#include "../models/ApiResponse.h"

class AuthController {

private:
    AuthenticationService &auth;

public:
    AuthController(AuthenticationService &a)
        : auth(a) {}

    ApiResponse registerUser(
        string username,
        string password,
        string role = "USER"
    ) {

        bool created =
            auth.registerUser(username,
                              password,
                              role);

        if (!created)
            return ApiResponse(false,
                "User already exists");

        return ApiResponse(true,
            "User registered successfully");
    }

    ApiResponse login(
        string username,
        string password
    ) {

        string token =
            auth.login(username, password);

        if (token == "")
            return ApiResponse(false,
                "Invalid credentials");

        return ApiResponse(true,
            "Login successful",
            token);
    }
};
