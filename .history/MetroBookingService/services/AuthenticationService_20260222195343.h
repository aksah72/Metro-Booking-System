#pragma once
#include <unordered_map>
#include <string>

using namespace std;

class AuthenticationService {
public:

    unordered_map<string,string> users;        // username -> password
    unordered_map<string,string> roles;        // username -> role
    unordered_map<string,string> tokens;       // token -> username

    bool registerUser(string username, string password, string role = "USER") {

        if(users.find(username) != users.end())
            return false;

        users[username] = password;
        roles[username] = role;

        return true;
    }

    string login(string username, string password) {

        if(users.find(username) == users.end())
            return "";

        if(users[username] != password)
            return "";

        string token = generateToken(username);
        tokens[token] = username;

        return token;
    }

    bool validateToken(string token) {
        return tokens.find(token) != tokens.end();
    }

    string getUserFromToken(string token) {

        if(tokens.find(token) == tokens.end())
            return "";

        return tokens[token];
    }

    string getUserRole(string token) {

        if(tokens.find(token) == tokens.end())
            return "";

        string user = tokens[token];

        if(roles.find(user) == roles.end())
            return "";

        return roles[user];
    }

private:

    string generateToken(string username) {
        static int counter = 1;
        return username + "_TOKEN_" + to_string(counter++);
    }
};