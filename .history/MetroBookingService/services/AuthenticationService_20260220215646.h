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

    int tokenValiditySeconds = 300; // 5 minutes

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

        users[username] = User(
            username,
            hashPassword(password)
        );

        cout << "User registered successfully\n";
        return true;
    }

    string login(string username, string password) {

        if(users.find(username) == users.end()) {
            cout << "User not found\n";
            return "";
        }

        if(users[username].passwordHash != hashPassword(password)) {
            cout << "Invalid password\n";
            return "";
        }

        string token = generateToken(username);

        Session session;
        session.username = username;
        session.expiryTime = time(0) + tokenValiditySeconds;

        {
            lock_guard<mutex> lock(sessionMutex);
            activeSessions[token] = session;
        }

        cout << "Login successful\n";
        return token;
    }

    bool validateToken(string token) {

        lock_guard<mutex> lock(sessionMutex);

        if(activeSessions.find(token) == activeSessions.end())
            return false;

        if(time(0) > activeSessions[token].expiryTime) {
            cout << "Token expired\n";
            activeSessions.erase(token);
            return false;
        }

        return true;
    }

    string getUserFromToken(string token) {

        lock_guard<mutex> lock(sessionMutex);

        if(activeSessions.find(token) != activeSessions.end())
            return activeSessions[token].username;

        return "";
    }

    void logout(string token) {
        lock_guard<mutex> lock(sessionMutex);
        activeSessions.erase(token);
        cout << "User logged out\n";
    }

    void cleanupExpiredSessions() {
        lock_guard<mutex> lock(sessionMutex);

        time_t now = time(0);

        for(auto it = activeSessions.begin();
            it != activeSessions.end();) {

            if(now > it->second.expiryTime)
                it = activeSessions.erase(it);
            else
                ++it;
        }
    }
};