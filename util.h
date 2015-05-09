#ifndef UTIL_H
#define UTIL_H
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;
void split(string s,vector<string> &v);
vector<string> generateMFstruct(const vector<string> &selectAttribute, const vector<string> &fvect);

#endif