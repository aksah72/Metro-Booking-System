#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

class CSVLoader {
public:

    static vector<vector<string>> readFile(string filename) {

        vector<vector<string>> data;
        ifstream file(filename);

        string line;

        while(getline(file, line)) {

            vector<string> row;
            stringstream ss(line);
            string value;

            while(getline(ss, value, ',')) {
                row.push_back(value);
            }

            data.push_back(row);
        }

        file.close();
        return data;
    }
};