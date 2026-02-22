#pragma once
#include <string>
using namespace std;

class ApiResponse {
public:
    bool success;
    string message;
    string data;

    ApiResponse(bool s, string m, string d = "")
        : success(s), message(m), data(d) {}
};