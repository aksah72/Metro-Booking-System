#pragma once
#include <sqlite3.h>
#include <string>
#include <iostream>

using namespace std;

class DatabaseService {
public:

    sqlite3* db = nullptr;

    DatabaseService() {

        int rc = sqlite3_open("metro.db", &db);

        if (rc != SQLITE_OK) {
            cout << "❌ Database open failed: "
                 << sqlite3_errmsg(db) << endl;
            db = nullptr;
        } else {
            cout << "✅ Database connected successfully\n";
        }
    }

    ~DatabaseService() {

        if (db != nullptr) {
            sqlite3_close(db);
            cout << "🔒 Database closed\n";
        }
    }

    bool execute(const string& sql) {

        if (db == nullptr) {
            cout << "❌ Database not initialized\n";
            return false;
        }

        char* errMsg = nullptr;

        int rc = sqlite3_exec(
            db,
            sql.c_str(),
            nullptr,
            nullptr,
            &errMsg
        );

        if (rc != SQLITE_OK) {
            cout << "❌ SQL Error: " << errMsg << endl;
            sqlite3_free(errMsg);
            return false;
        }

        return true;
    }
};