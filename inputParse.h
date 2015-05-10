#ifndef INPUT_PARSE_H
#define INPUT_PARSE_H
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;
int inputparse(string input_file, string output_file, 
    unordered_map<string, string> &information_schema,
    vector<string> &recVector, string dbconnection);

#endif