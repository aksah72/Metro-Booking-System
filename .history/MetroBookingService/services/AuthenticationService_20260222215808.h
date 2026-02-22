#pragma once
#include "DatabaseService.h"
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

class AuthenticationService {
public:

    DatabaseService db;

    string hashPassword(string password){
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(),
               password.length(), hash);

        stringstream ss;
        for(int i=0;i<SHA256_DIGEST_LENGTH;i++)
            ss<<hex<<setw(2)<<setfill('0')<<(int)hash[i];

        return ss.str();
    }

    bool registerUser(string email,string password){

        string hashed = hashPassword(password);

        string sql = "INSERT INTO users(email,password,role) VALUES('"+
                     email+"','"+hashed+"','USER');";

        return db.execute(sql);
    }

    string login(string email,string password){

        string hashed = hashPassword(password);

        string sql = "SELECT id FROM users WHERE email='"+email+
                     "' AND password='"+hashed+"';";

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db.db, sql.c_str(), -1, &stmt, 0);

        if(sqlite3_step(stmt)==SQLITE_ROW){
            int id = sqlite3_column_int(stmt,0);
            sqlite3_finalize(stmt);
            return "TOKEN_" + to_string(id);
        }

        sqlite3_finalize(stmt);
        return "";
    }

    bool validateToken(string token){
        return token.rfind("TOKEN_",0)==0;
    }

    int getUserIdFromToken(string token){
        if(!validateToken(token)) return -1;
        return stoi(token.substr(6));
    }
};