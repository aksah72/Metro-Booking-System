#pragma once
#include <sqlite3.h>
#include <string>
#include <iostream>

using namespace std;

class DatabaseService {
public:

    sqlite3* db;

    DatabaseService() {
        if(sqlite3_open("metro.db",&db)){
            cout<<"Database open failed\n";
        }
    }

    ~DatabaseService(){
        sqlite3_close(db);
    }

    bool execute(string sql){
        char* err;
        if(sqlite3_exec(db, sql.c_str(), 0,0,&err)!=SQLITE_OK){
            cout<<"SQL error: "<<err<<endl;
            sqlite3_free(err);
            return false;
        }
        return true;
    }
};