#pragma once
#include <string>
using namespace std;

class User {
public:
    string username;
    string passwordHash;
    string role; // "USER" or "ADMIN"

    User() {}

    User(string u, string pHash, string r) {
        username = u;
        passwordHash = pHash;
        role = r;
    }
};