#pragma once
#include "../models/User.h"
#include <unordered_map>
#include <string>
#include <iostream>
#include <ctime>
#include <mutex>

using namespace std;

struct Session {
    string username;
    time_t expiryTime;
};

class AuthenticationService {
private:
    unordered_map<string, User> users;
    unordered_map<string, Session> activeSessions;
    mutex sessionMutex;

    int tokenValiditySeconds = 300;

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
    bool registerUser(string username,
                      string password,
                      string role = "USER") {

        if(users.find(username) != users.end())
            return false;

        users[username] =
            User(username,
                 hashPassword(password),
                 role);

        return true;
    }

    string login(string username,
                 string password) {

        if(users.find(username) == users.end())
            return "";

        if(users[username].passwordHash
           != hashPassword(password))
            return "";

        string token = generateToken(username);

        Session s;
        s.username = username;
        s.expiryTime = time(0) + tokenValiditySeconds;

        lock_guard<mutex> lock(sessionMutex);
        activeSessions[token] = s;

        return token;
    }

    bool validateToken(string token) {

        lock_guard<mutex> lock(sessionMutex);

        if(activeSessions.find(token)
           == activeSessions.end())
            return false;

        if(time(0) >
           activeSessions[token].expiryTime) {

            activeSessions.erase(token);
            return false;
        }

        return true;
    }

    string getUserFromToken(string token) {

        lock_guard<mutex> lock(sessionMutex);

        if(activeSessions.find(token)
           != activeSessions.end())
            return activeSessions[token].username;

        return "";
    }

    string getUserRole(string token) {

        lock_guard<mutex> lock(sessionMutex);

        if(activeSessions.find(token)
           != activeSessions.end()) {

            string username =
                activeSessions[token].username;

            return users[username].role;
        }

        return "";
    }
};