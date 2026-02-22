#pragma once
#include <iostream>
#include <mutex>
#include <ctime>
using namespace std;

class Logger {
private:
    static mutex logMutex;

    static string getTime() {
        time_t now = time(0);
        return to_string(now);
    }

public:
    static void info(string message) {
        lock_guard<mutex> lock(logMutex);
        cout << "[INFO][" << getTime() << "] "
             << message << endl;
    }

    static void error(string message) {
        lock_guard<mutex> lock(logMutex);
        cout << "[ERROR][" << getTime() << "] "
             << message << endl;
    }

    static void warning(string message) {
        lock_guard<mutex> lock(logMutex);
        cout << "[WARNING][" << getTime() << "] "
             << message << endl;
    }
};

mutex Logger::logMutex;