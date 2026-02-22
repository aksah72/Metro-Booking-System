#pragma once
#include <fstream>
#include <unordered_map>
#include <string>
#include <sstream>

using namespace std;

class AuthenticationService {
public:

    unordered_map<string,string> tokens;

    bool registerUser(string email, string password, string role="USER") {

        if(userExists(email))
            return false;

        ofstream file("database/users.csv", ios::app);
        file << email << "," << password << "," << role << "\n";
        file.close();

        return true;
    }

    string login(string email, string password) {

        ifstream file("database/users.csv");
        string line;

        while(getline(file,line)){

            stringstream ss(line);
            string e,p,r;

            getline(ss,e,',');
            getline(ss,p,',');
            getline(ss,r,',');

            if(e==email && p==password){
                string token = generateToken(email);
                tokens[token]=email;
                return token;
            }
        }

        return "";
    }

    bool validateToken(string token){
        return tokens.find(token)!=tokens.end();
    }

    string getUserFromToken(string token){
        if(tokens.find(token)==tokens.end())
            return "";
        return tokens[token];
    }

    string getUserRole(string token){

        if(tokens.find(token)==tokens.end())
            return "";

        string email = tokens[token];

        ifstream file("database/users.csv");
        string line;

        while(getline(file,line)){
            stringstream ss(line);
            string e,p,r;

            getline(ss,e,',');
            getline(ss,p,',');
            getline(ss,r,',');

            if(e==email)
                return r;
        }

        return "";
    }

private:

    bool userExists(string email){

        ifstream file("database/users.csv");
        string line;

        while(getline(file,line)){
            stringstream ss(line);
            string e;
            getline(ss,e,',');
            if(e==email)
                return true;
        }
        return false;
    }

    string generateToken(string email){
        static int counter=1;
        return email+"_TOKEN_"+to_string(counter++);
    }
};