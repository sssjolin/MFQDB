#ifndef UTIL_H
#define UTIL_H
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;
void split(string s,vector<string> &v);
vector<string> generateMFstruct(const vector<string> &selectAttribute, const vector<string> &fvect);
void convertConditionToCmd(string &s, vector<vector<string>> &conditionCmd, vector<string> &groupingAttributes, vector<string> &mfstructure, unordered_map<string, string> &information_schema);
string functionHandler(string column);
string converHavingToCmd(string cmd, vector<string> &mfstructure);
#endif