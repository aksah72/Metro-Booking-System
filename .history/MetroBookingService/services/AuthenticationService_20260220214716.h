#pragma once
#include "../models/User.h"
#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

class AuthenticationService {
private:
    unordered_map<string, User> users;
    unordered_map<string, string> activeTokens;

    string hashPassword(string password) {
        hash<string> hasher;
        return to_string(hasher(password));
    }

    string generateToken(string username) {
        string raw = username + to_string(time(0));
        hash<string> hasher;
        return to_string(hasher(raw));
    }

public:
    bool registerUser(string username, string password) {

        if(users.find(username) != users.end()) {
            cout << "User already exists\n";
            return false;
        }

        string hashed = hashPassword(password);
        users[username] = User(username, hashed);

        cout << "User registered successfully\n";
        return true;
    }

    string login(string username, string password) {

        if(users.find(username) == users.end()) {
            cout << "User not found\n";
            return "";
        }

        string hashed = hashPassword(password);

        if(users[username].passwordHash != hashed) {
            cout << "Invalid password\n";
            return "";
        }

        string token = generateToken(username);
        activeTokens[token] = username;

        cout << "Login successful\n";
        return token;
    }

    bool validateToken(string token) {
        return activeTokens.find(token) != activeTokens.end();
    }

    string getUserFromToken(string token) {
        return activeTokens[token];
    }
};