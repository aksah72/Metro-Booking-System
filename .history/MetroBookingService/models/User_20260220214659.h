#pragma once
#include <string>
using namespace std;

class User {
public:
    string username;
    string passwordHash;

    User() {}
    User(string u, string pHash) {
        username = u;
        passwordHash = pHash;
    }
};