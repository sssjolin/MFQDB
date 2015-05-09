#include <postgresql/libpq-fe.h>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "inputParse.h"
#include "util.h"

using namespace std;


void split(string s, vector<string> &v){
    int index = s.find_first_not_of(" ,");
    while (index < s.size()){
        int t = s.find_first_of(" ,", index);
        if (t!=-1)
            v.push_back((s.substr(index, t - index)));
        else
            v.push_back((s.substr(index)));
        index = s.find_first_not_of(" ,",t);
    }
    return;
        

}