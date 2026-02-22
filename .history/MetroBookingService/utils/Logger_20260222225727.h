#pragma once

#include <iostream>
#include <fstream>
#include <mutex>
#include <ctime>
#include <iomanip>

using namespace std;

class Logger {
private:
    static mutex logMutex;
    static ofstream logFile;

    static string getCurrentTime() {
        time_t now = time(nullptr);
        tm *ltm = localtime(&now);

        stringstream ss;
        ss << put_time(ltm, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    static void write(string level, string message) {
        lock_guard<mutex> lock(logMutex);

        string logLine = "[" + level + "][" +
                         getCurrentTime() + "] " +
                         message;

        cout << logLine << endl;

        if (logFile.is_open())
            logFile << logLine << endl;
    }

public:
    static void init(string filename = "metro.log") {
        lock_guard<mutex> lock(logMutex);
        if (!logFile.is_open())
            logFile.open(filename, ios::app);
    }

    static void info(string message) {
        write("INFO", message);
    }

    static void error(string message) {
        write("ERROR", message);
    }

    static void warning(string message) {
        write("WARNING", message);
    }

    static void close() {
        if (logFile.is_open())
            logFile.close();
    }
};

mutex Logger::logMutex;
ofstream Logger::logFile;