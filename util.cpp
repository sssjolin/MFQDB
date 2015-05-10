#include <postgresql/libpq-fe.h>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
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



vector<string> generateMFstruct(const vector<string> &selectAttribute,const vector<string> &fvect){
    unordered_set<string> mfSet(selectAttribute.begin(),selectAttribute.end());
    vector<string> MFstructures = selectAttribute;
    for (auto s : fvect){
        string tmp = s;
        if (auto i=tmp.find("avg") != -1){
            mfSet.insert(tmp);
            tmp.replace(tmp.find("avg"), 3, "sum");
            mfSet.insert(tmp);
            tmp.replace(tmp.find("sum"), 3, "count");
            mfSet.insert(tmp);
        }
    }
    for (auto s : selectAttribute)
        mfSet.erase(s);
    for (auto s : mfSet)
        MFstructures.push_back(s);
    return MFstructures;
}

void convertConditionToCmd(string &s, vector<vector<string>> &conditionCmd, 
    vector<string> &groupingAttributes, vector<string> &mfstructure,
    unordered_map<string, string> &information_schema){
    int gv = s[0] - '0';
    size_t found=s.find(",");
    if (found != string::npos)
        s.replace(found, 1, "");
    string mapkey;

    for (auto ga : groupingAttributes){
        if (s.find("=" + ga) != -1 || s.find("= " + ga) != -1){
            if (!mapkey.empty())
                mapkey += "+";
            if (information_schema[ga]=="string")
                mapkey += "tmprec." + ga;
            else
                mapkey += "to_string(tmprec."+ga+")";
        }
    }

    string mapcmd = "vector<int> mfstructuresIndex=mfstructureMap[" + mapkey + "];";
    conditionCmd[gv].push_back(mapcmd);

    vector<string> replaceList({ "and", "&&", "or", "||", "=", "==", "<>", "!=", "'","\"" });
    for (int i = 0; i < replaceList.size(); i += 2){
        found = s.find(replaceList[i]);
        while (found != string::npos){
            s.replace(found, replaceList[i].size(), replaceList[i+1]);
            found = s.find(replaceList[i], found + replaceList[i + 1].size());
        }
    }

    string gvs = s.substr(0, 1);
    found = s.find(gvs);
    while (found != string::npos){
        if (found == 0 || s[found - 1] == ' ' || s[found - 1] == '|' || s[found - 1] == '&')
            s.replace(found, 1, "tmprec");
        found = s.find(gvs, found + 1);
    }


    for (auto mfAttribute : mfstructure){
        found = s.find(mfAttribute);
        while (found != string::npos){
            if (found == 0 || s[found - 1] != '.')
                s.insert(found, "mfstructureVector[index].");
            found = s.find(mfAttribute, found + 1);
        }
    }

    conditionCmd[gv].push_back(s);
}


string functionHandler(string column){
    string cmd;
    auto i1 = column.find_first_of("_");
    if (i1 == -1)
        return cmd;
    i1 = column.find_first_of("_",i1+1);
    auto i2 = column.find_last_of("_");
    
    if (i2 == -1)
        return column + "++;";

    string opt = column.substr(i1+1, i2 - i1-1);
    
    

    if (opt == "sum"){
        cmd = "mfstructureVector[index]."+column + "+=tmprec." + column.substr(i2 + 1) + ";";
    } 
    else if (opt == "count"){
        cmd = "mfstructureVector[index]."+column + "++;";
    }
    else if (opt == "max"){
        cmd = "mfstructureVector[index]." + column + "=max(" + "mfstructureVector[index]."+column + ",tmprec." + column.substr(i2 + 1) + ");";
    }
    else if (opt == "min"){
        cmd = "mfstructureVector[index]." + column + "=min(" + "mfstructureVector[index]."+column + ",tmprec." + column.substr(i2 + 1) + ");";
    } 
    else if (opt == "avg"){
        cmd = "avgmfstructureVector[index]." + column + "=" + "mfstructureVector[index]."+column.substr(0, i1) + "_sum_" + column.substr(i2 + 1) + "/" + "mfstructureVector[index]."+column.substr(0, i1) + "_count_" + column.substr(i2 + 1) + ";";
    }
    return cmd;

}