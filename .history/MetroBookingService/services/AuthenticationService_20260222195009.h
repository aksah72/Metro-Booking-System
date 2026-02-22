#pragma once
#include <unordered_map>
#include <string>
#include <random>

using namespace std;

class AuthenticationService {
public:

    unordered_map<string,string> users;
    unordered_map<string,string> tokens;

    string registerUser(string username,string password){

        if(users.find(username)!=users.end())
            return "";

        users[username]=password;
        return "User registered";
    }

    string login(string username,string password){

        if(users.find(username)==users.end())
            return "";

        if(users[username]!=password)
            return "";

        string token=generateToken();
        tokens[token]=username;

        return token;
    }

    bool validateToken(string token){
        return tokens.find(token)!=tokens.end();
    }

    string getUserFromToken(string token){
        if(tokens.find(token)!=tokens.end())
            return tokens[token];
        return "";
    }

private:
    string generateToken(){
        static int counter=1;
        return "TOKEN_"+to_string(counter++);
    }
};